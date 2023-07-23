#pragma once

namespace Dexy {

/// @brief Tasks for testing & debugging, to run on core 0
namespace TestTasks {

/// @brief Output raw ADC readings
class OutputAdc : public Tasks::Task
{
public:
    unsigned intervalMicros() const override { return 500'000; }

    void init() override {}

    void execute() override
    {
        AdcInput::adcBuffer_t adcBufferT;
        AdcInput::getCurrentValues(&adcBufferT);
        dprintf("ADC values: %hu %hu %hu %hu\n",
            adcBufferT[0], adcBufferT[1], adcBufferT[2], adcBufferT[3]);
    }
};

/// @brief Test pitch CV adjustment and output averaged values
class PitchCv : public Tasks::Task
{
public:
    unsigned intervalMicros() const override { return 200'000; }

    void init() override {}

    void execute() override
    {
        constexpr int maxCount = 2000;
        int adcValue = AdcInput::getCurrentValue<Gpio::adcInputPitch>();
        // Adjust for ADC non-linearity
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
        adcValue += adcAdjust;
        total += adcValue;
        if (adcValue < adcMin)
            adcMin = adcValue;
        if (adcValue > adcMax)
            adcMax = adcValue;
        if (++count >= maxCount) {
            double average = double(total) / double(count);
            double diff = average - prevAverage;
            if (std::abs(diff) > 1) {
                printed = false;
            } else if (!printed) {
                printed = true;
                double cv = 0;
                constexpr double adcValueMin = 15;
                constexpr double adcValueMax = 4127.5;
                constexpr double cvMax = 10.0;
                cv = (average - adcValueMin) * cvMax / (adcValueMax - adcValueMin);
                dprintf("%.3f, %.3f, %d, %d, %d\n", average, cv, adcMin, adcMax, adcMax - adcMin);
            }
            prevAverage = average;
            count = 0;
            total = 0;
            adcMin = 9999;
            adcMax = 0;
        }
    }

    static inline int count = 0;
    static inline int total = 0;
    static inline int adcMin = 9999;
    static inline int adcMax = 0;
    static inline double prevAverage = 0;
    static inline bool printed = false;
};

/// @brief Test pitch CV adjustment and output histogram values
class PitchCvHisto : public Tasks::Task
{
public:
    unsigned intervalMicros() const override { return 1'000'000; }

    void init() override {}

    void execute() override
    {
        AdcInput::adcResult_t adcValue = AdcInput::getCurrentValue<Gpio::adcInputPitch>();
#if false // Adjust for ADC non-linearity
        int adcAdjust = 0;
        if (adcValue >= 512) {
            adcAdjust += 9;
            if (adcValue >= 1024) {
                adcAdjust += 1;
                if (adcValue >= 1536) {
                    adcAdjust += 8;
                    if (adcValue >= 2048) {
                        adcAdjust -= 3;
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
        adcValue -= (adcValue >> 7);
        adcValue += adcAdjust;
#endif
        ++histo[adcValue];
        constexpr unsigned maxCount = 10'000;
        if (++count >= maxCount) {
            count = 0;
            for (auto&& [value, num] : histo) {
                dprintf("%u, %u\n", value, num);
            }
            dputs("-----------");
        }
    }

    static inline unsigned count = 0;
    static inline std::map<AdcInput::adcResult_t, unsigned> histo;
};

/// @brief Test pitch CV running average
class PitchCvAverage : public Tasks::Task
{
public:
    static constexpr unsigned numAdcValues = 4;

    unsigned intervalMicros() const override { return 1'000'000; }

    void init() override
    {
        dputs("PitchCvAverage init");
        for (auto&& value : adcValues) {
            dprintf(" %u", value);
        }
        dputs("");
    }

    void execute() override
    {
        AdcInput::adcResult_t adcValue = AdcInput::getCurrentValue<Gpio::adcInputPitch>();
        total = total + adcValue - adcValues[index];
        adcValues[index] = adcValue;
        index = (index + 1) % 4;
        constexpr unsigned maxCount = 1000;
        if (++count >= maxCount) {
            count = 0;
            unsigned sum = std::accumulate(adcValues.begin(), adcValues.end(), 0);
            AdcInput::adcResult_t average1 = AdcInput::adcResult_t(total / numAdcValues);
            AdcInput::adcResult_t average2 = AdcInput::adcResult_t(sum / numAdcValues);
            dprintf("%u, %u, %u, %u", total, sum, average1, average2);
            for (auto&& value : adcValues) {
                dprintf(", %u", value);
            }
            dputs("");
            total = 0;
            adcValues.fill(0);
        }
    }

    static inline unsigned count = 0;
    static inline unsigned total = 0;
    static inline unsigned index = 0;
    static inline std::array<AdcInput::adcResult_t, numAdcValues> adcValues;
};

/// @brief Measure CPU temperature
class MonitorTemp : public Tasks::Task
{
public:
    unsigned intervalMicros() const override { return 1'000'000; }

    void init() override
    {
        dputs("MonitorTemp init");
    }

    void execute() override
    {
        AdcInput::adcResult_t tempAdc = AdcInput::getCurrentValue<Gpio::adcInputTemp>();
        float tempCel = 27 - (tempAdc * 3.3f/4095 - 0.706f) / 0.001721f;
        dprintf("temp = %u %.3f\n", tempAdc, tempCel);
    }
};

} } // namespace TestTasks
