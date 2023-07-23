#pragma once

namespace Dexy {
    
/// @brief Analog input using the built-in ADC
namespace AdcInput {

/// @brief Result of reading the ADC
using adcResult_t = uint16_t;

/// @brief Data buffer containing results of reading all the analog inputs
using adcBuffer_t = adcResult_t[Gpio::numAdcInputs];

/// @brief Initialize the ADC input channels - must be called at startup
void init();

/// @brief Read all the ADC inputs into an internal buffer
void readAll();

/// @brief Get a copy of the current contents of the ADC input buffer
/// @param[out] buf Output buffer to copy the ADC results into
void getCurrentValues(adcBuffer_t* buf);

/// @brief Get one of the current values from the ADC input buffer
/// @tparam adcInput Which ADC input to read
/// @return The last-read value from the given input
template<unsigned adcInput>
adcResult_t getCurrentValue();

/// @brief Calculate the wavetable increment value for a note pitch corresponding
/// to a given ADC input value
/// @param value ADC input value corresponding to a pitch
/// @return Wavetable increment value for the desired pitch
phase_t getIncrementForAdcValue(adcResult_t value);

/// @brief Calculate the timbre modulation value for a note pitch corresponding
/// to a given ADC input value
/// @param value ADC input value corresponding to the modulation amount
/// @return Modulation amount
output_t getTimbreModForAdcValue(adcResult_t value);

} } // namespace AdcInput
