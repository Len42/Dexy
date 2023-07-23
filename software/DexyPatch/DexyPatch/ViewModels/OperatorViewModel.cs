using System;
using System.Diagnostics;
using Avalonia;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using CommunityToolkit.Mvvm.ComponentModel;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils;
using Dexy.DexyPatch.Utils.PatchChanges;

namespace Dexy.DexyPatch.ViewModels
{
    /// <summary>
    /// ViewModel for an operator within a patch
    /// </summary>
    /// <seealso cref="Dexy.DexyPatch.Models.OpParams"/>
    public partial class OperatorViewModel : ViewModelBase
    {
        public OperatorViewModel(PatchViewModel parentVM, int opNum, OpParams opParams)
        {
            this.parentVM = parentVM;
            this.opNum = opNum;
            this.opParams = opParams;
            InitializeFromModel();
        }

        /// <summary>
        /// ViewModel of the parent (container) view associated with this ViewModel
        /// </summary>
        private readonly PatchViewModel parentVM;

        #region Model data

        /// <summary>
        /// Index of the operator, 0 to 5
        /// </summary>
        private readonly int opNum;

        /// <summary>
        /// The operator settings from the <see cref="Dexy.DexyPatch.Models.Patch"/>
        /// </summary>
        private readonly OpParams opParams;

        /// <summary>
        /// What this operator does in the patch's algorithm
        /// </summary>
        /// <seealso cref="Dexy.DexyPatch.Models.Algorithm"/>
        private AlgoOp Algo { get => Algorithm.algorithms[parentVM.Algorithm].ops[opNum]; }

        #endregion

        #region ViewModel data

        /// <summary>
        /// Propagate the data-modified flag to the parent VM and set the changed data for live-updating
        /// </summary>
        private void SetDataModified(OpParamSetting field, object value) => parentVM.SetDataModified(opNum, field, value);

        /// <summary>
        /// Flag to ignore changes to data members during initialization
        /// </summary>
        private bool ignoreOnPropertyChanged = false;

        /// <summary>
        /// Initialize data members from the <see cref="Dexy.DexyPatch.Models.OpParams"/>
        /// </summary>
        private void InitializeFromModel()
        {
            // Ignore update notifications during this operation
            ignoreOnPropertyChanged = true;
            IsFixedFreq = opParams.fixedFreq;
            SetNoteOrFreq(); // opParams.noteOrFreq
            IsMuted = (opParams.outputLevel == 0);
            OutputLevel = opParams.outputLevel;
            ModSens = opParams.ampModSens;
            UseEnvelope = opParams.useEnvelope;
            Delay = opParams.env.delay;
            Attack = opParams.env.attack;
            Decay = opParams.env.decay;
            Sustain = opParams.env.sustain;
            Release = opParams.env.release;
            LoopEnvelope = opParams.env.loop;
            ignoreOnPropertyChanged = false;
        }

        /// <summary>
        /// Set the ViewModel members to indicate the frequency specified in the
        /// <see cref="Dexy.DexyPatch.Models.OpParams"/>
        /// </summary>
        private void SetNoteOrFreq()
        {
            if (IsFixedFreq) {
                int n = opParams.noteOrFreq + 32767;
                FixedCoarse = n / 512;
                FixedFine = (n % 512) / 2 - 128;
                RatioCoarse = 0;
                RatioFine = 0;
            } else {
                // This looks complicated but it gives good ranges for the sliders
                int x = opParams.noteOrFreq - 2048;
                RatioCoarse = (x + 1024) / 2048;
                int y = x - RatioCoarse * 2048;
                RatioFine = (y + 8) / 16;
                if (RatioFine > 63) {
                    RatioFine -= 128;
                    ++RatioCoarse;
                    y -= 2048;
                }
                RatioDetune = y - RatioFine * 16;
                if (RatioDetune < -8) {
                    RatioDetune += 16;
                    --RatioFine;
                }
                FixedCoarse = 0;
                FixedFine = 0;
            }
        }

