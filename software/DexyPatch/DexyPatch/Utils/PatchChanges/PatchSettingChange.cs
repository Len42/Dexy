using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Utils.PatchChanges
{
    /// <summary>
    /// One of the patch-wide numeric settings has been changed
    /// </summary>
    /// <seealso cref="Dexy.DexyPatch.Models.Patch"/>
    public class PatchSettingChange : PatchChangeBase
    {
        /// <summary>Which setting was changed</summary>
        public readonly PatchSetting field;

        /// <summary>The updated value of the setting</summary>
        public readonly ushort value;

        public PatchSettingChange(int iPatch, PatchSetting field, ushort value)
            : base(iPatch) { this.field = field; this.value = value; }

        public override bool IsSameSetting(PatchChangeBase? obj)
            => obj is PatchSettingChange other && base.IsSameSetting(other) && field == other.field;

        public override int GetCustomHashCode() => base.GetCustomHashCode() ^ field.GetHashCode();

        public override string? ToString() => $"{base.ToString()} field {field} value {value}";

        public override void Serialize(ZppWriter w)
        {
            w.Write(iPatch);
            w.Write((byte)field);
            w.Write(value);
        }
    }
}
