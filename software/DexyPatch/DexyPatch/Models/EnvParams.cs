using System;
using Dexy.DexyPatch.Utils;
using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Models
{
    /// <summary>
    /// Envelope level scaling curve type
    /// </summary>
    public enum ScalingCurve : sbyte
    {
        NExp = -2, NLin = -1, None = 0, Lin = 1, Exp = 2
    }

    /// <summary>
    /// Envelope level scaling parameters
    /// </summary>
    public class LevelScalingParams : IZppSerialize
    {
        /// <summary>Left-right break point</summary>
        public Int16 breakPoint = 60 * 256;
        /// <summary>Scaling curve to the left</summary>
        public ScalingCurve curveLeft = ScalingCurve.None;
        /// <summary>Scaling curve to the right</summary>
        public ScalingCurve curveRight = ScalingCurve.None;
        /// <summary>Scaling depth to the left</summary>
        public UInt16 depthLeft = 0;
        /// <summary>Scaling depth to the right</summary>
        public UInt16 depthRight = 0;

        /// <summary>
        /// Are all the settings in this object OK?
        /// </summary>
        /// <returns></returns>
        public bool IsValid()
        {
            return curveLeft >= ScalingCurve.NExp && curveLeft <= ScalingCurve.Exp
                && curveRight >= ScalingCurve.NExp && curveRight <= ScalingCurve.Exp
                && depthLeft <= DexyDefs.max_param_t
                && depthRight <= DexyDefs.max_param_t;
        }

        /// <summary>
        /// Return a copy of this object
        /// </summary>
        /// <returns></returns>
        public LevelScalingParams Clone()
        {
            return (LevelScalingParams)MemberwiseClone();
        }

        public void Serialize(ZppWriter w)
        {
            w.Write(breakPoint);
            w.Write((sbyte)curveLeft);
            w.Write((sbyte)curveRight);
            w.Write(depthLeft);
            w.Write(depthRight);
        }

        public void Deserialize(ZppReader r, ushort version)
        {
            breakPoint = r.ReadInt16();
            curveLeft = (ScalingCurve)r.ReadSByte();
            curveRight = (ScalingCurve)r.ReadSByte();
            depthLeft = r.ReadUInt16();
            depthRight = r.ReadUInt16();
        }
    }

    /// <summary>
    /// Envelope settings
    /// </summary>
    /// <remarks>
    /// Must match the corresponding C++ class
    /// </remarks>
    public class EnvParams : IZppSerialize
    {
        /// <summary>Delay time</summary>
        public UInt16 delay = 0;
        /// <summary>Attack rate</summary>
        public UInt16 attack = 0;
        /// <summary>Decay rate</summary>
        public UInt16 decay = 0;
        /// <summary>Sustain level</summary>
        public UInt16 sustain = DexyDefs.max_param_t;
        /// <summary>Release rate</summary>
        public UInt16 release = 0;
        /// <summary>Rate scaling by pitch</summary>
        public UInt16 rateScaling = 0;
        /// <summary>Level scaling parameters</summary>
        public LevelScalingParams levelScaling = new();
        /// <summary>If true the envelope repeats indefinitely</summary>
        public bool loop = false;

        /// <summary>
        /// Are all the settings in this object OK?
        /// </summary>
        /// <returns></returns>
        public bool IsValid()
        {
            return delay <= DexyDefs.max_param_t
                && attack <= DexyDefs.max_param_t
                && decay <= DexyDefs.max_param_t
                && sustain <= DexyDefs.max_param_t
                && release <= DexyDefs.max_param_t
                && rateScaling <= DexyDefs.max_param_t;
        }

        /// <summary>
        /// Return a copy of this object
        /// </summary>
        /// <returns></returns>
        public EnvParams Clone()
        {
            EnvParams copy = (EnvParams)MemberwiseClone();
            copy.levelScaling = levelScaling.Clone();
            return copy;
        }

        #region IZppSerialize interface

        public void Serialize(ZppWriter w)
        {
            w.Write(delay);
            w.Write(attack);
            w.Write(decay);
            w.Write(sustain);
            w.Write(release);
            w.Write(rateScaling);
            w.Write(levelScaling);
            w.Write(loop);
        }

        public void Deserialize(ZppReader r, ushort version)
        {
            delay = r.ReadUInt16();
            attack = r.ReadUInt16();
            decay = r.ReadUInt16();
            sustain = r.ReadUInt16();
            release = r.ReadUInt16();
            if (version >= 2) {
                rateScaling = r.ReadUInt16();
                r.Read(levelScaling, version);
            } else {
                rateScaling = 0;
                levelScaling = new();
            }
            loop = r.ReadBoolean();
        }

        #endregion
    }
}
