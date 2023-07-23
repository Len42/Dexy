using System;
using System.Threading.Tasks;
using Dexy.DexyPatch.Models;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Interface to load, save, and manage patch bank data
    /// </summary>
    /// <seealso cref="Dexy.DexyPatch.Services.DataManager"/>
    interface IDataManager : IService
    {
        /// <summary>
        /// Return a <see cref="Dexy.DexyPatch.Models.PatchBank"/> with a set of default patches
        /// </summary>
        /// <returns></returns>
        PatchBank LoadDefault();

        /// <summary>
        /// Load the <see cref="Dexy.DexyPatch.Models.PatchBank"/> to be displayed at app startup.
        /// Async.
        /// </summary>
        /// <returns>A tuple with:
        /// - a <see cref="Dexy.DexyPatch.Models.PatchBank"/>;
        /// - the <see cref="Dexy.DexyPatch.Services.PatchDataSource"/> from which the PatchBank was originally loaded;
        /// - an edited flag indicating whether the PatchBank has been modified from its original source.</returns>
        /// <remarks>
        /// Try to load the same patch bank that was being edited the last time the app was run.
        /// Load from either:
        /// - a saved working-data file, if editing was in progress;
        /// - a saved file;
        /// - the Dexy device.
        /// 
        /// If that fails, return a default patch bank.
        /// </remarks>
        Task<System.ValueTuple<PatchBank, PatchDataSource, bool>> LoadInitialAsync();

        /// <summary>
        /// Load a <see cref="Dexy.DexyPatch.Models.PatchBank"/> from a file
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        PatchBank LoadFile(string filename);

        /// <summary>
        /// Save a <see cref="Dexy.DexyPatch.Models.PatchBank"/> to a file
        /// </summary>
        /// <param name="patchBank"></param>
        /// <param name="filename"></param>
        void SaveFile(PatchBank patchBank, string filename);

        /// <summary>
        /// The source of the most-recently used patch bank
        /// </summary>
        /// <remarks>
        /// Should be set when a <see cref="Dexy.DexyPatch.Models.PatchBank"/> is loaded for editing
        /// </remarks>
        PatchDataSource LastUsed { get; set; }

        /// <summary>
        /// Save the <see cref="Dexy.DexyPatch.Models.PatchBank"/> currently being edited
        /// so that it will be reloaded the next time the app runs
        /// </summary>
        /// <param name="patchBank"></param>
        void SaveWorkingData(PatchBank? patchBank);
    }

    /// <summary>
    /// The various types of <see cref="Dexy.DexyPatch.Services.PatchDataSource"/>
    /// </summary>
    public enum DataSourceType
    {
        none,
        /// <summary>
        /// Patch bank was initialized with default patches
        /// </summary>
        init,
        /// <summary>
        /// Patch bank was uploaded from the Dexy module
        /// </summary>
        uploaded,
        /// <summary>
        /// Patch bank was loaded from a file
        /// </summary>
        file
    }

    /// <summary>
    /// The source from which a <see cref="Dexy.DexyPatch.Models.PatchBank"/> was loaded
    /// </summary>
    public class PatchDataSource
    {
        public DataSourceType Type { get; set; }

        public string? Pathname { get; set; }

        public PatchDataSource()
        {
            Type = DataSourceType.none;
            Pathname = null;
        }

        public PatchDataSource(DataSourceType type, string? pathname)
        {
            Type = type;
            Pathname = pathname;
        }
    }
}
