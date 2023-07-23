using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using Avalonia;
using CommunityToolkit.Mvvm.ComponentModel;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils;
using Dexy.DexyPatch.Utils.PatchChanges;

namespace Dexy.DexyPatch.ViewModels
{
    /// <summary>
    /// ViewModel for editing a <see cref="Dexy.DexyPatch.Models.Patch"/>
    /// </summary>
    public partial class PatchViewModel : ViewModelBase
    {
        public PatchViewModel(PatchBankViewModel parentVM, Patch patch)
        {
            this.parentVM = parentVM;
            this.patch = patch;
            BuildOperatorViews();
            InitializeFromModel();
        }

        /// <summary>
        /// ViewModel of the parent (container) view associated with this ViewModel
        /// </summary>
        private readonly PatchBankViewModel parentVM;

        #region Model data

        /// <summary>
        /// The Patch currently being edited.
        /// </summary>
        private readonly Patch patch;

        #endregion

        #region ViewModel data

        /// <summary>
        /// Propagate the data-modified flag to the parent VM and set the changed data for live-updating
        /// </summary>
        /// <param name="field"></param>
        /// <param name="value"></param>
        private void SetDataModified(PatchSetting field, object value) => parentVM.SetDataModified(field, value);

        /// <summary>
        /// Propagate the data-modified flag to the parent VM and set the changed data for live-updating
        /// </summary>
        /// <param name="iOp"></param>
        /// <param name="field"></param>
        /// <param name="value"></param>
        public void SetDataModified(int iOp, OpParamSetting field, object value) => parentVM.SetDataModified(iOp, field, value);

        /// <summary>
        /// Flag to ignore changes to data members during initialization
        /// </summary>
        private bool ignoreOnPropertyChanged = false;

        /// <summary>
        /// Initialize data members from the <see cref="Dexy.DexyPatch.Models.Patch"/>
        /// </summary>
        private void InitializeFromModel()
        {
            ignoreOnPropertyChanged = true;
            PatchName = patch.name.TrimEnd();
            Algorithm = patch.algorithm;
            Feedback = patch.feedbackAmount;
            ignoreOnPropertyChanged = false;
        }

        /// <summary>
        /// The name of the <see cref="Dexy.DexyPatch.Models.Patch"/>
        /// </summary>
        [ObservableProperty]
        private string? patchName;

        /// <summary>
        /// Update the patch's name when <see cref="patchName"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnPatchNameChanged(string? value)
        {
            if (!ignoreOnPropertyChanged && value != null) {
                byte[] asciiBytes = ConvertString.ToAsciiBytes(value);
                string nameAscii = ConvertString.ToString(asciiBytes);
                if (nameAscii.Length > Patch.patchNameLen) {
                    nameAscii = nameAscii[..Patch.patchNameLen];
                }
                string namePadded = nameAscii.PadRight(Patch.patchNameLen);
                patch.name = namePadded;
                parentVM.SetSelectedPatchName(namePadded);
                SetDataModified(PatchSetting.name, patch.name);
            }
        }

        /// <summary>
        /// Maximum length of a patch name (exported for the View)
        /// </summary>
        [ObservableProperty]
        private int maxNameLength = Patch.patchNameLen;

        /// <summary>
        /// The algorithm number of this <see cref="Dexy.DexyPatch.Models.Patch"/>
        /// </summary>
        /// <seealso cref="Dexy.DexyPatch.Models.Algorithm"/>
        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(OperatorVMs))]
        private byte algorithm;

        /// <summary>
        /// Update the patch's algorithm when <see cref="algorithm"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnAlgorithmChanged(byte value)
        {
            if (!ignoreOnPropertyChanged && value < DexyDefs.numAlgorithms) {
                patch.algorithm = value;
                SetDataModified(PatchSetting.algorithm, patch.algorithm);
                // This results in changes to the operator displays.
                // It's simplest to just re-initialize all the OperatorViews.
                BuildOperatorViews();
            }
        }

        /// <summary>
        /// The amount of operator feedback used in this <see cref="Dexy.DexyPatch.Models.Patch"/>
        /// </summary>
        [ObservableProperty]
        private ushort feedback;

        /// <summary>
        /// Update the patch's feedback amount when <see cref="feedback"/> is changed
        /// </summary>
        /// <param name="value"></param>
        partial void OnFeedbackChanged(ushort value)
        {
            if (!ignoreOnPropertyChanged) {
                patch.feedbackAmount = value;
                SetDataModified(PatchSetting.feedbackAmount, patch.feedbackAmount);
            }
        }

        #endregion

        #region Operator panes

        /// <summary>
        /// ViewModels for the contained <see cref="Dexy.DexyPatch.Views.OperatorView"/> views
        /// </summary>
        public OperatorViewModel[]? OperatorVMs { get; private set; }

        /// <summary>
        /// Create several sub-views for the patch's operators
        /// </summary>
        private void BuildOperatorViews()
        {
            // Create ViewModels and Views for the operators
            OperatorVMs = new OperatorViewModel[DexyDefs.numOperators];
            for (int i = 0; i < OperatorVMs.Length; i++) {
                OperatorVMs[i] = new(this, i, patch.opParams[i]);
            }
        }

        #endregion
    }
}
