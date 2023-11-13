using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Avalonia.Controls;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using MsBox.Avalonia.Enums;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Services;
using Dexy.DexyPatch.Views;
using Dexy.DexyPatch.Utils.PatchChanges;

namespace Dexy.DexyPatch.ViewModels
{
    /// <summary>
    /// ViewModel for editing a <see cref="Dexy.DexyPatch.Models.PatchBank"/>
    /// </summary>
    public partial class PatchBankViewModel : ViewModelBase
    {
        public PatchBankViewModel(MainWindowViewModel parentVM)
        {
            this.parentVM = parentVM;
            InitializeFromModel();
            SelectedPatch = 0;
        }

        /// <summary>
        /// ViewModel of the parent (container) view associated with this ViewModel
        /// </summary>
        private readonly MainWindowViewModel parentVM;

        /// <summary>
        /// The View attached to this ViewModel
        /// </summary>
        public PatchBankView? View { get; set; }

        /// <summary><see cref="Dexy.DexyPatch.Services.ILiveUpdater"/> service</summary>
        private readonly ILiveUpdater liveUpdater = Service<ILiveUpdater>.Get();

        #region Model data

        /// <summary>
        /// The patchbank currently being edited. Managed by the parent VM.
        /// </summary>
        private PatchBank? PatchBank { get => parentVM.PatchBank; }

        /// <summary>
        /// The patch currently being edited. Exposed for access by child VMs.
        /// </summary>
        private Patch? patch;

        #endregion

        #region ViewModel data

        /// <summary>
        /// Propagate the data-modified flag to the parent VM and set the changed data for live-updating
        /// </summary>
        private void SetDataModified(int iPatch, Patch value) => parentVM.SetDataModified(iPatch, value);

        /// <summary>
        /// Propagate the data-modified flag to the parent VM and set the changed data for live-updating
        /// </summary>
        public void SetDataModified(PatchSetting field, object value) => parentVM.SetDataModified(SelectedPatch, field, value);

        /// <summary>
        /// Propagate the data-modified flag to the parent VM and set the changed data for live-updating
        /// </summary>
        public void SetDataModified(int iOp, OpParamSetting field, object value) => parentVM.SetDataModified(SelectedPatch, iOp, field, value);

        /// <summary>
        /// Initialize data members from the <see cref="Dexy.DexyPatch.Models.OpParams"/>
        /// </summary>
        private void InitializeFromModel()
        {
            SelectedPatch = -1;
            PatchList = PatchBank?.patches.Select(p => p.name).ToArray();
        }

        #endregion

        #region Patch list pane

        /// <summary>
        /// List of patch names
        /// </summary>
        [ObservableProperty]
        private string[]? patchList;

        /// <summary>
        /// The index of the currently-selected patch, bound to a listbox of patch names
        /// </summary>
        [ObservableProperty]
        private int selectedPatch = -1;

        /// <summary>
        /// When a different patch is selected in the list, display it in the patch pane
        /// </summary>
        /// <param name="value"></param>
        partial void OnSelectedPatchChanged(int value) => DisplayPatch(value);

        /// <summary>
        /// Change the name of the currently-selected patch
        /// </summary>
        /// <param name="name"></param>
        public void SetSelectedPatchName(string name)
        {
            if (PatchList != null && SelectedPatch >= 0) {
                PatchList[SelectedPatch] = name;
                // BUG: PatchBankView.PatchListUpdated is supposed to update the
                // listbox to display the new patch name but it doesn't work. :(
                View?.PatchListUpdated();
            }
        }

        /// <summary>
        /// Replace the currently-selected patch with a new default patch
        /// </summary>
        [RelayCommand]
        private void InitializePatch()
        {
            ReplaceSelectedPatch(Patch.MakeDefaultPatch());
        }

        /// <summary>
        /// Saved copy of a patch, for copy/paste
        /// </summary>
        private Patch? copiedPatch = null;

        /// <summary>
        /// Save a copy of the currently-selected patch
        /// </summary>
        [RelayCommand]
        private void CopyPatch()
        {
            if (PatchBank != null && 0 <= SelectedPatch && SelectedPatch < DexyDefs.numPatches) {
                copiedPatch = PatchBank.patches[SelectedPatch].Clone();
            }
        }

