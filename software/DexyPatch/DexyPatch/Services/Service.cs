using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Service wrapper
    /// </summary>
    /// <typeparam name="IServiceInterface">The service's interface, derived from <see cref="IService"/></typeparam>
    /// <seealso cref="Dexy.DexyPatch.Services.IService"/>
    public static class Service<IServiceInterface>
        where IServiceInterface : IService
    {
        /// <summary>
        /// The object that implements the service
        /// </summary>
        private static IServiceInterface? serviceInstance;

        /// <summary>
        /// Create and initialize the service
        /// </summary>
        /// <typeparam name="ServiceClass">The class that implements the service</typeparam>
        /// <exception cref="InvalidOperationException"></exception>
        public static IService Initialize<ServiceClass>()
            where ServiceClass : IServiceInterface, new()
        {
            if (serviceInstance != null) {
                throw new InvalidOperationException($"{typeof(IServiceInterface).Name} initialized twice");
            }
            serviceInstance = new ServiceClass();
            return serviceInstance;
        }

        /// <summary>
        /// Return an interface to the service
        /// </summary>
        /// <returns></returns>
        /// <exception cref="InvalidOperationException"></exception>
        public static IServiceInterface Get()
        {
            if (serviceInstance == null) {
                throw new InvalidOperationException($"{typeof(IServiceInterface).Name} called before initialized");
            } else {
                return serviceInstance;
            }
        }
    }
}
