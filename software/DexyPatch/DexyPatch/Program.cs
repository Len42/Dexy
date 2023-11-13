using System;
using Avalonia;
using Dexy.DexyPatch.Utils;

namespace Dexy.DexyPatch
{
    internal class Program
    {
        /// <summary>
        /// Main entry point
        /// </summary>
        /// <param name="args"></param>
        /// <remarks>
        /// Initialization code. Don't use any Avalonia, third-party APIs or any
        /// SynchronizationContext-reliant code before AppMain is called: things aren't initialized
        /// yet and stuff might break.
        /// </remarks>
        [STAThread]
        public static void Main(string[] args)
        {
            try {
                BuildAvaloniaApp().StartWithClassicDesktopLifetime(args);
            } catch(Exception ex) {
                // last-ditch exception handler - log to file
                ErrorLog.TraceError(ex);
                ErrorLog.WriteError(ex);
                throw;
            }
        }

        /// <summary>
        /// Avalonia configuration, don't remove; also used by visual designer.
        /// </summary>
        /// <returns></returns>
        public static AppBuilder BuildAvaloniaApp()
            => AppBuilder.Configure<App>()
                .UsePlatformDetect()
                .LogToTrace();
    }
}