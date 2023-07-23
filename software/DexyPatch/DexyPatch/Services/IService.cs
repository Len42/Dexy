using System.Web.Services.Description;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Base for service interfaces
    /// </summary>
    /// <remarks>
    /// A service should subclass this and add its own interface methods.
    /// Service initialization should be done in the service's constructor.
    /// </remarks>
    /// <seealso cref="Dexy.DexyPatch.Services.Service"/>
    /// <example>
    /// To define a new service:
    /// - Define the service's interface as a subclass of IService:
    /// <code>
    /// interface INewService : IService
    /// {
    ///     void Method1();
    ///     void Method2();
    /// }
    /// </code>
    /// - Implement the service in a class that implements the interface:
    /// <code>
    /// public class NewService : INewService
    /// {
    ///     public NewService() { service initialization in the constructor }
    ///     public void Method1() { service stuff }
    ///     public void Method2() { service stuff }
    ///     // Also implement IService.Terminate:
    ///     public void Terminate() { service shutdown stuff }
    /// }
    /// </code>
    /// - Register the service in <see cref="Dexy.DexyPatch.Services.ServiceRunner.InitializeAll"/>:
    /// <code>
    /// public static void InitializeAll()
    /// {
    ///     services.Add(Service&lt;IOtherService&gt;.Initialize&lt;OtherService&gt;());
    ///     services.Add(Service&lt;INewService&gt;.Initialize&lt;NewService&gt;());
    /// }
    /// </code>
    /// </example>
    public interface IService
    {
        /// <summary>
        /// Shut down the service
        /// </summary>
        /// <remarks>
        /// Called at app termination. Not part of the Service's client interface.
        /// </remarks>
        public void Terminate();
    }
}
