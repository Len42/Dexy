using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Xml.Linq;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils;
using Dexy.DexyPatch.Utils.PatchChanges;
using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// This service handles communications with the Dexy device
    /// </summary>
    public class DexyDevice : IDexyDevice
    {
        public DexyDevice()
        {
            settingsManager = Service<ISettingsManager>.Get();
            deviceThread = new();
            PortSettings = new();
            DoConnect(GetInitialSettings());
        }

        public void Terminate()
        {
            Connected = null; // this seems to be necessary
            Disconnect();
        }

        #region Comm port settings

        private readonly ISettingsManager settingsManager;

        public CommPort.CommPortSettings PortSettings { get; private set; }

        /// <summary>
        /// Load the previously-saved comm port settings
        /// </summary>
        /// <returns></returns>
        private CommPort.CommPortSettings GetInitialSettings() => settingsManager.Load<CommPort.CommPortSettings>();

        public string PortName { get => PortSettings.portName; }

        #endregion

        #region Comm port connection

        /// <summary>
        /// The comm port used to connect to the Dexy module
        /// </summary>
        private CommPort? commPort;

        public void Connect(CommPort.CommPortSettings settings)
        {
            // (Re-)Connect the device and save the given settings as default
            DoConnect(settings);
            if (commPort != null) {
                settingsManager.Save(PortSettings); // for next time
            }
        }

        /// <summary>
        /// Connect to the device (without saving the settings) and start the worker thread
        /// </summary>
        /// <param name="settings"></param>
        private void DoConnect(CommPort.CommPortSettings settings)
        {
            // Disconnect & stop thread first
            Disconnect();
            PortSettings = settings;

            // Establish the comm port connection
            CommPort? port;
            try {
                port = new(PortSettings);
                port.CommPortConnected += (e) => { SendConnected(e.Connected); };
            } catch {
                port = null;
            }
            commPort = port;
            SendConnected(IsConnected);

            if (commPort != null) {
                // Start the worker thread
                deviceThread.Start(new DeviceThread.DeviceThreadArgs(commPort, commandQueue));
            }
        }

        /// <summary>
        /// Disconnect from the device and stop the worker thread
        /// </summary>
        private void Disconnect()
        {
            deviceThread.Stop();
            commPort?.Close();
            commPort?.Dispose();
            commPort = null;
            SendConnected(false);
        }

        public bool IsConnected { get => commPort != null && commPort.IsConnected; }

        #endregion

        #region Events

        public event IDexyDevice.ConnectedHandler? Connected;

        /// <summary>
        /// Send a <see cref="Dexy.DexyPatch.Services.IDexyDevice.Connected"/> event
        /// </summary>
        /// <param name="connected"></param>
        private void SendConnected(bool connected) { Connected?.Invoke(new(connected)); }

        public event IDexyDevice.TextReceivedHandler? TextReceived;

        /// <summary>
        /// Send a <see cref="Dexy.DexyPatch.Services.IDexyDevice.TextReceived"/> event
        /// </summary>
        /// <param name="text"></param>
        public void SendTextReceived(string text) { TextReceived?.Invoke(new(text)); }

        #endregion

        #region Async command methods

        /// <summary>
        /// A command passed to StartCommand
        /// </summary>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="port"></param>
        /// <param name="stream"></param>
        /// <returns>Variable return type, as required</returns>
        /// <remarks>
        /// In all of the asynchronous command methods, the code to send the
        /// command to the Dexy module is in a CommandFunc lambda expression
        /// that is passed to <see cref="Dexy.DexyPatch.Services.DexyDevice.StartCommand{TResult}(CommandFunc{TResult})"/>.
        /// </remarks>
        private delegate TResult CommandFunc<TResult>(CommPort port, CommPortStream stream);

        public async Task<string?> GetFirmwareVersionAsync()
        {
            return await StartCommand((port, stream) => {
                    SendCommandToDevice(DexyCommand.version, stream);
                    return ReadLineFromDevice(stream);
                });
        }

        public async Task<PatchBank?> UploadPatchBankAsync()
        {
            return await StartCommand((port, stream) => {
                    SendCommandToDevice(DexyCommand.upload, stream);
                    PatchBank patchBank = new();
                    ZppSerialize.Deserialize(stream, patchBank);
                    return patchBank;
                });
        }

        public async Task DownloadPatchBankAsync(PatchBank patchBank)
        {
            await StartCommand((port, stream) => {
                    SendCommandToDevice(DexyCommand.download, stream);
                    ZppSerialize.Serialize(stream, patchBank);
                    ReadAckFromDevice(stream);
                    return true;
                });
        }

        public async Task SendPatchChangedAsync(PatchChangeBase change)
        {
            // Which command is sent depends on the type of change
            DexyCommand command = change switch {
                PatchChange => DexyCommand.updPatch,
                NameChange => DexyCommand.updName,
                PatchSettingChange => DexyCommand.updSettings,
                OpSettingChange => DexyCommand.updOperator,
                _ => throw new NotImplementedException()
            };
            await StartCommand((port, stream) => {
                    SendCommandToDevice(command, stream);
                    ZppSerialize.Serialize(stream, change);
                    ReadAckFromDevice(stream);
                    return true;
                });
        }

        public async Task SelectPatchAsync(int iPatch)
        {
            Debug.WriteLine($"DexyDevice.SelectPatch: Select patch {iPatch}");
            await StartCommand((port, stream) => {
                    Debug.WriteLine($"command: Select patch {iPatch}");
                    SendCommandToDevice(DexyCommand.selectPatch, stream);
                    byte bPatch = (byte)iPatch;
                    ZppSerialize.Serialize(stream, bPatch);
                    ReadAckFromDevice(stream);
                    return true;
                });
        }


        public async Task RebootDeviceAsync(bool bootSelMode)
        {
            DexyCommand command = bootSelMode ? DexyCommand.bootLoad : DexyCommand.boot;
            await StartCommand((port, stream) => {
                    SendCommandToDevice(command, stream);
                    return true;
                });
        }

        public async Task DownloadFirmwareAsync(string firmwareFile)
        {
            // Reboot the module in BOOTSEL mode.
            // (If !IsConnected maybe the device is already in BOOTSEL mode.)
            if (IsConnected) {
                await RebootDeviceAsync(bootSelMode: true);
                await Task.Delay(1000);
            }
            // Find the pseudo-drive exposed by the device in BOOTSEL mode.
            // If not found, retry a few times to give it time to reboot.
            string? driveName = null;
            for (int i = 0; i < 5; i++) {
                driveName = SysUtils.FindDexyDrive();
                if (driveName != null) {
                    break;
                }
                await Task.Delay(1000);
            }
            if (driveName == null) {
                throw new DriveNotFoundException("Dexy device not found");
            }
            // Copy the firmware file to the device
            File.Copy(firmwareFile, Path.Combine(driveName, "firmware.uf2"));
        }

        /// <summary>
        /// Send a command word to the Dexy device over the serial port
        /// </summary>
        /// <param name="command"></param>
        /// <param name="stream"></param>
        /// <exception cref="ArgumentException"></exception>
        /// <exception cref="Exception"></exception>
        private static void SendCommandToDevice(DexyCommand command, Stream stream)
        {
            string strCommand = command switch {
                DexyCommand.version => "vers",
                DexyCommand.upload => "upld",
                DexyCommand.download => "dnld",
                DexyCommand.updPatch => "upd1",
                DexyCommand.updName => "upd2",
                DexyCommand.updSettings =>  "upd3",
                DexyCommand.updOperator =>"upd4",
                DexyCommand.selectPatch => "play",
                DexyCommand.boot => "boot",
                DexyCommand.bootLoad => "btld",
                _ => throw new ArgumentException(null, nameof(command)),
            };
            // Convert the command to ASCII (or similar) - must still be 4 chars
            byte[] asciiBytes = ConvertString.ToAsciiBytes(strCommand);
            if (asciiBytes.Length != strCommand.Length) {
                throw new Exception("Bad unicode conversion in SendCommandToDevice");
            }
            // Send the command
            stream.Write(asciiBytes);
        }

        /// <summary>
        /// Read a line of text (newline-terminated) from the Dexy device
        /// </summary>
        /// <param name="stream"></param>
        /// <returns>The text that was read, with leading and trailing whitespace removed</returns>
        private static string? ReadLineFromDevice(Stream stream)
        {
            using TextReader reader = new StreamReader(stream, Encoding.Latin1);
            string? st = reader.ReadLine();
            if (st != null) {
                st = st.Trim();
            }
            return st;
        }

        /// <summary>
        /// Read a command acknowledgement message from the Dexy device; throw on failure
        /// </summary>
        /// <param name="stream"></param>
        /// <exception cref="Exception"></exception>
        private static void ReadAckFromDevice(Stream stream)
        {
            string? st = ReadLineFromDevice(stream);
            if (st == null || st != "OK") {
                throw new Exception("Bad response from Dexy device");
            }
        }
        
        /// <summary>
        /// The commands recognized by the Dexy device
        /// </summary>
        private enum DexyCommand
        {
            /// <summary>
            /// <see cref="Dexy.DexyPatch.Services.IDexyDevice.GetFirmwareVersionAsync"/>
            /// </summary>
            version,
            /// <summary>
            /// <see cref="Dexy.DexyPatch.Services.IDexyDevice.UploadPatchBankAsync"/>
            /// </summary>
            upload,
            /// <summary>
            /// <see cref="Dexy.DexyPatch.Services.IDexyDevice.DownloadPatchBankAsync"/>
            /// </summary>
            download,
            /// <summary>
            /// An entire patch has been updated - <see cref="Dexy.DexyPatch.Services.IDexyDevice.SendPatchChangedAsync"/>
            /// </summary>
            updPatch,
            /// <summary>
            /// A patch's name has been updated - <see cref="Dexy.DexyPatch.Services.IDexyDevice.SendPatchChangedAsync"/>
            /// </summary>
            updName,
            /// <summary>
            /// A patch setting has been updated - <see cref="Dexy.DexyPatch.Services.IDexyDevice.SendPatchChangedAsync"/>
            /// </summary>
            updSettings,
            /// <summary>
            /// An operator setting has been updated - <see cref="Dexy.DexyPatch.Services.IDexyDevice.SendPatchChangedAsync"/>
            /// </summary>
            updOperator,
            /// <summary>
            /// <see cref="Dexy.DexyPatch.Services.IDexyDevice.SelectPatchAsync"/>
            /// </summary>
            selectPatch,
            /// <summary>
            /// Reboot normally - <see cref="Dexy.DexyPatch.Services.IDexyDevice.RebootDeviceAsync"/>
            /// </summary>
            boot,
            /// <summary>
            /// Reboot into bootloader mode - <see cref="Dexy.DexyPatch.Services.IDexyDevice.RebootDeviceAsync"/>
            /// </summary>
            bootLoad
        }

        #endregion

        #region Command queue

        /// <summary>
        /// A utility wrapper around a <see cref="Dexy.DexyPatch.Services.DexyDevice.CommandFunc{TResult}"/>
        /// </summary>
        /// <param name="port"></param>
        /// <param name="stream"></param>
        private delegate void CommandWrapper(CommPort port, CommPortStream stream);

        /// <summary>
        /// Thread-safe queue of commands to be handled by the worker thread
        /// </summary>
        private readonly BlockingCollection<CommandWrapper> commandQueue = new();

        /// <summary>
        /// Add a command to the queue to be handled by the worker thread
        /// </summary>
        /// <typeparam name="TResult"></typeparam>
        /// <param name="command"></param>
        /// <returns>A Task which upon completion will contain the command's return value</returns>
        private Task<TResult> StartCommand<TResult>(CommandFunc<TResult> command)
        {
            var tcs = new TaskCompletionSource<TResult>();
            commandQueue.Add((port, stream) => {
                    try {
                        tcs.SetResult(command(port, stream));
                    } catch (Exception ex) {
                        tcs.SetException(ex);
                        Debug.WriteLine($"DexyDevice exception {ex.GetType()} {ex.Message}");
                    }
                });
            return tcs.Task;
        }

        #endregion

        #region Device handler thread

        /// <summary>
        /// The worker thread that does the actual communication with the Dexy device
        /// </summary>
        private readonly DeviceThread deviceThread;

        /// <summary>
        /// The worker thread that does the actual communication with the Dexy device
        /// </summary>
        private class DeviceThread : ServiceThread
        {
            const int sleepMs = 1; // even the briefest sleep greatly reduces CPU usage

            public DeviceThread() : base(sleepMs) { }

            private IDexyDevice? dexyDevice;

            /// <summary>
            /// A stream used to communicate with the Dexy device
            /// </summary>
            private CommPortStream? stream;

            /// <summary>
            /// Arguments passed to <see cref="Dexy.DexyPatch.Services.DexyDevice.DeviceThread"/>
            /// </summary>
            public class DeviceThreadArgs : ServiceThread.ServiceThreadArgs
            {
                /// <summary>
                /// The comm port to which the Dexy device is connected
                /// </summary>
                public readonly CommPort port;

                /// <summary>
                /// The queue of commands to be executed
                /// </summary>
                public readonly BlockingCollection<CommandWrapper> commandQueue;

                public DeviceThreadArgs(CommPort port, BlockingCollection<CommandWrapper> commandQueue)
                {
                    this.port = port;
                    this.commandQueue = commandQueue;
                }
            }

            protected override void ThreadInit(ServiceThreadArgs argsIn)
            {
                DeviceThreadArgs args = (DeviceThreadArgs)argsIn;
                stream = new CommPortStream(args.port);
                dexyDevice = Service<IDexyDevice>.Get();
            }

            protected override void ThreadWork(ServiceThreadArgs argsIn)
            {
                DeviceThreadArgs args = (DeviceThreadArgs)argsIn;

                // If there is serial input available, read & echo it (non-blocking)
                if (stream!.BytesAvail > 0) {
                    string stRead = stream.ReadAvail();
                    dexyDevice!.SendTextReceived(stRead);
                }

                // If there's a command queued up, execute it
                if (args.commandQueue.Any()) {
                    Debug.WriteLine("ServiceThread: Handling a command");
                    CommandWrapper command = args.commandQueue.Take();
                    command(args.port, stream);
                    Debug.WriteLine("ServiceThread: Done command");
                }
            }

            protected override void ThreadEnd(ServiceThreadArgs args)
            {
                stream?.Dispose();
                stream = null;
            }
        }

        #endregion
    }
}
