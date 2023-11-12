using System;
using Avalonia.Controls;
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
            serialTextLogScroller.ScrollChanged += OnSerialTextLogScrollerScrollChanged;
        }

        protected override void OnDataContextChanged(EventArgs e)
        {
            base.OnDataContextChanged(e);
            if (DataContext is DexyViewModel viewModel) {
                viewModel.SerialTextLogChanging += OnSerialTextLogChanging;
            }
        }

        /// <summary>
        /// Is the <see cref="serialTextLogScroller"/> control currently scrolled to the very end?
        /// </summary>
        bool isSerialTextLogScrollerAtEnd = false;

        /// <summary>
        /// Update <see cref="isSerialTextLogScrollerAtEnd"/> when text is added to the serial text log
        /// </summary>
        private void OnSerialTextLogChanging()
        {
            // TODO: FUBAR
            /*
            isSerialTextLogScrollerAtEnd =
                (serialTextLogScroller.Offset.Y
                    == (serialTextLogScroller.Extent.Height - serialTextLogScroller.Bounds.Height));
            */
        }

        /// <summary>
        /// Keep the <see cref="serialTextLogScroller"/> control scrolled to the end,
        /// if that's where it was.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnSerialTextLogScrollerScrollChanged(object? sender, ScrollChangedEventArgs e)
        {
            if (isSerialTextLogScrollerAtEnd) {
                isSerialTextLogScrollerAtEnd = false;
                serialTextLogScroller.ScrollToEnd();
            }
        }
    }
}
