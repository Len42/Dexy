namespace Dexy { namespace SineWave {

/// @brief Sine wavetable
using SineTable = WaveTable<output_t, sizeLookupTable,
    [](std::size_t index, std::size_t numValues) {
        constexpr output_t max = max_output_t;
        double phase = 2 * std::numbers::pi / (numValues-1) * index;
        double sine = std::sin(phase) * max;
        return output_t(std::round(sine));
    }>;

void init()
{
    static_assert(sizeof(modulation_t) == sizeof(phase_t));
    SineTable::init();
}

/// @brief Calculate the frequency (in Hz) of a MIDI note number
/// @details Note that this function takes a standard MIDI note number (max 127)
/// _not_ a midiNote_t (which is a fixed-point number with a fraction).
/// Negative note numbers are valid
/// @param note MIDI note number (int, unsigned, float, or double)
/// @return Frequency in Hz
static constexpr auto getHzForMidiNoteNumber(auto note)
    { return 440 * pow(2.0, (note - 69) / 12); }

constexpr phase_t getIncrementForMidiNote(midiNote_t note)
{
    // This function works for positive and negative values of note.
    double freqHz = SineWave::getHzForMidiNoteNumber(double(note) / double(midiNoteSemitone));
    return SineWave::getIncrementForHz(freqHz);
}

#if 0 // UNUSED
// midiNoteTable - MIDI note frequency table
// There is an extra entry at the end to help with interpolation.
// NOTE: This relies on std::pow() being declared constexpr,
// which it is in gcc but not in other compilers or the C++20 standard. :(
static constexpr unsigned sizeMidiNoteTable = 1024;
static constexpr phase_t midiNoteCalc(std::size_t index, [[maybe_unused]]std::size_t numValues)
{
    // Table size = 1024, number of MIDI notes = 128
    static_assert(sizeMidiNoteTable == 128 * 8);
    double note = index / 8.0;
    // Note #69 is A4 = 440 Hz
    return getIncrementForHz(getHzForMidiNoteNumber(note));
}
static constexpr DataTable<phase_t, sizeMidiNoteTable+1, midiNoteCalc>
    midiNoteTable;

constexpr phase_t getIncrementForMidiNoteFast(midiNote_t note)
{
    // NOTE: This version fails for note numbers out of range [0,128)
    // TODO: assertion or error checking for that
    unsigned index = (note >> 5) & 0x3FF;
    phase_t entry0 = midiNoteTable[index];
    phase_t entry1 = midiNoteTable[index+1];
    phase_t value = entry0;
    value = (value + ((note & 0x04) ? entry1 : entry0)) / 2;
    value = (value + ((note & 0x08) ? entry1 : entry0)) / 2;
    value = (value + ((note & 0x10) ? entry1 : entry0)) / 2;
    return value;
}
#endif

output_t WaveGen::genNextOutput(output_t modulation)
{
    // Modulation Note: 16-bit amplitude is converted to a signed 24-bit phase
    // modulation value.
    // TODO: Adjust the multiplication so that a full-scale amplitude value gives a
    // reasonable max modulation, while converting output_t (16 bits) to
    // modulation_t (24 bits).
    // DONE: Compared to dexed, << 10 (giving 26 bits signed) sounds about right
    // for max modulation with no AMS or key vel. AMS seems to subtract, not add;
    // and key velocity gives a max of about << 12. (Dexy won't work that way.)
    // These max mod depths do seem to be over-modulating, i.e. max mod depth is
    // more than one whole wave.

    // Update the wave state to the next sample, using table lookup & interpolation.
    // This is where the modulation value is added.
    return SineTable::lookupInterpolate(&phase, increment, modulation << 12);
}

} } // namespace SineWave
