using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Dexy.DexyPatch.ViewModels;

namespace Dexy.DexyPatch.Views
{
    /// <summary>
    /// View to display and edit the settings of an operator within a patch
    /// </summary>
    /// <seealso cref="Dexy.DexyPatch.Models.OpParams"/>
    public partial class OperatorView : UserControl
    {
        public OperatorView()
        {
            InitializeComponent();
        }
    }
}
