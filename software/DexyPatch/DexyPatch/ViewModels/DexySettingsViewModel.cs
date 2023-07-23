using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Dexy.DexyPatch.Utils;

namespace Dexy.DexyPatch.ViewModels
{
    /// <summary>
    /// ViewModel to manage the comm port settings for the Dexy device
    /// </summary>
    /// <remarks>
    /// <see cref="Dexy.DexyPatch.Views.DexySettingsView"/> is basically a dialog
    /// that is embedded in the <see cref="Dexy.DexyPatch.Views.DexyView"/>
    /// </remarks>
    public partial class DexySettingsViewModel : ViewModelBase
    {
        public DexySettingsViewModel(DexyViewModel parentVM, CommPort.CommPortSettings portSettings)
        {
            this.parentVM = parentVM;
            this.portSettings = portSettings;
            SetDialogFields();
        }

        /// <summary>
        /// ViewModel of the parent (container) view associated with this ViewModel
        /// </summary>
        /// <remarks>
        /// Not needed in this class, but included for consistency and in case it's ever needed
        /// </remarks>
        private readonly DexyViewModel parentVM;

        #region View interface

        /// <summary>Comm port name, e.g. "COM1"</summary>
        [ObservableProperty]
        private string? portName;

        /// <summary>Comm port baud rate</summary>
        [ObservableProperty]
        private int baudRate;

        /// <summary>Comm port parity</summary>
        [ObservableProperty]
        private int parity;

        /// <summary>Comm port data bits</summary>
        [ObservableProperty]
        private int dataBitsIndex; // yeah, it has to be bound by index

        /// <summary>Comm port stop bits</summary>
        [ObservableProperty]
        private int stopBits;

        /// <summary>OK button handler - Save changes and close the dialog</summary>
        [RelayCommand]
        private void OkButton()
        {
            GetDialogFields();
            SendDialogSave();
        }

        /// <summary>Text displayed in the cancel button</summary>
        [ObservableProperty]
        private string cancelButtonText = "Close";

        /// <summary>Cancel button handler - Close the dialog without saving changes</summary>
        [RelayCommand]
        private void CancelButton() => SendDialogCancel();

        #endregion

        #region Events

        /// <summary>
        /// Sent when the user commits their changes
        /// </summary>
        public event DialogSaveHandler? DialogSave;

        /// <summary>
        /// Arguments for the <see cref="Dexy.DexyPatch.ViewModels.DexySettingsViewModel.DialogSave"/> event
        /// </summary>
        public class DialogSaveEventArgs
        {
            public DialogSaveEventArgs(CommPort.CommPortSettings settings) { Settings = settings; }

            /// <summary>
            /// Settings for the serial port that communicates with the Dexy device
            /// </summary>
            public CommPort.CommPortSettings Settings { get; set; }
        }
    
        /// <summary>
        /// Method definition for the <see cref="Dexy.DexyPatch.ViewModels.DexySettingsViewModel.DialogSave"/> event
        /// </summary>
        /// <param name="e"></param>
        public delegate void DialogSaveHandler(DialogSaveEventArgs e);

        /// <summary>
        /// Send a <see cref="Dexy.DexyPatch.ViewModels.DexySettingsViewModel.DialogSave"/> event
        /// </summary>
        void SendDialogSave() { DialogSave?.Invoke(new(portSettings)); }

        /// <summary>
        /// Sent when the user cancels their changes
        /// </summary>
        public event DialogCancelHandler? DialogCancel;

        /// <summary>
        /// Method definition for the <see cref="Dexy.DexyPatch.ViewModels.DexySettingsViewModel.DialogCancel"/> event
        /// </summary>
        public delegate void DialogCancelHandler();

        /// <summary>
        /// Send a <see cref="Dexy.DexyPatch.ViewModels.DexySettingsViewModel.DialogCancel"/> event
        /// </summary>
        void SendDialogCancel() { DialogCancel?.Invoke(); }

        #endregion

        #region Dialog data handling

        /// <summary>
        /// Flag to ignore changes to data members during initialization
        /// </summary>
        /// <seealso cref="Dexy.DexyPatch.ViewModels.DexySettingsViewModel.OnPropertyChanged"/>
        private bool ignoreOnPropertyChanged = false;

        /// <summary>
        /// Dialog data input/output:
        /// Settings for the serial port that communicates with the Dexy device
        /// </summary>
        private readonly CommPort.CommPortSettings portSettings;

        /// <summary>
        /// Set the properties of this from the <see cref="portSettings"/>
        /// </summary>
        private void SetDialogFields()
        {
            ignoreOnPropertyChanged = true;
            // Settings are in "settings".
            PortName = portSettings.portName;
            BaudRate = portSettings.baudRate;
            Parity = (int)portSettings.parity;
            DataBitsIndex = portSettings.dataBits - 5;
            StopBits = (int)portSettings.stopBits;
            ignoreOnPropertyChanged = false;
        }

        /// <summary>
        /// Get the updated properties into the <see cref="portSettings"/>
        /// </summary>
        private void GetDialogFields()
        {
            // NOTE: Using the saved "settings" variable because it contains
            // some properties that aren't represented in the dialog, which
            // should be returned unchanged.
            portSettings.portName = PortName ?? "";
            portSettings.baudRate = BaudRate;
            portSettings.parity = (Parity)Parity;
            portSettings.dataBits = DataBitsIndex + 5;
            portSettings.stopBits = (StopBits)StopBits;
        }

        /// <summary>
        /// Mark the data as modified
        /// </summary>
        private void SetModified()
        {
            // Change the text of the cancel button from "Close" to "Cancel" to
            // indicate that data has changed and will be committed.
            CancelButtonText = "Cancel";
        }

        /// <summary>
        /// Override to mark the data as modified when any of the data properties are modified
        /// </summary>
        /// <param name="e"></param>
        protected override void OnPropertyChanged(PropertyChangedEventArgs e)
        {
            base.OnPropertyChanged(e);
            // This gets called for every ObservableProperty.
            // Flag the data as modified if one of the data fields has changed.
            // Ignore changes that happen due to SetDialogFields.
            if (!ignoreOnPropertyChanged) {
                switch (e.PropertyName) {
                    case nameof(PortName):
                    case nameof(BaudRate):
                    case nameof(Parity):
                    case nameof(DataBitsIndex):
                    case nameof(StopBits):
                        SetModified();
                        break;
                }
            }
        }

        #endregion
    }
}
