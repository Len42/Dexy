using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils;
using Dexy.DexyPatch.Utils.PatchChanges;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Interface to the Dexy device
    /// </summary>
    public interface IDexyDevice : IService
    {
        #region Comm port connection

        /// <summary>
        /// Serial port configuration
        /// </summary>
        CommPort.CommPortSettings PortSettings { get; }

        /// <summary>
        /// Serial port name
        /// </summary>
        string? PortName { get; }

        /// <summary>
        /// Connect to the Dexy module
        /// </summary>
        /// <param name="settings"></param>
        void Connect(CommPort.CommPortSettings settings);

        /// <summary>
        /// Is a Dexy device currently connected?
        /// </summary>
        bool IsConnected { get; }

        #endregion

        #region Async command methods

        /// <summary>
        /// Send a command to the Dexy device to get its firmware version number.
        /// Async.
        /// </summary>
        /// <returns>Version number string or null if not available</returns>
        Task<string?> GetFirmwareVersionAsync();

        /// <summary>
        /// Send a command to the Dexy device to upload its patch bank.
        /// Async.
        /// </summary>
        /// <returns></returns>
        Task<PatchBank?> UploadPatchBankAsync();

        /// <summary>
        /// Download a <see cref="Dexy.DexyPatch.Models.PatchBank"/> to the Dexy device,
        /// completely replacing its current patch bank.
        /// Async.
        /// </summary>
        /// <param name="patchBank"></param>
        /// <returns></returns>
        Task DownloadPatchBankAsync(PatchBank patchBank);

        /// <summary>
        /// Live patch updating - Download a single editing change to the module.
        /// Async.
        /// </summary>
        /// <param name="change"></param>
        /// <returns></returns>
        Task SendPatchChangedAsync(PatchChangeBase change);

        /// <summary>
        /// Live patch updating - Tell the Dexy module to select the given patch.
        /// </summary>
        /// <param name="iPatch">Patch number</param>
        /// <returns></returns>
        Task SelectPatchAsync(int iPatch);

        /// <summary>
        /// Send a command to reboot the Dexy device.
        /// Async.
        /// </summary>
        /// <param name="bootSelMode">If true reboot into bootloader mode; else restart normally</param>
        /// <returns></returns>
        Task RebootDeviceAsync(bool bootSelMode);

        /// <summary>
        /// Download a firmware file to the Dexy device, completely replacing its current firmware.
        /// Async.
        /// </summary>
        /// <param name="firmwareFile">A UF2 file containing compiled firmware for the module</param>
        /// <returns></returns>
        /// <remarks>
        /// This method reboots the device in bootloader mode, finds the
        /// corresponding USB pseudo-drive, and copies the file <paramref name="firmwareFile"/>
        /// to the device. The Dexy module will then reboot and run the new firmware.
        /// </remarks>
        Task DownloadFirmwareAsync(string firmwareFile);

        #endregion

        #region Events

        /// <summary>
        /// Sent when a Dexy device connects or disconnects from its comm port
        /// </summary>
        public event ConnectedHandler? Connected;

        /// <summary>
        /// Arguments for the <see cref="Dexy.DexyPatch.Services.IDexyDevice.Connected"/> event
        /// </summary>
        public class ConnectedEventArgs
        {
            public ConnectedEventArgs(bool connected) { Connected = connected; }

            /// <summary>
            /// true if the device has been connected, false if it has been disconnected
            /// </summary>
            public bool Connected { get; set; }
        }

        /// <summary>
        /// Method definition for the <see cref="Dexy.DexyPatch.Services.IDexyDevice.Connected"/> event
        /// </summary>
        /// <param name="e"></param>
        public delegate void ConnectedHandler(ConnectedEventArgs e);

        /// <summary>
        /// Sent when text is received from the Dexy device over its comm port
        /// </summary>
        /// <remarks>
        /// This event is only triggered for text that is received when not expected,
        /// not for data that is explicitly read e.g. by <see cref="UploadPatchBankAsync"/>.
        /// This is mainly for debugging info that is sent by the module.
        /// </remarks>
        public event TextReceivedHandler? TextReceived;

        /// <summary>
        /// Arguments for the <see cref="Dexy.DexyPatch.Services.IDexyDevice.TextReceived"/> event
        /// </summary>
        public class TextReceivedEventArgs
        {
            public TextReceivedEventArgs(string text) { Text = text; }

            /// <summary>
            /// The text that was received
            /// </summary>
            public string Text { get; set; }
        }

        /// <summary>
        /// Method definition for the <see cref="Dexy.DexyPatch.Services.IDexyDevice.TextReceived"/> event
        /// </summary>
        /// <param name="e"></param>
        public delegate void TextReceivedHandler(TextReceivedEventArgs e);

        /// <summary>
        /// Send a TextReceived event.
        /// </summary>
        /// <param name="text"></param>
        public void SendTextReceived(string text);
        // TODO: This is a private method but it needs to be in the interface
        // for implementation reasons. Maybe make a separate implementation interface?

        #endregion
    }
}
