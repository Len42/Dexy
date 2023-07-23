using System;
using System.Configuration;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;
using System.Xml.Linq;
using Avalonia;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MessageBox.Avalonia.Enums;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Services;
using Dexy.DexyPatch.Utils.PatchChanges;

namespace Dexy.DexyPatch.ViewModels
{
    /// <summary>
    /// ViewModel for the main application window
    /// </summary>
    public partial class MainWindowViewModel : ViewModelBase
    {
        public MainWindowViewModel()
        {
            LoadInitialPatchBank();

            dexyVM = new(this); // This will initialize the DexyView as well

            // Keep track of whether the Dexy device is connected
            IsDexyConnected = dexyDevice.IsConnected;
            dexyDevice.Connected += (e) => { IsDexyConnected = e.Connected; };
        }

        #region Services used

        /// <summary><see cref="Dexy.DexyPatch.Services.IDataManager"/> service</summary>
        private readonly IDataManager dataManager = Service<IDataManager>.Get();

        /// <summary><see cref="Dexy.DexyPatch.Services.IDexyDevice"/> service</summary>
        private readonly IDexyDevice dexyDevice = Service<IDexyDevice>.Get();

        /// <summary><see cref="Dexy.DexyPatch.Services.ILiveUpdater"/> service</summary>
        private readonly ILiveUpdater liveUpdater = Service<ILiveUpdater>.Get();

        /// <summary><see cref="Dexy.DexyPatch.Services.IMessageBoxService"/> service</summary>
        private readonly IMessageBoxService messageBoxService = Service<IMessageBoxService>.Get();

        /// <summary><see cref="Dexy.DexyPatch.Services.ISettingsManager"/> service</summary>
        private readonly ISettingsManager settingsManager = Service<ISettingsManager>.Get();

        #endregion

        #region App info

        /// <summary>
        /// Application name
        /// </summary>
        public static string AppName { get => ProjectVersion.AppName; }

        /// <summary>
        /// Application name and version number
        /// </summary>
        public static string VersionInfo { get =>
                $"{ProjectVersion.AppName} v{ProjectVersion.VersionLong} {ProjectVersion.Date}"; }

        #endregion

        #region Dexy device pane

        /// <summary>
        /// ViewModel for the contained Dexy device view
        /// </summary>
        [ObservableProperty]
        private DexyViewModel? dexyVM;

        /// <summary>
        /// Is the Dexy device currently connected?
        /// </summary>
        /// <remarks>
        /// This is kept up-to-date by the <see cref="Dexy.DexyPatch.Services.IDexyDevice.Connected"/> event.
        /// </remarks>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(CanDownloadPatchBank))]
        private bool isDexyConnected = false;

        /// <summary>
        /// Is the <see cref="Dexy.DexyPatch.ViewModels.DexyViewModel"/> pane open?
        /// </summary>
        [ObservableProperty]
        private bool isDexyPaneOpen = false;

        /// <summary>
        /// Toggle the <see cref="Dexy.DexyPatch.ViewModels.DexyViewModel"/> pane open or closed
        /// </summary>
        [RelayCommand]
        private void ShowHideDexyPane() => IsDexyPaneOpen = !IsDexyPaneOpen;

        /// <summary>
        /// Enable or disable live patch updating to the connected Dexy module
        /// </summary>
        /// <param name="f"></param>
        private void SetLiveUpdating(bool f)
        {
            liveUpdater.CheckSendChanges();
            liveUpdater.IsLive = f;
        }

        #endregion

        #region Patch bank data

        /// <summary>
        /// ViewModel for the contained <see cref="Dexy.DexyPatch.Models.PatchBank"/> view
        /// </summary>
        [ObservableProperty]
        private PatchBankViewModel? patchBankVM = null;

        /// <summary>
        /// The <see cref="Dexy.DexyPatch.Models.PatchBank"/> currently being edited.
        /// Exposed for access by child VMs.
        /// </summary>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(CanDownloadPatchBank))]
        private PatchBank? patchBank;

        /// <summary>
        /// The <see cref="Dexy.DexyPatch.Services.PatchDataSource"/> from which the
        /// <see cref="Dexy.DexyPatch.Models.PatchBank"/> was loaded
        /// </summary>
        private PatchDataSource? dataSource;

        /// <summary>
        /// The name of the patch bank that is being edited
        /// </summary>
        /// <remarks>
        /// May be a filename or "new patchbank" or "uploaded from Dexy"
        /// </remarks>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(Title))]
        private string? patchBankName;

        /// <summary>
        /// Window title
        /// </summary>
        public string Title { get => PatchBankName ?? "No patch bank loaded"; }

        #endregion

        #region Track data modifications

        /// <summary>
        /// Has the <see cref="Dexy.DexyPatch.Models.PatchBank"/> data been modified?
        /// </summary>
        [ObservableProperty]
        private bool isModified = false;

        /// <summary>
        /// Mark the current <see cref="Dexy.DexyPatch.Models.PatchBank"/> data as modified
        /// </summary>
        /// <param name="f"></param>
        private void SetModified(bool f) => IsModified = f;

