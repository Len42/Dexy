#pragma once

namespace Dexy {

/// @brief Table lookup and interpolation for waveforms and envelopes
/// @details Multiple WaveTable classes can be defined, each using a different lookup table.
/// @tparam VALUE_T Type of values in the wavetable
/// @tparam NUM_VALUES Number of values in the wavetable
/// @tparam FUNC_CALC1 Function/lambda to calculate one table entry
template<typename VALUE_T, std::size_t NUM_VALUES, VALUE_T FUNC_CALC1(std::size_t index, std::size_t numValues)>
class WaveTable
{
public:
    /// @brief Initialize the WaveTable class
    /// @details This does not populate the wavetable data - that is done at compile time.
    static void init();

    /// @brief Return the interpolated value at the given position in the wavetable,
    /// which is modified by a modulation value
    /// @details pCurrent is updated to the next position by adding increment, which
    /// controls the frequency/rate.
    /// pCurrent is a phase_t which includes the table index and a fractional value
    /// for interpolation.
    /// @param[inout] pCurrent Wavetable position as a fixed-point phase_t value
    /// @param increment phase_t value added to pCurrent to get the next position
    /// @param modulation Phase modulation value
    /// @return Interpolated wavetable value
    static VALUE_T lookupInterpolate(phase_t* pCurrent, phase_t increment, modulation_t modulation);
 
    /// @brief Return the interpolated value at the given position in the wavetable
    /// (without modulation)
    /// @details pCurrent is updated to the next position by adding increment, which
    /// controls the frequency/rate.
    /// pCurrent is a phase_t which includes the table index and a fractional value
    /// for interpolation.
    /// @param[inout] pCurrent Wavetable position as a fixed-point phase_t value
    /// @param increment phase_t value added to pCurrent to get the next position
    /// @return Interpolated wavetable value
    static VALUE_T lookupInterpolate(phase_t* pCurrent, phase_t increment)
    {
        return lookupInterpolate(pCurrent, increment, 0);
    }

private:
    /// @brief Pre-calculated wavetable data
    static constexpr DataTable<VALUE_T, NUM_VALUES, FUNC_CALC1> lookupTable = DataTable<VALUE_T, NUM_VALUES, FUNC_CALC1>();
};

}
