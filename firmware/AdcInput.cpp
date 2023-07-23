namespace Dexy { namespace AdcInput {

constexpr unsigned maskAdcInputs = mask_low_bits(Gpio::numAdcInputs);

constexpr unsigned maxAdcValue = 4095;

/// @brief ADC capture buffer
static adcBuffer_t adcBuffer;

/// @brief Critical section to protect adcBuffer
using CritSecAdcBuffer = CritSec<adcBuffer_t>;

/// @brief Adjust for ADC non-linearity by fudging the ADC input value
/// @param adcValue Analog input value
/// @return Adjusted analog value
/// @note See RP2040 Datasheet section 4.9.4 and errata E11
static constexpr int linearizeAdcInput(int adcValue)
{
    int adcAdjust = 0;
    // Calculate the accumulated differential error
    if (adcValue >= 512) {
        adcAdjust += 9;
        if (adcValue >= 1024) {
            adcAdjust += 1;
            if (adcValue >= 1536) {
                adcAdjust += 8;
                if (adcValue >= 2048) {
                    adcAdjust += -3;
                    if (adcValue >= 2560) {
                        adcAdjust += 8;
                        if (adcValue >= 3072) {
                            adcAdjust += 1;
                            if (adcValue >= 3584) {
                                adcAdjust += 9;
                            }
                        }
                    }
                }
            }
        }
    }
    // Subtract an overall linear correction
    adcAdjust -= (adcValue >> 7);
    return adcValue + adcAdjust;
}

/// @brief Helper for initializing adcToIncrementMap
/// @param index Lookup table index corresponding to a pitch CV
/// @return Wavetable increment value for the specified pitch CV
/// @note This relies on math functions (e.g. std::pow()) being declared
/// constexpr, which they are in gcc but not in other compilers or the
/// C++20/23 standard. :(
static constexpr auto adcToIncrement(std::size_t index, std::size_t /*unused*/)
{
    // Table is indexed by the ADC input value, representing pitch CV
    int adcValue = index;
    // ADC input adjustment
    adcValue = linearizeAdcInput(adcValue);
    // ADC input to pitch CV
    // NOTE: ADC might not read all the way up to 3.3V (I measured 3.275V on mine).
    // Set CV voltage divider and adcValueMax appropriately.
    constexpr double adcValueMin = 26; //17;        // ADC reading with 0V input
    constexpr double adcValueMax = 3626; //4128.4;  // ADC reading with cvMax input
    constexpr double cvMax = 9.0; // 10.0;          // Max CV used for calibration
    double cv = (adcValue - adcValueMin) * cvMax / (adcValueMax - adcValueMin);
    // pitch CV to frequency
    constexpr double freqHzBase = 16.3516;
    constexpr double freqHzAdj = 0; // 0.15;
    double freqHz = (freqHzBase + freqHzAdj) * std::pow(2.0, cv);
    // frequency to wavetable increment
    double waveIncrement = SineWave::getIncrementForHz(freqHz);
    return phase_t(std::round(waveIncrement));
}

/// @brief Lookup table to map an ADC input value to a note pitch, expressed as
/// a wavetable increment value (phase_t).
static constexpr DataTable<phase_t, maxAdcValue+1, adcToIncrement> adcToIncrementMap;

/// @brief Helper for initializing adcToTimbreModMap
/// @param index Lookup table index corresponding to a timbre mod CV
/// @return Wavetable increment value for the specified CV
static constexpr auto adcToTimbreMod(std::size_t index, std::size_t /*unused*/)
{
    int adcValue = index;
    adcValue = linearizeAdcInput(adcValue);
    constexpr double adcValueMin = 31.5;    // ADC min reading
    constexpr double adcValueMax = 4007.3;  // ADC max reading
    double out = (adcValue - adcValueMin) / (adcValueMax - adcValueMin)
        * (max_output_t - min_output_t) + min_output_t;
    out = std::min(std::max(out, double(min_output_t)), double(max_output_t));
    return output_t(out);
}

/// @brief Lookup table to map an ADC input value to a timbre modulation value
static constexpr DataTable<output_t, maxAdcValue+1, adcToTimbreMod> adcToTimbreModMap;

IN_FLASH("AdcInput")
void init()
{
    static_assert(Gpio::numAdcInputs <= NUM_ADC_CHANNELS);
    static_assert(Gpio::adcInputTemp < Gpio::numAdcInputs);
    static_assert(std::size(adcBuffer) == Gpio::numAdcInputs);

    adc_init();
    for (unsigned i = 0; i < Gpio::numAdcInputs; ++i) {
        if (i == Gpio::adcInputTemp)
            adc_set_temp_sensor_enabled(true);
        else
            adc_gpio_init(Gpio::pinAdcFirst + i);
    }
    // Round-robin ADC input to sample all the input channels
    adc_set_round_robin(maskAdcInputs);

    CritSecAdcBuffer::init();
}

void readAll()
{
    // All of the ADC input channels will be read, including the temperature sensor.
    // TODO: Set Gpio::numAdcInputs to only read as many ADC channels as required.
    // The ADC is set to round-robin so multiple reads come from successive channels.
    adcBuffer_t adcBufferT;
    adc_select_input(0);
    for (auto&& val : adcBufferT) {
        val = adc_read();
    }
    {
        CritSecAdcBuffer critSec;
        std::copy(std::begin(adcBufferT), std::end(adcBufferT), std::begin(adcBuffer));
    }
}

void getCurrentValues(adcBuffer_t* buf)
{
    CritSecAdcBuffer critSec;
    std::copy(std::begin(adcBuffer), std::end(adcBuffer), std::begin(*buf));
}

template<unsigned adcInput>
adcResult_t getCurrentValue()
{
    static_assert(adcInput < Gpio::numAdcInputs);
    CritSecAdcBuffer critSec;
    return adcBuffer[adcInput];
}

phase_t getIncrementForAdcValue(adcResult_t value)
{
    assert(value <= maxAdcValue);
    return adcToIncrementMap[value];
}

output_t getTimbreModForAdcValue(adcResult_t value)
{
    assert(value <= maxAdcValue);
    return adcToTimbreModMap[value];
}

} } // namespace AdcInput
