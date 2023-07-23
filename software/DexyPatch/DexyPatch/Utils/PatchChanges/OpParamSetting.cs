namespace Dexy.DexyPatch.Utils.PatchChanges
{
    /// <summary>
    /// Designates one of the settings that apply to an operator in a patch
    /// </summary>
    /// <seealso cref="Dexy.DexyPatch.Models.OpParams"/>
    /// <seealso cref="Dexy.DexyPatch.Models.EnvParams"/>
    public enum OpParamSetting : byte
    {
        /// <summary>Is the frequency fixed or set by pitch CV?</summary>
        fixedFreq,
        /// <summary>freqRatio_t or midiNote_t depending on fixedFreq</summary>
        noteOrFreq,
        /// <summary>Output level nominal maximum</summary>
        outputLevel,
        /// <summary>Is output level modulated by the envelope?</summary>
        useEnvelope,
        /// <summary>Operator sensitivity to amplitude modulation</summary>
        ampModSens,
        /// <summary>Envelope delay time</summary>
        envDelay,
        /// <summary>Envelope attack rate</summary>
        envAttack,
        /// <summary>Envelope decay rate</summary>
        envDecay,
        /// <summary>Envelope sustain level</summary>
        envSustain,
        /// <summary>Envelope release rate</summary>
        envRelease,
        /// <summary>If true the envelope repeats indefinitely</summary>
        envLoop
    }
}
