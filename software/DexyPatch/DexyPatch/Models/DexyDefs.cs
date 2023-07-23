using System;

namespace Dexy.DexyPatch.Models
{
    /// <summary>
    /// Definitions of values used in the Dexy firmware
    /// </summary>
    /// <remarks>
    /// These must match the corresponding C++ definitions.
    /// </remarks>
    public class DexyDefs
    {
        // It's too bad that we can't typedef aliases for integer types like in C.

        // param_t = UInt16
        // Parameter value in patches & operators - 10-bit unsigned

        /// <summary>Maximum valid value of a param_t</summary>
        public const UInt16 max_param_t = 1023;

        // output_t = Int16
        // Output of an operator/oscillator - 16-bit signed

        /// <summary>Minimum valid value of an output_t</summary>
        public const Int16 min_output_t = Int16.MinValue;
     
        /// <summary>Maximum valid value of an output_t</summary>
        public const Int16 max_output_t = Int16.MaxValue;

        // level_t = UInt16
        // Output level of an operator or envelope generator - 16-bit unsigned

        /// <summary>Maximum valid value of a level_t</summary>
        public const UInt16 max_level_t = UInt16.MaxValue;

        // phase_t = UInt32
        // Current position in a lookup table, representing the phase of an
        // oscillator or the progress of an envelope. Also the wavetable increment
        // which sets the frequency of a waveform or rate of an envelope.
        // 24-bit fixed-point value: 9-bit integer + 15-bit fraction, stored in a 32-bit unsigned.
        // Only the 9 bits of the integer and the top few bits of the fraction are used
        // to index the wavetable and interpolate. The rest of the fraction bits
        // give precision when advancing through the wavetable.

        /// <summary>Number of significant bits in phase_t</summary>
        public const UInt32 cbitsPhase = 24;

        /// <summary>Number of integer bits in phase_t, used as an index for table lookup</summary>
        public const UInt32 cbitsLookupIndex = 9;

        /// <summary>Number of fraction bits in phase_t</summary>
        public const UInt32 cbitsLookupFraction = cbitsPhase - cbitsLookupIndex;

        /// <summary>Number of patches in a <see cref="Dexy.DexyPatch.Models.PatchBank"/></summary>
        public const UInt32 numPatches = 32;

        /// <summary>Number of operators in a <see cref="Dexy.DexyPatch.Models.Patch"/></summary>
        public const UInt32 numOperators = 6;

        /// <summary>Number of synthesis algorithms</summary>
        public const UInt32 numAlgorithms = 32;

        // freqRatio_t = UInt16
        // Frequency ratio defines a frequency as a multiple of the current note's
        // base frequency.
        // Fixed-point value, 5-bit integer + 11-bit fraction

        /// <summary>freqRatio_t value representing a ratio of 1.0</summary>
        public const UInt16 freqRatio1 = 1 << 11;

        // midiNote_t = Int16
        // MIDI note number - Fixed-point value: 8-bit integer + 8-bit fraction.
        // Negative note numbers give low frequencies.

        /// <summary>midiNote_t difference value representing a semitone</summary>
        public const Int16 midiNoteSemitone = 1 << 8;

        /// <summary>Maximum valid value of a midiNote_t</summary>
        public const Int16 max_midiNote_t = 128 * midiNoteSemitone - 1;
    }
}
