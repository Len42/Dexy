using System;
using Avalonia.Controls;
using Dexy.DexyPatch.ViewModels;

namespace Dexy.DexyPatch.Views
{
    /// <summary>
    /// View to display and edit a <see cref="Dexy.DexyPatch.Models.PatchBank"/>
    /// </summary>
    public partial class PatchBankView : UserControl
    {
        public PatchBankView()
        {
            InitializeComponent();
        }

        protected override void OnDataContextChanged(EventArgs e)
        {
            base.OnDataContextChanged(e);
            // The ViewModel needs a pointer to this
            if (DataContext is PatchBankViewModel viewModel) {
                viewModel.View = this;
            }
        }

        /// <summary>
        /// Redraw the patch listbox after a change
        /// </summary>
        public void PatchListUpdated()
        {
            // BUG: InvalidateVisual() does nothing!
            // Scrolling the ListBox does make it display the new patch name.
            patchListBox.InvalidateVisual();
        }
    }
}
