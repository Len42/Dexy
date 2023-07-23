#pragma once

namespace Dexy {

/// @brief FM synth engine
namespace Synth {

/// @brief Initialization - must be called at startup
void init();

/// @brief Load the given patch from the current patchbank
/// @details The patch is loaded asynchronously after this function returns.
/// @param i Patch index in the patchbank
/// @see Dexy::Patches::Patch Dexy::Patches::PatchBank
void loadPatch(unsigned i);

/// @brief Get the index of the currently-playing patch in the patchbank
/// @return Patch index
unsigned getCurrentPatchNum();

/// @brief Get the name of the current patch
/// @return Patch name as a fixed-length array (not 0-terminated)
const Patches::patchName_t& getCurrentPatchName();

/// @brief Index of the patch that is initially selected at startup
constexpr unsigned initialPatch = 2;

/// @brief Set the pitch of the note being played
/// @details Sets the frequency of all operators based on the given pitch, which
/// is usually derived from a CV input.
/// @param pitch Phase increment corresponding to the note pitch
/// @see AdcInput::getIncrementForAdcValue() SineWave::getIncrementForMidiNote() 
void setNotePitch(phase_t pitch);

/// @brief Set the timbre modulation value which affects the amplitudes of operators
/// @param value 
void setTimbreMod(output_t value);

/// @brief Gate start signal has been received - Start playing a note
void gateStart();

/// @brief Gate stop signal has been received - Stop playing the note
void gateStop();

/// @brief Generate the next audio output sample to be output
/// @return Output value
inline output_t genNextOutput();

} } // namespace Synth
