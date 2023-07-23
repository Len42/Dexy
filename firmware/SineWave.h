#pragma once

namespace Dexy {

/// @brief Sine wave generation by table lookup
namespace SineWave {

/// @brief Waveform frequency
/// @details This is a 32-bit fixed-point value: 17-bit integer + 15-bit fraction,
/// i.e. frequency = Hz << 15.
using freq_t = uint32_t;

/// @brief Number of fraction bits in freq_t
constexpr unsigned cbitsFreqFraction = 15;

/// @brief Maximum frequency in Hz
constexpr unsigned freqHzMax = 32767;

/// @brief Maximum freq_t value
constexpr freq_t freqMax = freqHzMax << cbitsFreqFraction;

/// @brief Sample rate for waveform generation in Hz
constexpr unsigned freqSample = 49152;

/// @brief Initialization - must be called at startup
void init();

/// @brief Calculate the wavetable increment that gives a specified frequency
/// @param freq Frequency (freq_t)
/// @return Wavetable increment value (phase_t)
constexpr phase_t getIncrementForFrequency(freq_t freq)
{
    // Optimize this calculation to avoid 64-bit arithmetic by factoring freqSample
    // into a large power of 2 and a small number.
    constexpr unsigned cbitsFreqSampleShift = bits_in_num(freqSample) - 2;
    constexpr unsigned div = freqSample >> cbitsFreqSampleShift;
    static_assert(cbitsFreqSampleShift > cbitsLookupIndex, "cbitsFreqSampleShift must be > cbitsLookupIndex");
    static_assert((1 << cbitsFreqSampleShift) * div == freqSample, "cbitsFreqSampleShift must be small enough to not lose precision");
    return (freq >> (cbitsFreqSampleShift - cbitsLookupIndex)) / div;
}

/// @brief Convert a frequency in Hertz to a freq_t value
/// @param freqHz Frequency in Hz (unsigned, float, or double)
/// @return Frequency as a freq_t
constexpr freq_t getFreqValueForHz(auto freqHz)
    { return unsigned(freqHz * (1 << cbitsFreqFraction)); }

/// @brief Calculate the wavetable increment that gives a specified frequency in Hz
/// @param freqHz Frequency in Hz (unsigned, float, or double)
/// @return Wavetable increment value (phase_t)
constexpr phase_t getIncrementForHz(auto freqHz)
    { return getIncrementForFrequency(getFreqValueForHz(freqHz)); }

/// @brief Calculate the wavetable increment that gives a specified MIDI note
/// @details This function works for positive and negative values of note.
/// @param note midiNote_t
/// @return Wavetable increment value (phase_t)
constexpr phase_t getIncrementForMidiNote(midiNote_t note);

/// @brief Sine wave generator
/// @details There can be multiple instances of WaveGen running at different
// frequencies, but they all share the same wavetable.
class WaveGen
{
public:
    /// @brief Set the phase increment value, which controls the frequency
    /// @param value 
    void setIncrement(phase_t value) { increment = value; }

    /// @brief Generate the next wave sample
    /// @param modulation Phase modulation value 
    /// @return output_t
    output_t genNextOutput(output_t modulation);

    /// @brief Reset to the start of the wavetable
    void reset() { phase = 0; }

private:
    phase_t phase = 0; ///< Current phase of the wave (used to an index into the wavetable)
    phase_t increment = 1; ///< Amount to increment phase at each step (related to frequency)
};

} } // namespace SineWave
