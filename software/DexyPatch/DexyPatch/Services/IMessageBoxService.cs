using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MessageBox.Avalonia.Enums;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Interface for displaying various message boxes and dialogs
    /// </summary>
    interface IMessageBoxService : IService
    {
        /// <summary>
        /// Display an error message box
        /// </summary>
        /// <param name="message">Error message</param>
        /// <param name="ex">(optional) Exception - if given, ex.Message will be displayed as well</param>
        Task ShowErrorMessage(string message, Exception? ex);

        /// <summary>
        /// Display a message box with "Yes" and "No" buttons
        /// </summary>
        /// <param name="header">A brief yes/no question</param>
        /// <param name="message">(optional) A longer, more detailed message</param>
        /// <returns>ButtonResult.Yes, ButtonResult.No, or ButtonResult.None</returns>
        Task<ButtonResult> AskYesNo(string header, string? message);

        /// <summary>
        /// Ask whether to save the patchbank with the given name
        /// </summary>
        /// <param name="name"></param>
        /// <returns>ButtonResult.Yes, ButtonResult.No, ButtonResult.Cancel, or ButtonResult.None</returns>
        Task<ButtonResult> AskSaveChanges(string? name);

        /// <summary>
        /// Ask whether to download a firmware file to the Dexy module
        /// </summary>
        /// <param name="pathname"></param>
        /// <returns>
        /// ButtonResult.Yes: Download firmware;
        /// ButtonResult.No: Select a different firmware file, then prompt again;
        /// ButtonResult.Cancel or ButtonResult.None: Cancel
        /// </returns>
        Task<ButtonResult> AskDownloadFirmware(string pathname);

        /// <summary>
        /// Display a dialog to select a file to be opened
        /// </summary>
        /// <param name="initPathname">Pathname to use to initialize the dialog</param>
        /// <param name="fileTypeName">Preferred file type</param>
        /// <param name="fileTypeExt">Extension of the preferred file type</param>
        /// <returns>The selected file pathname, or null if cancelled</returns>
        Task<string?> OpenFileDialog(string? initPathname, string fileTypeName, string fileTypeExt);

        /// <summary>
        /// Display a dialog to select a file to save to
        /// </summary>
        /// <param name="initPathname">Pathname to use to initialize the dialog</param>
        /// <param name="fileTypeName">Preferred file type</param>
        /// <param name="fileTypeExt">Extension of the preferred file type</param>
        /// <returns>The selected file pathname, or null if cancelled</returns>
        Task<string?> SaveFileDialog(string? initPathname, string fileTypeName, string fileTypeExt);
    }
}
