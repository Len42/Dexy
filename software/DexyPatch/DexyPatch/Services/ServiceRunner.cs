using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Start and stop all of the services
    /// </summary>
    public static class ServiceRunner
    {
        /// <summary>
        /// Initialize all of the services.
        /// </summary>
        /// <remarks>
        /// This must be called during app startup.
        /// Add new services here. Make sure they are initialized in order so that
        /// earlier ones don't depend on later ones.
        /// </remarks>
        public static void InitializeAll()
        {
            services.Add(Service<ISettingsManager>.Initialize<SettingsManager>());
            services.Add(Service<IDexyDevice>.Initialize<DexyDevice>());
            services.Add(Service<IDataManager>.Initialize<DataManager>());
            services.Add(Service<IDialogService>.Initialize<DialogService>());
            services.Add(Service<ILiveUpdater>.Initialize<LiveUpdater>());
        }

        /// <summary>
        /// Terminate all of the services
        /// </summary>
        /// <remarks>
        /// This must be called during app shutdown.
        /// All the services in <see cref="InitializeAll"/> are stopped in the reverse
        /// order that they were started.
        /// </remarks>
        public static void TerminateAll()
        {
            foreach(var service in services.Reverse<IService>()) {
                // Catch exceptions so they don't disrupt app shutdown
                try { service.Terminate(); } catch { }
            }
            services = new();
        }

        /// <summary>
        /// List of running services
        /// </summary>
        private static List<IService> services = new();
    }
}
