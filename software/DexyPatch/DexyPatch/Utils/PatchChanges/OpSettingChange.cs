using System;
using Dexy.DexyPatch.Models;
using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Utils.PatchChanges
{
    /// <summary>
    /// One of the per-operator settings has been changed
    /// </summary>
    /// <seealso cref="Dexy.DexyPatch.Models.OpParams"/>
    /// <seealso cref="Dexy.DexyPatch.Models.EnvParams"/>
    public class OpSettingChange : PatchChangeBase
    {
        /// <summary>Operator number</summary>
        public readonly byte iOp;

        /// <summary>Which setting was changed</summary>
        public readonly OpParamSetting field;

        /// <summary>The updated value of the setting</summary>
        public readonly ushort value;

        public OpSettingChange(int iPatch, int iOp, OpParamSetting field, ushort value)
            : base(iPatch)
        {
            if (iOp < 0 || iOp >= DexyDefs.numOperators) {
                throw new ArgumentOutOfRangeException(nameof(iOp));
            }
            this.iOp = (byte)iOp;
            this.field = field;
            this.value = value;
        }

        public override bool IsSameSetting(PatchChangeBase? obj)
            => obj is OpSettingChange other && base.IsSameSetting(other) && iOp == other.iOp && field == other.field;

        public override int GetCustomHashCode() => base.GetCustomHashCode() ^ (iOp, field).GetHashCode();

        public override string? ToString() => $"{base.ToString()} iOp {iOp} field {field} value {value}";

        public override void Serialize(ZppWriter w)
        {
            w.Write(iPatch);
            w.Write(iOp);
            w.Write((byte)field);
            w.Write(value);
        }
    }
}