        /// <summary>
        /// Load an icon that shows the operator number and whether it's an output or a modulator
        /// </summary>
        /// <remarks>
        /// In the UI, the operators are presented in reverse order and numbered from 1,
        /// just because that's how you-know-who did it.
        /// </remarks>
        public IImage OpNumIcon
        {
            get
            {
                bool isOutput = Algo.isOutput;
                string suffix = isOutput ? "out" : "mod";
                string resourcePath = $"avares://DexyPatch/Assets/op-nums/{opNum}-{suffix}.png";
                Uri uri = new(resourcePath);
                var assets = AvaloniaLocator.Current.GetService<IAssetLoader>();
                return new Bitmap(assets?.Open(uri));
            }
        }

        /// <summary>
        /// Is this operator's frequency fixed or variable (multiple of the fundamental)?
        /// </summary>
        [ObservableProperty]
        private bool isFixedFreq;

        /// <summary>
        /// Update operator data and ViewModel members when <see cref="isFixedFreq"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnIsFixedFreqChanged(bool value)
        {
            if (!ignoreOnPropertyChanged) {
                opParams.fixedFreq = value;
                SetNoteOrFreq();
                SetDataModified(OpParamSetting.fixedFreq, value);
            }
        }

        /// <summary>
        /// Operator fixed frequency coarse control
        /// </summary>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(FixedFreqString))]
        private int fixedCoarse = 0;

        /// <summary>
        /// Update operator data when <see cref="fixedCoarse"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnFixedCoarseChanged(int value)
        {
            if (!ignoreOnPropertyChanged) {
                UpdateFixedFreq(value, FixedFine);
            }
        }

        /// <summary>
        /// Operator fixed frequency fine control
        /// </summary>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(FixedFreqString))]
        private int fixedFine = 0;

        /// <summary>
        /// Update operator data when <see cref="fixedFine"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnFixedFineChanged(int value)
        {
            if (!ignoreOnPropertyChanged) {
                UpdateFixedFreq(FixedCoarse, value);
            }
        }

        /// <summary>
        /// Update the operator's fixed frequency according to the selected coarse and fine values
        /// </summary>
        /// <param name="coarse"></param>
        /// <param name="fine"></param>
        private void UpdateFixedFreq(int coarse, int fine)
        {
            opParams.noteOrFreq = (ushort)(512 * coarse - 32511 + 2 * fine);
            SetDataModified(OpParamSetting.noteOrFreq, opParams.noteOrFreq);
        }

        /// <summary>
        /// Return a string displaying the operator's fixed frequency
        /// </summary>
        public string FixedFreqString
        {
            get
            {
                // Fixed frequency: Display the frequency and note name,
                // but omit the note name if it has a minus sign because
                // no-one wants that.
                Int16 note = (Int16)opParams.noteOrFreq;
                if (note >= 12 * DexyDefs.midiNoteSemitone) {
                    return string.Format("{0} ({1})",
                        MidiNotes.NoteToHzString(note),
                        MidiNotes.NoteToString(note));
                } else {
                    return string.Format("{0}",
                        MidiNotes.NoteToHzString(note));
                }
            }
        }

        /// <summary>
        /// Operator variable frequency coarse control
        /// </summary>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(RatioString))]
        private int ratioCoarse = 0;

        /// <summary>
        /// Update operator data when <see cref="ratioCoarse"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnRatioCoarseChanged(int value)
        {
            if (!ignoreOnPropertyChanged) {
                UpdateRatio(value, RatioFine, RatioDetune);
            }
        }

        /// <summary>
        /// Operator variable frequency fine control
        /// </summary>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(RatioString))]
        private int ratioFine = 0;

        /// <summary>
        /// Update operator data when <see cref="ratioFine"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnRatioFineChanged(int value)
        {
            if (!ignoreOnPropertyChanged) {
                UpdateRatio(RatioCoarse, value, RatioDetune);
            }
        }

        /// <summary>
        /// Operator variable frequency detune control
        /// </summary>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(RatioString))]
        private int ratioDetune = 0;

