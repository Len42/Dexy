// Defs - Global type & constant definitions

#pragma once

namespace Dexy {

/// @brief Parameter value in patches & operators - 10-bit unsigned
using param_t = uint16_t;

/// @brief Maximum valid value of a param_t
constexpr param_t max_param_t = 1023;

/// @brief Output of an operator/oscillator - 16-bit signed
using output_t = int16_t;

/// @brief Minimum valid value of an output_t
constexpr output_t min_output_t = std::numeric_limits<output_t>::min();

/// @brief Maximum valid value of an output_t
constexpr output_t max_output_t = std::numeric_limits<output_t>::max();

/// @brief Output level of an operator or envelope generator - 16-bit unsigned
using level_t = uint16_t;

/// @brief Maximum valid value of a level_t
constexpr level_t max_level_t = std::numeric_limits<level_t>::max();

/// @brief Current position in a lookup table or wavetable
/// @details This represents the phase of an oscillator or the progress of an
/// envelope, and also the wavetable increment which sets the frequency of a
/// waveform or the rate of an envelope.
///
/// This is a 24-bit fixed-point value: 9-bit integer + 15-bit fraction, stored
/// in a 32-bit unsigned. Only the 9 bits of the integer and the top few bits of
/// the fraction are used to index the wavetable and interpolate. The rest of
/// the fraction bits give precision when advancing through the wavetable.
/// @todo Better name for phase_t?
using phase_t = uint32_t;

/// @brief Number of significant bits in phase_t
constexpr unsigned cbitsPhase = 24;

/// @brief Number of integer bits in phase_t, used as an index for table lookup
constexpr unsigned cbitsLookupIndex = 9;

/// @brief Number of fraction bits in phase_t
constexpr unsigned cbitsLookupFraction = cbitsPhase - cbitsLookupIndex;

/// @brief Number of entries in a lookup table or wavetable
/// @details There is one extra entry to allow for interpolation
constexpr unsigned sizeLookupTable = (1 << cbitsLookupIndex) + 1;

/// @brief Modulation is a signed value added to the current phase value
using modulation_t = int32_t;

/// @brief Number of patches in a Dexy::Patches::PatchBank
constexpr unsigned numPatches = 32;

/// @brief Number of operators in a Dexy::Patches::Patch
constexpr unsigned numOperators = 6;

/// @brief Number of synthesis algorithms
constexpr unsigned numAlgorithms = 32;

/// @brief Frequency ratio defines a frequency as a multiple of the current
/// note's base frequency
/// @details Fixed-point value, 5-bit integer + 11-bit fraction
using freqRatio_t = uint16_t;

/// @brief freqRatio_t value representing a ratio of 1.0
constexpr freqRatio_t freqRatio1 = 1 << 11;  // 1:1 frequency ratio

/// @brief MIDI note number
/// @details Fixed-point value: 8-bit integer + 8-bit fraction.
/// Negative note numbers give low frequencies.
using midiNote_t = int16_t;

/// @brief midiNote_t difference value representing a semitone
constexpr midiNote_t midiNoteSemitone = 1 << 8;

/// @brief Maximum valid value of a midiNote_t
constexpr midiNote_t max_midiNote_t = 128 * midiNoteSemitone - 1;

}
