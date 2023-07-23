using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace Dexy.DexyPatch.Utils.PatchChanges
{
    /// <summary>
    /// Stores a set of editing changes waiting to be send to the Dexy module
    /// (if live updating is enabled).
    /// </summary>
    public class PatchBankChanges
    {
        /// <summary>
        /// The collection of pending changes
        /// </summary>
        /// <remarks>
        /// A custom <see cref="EqualityComparer{T}"/> is used so that when a
        /// single setting is edited repeatedly, only the most recent change is
        /// stored.
        /// </remarks>
        private readonly HashSet<PatchChangeBase> changes = new(new PatchChangeComparer());

        /// <summary>
        /// Are there any pending changes?
        /// </summary>
        public bool HasAnyChanges { get => changes.Count > 0; }

        /// <summary>
        /// Get a list of the changes waiting to be sent
        /// </summary>
        public IEnumerable<PatchChangeBase> Changes { get => changes; }

        /// <summary>
        /// Get the number of pending changes
        /// </summary>
        public int Count { get => changes.Count; }

        /// <summary>
        /// Add a new change to be sent to the module
        /// </summary>
        /// <param name="obj"></param>
        /// <remarks>
        /// This is called whenever an edit is made to a patch (when in live-update
        /// mode) so that the change can be sent to the module.
        /// </remarks>
        public void Add(PatchChangeBase obj)
        {
            // Must delete and re-add the object so that later changes override earlier ones.
            // BUG: Order of changes is not preserved, which may cause errors if whole-Patch
            // changes are mixed with individual setting changes.
            // It's hard to make that actually happen, though.
            // Could be fixed by storing changes in a List or LinkedList but then must
            // scan the list to remove earlier same-setting changes (RemoveAll or Filter).
            changes.Remove(obj);
            changes.Add(obj);
        }

        /// <summary>
        /// Custom <see cref="EqualityComparer{T}"/> for <see cref="changes"/>
        /// </summary>
        private class PatchChangeComparer : EqualityComparer<PatchChangeBase>
        {
            public override bool Equals(PatchChangeBase? x, PatchChangeBase? y)
                => x is null ? y is null : x.IsSameSetting(y);

            public override int GetHashCode([DisallowNull] PatchChangeBase obj)
                => obj.GetCustomHashCode();
        }
    }
}