        /// <summary>
        /// Update operator data when <see cref="ratioDetune"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnRatioDetuneChanged(int value)
        {
            if (!ignoreOnPropertyChanged) {
                UpdateRatio(RatioCoarse, RatioFine, value);
            }
        }

        /// <summary>
        /// Update the operator's variable frequency according to the selected
        /// coarse, fine, and detune values
        /// </summary>
        /// <param name="coarse"></param>
        /// <param name="fine"></param>
        /// <param name="detune"></param>
        private void UpdateRatio(int coarse, int fine, int detune)
        {
            opParams.noteOrFreq =
                (ushort)Math.Min(2048 * coarse + 16 * fine + detune + 2048, 65535);
            SetDataModified(OpParamSetting.noteOrFreq, opParams.noteOrFreq);
        }

        /// <summary>
        /// Return a string displaying the operator's variable frequency
        /// </summary>
        public string RatioString
        {
            get
            {
                // Numeric frequency ratio is shown as a decimal number for
                // the coarse/fine settings +/- the detune setting.
                return string.Format("{0:0.##}{1: +#; -#;''}",
                    (double)((opParams.noteOrFreq + 8) & 0xFFF0) / DexyDefs.freqRatio1,
                    RatioDetune);
            }
        }

        /// <summary>
        /// The operator's overall output level
        /// </summary>
        [ObservableProperty]
        private ushort outputLevel;

        /// <summary>
        /// Update operator data when <see cref="outputLevel"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnOutputLevelChanged(ushort value)
        {
            if (!ignoreOnPropertyChanged) {
                opParams.outputLevel = value;
                SetDataModified(OpParamSetting.outputLevel, value);
            }
        }

        /// <summary>
        /// Is this operator muted?
        /// </summary>
        /// <remarks>
        /// IsMuted is equivalent to outputLevel == 0.
        /// </remarks>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(IsEnvEnabled))]
        [NotifyPropertyChangedFor(nameof(OutputTextColour))]
        [NotifyPropertyChangedFor(nameof(EnvTextColour))]
        [NotifyPropertyChangedFor(nameof(EnvStrokeColour))]
        private bool isMuted;

        private ushort outputLevelSave = 0;

        /// <summary>
        /// Update operator data when <see cref="isMuted"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnIsMutedChanged(bool value)
        {
            if (!ignoreOnPropertyChanged) {
                if (value) {
                    outputLevelSave = opParams.outputLevel;
                    opParams.outputLevel = 0;
                } else {
                    opParams.outputLevel = outputLevelSave;
                }
                SetDataModified(OpParamSetting.outputLevel, opParams.outputLevel);
            }
        }

        /// <summary>
        /// Brush used for the envelope text labels
        /// </summary>
        /// <seealso cref="Dexy.DexyPatch.Controls.EnvelopePic"/>
        public IBrush OutputTextColour { get => IsMuted ? TextDisabledColour : TextEnabledColour; }

        /// <summary>
        /// The operator's modulation sensitivity
        /// </summary>
        [ObservableProperty]
        private ushort modSens;

        /// <summary>
        /// Update operator data when <see cref="outputLevel"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnModSensChanged(ushort value)
        {
            if (!ignoreOnPropertyChanged) {
                opParams.ampModSens = value;
                SetDataModified(OpParamSetting.ampModSens, value);
            }
        }

        /// <summary>
        /// Is this operator's output level modulated by the envelope?
        /// </summary>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(IsEnvEnabled))]
        [NotifyPropertyChangedFor(nameof(EnvTextColour))]
        [NotifyPropertyChangedFor(nameof(EnvStrokeColour))]
        private bool useEnvelope;

        /// <summary>
        /// Update operator data when <see cref="useEnvelope"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnUseEnvelopeChanged(bool value)
        {
            if (!ignoreOnPropertyChanged) {
                opParams.useEnvelope = value;
                SetDataModified(OpParamSetting.useEnvelope, value);
            }
        }

