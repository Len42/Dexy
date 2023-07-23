using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Utils.PatchChanges
{
    /// <summary>
    /// A patch has been changed entirely, e.g. by copy/paste
    /// </summary>
    /// <seealso cref="Dexy.DexyPatch.Models.Patch"/>
    public class PatchChange : PatchChangeBase
    {
        /// <summary>The updated <see cref="Dexy.DexyPatch.Models.Patch"/></summary>
        /// <remarks>This is a deep-copy of the given Patch.</remarks>
        public readonly Patch value;

        public PatchChange(int iPatch, Patch value) : base(iPatch) { this.value = value.Clone(); }

        public override bool IsSameSetting(PatchChangeBase? obj)
            => obj is PatchChange && base.IsSameSetting(obj);

        public override string? ToString() => $"{base.ToString()} value {value}";

        public override void Serialize(ZppWriter w)
        {
            w.Write(iPatch);
            w.Write(value);
        }
    }
}
