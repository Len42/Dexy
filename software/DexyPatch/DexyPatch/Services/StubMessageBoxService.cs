using Dexy.DexyPatch.Utils;
using MessageBox.Avalonia.Enums;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Stub implementation of IMessageBoxService
    /// </summary>
    public class StubMessageBoxService : IMessageBoxService
    {
        public void Terminate() { }

        public Task ShowErrorMessage(string message, Exception? ex)
        {
            ErrorLog.TraceError(message);
            if (ex != null) {
                ErrorLog.TraceError(ex);
            }
            return Task.CompletedTask;
        }

        public Task<ButtonResult> AskYesNo(string header, string? message)
        {
            // TODO: Need a way to return either yes or no based on a default param or something?
            return Task.FromResult<ButtonResult>(ButtonResult.No);
        }

        public Task<ButtonResult> AskSaveChanges(string? name)
        {
            // TODO: Need a way to return either yes or no based on a default param or something?
            return Task.FromResult<ButtonResult>(ButtonResult.No);
        }

        public Task<ButtonResult> AskDownloadFirmware(string pathname)
        {
            // TODO: Need a way to return either yes or no based on a default param or something?
            return Task.FromResult<ButtonResult>(ButtonResult.No);
        }

        public Task<string?> OpenFileDialog(string? initPathname, string fileTypeName, string fileTypeExt)
        {
            return Task.FromResult<string?>(null);
        }

        public Task<string?> SaveFileDialog(string? initPathname, string fileTypeName, string fileTypeExt)
        {
            return Task.FromResult<string?>(null);
        }
    }
}