        /// <summary>
        /// Set the <see cref="isModified"/> flag and also mark the changed data for live updating
        /// </summary>
        /// <param name="iPatch"></param>
        /// <param name="value"></param>
        public void SetDataModified(int iPatch, Patch value)
        {
            SetModified(true);
            liveUpdater.PatchUpdated(iPatch, value);
        }

        /// <summary>
        /// Set the <see cref="isModified"/> flag and also mark the changed data for live updating
        /// </summary>
        /// <param name="iPatch"></param>
        /// <param name="field"></param>
        /// <param name="value"></param>
        public void SetDataModified(int iPatch, PatchSetting field, object value)
        {
            SetModified(true);
            liveUpdater.PatchSettingUpdated(iPatch, field, value);
        }

        /// <summary>
        /// Set the <see cref="isModified"/> flag and also mark the changed data for live updating
        /// </summary>
        /// <param name="iPatch"></param>
        /// <param name="iOp"></param>
        /// <param name="field"></param>
        /// <param name="value"></param>
        public void SetDataModified(int iPatch, int iOp, OpParamSetting field, object value)
        {
            SetModified(true);
            liveUpdater.PatchOperatorUpdated(iPatch, iOp, field, value);
        }

        #endregion

        #region Load and save patchbank data

        /// <summary>
        /// Load the <see cref="Dexy.DexyPatch.Models.PatchBank"/> to be edited at app startup
        /// </summary>
        private async void LoadInitialPatchBank()
        {
            PatchBank patchBankInit;
            PatchDataSource dataSourceInit;
            bool isModified;
            (patchBankInit, dataSourceInit, isModified) = await dataManager.LoadInitialAsync();
            UpdateViewModel(patchBankInit, dataSourceInit);
            SetModified(isModified);
        }

        /// <summary>
        /// Load a new <see cref="Dexy.DexyPatch.Models.PatchBank"/> with a set of default patches
        /// </summary>
        [RelayCommand]
        private async void LoadDefaultPatchBank()
        {
            if (await CheckSaveModel()) {
                try {
                    PatchBank patchBank = dataManager.LoadDefault();
                    UpdateViewModel(patchBank, new(DataSourceType.init, null));
                } catch (Exception ex) {
                    await messageBoxService.ShowErrorMessage($"Failed to load the default patch bank", ex);
                }
            }
        }

        /// <summary>
        /// Uniquely-named subclass of <see cref="Dexy.DexyPatch.Services.ISettingsManager.SavedFilenameBase"/>
        /// for use with <see cref="Dexy.DexyPatch.Services.SettingsManager"/>
        /// </summary>
        private class LastPatchBankFile : ISettingsManager.SavedFilenameBase { }

        /// <summary>
        /// Load a <see cref="Dexy.DexyPatch.Models.PatchBank"/> from a file
        /// </summary>
        [RelayCommand]
        private async void LoadFile()
        {
            if (await CheckSaveModel()) {
                var savedPathname = settingsManager.Load<LastPatchBankFile>();
                string? fileName = await messageBoxService.OpenFileDialog(savedPathname.Filename, "Dexy patch files", "dexy");
                if (fileName != null) {
                    try {
                        PatchBank patchBank = dataManager.LoadFile(fileName);
                        UpdateViewModel(patchBank, new(DataSourceType.file, fileName));
                        savedPathname.Filename = fileName;
                        settingsManager.Save(savedPathname);
                    } catch (Exception ex) {
                        await messageBoxService.ShowErrorMessage($"Failed to load the file \"{Path.GetFileName(fileName)}\"", ex);
                    }
                }
            }
        }

        /// <summary>
        /// Save the <see cref="Dexy.DexyPatch.Models.PatchBank"/> currently being edited to a file
        /// </summary>
        /// <returns></returns>
        /// <remarks>
        /// If the current patchbank was loaded from a file it is saved to the same file.
        /// If not, the user is prompted for a filename.
        /// </remarks>
        [RelayCommand]
        private async Task SaveFile()
        {
            if (IsModified && PatchBank != null) {
                if (dataSource != null && dataSource.Type == DataSourceType.file && dataSource.Pathname != null) {
                    try {
                        dataManager.SaveFile(PatchBank, dataSource.Pathname);
                        SetModified(false);
                    } catch (Exception ex) {
                        await messageBoxService.ShowErrorMessage($"Failed to save the file \"{Path.GetFileName(dataSource.Pathname)}\"", ex);
                    }
                } else {
                    await SaveFileAs();
                }
            }
        }

        /// <summary>
        /// Save the <see cref="Dexy.DexyPatch.Models.PatchBank"/> currently
        /// being edited to a file selected by the user
        /// </summary>
        /// <returns></returns>
        [RelayCommand]
        private async Task SaveFileAs()
        {
            if (PatchBank != null) {
                var savedPathname = settingsManager.Load<LastPatchBankFile>();
                string? fileName = await messageBoxService.SaveFileDialog(savedPathname.Filename, "Dexy patch files", "dexy");
                if (fileName != null) {
                    try {
                        dataManager.SaveFile(PatchBank, fileName);
                        UpdateDataSource(new(DataSourceType.file, fileName));
                        SetModified(false);
                        savedPathname.Filename = fileName;
                        settingsManager.Save(savedPathname);
                    } catch (Exception ex) {
                        await messageBoxService.ShowErrorMessage($"Failed to save the file \"{Path.GetFileName(fileName)}\"", ex);
                    }
                }
            }
        }

