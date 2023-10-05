using System;
using Avalonia;
using Avalonia.Controls.Shapes;
using Avalonia.Media;
using Dexy.DexyPatch.Models;

namespace Dexy.DexyPatch.Controls
{
    /// <summary>
    /// Visual representation of an envelope
    /// </summary>
    public class EnvelopePic : Shape
    {
        static EnvelopePic()
        {
            AffectsGeometry<EnvelopePic>(
                DelayProperty,
                AttackProperty,
                DecayProperty,
                SustainProperty,
                ReleaseProperty,
                LoopProperty,
                BoundsProperty,
                StrokeThicknessProperty);
        }

        public double Delay
        {
            get => GetValue(DelayProperty);
            set => SetValue(DelayProperty, value);
        }
        public static readonly StyledProperty<double> DelayProperty =
          AvaloniaProperty.Register<EnvelopePic, double>(nameof(Delay));

        public double Attack
        {
            get => GetValue(AttackProperty);
            set => SetValue(AttackProperty, value);
        }
        public static readonly StyledProperty<double> AttackProperty =
            AvaloniaProperty.Register<EnvelopePic, double>(nameof(Attack));

        public double Decay
        {
            get => GetValue(DecayProperty);
            set => SetValue(DecayProperty, value);
        }
        public static readonly StyledProperty<double> DecayProperty =
          AvaloniaProperty.Register<EnvelopePic, double>(nameof(Decay));

        public double Sustain
        {
            get => GetValue(SustainProperty);
            set => SetValue(SustainProperty, value);
        }
        public static readonly StyledProperty<double> SustainProperty =
          AvaloniaProperty.Register<EnvelopePic, double>(nameof(Sustain));

        public double Release
        {
            get => GetValue(ReleaseProperty);
            set => SetValue(ReleaseProperty, value);
        }
        public static readonly StyledProperty<double> ReleaseProperty =
          AvaloniaProperty.Register<EnvelopePic, double>(nameof(Release));

        public bool Loop
        {
            get => GetValue(LoopProperty);
            set => SetValue(LoopProperty, value);
        }
        public static readonly StyledProperty<bool> LoopProperty =
          AvaloniaProperty.Register<EnvelopePic, bool>(nameof(Loop));

        protected override Geometry CreateDefiningGeometry()
        {
            // Envelope parameter to time conversion matches the firmware
            double ParamToTime(double param) { return 1 / (Math.Exp(param * 0.01) * 12.0243 - 11); }

            double delay = (Delay == 0) ? 0 : ParamToTime(DexyDefs.max_param_t - Delay);
            double attack = ParamToTime(Attack) * 2.0 / 3.0; // Attack is faster by design
            double sustainLevel = Sustain;
            double decay = ParamToTime(Decay + sustainLevel);
            double release = ParamToTime(Release + DexyDefs.max_param_t - sustainLevel);
            double totalTime = delay + attack + decay + release;
            double sustainTime = Loop ? 0 : totalTime / 3;
            totalTime += sustainTime;

            var rect = new Rect(Bounds.Size).Deflate(StrokeThickness / 2);

            double SegmentWidth(double segmentTime) { return segmentTime / totalTime * rect.Width; }

            var geometry = new StreamGeometry();
            using (StreamGeometryContext context = geometry.Open()) {
                context.BeginFigure(new Point(rect.Left, rect.Bottom), true);
                double x = rect.Left;
                double xNext = x + SegmentWidth(delay);
                context.LineTo(new Point(xNext, rect.Bottom));
                x = xNext;
                xNext = x + SegmentWidth(attack);
                context.LineTo(new Point(xNext, rect.Top));
                x = xNext;
                xNext = x + SegmentWidth(decay);
                double ySustain = rect.Bottom - (double)sustainLevel / (double)DexyDefs.max_param_t * rect.Height;
                context.LineTo(new Point(xNext, ySustain));
                x = xNext;
                if (!Loop) {
                    xNext = x + SegmentWidth(sustainTime);
                    context.LineTo(new Point(xNext, ySustain));
                    x = xNext;
                }
                context.LineTo(new Point(rect.Right, rect.Bottom));
            }

            return geometry;
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            return new Size(StrokeThickness, StrokeThickness);
        }
    }
}
