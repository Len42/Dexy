using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Interface for saving and loading app configuration settings
    /// </summary>
    /// <remarks>
    /// To save/load an object of a class that contains some related settings, call 
    /// the Save/Load method with the class as a type parameter. The data is saved
    /// in a file named after the class that is being saved.
    /// Settings files are saved in a subdirectory in the user's AppData\\Local directory.
    /// </remarks>
    public interface ISettingsManager : IService
    {
        /// <summary>
        /// The directory where settings files are stored
        /// </summary>
        /// <remarks>
        /// SettingsDir is a subdirectory in the user's AppData\\Local directory.
        /// </remarks>
        string SettingsDir { get; }

        /// <summary>
        /// Load the saved settings of the given class type
        /// </summary>
        /// <typeparam name="T">A class that contains some settings</typeparam>
        /// <returns>The settings loaded from a config file</returns>
        T Load<T>() where T : class?, new();

        /// <summary>
        /// Save an object of the given class type that contains some settings
        /// </summary>
        /// <typeparam name="T">A class that contains some settings</typeparam>
        /// <param name="settings">The settings to be saved</param>
        void Save<T>(T settings) where T : class?;

        /// <summary>
        /// Convenience base class to contain a single setting which is a filename
        /// </summary>
        /// <remarks>
        /// To save a filename setting (e.g. the last edited file), create an empty
        /// subclass of this class and use it to call
        /// <see cref="Dexy.DexyPatch.Services.ISettingsManager.Save{T}(T)"/>
        /// and <see cref="Dexy.DexyPatch.Services.ISettingsManager.Load{T}"/>.
        /// </remarks>
        public abstract class SavedFilenameBase
        {
            public string? Filename { get; set; }
        }
    }
}