        /// <summary>
        /// Upload the patchbank from the connected Dexy module
        /// </summary>
        [RelayCommand]
        private async void UploadPatchBank()
        {
            if (await CheckSaveModel()) {
                try {
                    PatchBank? patchBank = await dexyDevice.UploadPatchBankAsync();
                    UpdateViewModel(patchBank, new(DataSourceType.uploaded, null));
                } catch (Exception ex) {
                    await messageBoxService.ShowErrorMessage($"Failed to upload from Dexy", ex);
                }
            }
        }

        /// <summary>
        /// Can the currently-loaded <see cref="Dexy.DexyPatch.Models.PatchBank"/>
        /// be downloaded to the Dexy module?
        /// </summary>
        public bool CanDownloadPatchBank
        {
            get
            {
                return PatchBank != null && IsDexyConnected;
            }
        }

        /// <summary>
        /// Download the currently <see cref="Dexy.DexyPatch.Models.PatchBank"/> to the Dexy module
        /// </summary>
        [RelayCommand]
        private async void DownloadPatchBank()
        {
            if (CanDownloadPatchBank) {
                ButtonResult result = 
                    await messageBoxService.AskYesNo("Download patches?", "Download this patch bank to the Dexy module?");
                if (result == ButtonResult.Yes) {
                    try {
                        await dexyDevice.DownloadPatchBankAsync(PatchBank!);
                        UpdateDataSource(new(DataSourceType.uploaded, null));
                        //SetModified(false);
                    } catch (Exception ex) {
                        await messageBoxService.ShowErrorMessage($"Failed to download to Dexy", ex);
                    }
                }
            }
        }

        /// <summary>
        /// If the current <see cref="Dexy.DexyPatch.Models.PatchBank"/> has been modified
        /// prompt the user to save it
        /// </summary>
        /// <returns></returns>
        private async Task<bool> CheckSaveModel()
        {
            if (!IsModified) {
                // Nothing to do
                return true;
            } else {
                // Data modified - Ask to save
                ButtonResult result =
                    await messageBoxService.AskSaveChanges(PatchBankName);
                switch (result) {
                    case ButtonResult.Yes:
                        await SaveFile();
                        return true;
                    case ButtonResult.No:
                        // Discard changes
                        SetModified(false);
                        return true;
                    case ButtonResult.Cancel:
                    default:
                        // Cancel the operation
                        return false;
                }
            }
        }

        /// <summary>
        /// If the current <see cref="Dexy.DexyPatch.Models.PatchBank"/> has been modified,
        /// save it to a temp file so editing can resume later.
        /// Call this when the app is shutting down.
        /// </summary>
        public void SaveWorkingData()
        {
            dataManager.SaveWorkingData(IsModified ? PatchBank : null);
        }

        /// <summary>
        /// Update this ViewModel and children after loading a new <see cref="Dexy.DexyPatch.Models.PatchBank"/>
        /// from a <see cref="Dexy.DexyPatch.Services.PatchDataSource"/>
        /// </summary>
        /// <param name="patchBankNew"></param>
        /// <param name="dataSourceNew"></param>
        private void UpdateViewModel(PatchBank? patchBankNew, PatchDataSource dataSourceNew)
        {
            PatchBank = patchBankNew;
            SetModified(false);
            UpdateDataSource(dataSourceNew);
            if (patchBankNew == null || dataSourceNew.Type == DataSourceType.none) {
                //PatchBankName = null;
                PatchBankVM = null;
            } else {
                PatchBankVM = new(this); // This will reinitialize the PatchBankView as well.
            }
        }

        /// <summary>
        /// Update the current <see cref="Dexy.DexyPatch.Services.PatchDataSource"/>
        /// after loading or saving a <see cref="Dexy.DexyPatch.Models.PatchBank"/>
        /// </summary>
        /// <param name="dataSourceNew"></param>
        /// <exception cref="NotImplementedException"></exception>
        private void UpdateDataSource(PatchDataSource dataSourceNew)
        {
            dataSource = dataSourceNew;
            dataManager.LastUsed = dataSourceNew;
            PatchBankName = dataSource.Type switch {
                DataSourceType.none => null,
                DataSourceType.init => "New patch bank",
                DataSourceType.uploaded => "Dexy patch bank",
                DataSourceType.file => Path.GetFileName(dataSource.Pathname),
                _ => throw new NotImplementedException()
            };
            // Enable live updating only if the current patchbank is the one in
            // the connected Dexy module
            SetLiveUpdating(dataSource.Type == DataSourceType.uploaded);
        }

        #endregion
    }
}
