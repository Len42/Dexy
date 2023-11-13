using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using MsBox.Avalonia;
using MsBox.Avalonia.Dto;
using MsBox.Avalonia.Enums;
using MsBox.Avalonia.Models;
using Dexy.DexyPatch.Utils;
using Avalonia.Platform.Storage;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Service for displaying various message boxes and dialogs
    /// </summary>
    /// <remarks>
    /// Uses the MsBox.Avalonia package
    /// </remarks>
    public class DialogService : IDialogService
    {
        public DialogService() { }

        public void Terminate() { }

        public async Task ShowErrorMessage(string message, Exception? ex)
        {
            string? stError = null;
            if (ex != null) {
                stError = $"{ex.Message} ({ex.GetType().Name})";
            }
            var messageBox = MessageBoxManager.GetMessageBoxStandard(
                new MessageBoxStandardParams {
                    ContentTitle = $"ERROR - {Application.Current?.Name}",
                    ContentHeader = message,
                    ContentMessage = stError,
                    Icon = Icon.Error,
                    ButtonDefinitions = ButtonEnum.Ok,
                    SizeToContent = SizeToContent.Height, // FUBAR: This property does the opposite!
                    MaxWidth = 500,
                    SystemDecorations = SystemDecorations.Full,
                    WindowStartupLocation = WindowStartupLocation.CenterOwner
                });
            if (MainWindow == null) {
                throw new InvalidOperationException("No parent window for dialog");
            }
            await messageBox.ShowAsPopupAsync(MainWindow);
        }

        public async Task<ButtonResult> AskYesNo(string header, string? message)
        {
            var messageBox = MessageBoxManager.GetMessageBoxStandard(
                new MessageBoxStandardParams {
                    ContentTitle = Application.Current?.Name,
                    ContentHeader = header,
                    ContentMessage = message,
                    Icon = Icon.Question,
                    ButtonDefinitions = ButtonEnum.YesNo,
                    SizeToContent = SizeToContent.Height, // FUBAR: This property does the opposite!
                    MaxWidth = 500,
                    SystemDecorations = SystemDecorations.Full,
                    WindowStartupLocation = WindowStartupLocation.CenterOwner
                });
            if (MainWindow == null) {
                throw new InvalidOperationException("No parent window for dialog");
            }
            return await messageBox.ShowAsPopupAsync(MainWindow);
        }

        public async Task<ButtonResult> AskSaveChanges(string? name)
        {
            const string btnYes = "Save";
            const string btnNo = "Discard Changes";
            const string btnCancel = "Cancel";
            var messageBox = MessageBoxManager.GetMessageBoxCustom(
                new MessageBoxCustomParams {
                    ContentTitle = Application.Current?.Name,
                    ContentHeader = "Save changes?",
                    ContentMessage = $"Patchbank \"{name?.Trim()??""}\" has changed.",
                    Icon=Icon.Question,
                    ButtonDefinitions = new[] {
                        new ButtonDefinition {Name = btnYes, IsDefault = true},
                        new ButtonDefinition {Name = btnNo},
                        new ButtonDefinition {Name = btnCancel, IsCancel = true },
                    },
                    SizeToContent = SizeToContent.Height, // FUBAR: This property does the opposite!
                    MaxWidth = 500,
                    SystemDecorations = SystemDecorations.Full,
                    WindowStartupLocation = WindowStartupLocation.CenterOwner
                });
            if (MainWindow == null) {
                throw new InvalidOperationException("No parent window for dialog");
            }
            string stResult = await messageBox.ShowAsPopupAsync(MainWindow);
            switch (stResult) {
                case btnYes:
                    return ButtonResult.Yes;
                case btnNo:
                    return ButtonResult.No;
                case btnCancel:
                    return ButtonResult.Cancel;
                default:
                    return ButtonResult.None;
            }
        }

        public async Task<ButtonResult> AskDownloadFirmware(string pathname)
        {
            const string btnYes = "Download";
            const string btnNo = "Select File";
            const string btnCancel = "Cancel";
            const int maxPathnameChars = 40;
            var messageBox = MessageBoxManager.GetMessageBoxCustom(
                new MessageBoxCustomParams {
                    ContentTitle = Application.Current?.Name,
                    ContentHeader = "Download firmware file to Dexy module?",
                    ContentMessage = $"File: {SysUtils.AbbreviatePathname(pathname, maxPathnameChars)}",
                    Icon = Icon.Question,
                    ButtonDefinitions = new[] {
                        new ButtonDefinition {Name = btnYes, IsDefault = true},
                        new ButtonDefinition {Name = btnNo},
                        new ButtonDefinition {Name = btnCancel, IsCancel = true },
                    },
                    SizeToContent = SizeToContent.Height, // FUBAR: This property does the opposite!
                    MaxWidth = 500,
                    SystemDecorations = SystemDecorations.Full,
                    WindowStartupLocation = WindowStartupLocation.CenterOwner
                });
            if (MainWindow == null) {
                throw new InvalidOperationException("No parent window for dialog");
            }
            string stResult = await messageBox.ShowAsPopupAsync(MainWindow);
            switch (stResult) {
                case btnYes:
                    return ButtonResult.Yes;
                case btnNo:
                    return ButtonResult.No;
                case btnCancel:
                    return ButtonResult.Cancel;
                default:
                    return ButtonResult.None;
            }
        }

        public async Task<string?> OpenFileDialog(string? initPathname, string fileTypeName, string fileTypeExt)
        {
            var storageProvider = GetStorageProvider();
            var startLocation = await GetFolderLocationFromPath(initPathname);
            var options = new FilePickerOpenOptions() {
                Title = "Load File",
                SuggestedStartLocation = startLocation,
                AllowMultiple = false,
                FileTypeFilter = GetFileFilterList(fileTypeName, fileTypeExt)
            };
            IReadOnlyList<IStorageFile> files = await storageProvider.OpenFilePickerAsync(options);
            return (files.Count > 0) ? GetPathFromStorageFile(files[0]) : null;
        }

        public async Task<string?> SaveFileDialog(string? initPathname, string fileTypeName, string fileTypeExt)
        {
            var storageProvider = GetStorageProvider();
            var startLocation = await GetFolderLocationFromPath(initPathname);
            var options = new FilePickerSaveOptions()
            {
                Title = "Save File",
                SuggestedStartLocation = startLocation,
                SuggestedFileName="New File",
                DefaultExtension = fileTypeExt,
                ShowOverwritePrompt = true,
                FileTypeChoices = GetFileFilterList(fileTypeName, fileTypeExt)
            };
            IStorageFile? file = await storageProvider.SaveFilePickerAsync(options);
            return GetPathFromStorageFile(file);
        }

        /// <summary>
        /// Get a list of file types, including the given file type, to display in the dialog
        /// </summary>
        /// <param name="fileTypeName"></param>
        /// <param name="fileTypeExt"></param>
        /// <returns></returns>
        private static List<FilePickerFileType> GetFileFilterList(string fileTypeName, string fileTypeExt)
        {
            FilePickerFileType fileType = new(fileTypeName) {
                Patterns = new[] { Path.ChangeExtension("*", fileTypeExt) },
                MimeTypes = new[] {"application/octet-stream"}
            };
            var fileTypes = new List<FilePickerFileType>{ fileType, FilePickerFileTypes.All };
            return fileTypes;
        }

        /// <summary>
        /// Get the main window of the application
        /// </summary>
        private static Window? MainWindow
        {
            get
            {
                IApplicationLifetime? lifetime = Application.Current?.ApplicationLifetime;
                if (lifetime != null && lifetime is IClassicDesktopStyleApplicationLifetime lifetimeDesktop) {
                    return lifetimeDesktop.MainWindow;
                } else {
                    return null;
                }
            }
        }

        /// <summary>
        /// Get the Avalonia storage provider
        /// </summary>
        /// <returns></returns>
        /// <exception cref="Exception"></exception>
        private static IStorageProvider GetStorageProvider()
        {
            var storageProvider = MainWindow?.StorageProvider;
            if(storageProvider is null) {
                throw new Exception("Cannot get IStorageProvider");
            }
            return storageProvider;
        }

        /// <summary>
        /// Get the folder containing the given file
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        private static Task<IStorageFolder?> GetFolderLocationFromPath(string? path)
        {
            if (path is not null) {
                var stDir = Path.GetDirectoryName(path);
                if (stDir is not null) {
                    return GetStorageProvider().TryGetFolderFromPathAsync(stDir);
                }
            }
            return Task.FromResult<IStorageFolder?>(null);
        }

        /// <summary>
        /// Get the file pathname corresponding to the given IStorageFile
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        private static string? GetPathFromStorageFile(IStorageFile? file)
        {
            return (file is null) ? null : file.Path.LocalPath;
        }
    }
}
