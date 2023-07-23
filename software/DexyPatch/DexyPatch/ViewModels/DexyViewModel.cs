using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Avalonia.Media;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MessageBox.Avalonia.Enums;
using Dexy.DexyPatch.Services;
using Dexy.DexyPatch.Utils;

namespace Dexy.DexyPatch.ViewModels
{
    /// <summary>
    /// ViewModel to manage the Dexy device
    /// </summary>
    public partial class DexyViewModel : ViewModelBase
    {
        public DexyViewModel(MainWindowViewModel parentVM)
        {
            this.parentVM = parentVM;
            serialBuffer = new TextTailBuffer();
            SetConnectedStatus(dexyDevice.IsConnected);
            dexyDevice.Connected += OnConnected;
            dexyDevice.TextReceived += OnTextReceived;
        }

        /// <summary>
        /// ViewModel of the parent (container) view associated with this ViewModel
        /// </summary>
        private readonly MainWindowViewModel parentVM;

        /// <summary><see cref="Dexy.DexyPatch.Services.IDexyDevice"/> service</summary>
        private readonly IDexyDevice dexyDevice = Service<IDexyDevice>.Get();

        /// <summary><see cref="Dexy.DexyPatch.Services.IMessageBoxService"/> service</summary>
        private readonly IMessageBoxService messageBoxService = Service<IMessageBoxService>.Get();

        /// <summary>
        /// Command handler for the "Hide" button - Hides the <see cref="Dexy.DexyPatch.Views.DexyView"/> pane
        /// </summary>
        [RelayCommand]
        private void Hide() => parentVM.IsDexyPaneOpen = false;

        #region Dexy device connection

        /// <summary>Is the Dexy device currently connected?</summary>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(DexyConnection))]
        private bool isDexyConnected = false;

        /// <summary>Name of the comm port connected to the Dexy device</summary>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(DexyConnection))]
        private string? dexyPort = null;

        /// <summary>Formatted string displaying the device connection status</summary>
        public string DexyConnection
        {
            get
            {
                if (IsDexyConnected) {
                    return $"Connected on {DexyPort}";
                } else if (DexyPort != null && DexyPort != String.Empty) {
                    return $"Disconnected ({DexyPort})";
                } else {
                    return "No Dexy connected";
                }
            }
        }

        /// <summary>The version number of the Dexy device's firmware, or null if not available</summary>
        [ObservableProperty]
        private string? firmwareVersion = null;

        /// <summary>
        /// Update the device connection status
        /// </summary>
        /// <param name="connected"></param>
        private void SetConnectedStatus(bool connected)
        {
            IsDexyConnected = connected;
            DexyPort = dexyDevice.PortName;
            // Update the firmware version number asynchronously
            FirmwareVersion = null;
            if (connected) {
                Task.Run(async () => {
                    await Task.Delay(250); // wait for device initialization
                    FirmwareVersion = await dexyDevice.GetFirmwareVersionAsync();
                });
            }
        }

        /// <summary>
        /// Event handler for <see cref="Dexy.DexyPatch.Services.IDexyDevice.Connected"/>
        /// </summary>
        /// <param name="e"></param>
        private void OnConnected(IDexyDevice.ConnectedEventArgs e) { SetConnectedStatus(e.Connected); }

        #endregion

        #region Dexy device operations

        /// <summary>
        /// Command handler for the "Reboot" button - Reboots the Dexy device
        /// </summary>
        [RelayCommand]
        private async void RebootCommand()
        {
            try {
                await dexyDevice.RebootDeviceAsync(bootSelMode: false);
            } catch (Exception ex) {
                await messageBoxService.ShowErrorMessage($"Reboot command failed", ex);
            }
        }

