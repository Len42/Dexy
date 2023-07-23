using System;
using System.ComponentModel;
using Avalonia.Controls;
using Dexy.DexyPatch.ViewModels;

namespace Dexy.DexyPatch.Views
{
    /// <summary>
    /// View for the main application window
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            // Always open the window maximized because it really needs the whole screen.
            WindowState = WindowState.Maximized;
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            // Before closing the app, save the data currently being edited.
            // Alternatively, one might want to prompt the user to save the
            // patchbank being edited, but I cannot make that work - apparently
            // it's not possible to display a dialog during app shutdown. :(
            if (DataContext is MainWindowViewModel viewModel) {
                viewModel.SaveWorkingData();
            }
            base.OnClosing(e);
        }

        /// <summary>
        /// This seems like a good place to catch final app termination?
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        public void OnClosed(object sender, EventArgs e)
        {
            App.TerminateServices();
        }
    }
}