        /// <summary>
        /// The operator envelope's delay value
        /// </summary>
        [ObservableProperty]
        private ushort delay;

        /// <summary>
        /// Update operator data when <see cref="delay"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnDelayChanged(ushort value)
        {
            if (!ignoreOnPropertyChanged) {
                opParams.env.delay = value;
                SetDataModified(OpParamSetting.envDelay, value);
            }
        }

        /// <summary>
        /// The operator envelope's attack value
        /// </summary>
        [ObservableProperty]
        private ushort attack;

        /// <summary>
        /// Update operator data when <see cref="attack"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnAttackChanged(ushort value)
        {
            if (!ignoreOnPropertyChanged) {
                opParams.env.attack = value;
                SetDataModified(OpParamSetting.envAttack, value);
            }
        }

        /// <summary>
        /// The operator envelope's decay value
        /// </summary>
        [ObservableProperty]
        private ushort decay;

        /// <summary>
        /// Update operator data when <see cref="decay"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnDecayChanged(ushort value)
        {
            if (!ignoreOnPropertyChanged) {
                opParams.env.decay = value;
                SetDataModified(OpParamSetting.envDecay, value);
            }
        }

        /// <summary>
        /// The operator envelope's sustain value
        /// </summary>
        [ObservableProperty]
        private ushort sustain;

        /// <summary>
        /// Update operator data when <see cref="sustain"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnSustainChanged(ushort value)
        {
            if (!ignoreOnPropertyChanged) {
                opParams.env.sustain = value;
                SetDataModified(OpParamSetting.envSustain, value);
            }
        }

        /// <summary>
        /// The operator envelope's release value
        /// </summary>
        [ObservableProperty]
        private ushort release;

        /// <summary>
        /// Update operator data when <see cref="release"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnReleaseChanged(ushort value)
        {
            if (!ignoreOnPropertyChanged) {
                opParams.env.release = value;
                SetDataModified(OpParamSetting.envRelease, value);
            }
        }

        /// <summary>
        /// Should the operator's envelope loop indefinitely?
        /// </summary>
        [ObservableProperty]
        private bool loopEnvelope;

        /// <summary>
        /// Update operator data when <see cref="loopEnvelope"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnLoopEnvelopeChanged(bool value)
        {
            if (!ignoreOnPropertyChanged) {
                opParams.env.loop = value;
                SetDataModified(OpParamSetting.envLoop, value);
            }
        }

        #endregion

        #region Brushes for enabled and disabled stuff

        /// <summary>
        /// Brush used for enabled text
        /// </summary>
        static private readonly IBrush TextEnabledColour = Brushes.White;

        /// <summary>
        /// Brush used for disabled text
        /// </summary>
        static private readonly IBrush TextDisabledColour = Brushes.DimGray;

        /// <summary>
        /// Brush used for the envelope diagram's lines when enabled
        /// </summary>
        static private readonly IBrush StrokeEnabledColour = Brushes.DimGray;

        /// <summary>
        /// Brush used for the envelope diagram's lines when disabled
        /// </summary>
        static private readonly IBrush StrokeDisabledColour = new SolidColorBrush(0xFF333333);

        #endregion

        #region Envelope drawing

        /// <summary>
        /// Are the envelope controls enabled?
        /// </summary>
        private bool IsEnvEnabled { get => UseEnvelope && !IsMuted; }

        /// <summary>
        /// Brush used for the envelope text labels
        /// </summary>
        /// <seealso cref="Dexy.DexyPatch.Controls.EnvelopePic"/>
        public IBrush EnvTextColour { get => IsEnvEnabled ? TextEnabledColour : TextDisabledColour; }

        /// <summary>
        /// Brush used for the envelope diagram's lines
        /// </summary>
        /// <seealso cref="Dexy.DexyPatch.Controls.EnvelopePic"/>
        public IBrush EnvStrokeColour { get => IsEnvEnabled ? StrokeEnabledColour : StrokeDisabledColour; }

        #endregion
    }
}
