using System;
using Avalonia.Controls;
using Avalonia.Styling;

namespace Dexy.DexyPatch.Controls
{
    /// <summary>
    /// Just a subclass of Slider with different default settings
    /// </summary>
    public class ParamSlider : Slider
    {
        public ParamSlider()
        {
            Minimum = 0;
            Maximum = 1023;
            TickFrequency = 256;
            TickPlacement = TickPlacement.Outside;
            Orientation = Avalonia.Layout.Orientation.Vertical;
            Height = 128;
            HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Center;
        }

        protected override Type StyleKeyOverride => typeof(Slider);
    }
}
