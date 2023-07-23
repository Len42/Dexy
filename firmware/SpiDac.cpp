namespace Dexy { namespace SpiDac {

// Settings for MCP482x
static constexpr unsigned clkSpeed = 20000000;  ///< SPI clock speed
static constexpr unsigned bitDacAB = 15;        ///< bit to select DAC unit A or B
static constexpr unsigned bitDacGain = 13;      ///< bit to select gain: 0 = 2x, 1 = 1x
static constexpr unsigned bitDacShutdown = 12;  ///< shutdown bit - active low
static constexpr unsigned dacUnitA = 0;         ///< DAC unit A = 0
static constexpr unsigned dacUnitB = 1;         ///< DAC unit B = 1

/// @brief DAC command word - invariant part
/// @details Always has gain = 1x and shutdown = nope.
/// Gain must be set to 1x because the chip supply voltage (3.3V) is too low for 2x gain.
static constexpr uint16_t wDacCmdBase = bitmask(bitDacGain, bitDacShutdown);

/// @brief DAC output buffer
/// @details The output is stored in a buffer by setOutput() rather than being
/// passed to sendToDac() directly.
/// @todo Does this need to be protected by a CritSec?
/// I don't think it's necessary and I want to avoid the overhead.
static volatile dacdata_t output = 0;

/// @brief DAC output pending flag
/// @todo CritSec?
static volatile bool fOutputPending = false;

void init()
{
    [[maybe_unused]] unsigned clkSpeedActual = spi_init(Gpio::spiInstance, clkSpeed);
    //dprintf("SPI clock speed: %u\n", clkSpeedActual);
    spi_set_format(Gpio::spiInstance, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(Gpio::pinSpiSck, GPIO_FUNC_SPI);
    gpio_set_function(Gpio::pinSpiTx, GPIO_FUNC_SPI);
    //gpio_set_function(Gpio::pinSpiRx, GPIO_FUNC_SPI); // MISO not used
    gpio_init(Gpio::pinSpiCs1);
    gpio_set_dir(Gpio::pinSpiCs1, GPIO_OUT);
    gpio_put(Gpio::pinSpiCs1, 1);
    gpio_init(Gpio::pinSpiLdac);
    gpio_set_dir(Gpio::pinSpiLdac, GPIO_OUT);
    gpio_put(Gpio::pinSpiLdac, 0);
    setOutput(SpiDac::dacdataZero);
}

void setOutput(dacdata_t value)
{
    output = value;
    fOutputPending = true;
}

// TODO: documentation from here

/// @brief Get the pending DAC output value
/// @return 
static dacdata_t getOutput()
{
    return output;
}

/// @brief Is there a pending output value?
/// @return Yes or no
static bool isOutputPending()
{
    return fOutputPending;
}

/// @brief Reset the output-pending flag
static void resetOutputPending()
{
    fOutputPending = false;
}

void waitForOutputSent()
{
    while (fOutputPending) {
        // waiting...
    }
}

/// @brief Send the given output value to the DAC via SPI
/// @param dacData DAC output data
/// @param pinCS SPI CS pin for the DAC chip
/// @param dacUnit Which of two DAC units to use
static void sendToDac(dacdata_t dacData, unsigned pinCS, unsigned dacUnit)
{
    // De-assert LDAC (active low)
    gpio_put(Gpio::pinSpiLdac, 1);
    // Make up the SPI command word for the MCP4821/4822 DAC
    // dacData is the value to output - only the top 12 bits are used.
    uint16_t wSpiData = dacdata_t(wDacCmdBase | (dacUnit << bitDacAB) | (dacData >> 4));
    // Assert CS. Do nops before & after for timing.
    asm volatile("nop \n nop \n nop");
    gpio_put(pinCS, 0); // Active low
    asm volatile("nop \n nop \n nop");
    // Send 16-bit command word.
    spi_write16_blocking(Gpio::spiInstance, &wSpiData, 1);
    // De-assert CS.
    asm volatile("nop \n nop \n nop");
    gpio_put(pinCS, 1);
    asm volatile("nop \n nop \n nop");
    // Assert LDAC to latch the new value to the output.
    gpio_put(Gpio::pinSpiLdac, 0);
}

void onOutputTimer()
{
    // Check if the output data is ready (it should be!)
    if (!isOutputPending()) {
        // oh noes!
        Error::set<Error::Err::DataNotReady>();
    } else {
        // Get the data to be output from the buffer and mark the buffer empty
        // so the next value can be set as soon as it's ready.
        dacdata_t dacData = getOutput();
        resetOutputPending();

        // Send the output to the DAC (unit A)
        sendToDac(dacData, Gpio::pinSpiCs1, dacUnitA);

        // Set the LED brightness to indicate the output value
        // Adjust the value to give a better lightness curve
        if constexpr (Gpio::pinLed != Gpio::pinNone) {
            dacdata_t outputReduced = (dacData >> 8);
            uint16_t pwmLedLevel = outputReduced * outputReduced;
            if (pwmLedLevel >= 32768) {
                pwmLedLevel -= 32768;
            } else {
                pwmLedLevel = 0;
            }
            pwm_set_gpio_level(Gpio::pinLed, pwmLedLevel);
        }
    }
}

} } // namespace SpiDac
