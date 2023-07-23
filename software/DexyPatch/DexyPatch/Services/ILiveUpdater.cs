using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils.PatchChanges;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Live updating of patches in the connected Dexy module as they are edited
    /// </summary>
    interface ILiveUpdater : IService
    {
        /// <summary>
        /// Is the Dexy module's patchbank being edited? If so, changes should
        /// be downloaded as they are done.
        /// </summary>
        /// <remarks>
        /// This must be set when the patchbank being edited in the UI is the
        /// one that was uploaded from the module.
        /// </remarks>
        bool IsLive { get; set; }

        /// <summary>
        /// Tell the Dexy module to select the given patch, only if IsLive
        /// </summary>
        /// <param name="iPatch">Patch number</param>
        void SelectPatch(int iPatch);

        /// <summary>
        /// If any editing changes are pending, send them to the Dexy module
        /// </summary>
        /// <remarks>
        /// It's only necessary to call this method to clean up the last pending
        /// changes before shutting down or turning off IsLive or something.
        /// Otherwise, changes will be sent to the module automatically.
        /// </remarks>
        void CheckSendChanges();

        /// <summary>
        /// Get the collection of currently pending changes and reset it to empty
        /// </summary>
        /// <returns></returns>
        public PatchBankChanges GetAndClear();

        /// <summary>
        /// Add an entire patch to the list of pending changes
        /// </summary>
        /// <param name="iPatch">Patch number</param>
        /// <param name="value">The updated Patch - a copy will be saved</param>
        void PatchUpdated(int iPatch, Patch value);

        /// <summary>
        /// Add a patch setting change to the list of pending changes
        /// </summary>
        /// <param name="iPatch">Patch number</param>
        /// <param name="field">Which setting to change</param>
        /// <param name="value">The new value - 
        /// string if <c><paramref name="field"/> == <see cref="PatchSetting.name"/></c>,
        /// <c>UInt16</c> otherwise</param>
        void PatchSettingUpdated(int iPatch, PatchSetting field, object value);

        /// <summary>
        /// Add an operator-specific setting change to the list of pending changes
        /// </summary>
        /// <param name="iPatch">Patch number</param>
        /// <param name="iOp">Operator number</param>
        /// <param name="field">Which setting to change</param>
        /// <param name="value">The new value - bool, byte, or UInt16</param>
        void PatchOperatorUpdated(int iPatch, int iOp, OpParamSetting field, object value);
    }
}
