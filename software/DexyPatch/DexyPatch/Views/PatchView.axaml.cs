using System;
using System.Collections.Generic;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.ViewModels;

namespace Dexy.DexyPatch.Views
{
    /// <summary>
    /// View to display and edit a <see cref="Dexy.DexyPatch.Models.Patch"/>
    /// </summary>
    public partial class PatchView : UserControl
    {
        public PatchView()
        {
            InitializeComponent();
        }

        /// <summary>
        /// This View's ViewModel
        /// </summary>
        private PatchViewModel? viewModel;

        protected override void OnDataContextChanged(EventArgs e)
        {
            base.OnDataContextChanged(e);
            if (DataContext is PatchViewModel viewModel) {
                this.viewModel = viewModel;
            }
        }

        protected override void OnInitialized()
        {
            base.OnInitialized();

            // Initialize the list of icons used in the Algorithm dropdown
            algorithm.ItemsSource = GetAlgorithmIconList();

            // Must initialize the Algorithm list selection here, otherwise it
            // doesn't get picked up.
            algorithm.SelectedIndex = viewModel?.Algorithm ?? 0;
        }

        /// <summary>
        /// List of algorithm icons displayed in the <see cref="algorithm"/> dropdown
        /// </summary>
        private static List<Bitmap>? algorithmIcons = null;

        /// <summary>
        /// Make a list of algorithm icons
        /// </summary>
        /// <returns></returns>
        private static List<Bitmap> GetAlgorithmIconList()
        {
            if (algorithmIcons == null) {
                algorithmIcons = new List<Bitmap>();
        ////        var assets = AvaloniaLocator.Current.GetService<IAssetLoader>();
                for (uint i = 0; i < DexyDefs.numAlgorithms; ++i) {
                    string resourcePath = $"avares://DexyPatch/Assets/algorithms/algo-{i+1}.png";
                    Uri uri = new(resourcePath);
                    algorithmIcons.Add(new(AssetLoader.Open(uri)));
                }
            }
            return algorithmIcons;
        }
    }
}
