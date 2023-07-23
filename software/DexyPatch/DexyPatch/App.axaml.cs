using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Dexy.DexyPatch.Services;
using Dexy.DexyPatch.ViewModels;
using Dexy.DexyPatch.Views;

namespace Dexy.DexyPatch
{
    /// <summary>
    /// The application shell
    /// </summary>
    public partial class App : Application
    {
        public App()
        {
            Name = ProjectVersion.AppName;
        }

        public override void Initialize()
        {
            AvaloniaXamlLoader.Load(this);
        }

        public override void OnFrameworkInitializationCompleted()
        {
            if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop) {
                desktop.MainWindow = new MainWindow {
                    DataContext = new MainWindowViewModel(),
                };
            }

            base.OnFrameworkInitializationCompleted();
        }

        /// <summary>
        /// Initialize this app's services as well as Avalonia's services
        /// </summary>
        public override void RegisterServices()
        {
            base.RegisterServices();
            ServiceRunner.InitializeAll();
        }

        /// <summary>
        /// Shut down this app's services
        /// </summary>
        /// <remarks>
        /// This is called from... somewhere. There isn't a built-in hook that
        /// corresponds to <see cref="Application.RegisterServices"/>.
        /// </remarks>
        public static void TerminateServices()
        {
            ServiceRunner.TerminateAll();
        }
    }
}
