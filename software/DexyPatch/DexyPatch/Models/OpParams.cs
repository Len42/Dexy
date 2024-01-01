using System;
using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Models
{
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

        /// <summary>
        /// Are all the settings in this object OK?
        /// </summary>
        /// <returns></returns>
        public bool IsValid()
        {
            return outputLevel <= DexyDefs.max_param_t
                && ampModSens <= DexyDefs.max_param_t
                && env.IsValid();
        }

        /// <summary>
        /// Return a copy of this object
        /// </summary>
        /// <returns></returns>
        public OpParams Clone()
        {
            OpParams copy = (OpParams)MemberwiseClone();
            copy.env = env.Clone();
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
        }

        public void Deserialize(ZppReader r, ushort version)
        {
            fixedFreq = r.ReadBoolean();
            noteOrFreq = r.ReadUInt16();
            outputLevel = r.ReadUInt16();
            useEnvelope = r.ReadBoolean();
            r.Read(env, version);
            ampModSens = r.ReadUInt16();
        }

        #endregion
    }
}
