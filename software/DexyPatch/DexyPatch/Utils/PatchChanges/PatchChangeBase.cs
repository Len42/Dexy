using System;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Utils.PatchChanges
{
    /// <summary>
    /// Base class representing a single editing change to a particular
    /// <see cref="Dexy.DexyPatch.Models.Patch"/> in a <see cref="Dexy.DexyPatch.Models.PatchBank"/>
    /// </summary>
    /// <remarks>
    /// Used in live updating mode.
    /// Subclasses must store which setting in the patch was changed and the updated value.
    /// Subclasses must implement IZppSerialize::Serialize to send the object to
    /// the Dexy module. It must match the corresponding deserialization in the
    /// firmware. Deserialize is not required.
    /// </remarks>
    /// <seealso cref="Dexy.DexyPatch.Services.LiveUpdater"/>
    public abstract class PatchChangeBase : IZppSerialize
    {
        /// <summary>Patch number</summary>
        public readonly byte iPatch;

        public PatchChangeBase(int iPatch)
        {
            if (iPatch < 0 || iPatch >= DexyDefs.numPatches) {
                throw new ArgumentOutOfRangeException(nameof(iPatch));
            }
            this.iPatch = (byte)iPatch;
        }

        /// <summary>
        /// Return true if another <see cref="PatchChangeBase"/>
        /// represents a change to the same patch setting as this one.
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        /// <remarks>
        /// Subclasses must override and call this base implementation.
        /// This is used to ensure that later updates override earlier changes
        /// to the same setting.
        /// </remarks>
        /// <seealso cref="Dexy.DexyPatch.Utils.PatchChanges.PatchBankChanges.PatchChangeComparer"/>
        public virtual bool IsSameSetting(PatchChangeBase? other) => iPatch == other?.iPatch;

        /// <summary>
        /// Return a hash code that corresponds to IsSameSetting
        /// </summary>
        /// <returns></returns>
        /// <remarks>
        /// Invariant:
        /// <code>
        /// (obj1.GetCustomHashCode() == obj2.GetCustomHashCode()) == obj1.IsSameSetting(obj2)
        /// </code>
        /// This is required by <see cref="Dexy.DexyPatch.Utils.PatchChanges.PatchBankChanges.PatchChangeComparer"/>.
        /// </remarks>
        /// <seealso cref="Dexy.DexyPatch.Utils.PatchChanges.PatchChangeBase.IsSameSetting"/>
        public virtual int GetCustomHashCode() => iPatch.GetHashCode();

        public override string? ToString() => $"{base.ToString()} patch {iPatch}";

        #region IZppSerialize interface

        // Subclasses must implement
        public abstract void Serialize(ZppWriter w);

        // Not required
        public virtual void Deserialize(ZppReader r) { throw new NotSupportedException(); }

        #endregion
    }
}
