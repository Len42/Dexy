namespace Dexy {

void Operator::setOpParams(const Patches::OpParams& params)
{
    fixedFreq = params.fixedFreq;
    if (fixedFreq) {
        // Set the operator's fixed pitch
        setFrequency(SineWave::getIncrementForMidiNote(midiNote_t(params.noteOrFreq)));
    } else {
        // Set operator's frequency ratio, to be used to set the frequency when running
        freqRatio = freqRatio_t(params.noteOrFreq);
    }
    // TODO: Scale this down to give headroom for modulation
    outputLevel = levelFromParam(params.outputLevel);
    useEnvelope = params.useEnvelope;
    ampModSens = params.ampModSens;
    // params includes the envelope parameters
    env.setParams(params.env);
}

void Operator::setNotePitch(phase_t pitch)
{
    if (!fixedFreq) {
        // Multiply pitch by frequency ratio. This requires 64-bit arithmetic.
        // freqRatio is fixed-point, 5-bit int + 11-bit fraction.
        uint64_t temp = uint64_t(pitch) * uint64_t(freqRatio);
        pitch = phase_t(temp >> 11);
        setFrequency(pitch);
        // TODO: Keyboard (pitch) level scaling - per-op break, curve, amount; see Complete DX7
    }
}

void Operator::setFrequency(phase_t pitch)
{
    sineWave.setIncrement(pitch);
}

void Operator::gateStart()
{
    env.gateStart();
}

void Operator::gateStop()
{
    env.gateStop();
}

void Operator::resetWave()
{
    sineWave.reset();
}

/// @brief Adjust the Operator's output according to the given level setting
/// @param amplitude Output amplitude
/// @param level Level setting
/// @return Adjusted output amplitude
static output_t adjustOutputLevel(output_t amplitude, level_t level)
{
    int32_t output32 = int32_t(amplitude) * int32_t(level + 1);
    return output_t(output32 >> 16);
}

output_t Operator::genNextOutput(output_t freqMod, output_t ampMod)
{
    // Sine oscillator
    output_t output = sineWave.genNextOutput(freqMod);
    // Apply envelope to amplitude
    if (useEnvelope) {
        output = adjustOutputLevel(output, env.genNextOutput());
        // TODO: level scaling - is this the place to do it?
    }
    // Apply amplitude modulation
    level_t level = max_level_t;
    level = level_t(int32_t(ampModSens) * int32_t(ampMod) / 1024
                            + (65535 - int32_t(ampModSens) * 32));
    output = adjustOutputLevel(output, level);
    // Apply operator output level
    output = adjustOutputLevel(output, outputLevel);
    return output;
}

/// @brief Lookup table to map a level parameter (param_t) to an actual level (level_t).
static constexpr DataTable<level_t, max_param_t+1,
    [](std::size_t index, [[maybe_unused]] std::size_t numValues) {
        // This function must map index 0 -> value 0 and
        // index max_param_t -> value max_level_t
        double value = std::round(std::exp(index * 0.00775) * 23.6285 - 24);
        return level_t(value);
    }> expLevelMap;

constexpr level_t Operator::levelFromParam(param_t param)
{
    if (param > max_param_t)
        param = max_param_t;
    return expLevelMap[param]; // exponential
}

}
