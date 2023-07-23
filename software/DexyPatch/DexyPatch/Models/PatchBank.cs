using Dexy.DexyPatch.Utils.Zpp;
using System;

namespace Dexy.DexyPatch.Models
{
    /// <summary>
    /// A patch bank
    /// </summary>
    /// <remarks>
    /// Must match the corresponding C++ class
    /// </remarks>
    public class PatchBank : IZppSerialize
    {
        /// <summary>
        /// List of patches
        /// </summary>
        public Patch[] patches;

        public PatchBank()
        {
            patches = new Patch[DexyDefs.numPatches];
            for (int i = 0; i < DexyDefs.numPatches; i++) {
                patches[i] = new();
            }
        }

        /// <summary>
        /// Are all the settings in this object OK?
        /// </summary>
        /// <returns></returns>
        public bool IsValid()
        {
            if (patches.Length != DexyDefs.numPatches)
                return false;
            foreach (Patch patch in patches) {
                if (!patch.IsValid())
                    return false;
            }
            return true;
        }

        /// <summary>
        /// Return a copy of this object
        /// </summary>
        /// <returns></returns>
        public PatchBank Clone()
        {
            PatchBank copy = new();
            for (int i = 0; i < DexyDefs.numPatches; i++) {
                copy.patches[i] = patches[i].Clone();
            }
            return copy;
        }

        #region IZppSerialize interface
      
        public void Serialize(ZppWriter w)
            => w.WriteArray(patches, DexyDefs.numPatches);

        public void Deserialize(ZppReader r)
        {
            r.ReadArray(patches, DexyDefs.numPatches);
            if (!IsValid()) {
                throw new InvalidPatchException();
            }
        }

        #endregion

        /// <summary>
        /// Return a patch bank with a set of default patches
        /// </summary>
        /// <returns></returns>
        /// <exception cref="InvalidPatchException"></exception>
        public static PatchBank MakeDefault()
        {
            PatchBank patchBank = new();
            // Initialize with a bunch of default patches
            // TODO: Load some patches that are more interesting
            patchBank.patches[0] = Patch.MakeBellPatch();
            patchBank.patches[1] = Patch.MakeSinePatch();
            for (int i = 2; i < DexyDefs.numPatches; i++) {
                patchBank.patches[i] = Patch.MakeDefaultPatch();
            }
            if (!patchBank.IsValid()) {
                throw new InvalidPatchException();
            }
            return patchBank;
        }

        /// <summary>
        /// Exception thrown when invalid patch data is detected
        /// </summary>
        private class InvalidPatchException : Exception
        {
            public InvalidPatchException() : base("Invalid patch") { }

            public InvalidPatchException(Exception innerException)
                : base("Invalid patch", innerException) { }
        }
    }
}
