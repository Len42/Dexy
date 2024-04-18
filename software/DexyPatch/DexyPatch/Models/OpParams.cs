using System;
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
    /// Operator settings
    /// </summary>
    /// <remarks>
    /// Must match the corresponding C++ class
    /// </remarks>
    public class OpParams : IZppSerialize
    {
        /// <summary>Is the frequency fixed or set by pitch CV?</summary>
        public bool fixedFreq = false;
        /// <summary>freqRatio_t or midiNote_t depending on fixedFreq</summary>
        public UInt16 noteOrFreq = DexyDefs.freqRatio1;
        /// <summary>Output level nominal maximum</summary>
        public UInt16 outputLevel = DexyDefs.max_param_t;
        /// <summary>Is output level modulated by the envelope?</summary>
        public bool useEnvelope = true;
        /// <summary>Envelope parameters</summary>
        public EnvParams env = new();
        /// <summary>Operator sensitivity to amplitude modulation</summary>
        public UInt16 ampModSens = 0;
        /// <summary>Level scaling parameters</summary>
        public LevelScalingParams levelScaling = new();

        /// <summary>
        /// Are all the settings in this object OK?
        /// </summary>
        /// <returns></returns>
        public bool IsValid()
        {
            return outputLevel <= DexyDefs.max_param_t
                && ampModSens <= DexyDefs.max_param_t
                && env.IsValid()
                && levelScaling.IsValid();
        }

        /// <summary>
        /// Return a copy of this object
        /// </summary>
        /// <returns></returns>
        public OpParams Clone()
        {
            OpParams copy = (OpParams)MemberwiseClone();
            copy.env = env.Clone();
            copy.levelScaling = levelScaling.Clone();
            return copy;
        }

        #region IZppSerialize interface
     
        public void Serialize(ZppWriter w)
        {
            w.Write(fixedFreq);
            w.Write(noteOrFreq);
            w.Write(outputLevel);
            w.Write(useEnvelope);
            w.Write(env);
            w.Write(ampModSens);
            w.Write(levelScaling);
        }

        public void Deserialize(ZppReader r, ushort version)
        {
            fixedFreq = r.ReadBoolean();
            noteOrFreq = r.ReadUInt16();
            outputLevel = r.ReadUInt16();
            useEnvelope = r.ReadBoolean();
            r.Read(env, version);
            ampModSens = r.ReadUInt16();
            if (version >= 2) {
                r.Read(levelScaling, version);
            } else {
                levelScaling = new();
            }
        }

        #endregion
    }
}