        /// <summary>
        /// Replace the currently-selected patch with one that was previously copied
        /// </summary>
        [RelayCommand]
        private void PastePatch()
        {
            if (copiedPatch != null) {
                ReplaceSelectedPatch(copiedPatch.Clone());
            }
        }

        /// <summary>
        /// Replace the currently-selected patch with the given <see cref="Dexy.DexyPatch.Models.Patch"/>
        /// </summary>
        /// <param name="patch"></param>
        private async void ReplaceSelectedPatch(Patch patch)
        {
            if (PatchBank != null && 0 <= SelectedPatch && SelectedPatch < DexyDefs.numPatches) {
                ButtonResult result = await Service<IDialogService>.Get().AskYesNo(
                    "Paste patch?",
                    string.Format("Replace patch \"{0}\"?", PatchList?[SelectedPatch]?.Trim() ?? ""));
                if (result == ButtonResult.Yes) {
                    PatchBank.patches[SelectedPatch] = patch;
                    SetDataModified(SelectedPatch, patch);
                    DisplayPatch(SelectedPatch);
                    SetSelectedPatchName(PatchBank.patches[SelectedPatch].name);
                    // BUG: Name in PatchList is not updated (known problem with ListBox)
                }
            }
        }

        /// <summary>
        /// Move the currently-selected patch one position higher in the patchbank
        /// </summary>
        [RelayCommand]
        private void MovePatchUp()
        {
            SwapPatches(SelectedPatch, SelectedPatch - 1);
        }

        /// <summary>
        /// Move the currently-selected patch one position lower in the patchbank
        /// </summary>
        [RelayCommand]
        private void MovePatchDown()
        {
            SwapPatches(SelectedPatch, SelectedPatch + 1);
        }

        /// <summary>
        /// Swap the two Patches at the given indices in PatchList.
        /// Display and select the patch at index2 after the swap.
        /// </summary>
        /// <param name="index1"></param>
        /// <param name="index2"></param>
        private void SwapPatches(int index1, int index2)
        {
            if (PatchBank != null
                && 0 <= index1 && index1 < DexyDefs.numPatches
                && 0 <= index2 && index2 < DexyDefs.numPatches) {
                (PatchBank.patches[index1], PatchBank.patches[index2])
                    = (PatchBank.patches[index2], PatchBank.patches[index1]);
                SetDataModified(index1, PatchBank.patches[index1]);
                SetDataModified(index2, PatchBank.patches[index2]);
                // TODO: more efficient way to do this without re-creating the PatchView:
                InitializeFromModel();
                SelectedPatch = index2;
            }
        }

        #endregion

        #region Patch pane

        /// <summary>
        /// ViewModel for the contained <see cref="Dexy.DexyPatch.Models.Patch"/> view
        /// </summary>
        [ObservableProperty]
        private PatchViewModel? patchVM = null;

        /// <summary>
        /// Display the <see cref="Dexy.DexyPatch.Models.Patch"/> at the given
        /// index in the <see cref="Dexy.DexyPatch.Models.PatchBank"/>
        /// </summary>
        /// <param name="iPatch"></param>
        /// <remarks>
        /// This (re-)creates the <see cref="Dexy.DexyPatch.ViewModels.PatchViewModel"/>
        /// and the <see cref="Dexy.DexyPatch.Views.PatchView"/>.
        /// </remarks>
        private void DisplayPatch(int iPatch)
        {
            if (PatchBank == null
                || PatchBank.patches.Length <= 0
                || iPatch < 0
                || iPatch >= PatchBank.patches.Length)
            {
                patch = null;
            } else {
                iPatch = int.Clamp(iPatch, 0, PatchBank.patches.Length - 1);
                // Tell the Dexy module to select the given patch (if live-updating)
                liveUpdater.SelectPatch(iPatch);
                // Display the given patch
                patch = PatchBank.patches[iPatch];
                PatchVM = new(this, patch); // This will reinitialize the PatchView as well.
            }
        }

        #endregion
    }
}
