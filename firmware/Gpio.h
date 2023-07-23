#pragma once

namespace Dexy {

/// @brief GPIO pin definitions & initialization
///
/// Pin assignments depend on which RP2040 module is used and on the PCB design.
/// Note that the PICO_DEFAULT_* definitions are different on different boards.
namespace Gpio {

/// @brief Initialization - must be called at startup
///
/// Most GPIO initialization is done in the appropriate modules.
/// This function is a catch-all for GPIO initialization that isn't done elsewhere.
void init();

/// @brief Not-a-pin used for features that are not supported on a particular board
constexpr unsigned pinNone = std::numeric_limits<unsigned>::max();

#pragma message "Using I/O pin definitions for board " PICO_BOARD

#if defined(ADAFRUIT_FEATHER_RP2040)
constexpr unsigned pinLed = PICO_DEFAULT_LED_PIN; ///< Built-in LED
constexpr unsigned pinGateIn = 24; ///< Input for gate signal
constexpr unsigned pinCore0Timer = 12; ///< Used in software to propagate timer interrupts to core 0
constexpr unsigned pinEncoderA = 10; ///< Input pin for the rotary encoder
constexpr unsigned pinEncoderB = 11; ///< Input pin for the rotary encoder
constexpr unsigned pinEncoderSw = 9; ///< Input pin for the rotary encoder
auto spiInstance = PICO_DEFAULT_SPI_INSTANCE; ///< SPI instance for the DAC
constexpr unsigned pinSpiSck = PICO_DEFAULT_SPI_SCK_PIN; ///< SPI pins for the DAC
constexpr unsigned pinSpiTx = PICO_DEFAULT_SPI_TX_PIN; ///< SPI pins for the DAC
//constexpr unsigned pinSpiRx = PICO_DEFAULT_SPI_RX_PIN; ///< SPI pins for the DAC (MISO not used)
constexpr unsigned pinSpiCs1 = 1; ///< SPI pins for the DAC
constexpr unsigned pinSpiLdac = 8; ///< DAC LDAC signal
constexpr auto i2cInstance = PICO_DEFAULT_I2C_INSTANCE; ///< I2C instance for the display
constexpr unsigned pinI2cSda = PICO_DEFAULT_I2C_SDA_PIN; ///< I2C pin for the display
constexpr unsigned pinI2cScl = PICO_DEFAULT_I2C_SCL_PIN; ///< I2C pin for the display
constexpr unsigned pwmTimerSlice = 0; ///< PWM slice for timer interrupt (no pin assigned)
constexpr unsigned adcInputPitch = 0; ///< ADC input channel for pitch CV
constexpr unsigned adcInputTimbre = 1; ///< ADC input channel for timbre CV

#elif defined(ADAFRUIT_KB2040)
constexpr unsigned pinLed = pinNone; ///< Built-in LED (not available on this board)
constexpr unsigned pinGateIn = 6; ///< Input for gate signal
constexpr unsigned pinCore0Timer = 21; ///< Used in software to propagate timer interrupts to core 0
constexpr unsigned pinEncoderA = 8; ///< Input pin for the rotary encoder
constexpr unsigned pinEncoderB = 9; ///< Input pin for the rotary encoder
constexpr unsigned pinEncoderSw = 7; ///< Input pin for the rotary encoder
auto spiInstance = PICO_DEFAULT_SPI_INSTANCE; ///< SPI instance for the DAC
constexpr unsigned pinSpiSck = PICO_DEFAULT_SPI_SCK_PIN; ///< SPI pins for the DAC
constexpr unsigned pinSpiTx = PICO_DEFAULT_SPI_TX_PIN; ///< SPI pins for the DAC
//constexpr unsigned pinSpiRx = PICO_DEFAULT_SPI_RX_PIN;SPI pins for the DAC (MISO not used)
constexpr unsigned pinSpiCs1 = 20; ///< SPI pins for the DAC
constexpr unsigned pinSpiLdac = 10; ///< DAC LDAC signal
constexpr auto i2cInstance = i2c1; ///< I2C instance for the display
constexpr unsigned pinI2cSda = 2; ///< I2C pin for the display
constexpr unsigned pinI2cScl = 3; ///< I2C pin for the display
constexpr unsigned pwmTimerSlice = 0; ///< PWM slice for timer interrupt (no pin assigned)
constexpr unsigned adcInputPitch = 0; ///< ADC input channel for pitch CV
constexpr unsigned adcInputTimbre = 1; ///< ADC input channel for timbre CV

#else
#error "I/O pins not defined for the selected board"
#endif

/// @brief The number of ADC inputs that are used
/// @details This many ADC input channels will be sampled, starting with channel 0.
constexpr unsigned numAdcInputs = 5; // <= 5

/// @brief ADC channel for the chip temperature sensor
constexpr unsigned adcInputTemp = 4;

/// @brief GPIO pin for the first ADC channel
constexpr unsigned pinAdcFirst = 26;

} } // namespace Gpio