        /// <summary>
        /// Command handler for the "Download" button - Download new firmware to the Dexy device
        /// </summary>
        [RelayCommand]
        private async void DownloadFirmware()
        {
            // Get the previously-used firmware filename
            var settingsManager = Service<ISettingsManager>.Get();
            var savedPathname = settingsManager.Load<FirmwarePathname>();
            string? firmwarePathname = savedPathname.Filename;
            while (true) {
                // Select a firmware file, if not using the same file as last time
                firmwarePathname ??=
                    await messageBoxService.OpenFileDialog(null, "Dexy firmware file", "uf2");
                if (firmwarePathname == null) {
                    // Cancelled
                    return;
                }
                // Dialog to confirm firmware download and optionally select a
                // different firmware file
                var result = await messageBoxService.AskDownloadFirmware(firmwarePathname);
                switch (result) {
                    case ButtonResult.Yes:
                        // Yes: Download the firmware file to the Dexy device
                        try {
                            await dexyDevice.DownloadFirmwareAsync(firmwarePathname);
                        } catch (Exception ex) {
                            await messageBoxService.ShowErrorMessage($"Firmware download failed", ex);
                        }
                        // Can't expect a meaningful return value.
                        savedPathname.Filename = firmwarePathname;
                        settingsManager.Save(savedPathname);
                        return;
                    case ButtonResult.No:
                        // No: This means "ask for a different filename and try again"
                        firmwarePathname = null;
                        break;
                    case ButtonResult.Cancel:
                    default:
                        // Cancelled
                        return;
                }
            }
        }

        /// <summary>
        /// Uniquely-named subclass of <see cref="Dexy.DexyPatch.Services.ISettingsManager.SavedFilenameBase"/>
        /// for use with <see cref="Dexy.DexyPatch.Services.SettingsManager"/>
        /// </summary>
        private class FirmwarePathname : ISettingsManager.SavedFilenameBase { }

        #endregion

        #region Device connection settings dialog

        /// <summary>
        /// ViewModel of the Dexy connection settings dialog
        /// </summary>
        [ObservableProperty]
        DexySettingsViewModel? dexySettingsVM = null;

        /// <summary>
        /// Command handler to open and close the settings dialog
        /// </summary>
        [RelayCommand]
        private void ShowHideSettings()
        {
            if (DexySettingsVM == null) {
                ShowSettingsDialog();
            } else {
                HideSettingsDialog();
            }
        }

        /// <summary>
        /// Open the settings dialog
        /// </summary>
        private void ShowSettingsDialog()
        {
            if (DexySettingsVM == null) {
                // Creating the ViewModel will initialize the DexySettingsView as well
                DexySettingsVM = new(this, dexyDevice.PortSettings);
                DexySettingsVM.DialogSave += OnDialogSave;
                DexySettingsVM.DialogCancel += OnDialogCancel;
            }
        }

        /// <summary>
        /// Close the settings dialog
        /// </summary>
        private void HideSettingsDialog() { DexySettingsVM = null; }

        /// <summary>
        /// Event handler to close the dialog and save the edited data
        /// </summary>
        /// <param name="e"></param>
        private void OnDialogSave(DexySettingsViewModel.DialogSaveEventArgs e)
        {
            dexyDevice.Connect(e.Settings);
            SetConnectedStatus(dexyDevice.IsConnected);
            HideSettingsDialog();
        }

        /// <summary>
        /// Event handler to close the dialog without saving
        /// </summary>
        private void OnDialogCancel() { HideSettingsDialog(); }

        #endregion

        #region Logging window for device debugging text

        /// <summary>
        /// Displayed text from the Dexy device
        /// </summary>
        [ObservableProperty]
        private string? serialTextLog;

        /// <summary>
        /// Text buffer with the most recent messages from the Dexy device
        /// </summary>
        private readonly TextTailBuffer serialBuffer;

        /// <summary>
        /// Event handler to receive text from the Dexy device
        /// </summary>
        /// <param name="e"></param>
        private void OnTextReceived(IDexyDevice.TextReceivedEventArgs e)
        {
            // Text received from DexyDevice. Append it to the buffer.
            serialBuffer.Append(e.Text);
            SendSerialTextLogChanging();
            SerialTextLog = serialBuffer.ToString();
        }

        /// <summary>
        /// Sent when text is received from the Dexy device
        /// </summary>
        /// <remarks>
        /// This event helps to support auto-scroll functionality in
        /// <see cref="Dexy.DexyPatch.Views.DexyView"/>
        /// </remarks>
        public event SerialTextLogChangingHandler? SerialTextLogChanging;

        /// <summary>
        /// Method definition for the <see cref="Dexy.DexyPatch.ViewModels.DexyViewModel.SerialTextLogChanging"/> event
        /// </summary>
        public delegate void SerialTextLogChangingHandler();

        /// <summary>
        /// Send a <see cref="Dexy.DexyPatch.ViewModels.DexyViewModel.SerialTextLogChanging"/> event
        /// </summary>
        private void SendSerialTextLogChanging() => SerialTextLogChanging?.Invoke();

        #endregion
    }
}
