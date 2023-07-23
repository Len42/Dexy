#pragma once

namespace Dexy {

/// @brief An FM synth operator, consisting of a sine wave oscillator and an envelope
/// generator
class Operator
{
public:
    /// @brief Set this Operator's settings based on the currently selected
    /// Dexy::Patches::Patch
    /// @param params Operator settings from the Patch
    void setOpParams(const Patches::OpParams& params);

    /// @brief Set this Operator's frequency based on the note pitch (derived
    /// from a CV input).
    /// @details This uses freqRatio and doesn't affect a fixed-frequency operator.
    /// @param pitch Fundamental note frequency, represented as a Dexy::phase_t
    void setNotePitch(phase_t pitch);

    /// @brief Set this operator's frequency explicity, even if fixedFreq.
    /// Ignores freqRatio.
    /// @param pitch Frequency represented as a Dexy::phase_t
    void setFrequency(phase_t pitch);

    /// @brief Gate start signal has been received - Start playing a note
    void gateStart();

    /// @brief Gate stop signal has been received - Stop playing the note
    void gateStop();

    /// @brief Reset the Operator's sine wave oscillator.
    /// @details This is called when the Operator's settings are changed via
    /// live updating.
    void resetWave();

    /// @brief Generate the Operator's next output value by advancing both the
    /// waveform and the envelope.
    /// @param freqMod Frequency modulation value
    /// @param ampMod Amplitude modulation value
    /// @return The Operator's output value
    output_t genNextOutput(output_t freqMod, output_t ampMod);

    /// @brief Helper function to scale the output level setting
    /// @param param Output level setting
    /// @return The output level corresponding to param
    static constexpr level_t levelFromParam(param_t param);

private:
    // Operator settings - based on Dexy::Patches::OpParams but stored as
    // implementation-friendly types
    bool fixedFreq = false;             ///< Operator freq is fixed or set by pitch CV
    freqRatio_t freqRatio = freqRatio1; ///< Frequency ratio (only if fixedFreq = false)
    level_t outputLevel = max_level_t;  ///< Operator output level (nominal maximum)
    bool useEnvelope = true;            ///< Is outputLevel modulated by the envelope?
    param_t ampModSens = 0;             ///< Operator sensitivity to amplitude modulation

    SineWave::WaveGen sineWave;         ///< The Operator's sine wave oscillator
    Envelope env;                       ///< The Operator's envelope generator
};

}
