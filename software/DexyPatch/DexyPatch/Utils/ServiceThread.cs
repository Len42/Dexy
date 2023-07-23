using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;

namespace Dexy.DexyPatch.Utils
{
    /// <summary>
    /// Base class for a background worker thread running in a service
    /// </summary>
    /// <remarks>
    /// Provides a <see cref="Dexy.DexyPatch.Utils.ServiceThread.Stop"/> method to kill the thread
    /// </remarks>
    /// <seealso cref="Dexy.DexyPatch.Services.DexyDevice"/>
    /// <seealso cref="Dexy.DexyPatch.Services.LiveUpdater"/>
    public abstract class ServiceThread
    {
        /// <summary>
        /// Create a <see cref="Dexy.DexyPatch.Utils.ServiceThread"/> but don't start it yet
        /// </summary>
        /// <param name="sleepMs">Idle time between thread loop iterations,
        /// in milliseconds. Must be > 0.</param>
        /// <exception cref="ArgumentOutOfRangeException"></exception>
        public ServiceThread(int sleepMs)
        {
            if (sleepMs < 1) {
                throw new ArgumentOutOfRangeException(nameof(sleepMs));
            }
            this.sleepMs = sleepMs;
        }

        /// <summary>
        /// Idle time between thread loop iterations
        /// </summary>
        private readonly int sleepMs;

        #region Abstract thread functions

        /// <summary>
        /// Thread initialization function called when the thread starts.
        /// Subclasses must implement.
        /// </summary>
        /// <param name="args"></param>
        protected abstract void ThreadInit(ServiceThreadArgs args);

        /// <summary>
        /// Thread work function called repeatedly while the thread runs.
        /// Subclasses must implement.
        /// </summary>
        /// <param name="args"></param>
        /// <remarks>
        /// Note that this is not the thread's main loop, just one iteration.
        /// ThreadWork must do something then return, not loop forever.
        /// </remarks>
        protected abstract void ThreadWork(ServiceThreadArgs args);

        /// <summary>
        /// Thread cleanup function called when the thread ends.
        /// Subclasses must implement.
        /// </summary>
        /// <param name="args"></param>
        protected abstract void ThreadEnd(ServiceThreadArgs args);

        #endregion

        #region Thread management

        /// <summary>
        /// Start the thread
        /// </summary>
        /// <param name="args"></param>
        /// <exception cref="InvalidOperationException"></exception>
        public void Start(ServiceThreadArgs args)
        {
            if (IsRunning) {
                throw new InvalidOperationException("ServiceThread was already started");
            }
            threadCancelSource = new();
            thread = new(ServiceThreadFunc) { IsBackground = true };
            args.cancelToken = threadCancelSource.Token;
            thread.Start(args);
        }

        /// <summary>
        /// Stop the thread
        /// </summary>
        public void Stop()
        {
            if (IsRunning) {
                // Thred-B-Gon
                threadCancelSource?.Cancel();
                thread?.Join(5000);
                thread = null;
                threadCancelSource?.Dispose();
                threadCancelSource = null;
            }
        }

        /// <summary>
        /// Is the thread running?
        /// </summary>
        public bool IsRunning { get => thread != null; }

        #endregion

        /// <summary>
        /// Base class for arguments passed to a <see cref="Dexy.DexyPatch.Utils.ServiceThread"/>
        /// </summary>
        /// <remarks>
        /// This class may be used as-is or subclassed to add more members for a
        /// particular ServiceThread subclass.
        /// </remarks>
        public class ServiceThreadArgs
        {
            /// <summary>
            /// Thread cancellation token
            /// </summary>
            public CancellationToken cancelToken;

            /// <summary>
            /// Check if thread cancellation has been requested
            /// </summary>
            public bool IsCancellationRequested { get => cancelToken.IsCancellationRequested; }
        }

        /// <summary>
        /// The main function for the worker thread.
        /// Executes the subclass work function repeatedly while checking for thread cancellation.
        /// </summary>
        /// <param name="oParams">The <see cref="Dexy.DexyPatch.Utils.ServiceThread.ServiceThreadArgs"/> that was passed to
        /// <see cref="Dexy.DexyPatch.Utils.ServiceThread.Start"/></param>
        /// <exception cref="ArgumentNullException"></exception>
        private void ServiceThreadFunc(object? oParams)
        {
            if (oParams is null) {
                throw new ArgumentNullException(nameof(oParams));
            }
            ServiceThreadArgs args = (ServiceThreadArgs)oParams;
            ThreadInit(args);
            while (!args.IsCancellationRequested) {
                ThreadWork(args);
                Thread.Sleep(sleepMs);
            }
            ThreadEnd(args);
        }

        /// <summary>
        /// The actual background thread
        /// </summary>
        private Thread? thread;
        
        /// <summary>
        /// Used to signal <see cref="thread"/> to stop
        /// </summary>
        private CancellationTokenSource? threadCancelSource;
    }
}
