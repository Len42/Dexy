using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Utils.PatchChanges
{
    /// <summary>
    /// Subclass representing a change to a patch's name
    /// </summary>
    /// <seealso cref="Dexy.DexyPatch.Models.Patch"/>
    public class NameChange : PatchChangeBase
    {
        /// <summary>The new name</summary>
        public readonly string value;

        public NameChange(int iPatch, string value) : base(iPatch) { this.value = value; }

        public override bool IsSameSetting(PatchChangeBase? obj)
            => obj is PatchChange && base.IsSameSetting(obj);

        public override string? ToString() => $"{base.ToString()} value {value}";

        public override void Serialize(ZppWriter w)
        {
            w.Write(iPatch);
            w.WriteFixedLength(value, Patch.patchNameLen);
        }
    }
}
