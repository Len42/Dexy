using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using MessageBox.Avalonia;
using MessageBox.Avalonia.DTO;
using MessageBox.Avalonia.Enums;
using MessageBox.Avalonia.Models;
using Dexy.DexyPatch.Utils;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Service for displaying various message boxes and dialogs
    /// </summary>
    /// <remarks>
    /// Uses the MessageBox.Avalonia package
    /// </remarks>
    public class MessageBoxService : IMessageBoxService
    {
        public MessageBoxService() { }

        public void Terminate() { }

        public async Task ShowErrorMessage(string message, Exception? ex)
        {
            string? stError = null;
            if (ex != null) {
                stError = $"{ex.Message} ({ex.GetType().Name})";
            }
            var messageBox = MessageBoxManager.GetMessageBoxStandardWindow(
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
            await messageBox.ShowDialog(MainWindow);
        }

        public async Task<ButtonResult> AskYesNo(string header, string? message)
        {
            var messageBox = MessageBoxManager.GetMessageBoxStandardWindow(
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
            return await messageBox.ShowDialog(MainWindow);
        }

        public async Task<ButtonResult> AskSaveChanges(string? name)
        {
            const string btnYes = "Save";
            const string btnNo = "Discard Changes";
            const string btnCancel = "Cancel";
            var messageBox = MessageBoxManager.GetMessageBoxCustomWindow(
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
            string stResult = await messageBox.ShowDialog(MainWindow);
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
            var messageBox = MessageBoxManager.GetMessageBoxCustomWindow(
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
            string stResult = await messageBox.ShowDialog(MainWindow);
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
            var dialogBox = new OpenFileDialog() {
                Title = "Load File",
                Directory = Path.GetDirectoryName(initPathname),
                InitialFileName = Path.GetFileName(initPathname),
                AllowMultiple = false,
                Filters = GetFileFilterList(fileTypeName, fileTypeExt)
            };
            var result = await dialogBox.ShowAsync(MainWindow!);
            if (result == null) {
                return null;
            } else if (result.Length == 0) {
                return null;
            } else {
                return result[0];
            }
        }

        public async Task<string?> SaveFileDialog(string? initPathname, string fileTypeName, string fileTypeExt)
        {
            var dialogBox = new SaveFileDialog() {
                Title = "Save File",
                Directory = Path.GetDirectoryName(initPathname),
                InitialFileName = "New File", // not Path.GetFileName(initPathname),
                DefaultExtension ="dexy",
                Filters = GetFileFilterList(fileTypeName, fileTypeExt)
};
            return await dialogBox.ShowAsync(MainWindow!);
        }

        /// <summary>
        /// Get a list of file types, including the given file type, to display in the dialog
        /// </summary>
        /// <param name="fileTypeName"></param>
        /// <param name="fileTypeExt"></param>
        /// <returns></returns>
        private static List<FileDialogFilter> GetFileFilterList(string fileTypeName, string fileTypeExt)
        {
            return new() {
                new () { Name = fileTypeName, Extensions = new() { fileTypeExt } },
                new () { Name = "All files", Extensions = new() { "*" } }
            };
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
    }
}
