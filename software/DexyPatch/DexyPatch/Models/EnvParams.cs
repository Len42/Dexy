using System;
using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Models
{
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
                && release <= DexyDefs.max_param_t;
        }

        /// <summary>
        /// Return a copy of this object
        /// </summary>
        /// <returns></returns>
        public EnvParams Clone()
        {
            return (EnvParams)MemberwiseClone();
        }

        #region IZppSerialize interface

        public void Serialize(ZppWriter w)
        {
            w.Write(delay);
            w.Write(attack);
            w.Write(decay);
            w.Write(sustain);
            w.Write(release);
            w.Write(loop);
        }

        public void Deserialize(ZppReader r)
        {
            delay = r.ReadUInt16();
            attack = r.ReadUInt16();
            decay = r.ReadUInt16();
            sustain = r.ReadUInt16();
            release = r.ReadUInt16();
            loop = r.ReadBoolean();
        }

        #endregion
    }
}
