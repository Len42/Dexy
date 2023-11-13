using Avalonia.Controls;
using Avalonia.Media;
using Dexy.DexyPatch.Utils;

namespace Dexy.DexyPatch.Views
{
    /// <summary>
    /// View to display and edit the comm port settings for the Dexy device
    /// </summary>
    public partial class DexySettingsView : UserControl
    {
        public DexySettingsView()
        {
            InitializeComponent();
            // Fill the comm port listbox
            portListbox.ItemsSource = CommPort.GetPortNames();
        }
    }
}
