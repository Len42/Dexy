using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Load, save, and manage patch bank data
    /// </summary>
    /// <seealso cref="Dexy.DexyPatch.Services.IDataManager"/>
    public class DataManager : IDataManager
    {
        private readonly ISettingsManager settingsManager = Service<ISettingsManager>.Get();

        #region IDataManager interface

        /// <summary>
        /// On shutdown, save LastUsed in the configuration settings
        /// </summary>
        public void Terminate() { settingsManager.Save(LastUsed); }

        public PatchBank LoadDefault() { return PatchBank.MakeDefault(); }

        public async Task<System.ValueTuple<PatchBank, PatchDataSource, bool>> LoadInitialAsync()
        {
            PatchBank? patchBank = null;
            bool isModified = false;

            try {

                // Get the source of the data that was being edited last time.
                LastUsed = settingsManager.Load<PatchDataSource>();
                var workingData = settingsManager.Load<WorkingData>();

                // Try to load PatchBank data from either:
                // - a saved working-data file, if editing was in progress
                // - a saved file
                // - the Dexy device
                // It's not an error if this fails.
                if (!string.IsNullOrEmpty(workingData.Filename)) {
                    try {
                        patchBank = LoadFile(workingData.Filename);
                        isModified = true;
                        File.Delete(workingData.Filename); // just to be tidy
                    } catch { }
                } else if (LastUsed.Type == DataSourceType.uploaded) {
                    IDexyDevice dexy = Service<IDexyDevice>.Get();
                    if (dexy.IsConnected) {
                        try {
                            patchBank = await dexy.UploadPatchBankAsync();
                        } catch { }
                    }
                } else if (LastUsed.Type == DataSourceType.file) {
                    if (LastUsed.Pathname != null) {
                        try {
                            patchBank = LoadFile(LastUsed.Pathname);
                        } catch { }
                    }
                }
       
                // If nothing was loaded, initialize an empty PatchBank.
                if (patchBank == null) {
                    patchBank = LoadDefault();
                    LastUsed = new(DataSourceType.init, null);
                }
 
            } catch {
                // Last chance to return something reasonable - a "non-patchbank"
                patchBank = new();
                LastUsed = new(DataSourceType.none, null);
            }
    
            return (patchBank, LastUsed, isModified);
        }

        public PatchBank LoadFile(string filename)
        {
            PatchBank patchBank = new();
            using var stream = File.Open(filename, FileMode.Open, FileAccess.Read);
            ZppSerialize.Deserialize(stream, patchBank);
            return patchBank;
        }

        public void SaveFile(PatchBank patchBank, string filename)
        {
            using var stream = File.Open(filename, FileMode.Create, FileAccess.Write);
            ZppSerialize.Serialize(stream, patchBank);
        }

        public PatchDataSource LastUsed { get; set; } = new();

        /// <summary>
        /// Uniquely-named subclass of <see cref="Dexy.DexyPatch.Services.ISettingsManager.SavedFilenameBase"/>
        /// for use with <see cref="Dexy.DexyPatch.Services.SettingsManager"/>
        /// </summary>
        private class WorkingData : ISettingsManager.SavedFilenameBase { }

        public void SaveWorkingData(PatchBank? patchBank)
        {
            var saveFile = new WorkingData();
            if (patchBank == null) {
                saveFile.Filename = null;
            } else {
                // Save the PatchBank in a file in the settings directory
                saveFile.Filename = Path.Combine(settingsManager.SettingsDir, "savePatchBank");
                try {
                    SaveFile(patchBank, saveFile.Filename);
                } catch {
                    saveFile.Filename = null;
                }
            }
            // Record whether and where the PatchBank was saved
            settingsManager.Save(saveFile);
        }

        #endregion
    }
}
