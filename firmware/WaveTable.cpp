namespace Dexy {

template<typename VALUE_T, std::size_t NUM_VALUES, VALUE_T FUNC_CALC1(std::size_t index, std::size_t numValues)>
void WaveTable<VALUE_T, NUM_VALUES, FUNC_CALC1>::init()
{
    // Requires a lookup table of the correct size
    static_assert(sizeof(VALUE_T) == 2, "Lookup value type must be 16 bits as currently coded");
    //static_assert(sizeLookupTable == 512+1, "sizeLookupTable must be 512+1 as currently coded");
    static_assert(cbitsPhase - cbitsLookupFraction == 9, "cbitsPhase and cbitsLookupFraction must match table size");
    static_assert(sizeof(phase_t) * CHAR_BIT >= cbitsPhase, "phase_t must be large enough to hold cbitsPhase");
}

template<typename VALUE_T, std::size_t NUM_VALUES, VALUE_T FUNC_CALC1(std::size_t index, std::size_t numValues)>
VALUE_T WaveTable<VALUE_T, NUM_VALUES, FUNC_CALC1>::
    lookupInterpolate(phase_t* pCurrent, phase_t increment, modulation_t modulation)
{
    // Find the nearest pair of values in the lookup table
    phase_t phase = *pCurrent + modulation;
    unsigned index = ((phase >> 15) % (sizeLookupTable-1));
    //unsigned index = ((phase >> 15) & 0x1FF); // if the above doesn't get optimized
    VALUE_T entry0 = lookupTable[index];
    VALUE_T entry1 = lookupTable[index+1];
    // Interpolate between the two table values
    VALUE_T value = entry0;
    value = VALUE_T((value + ((phase & 0x1000) ? entry1 : entry0)) / 2);
    value = VALUE_T((value + ((phase & 0x2000) ? entry1 : entry0)) / 2);
    value = VALUE_T((value + ((phase & 0x4000) ? entry1 : entry0)) / 2);
    // Increment to the next sample
    *pCurrent += increment;
    return value;
}

}
