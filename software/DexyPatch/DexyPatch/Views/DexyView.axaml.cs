using System;
using Avalonia.Controls;
using Avalonia.Threading;
using Dexy.DexyPatch.Utils;
using Dexy.DexyPatch.ViewModels;

namespace Dexy.DexyPatch.Views
{
    /// <summary>
    /// View for managing the Dexy device
    /// </summary>
    /// <remarks>
    /// Most of this class is to implement auto-scrolling of the serial text log.
    /// It's messy but it works.
    /// </remarks>
    public partial class DexyView : UserControl
    {
        public DexyView()
        {
            InitializeComponent();
            serialScroller.ScrollChanged += OnSerialScrollerScrollChanged;
        }

        protected override void OnDataContextChanged(EventArgs e)
        {
            base.OnDataContextChanged(e);
            if (DataContext is DexyViewModel viewModel) {
                viewModel.SerialTextLogChanging += OnSerialTextLogChanging;
            }
        }

        /// <summary>
        /// Is the <see cref="serialScroller"/> control currently scrolled to the very end?
        /// </summary>
        bool isserialScrollerAtEnd = false;

        /// <summary>
        /// Update <see cref="isserialScrollerAtEnd"/> when text is added to the serial text log
        /// </summary>
        private async void OnSerialTextLogChanging()
        {
            // serialScroller must be accessed on the UI thread
            isserialScrollerAtEnd =
                await Dispatcher.UIThread.InvokeAsync(
                    () => (serialScroller.Offset.Y
                        == (serialScroller.Extent.Height - serialScroller.Bounds.Height)),
                    DispatcherPriority.Send);
        }

        /// <summary>
        /// Keep the <see cref="serialScroller"/> control scrolled to the end,
        /// if that's where it was.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void OnSerialScrollerScrollChanged(object? sender, ScrollChangedEventArgs e)
        {
            if (isserialScrollerAtEnd) {
                isserialScrollerAtEnd = false;
                // serialScroller must be accessed on the UI thread
                await Dispatcher.UIThread.InvokeAsync(
                    () => serialScroller.ScrollToEnd(),
                    DispatcherPriority.Send);
            }
        }
    }
}
