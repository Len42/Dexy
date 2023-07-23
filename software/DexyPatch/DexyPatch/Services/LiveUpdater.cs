using System;
using System.CodeDom.Compiler;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing.Text;
using System.Threading;
using System.Threading.Tasks;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils;
using Dexy.DexyPatch.Utils.PatchChanges;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Live updating of patches in the connected Dexy module as they are edited
    /// </summary>
    public class LiveUpdater : ILiveUpdater
    {
        public LiveUpdater()
        {
            // Start a worker thread to send updates to the Dexy module
            updateThread = new();
            updateThread.Start(new());
        }

        /// <summary>
        /// Collection of pending changes
        /// </summary>
        private PatchBankChanges patchBankChanges = new();

        /// <summary>
        /// Mutex lock used for thread-safe access to <see cref="patchBankChanges"/>
        /// </summary>
        private readonly object mutex = new();

        #region ILiveUpdater interface

        public void Terminate() { updateThread.Stop(); }

        public bool IsLive { get; set; }

        public void SelectPatch(int iPatch)
        {
            if (IsLive) {
                Debug.WriteLine($"LiveUpdater.SelectPatch: Select patch {iPatch}");
                IDexyDevice dexyDevice = Service<IDexyDevice>.Get();
                dexyDevice.SelectPatchAsync(iPatch);
            }
        }

        public void CheckSendChanges()
        {
            PatchBankChanges changes = GetAndClear();
            if (IsLive && changes.HasAnyChanges) {
                SendUpdates(changes);
            }
        }

        public PatchBankChanges GetAndClear()
        {
            lock (mutex) {
                PatchBankChanges changes = patchBankChanges;
                patchBankChanges = new();
                return changes;
            }
        }

        public void PatchUpdated(int iPatch, Patch value)
        {
            lock (mutex) patchBankChanges.Add(new PatchChange(iPatch, value));
        }

        public void PatchSettingUpdated(int iPatch, PatchSetting field, object value)
        {
            // value must be UInt16 except for the patch name field
            PatchChangeBase change;
            if (field == PatchSetting.name) {
                change = new NameChange(iPatch, (string)value);
            } else {
                // #kludge cannot cast a byte to a ushort unless you cast it to itself first???
                UInt16 v;
                if (field == PatchSetting.algorithm) {
                    Debug.Assert(value is byte);
                    v = (UInt16)(byte)value; // why do I have to do this? it's already a byte
                } else {
                    v = (UInt16)value;
                }
                change = new PatchSettingChange(iPatch, field, v);
            }
            lock (mutex) patchBankChanges.Add(change);
        }

        public void PatchOperatorUpdated(int iPatch, int iOp, OpParamSetting field, object value)
        {
            // value must be either bool or UInt16
            UInt16 uValue;
            if (value is bool bValue) {
                uValue = (UInt16)(bValue ? 1 : 0);
            } else {
                uValue = (UInt16)value;
            }
            lock (mutex) patchBankChanges.Add(new OpSettingChange(iPatch, iOp, field, uValue));
        }

        #endregion ILiveUpdater interface

        /// <summary>
        /// Send pending updates to the Dexy device
        /// </summary>
        /// <param name="changes"></param>
        private static void SendUpdates(PatchBankChanges changes)
        {
            IDexyDevice dexyDevice = Service<IDexyDevice>.Get();
            if (changes.HasAnyChanges && dexyDevice.IsConnected) {
                // Download changes to the module.
                foreach(var change in changes.Changes) {
                    dexyDevice.SendPatchChangedAsync(change);
                }
            }
        }

        /// <summary>
        /// Worker thread to send updates to the Dexy module
        /// </summary>
        private readonly UpdateThread updateThread;

        /// <summary>
        /// Worker thread to send updates to the Dexy module
        /// </summary>
        private class UpdateThread : ServiceThread
        {
            /// <summary>
            /// Short sleep time between thread iterations for low latency
            /// </summary>
            const int sleepMs = 50;

            /// <summary>
            /// Additional sleep time after sending a batch of updates to limit the update rate
            /// </summary>
            const int sleepMsAfterUpdate = 200;

            public UpdateThread() : base(sleepMs) { }

            private ILiveUpdater? liveUpdater;

            protected override void ThreadInit(ServiceThreadArgs args)
            {
                liveUpdater = Service<ILiveUpdater>.Get();
            }

            protected override void ThreadWork(ServiceThreadArgs args)
            {
                // If in live update mode and there are updates to send, send them
                if (liveUpdater!.IsLive) {
                    try {
                        SendUpdates(liveUpdater.GetAndClear());
                    } catch (Exception ex) {
                        Debug.WriteLine($"LiveUpdater exception {ex.GetType()} {ex.Message}");
                    }
                    Thread.Sleep(sleepMsAfterUpdate);
                }
            }

            protected override void ThreadEnd(ServiceThreadArgs args) { }
        }
    }
}
