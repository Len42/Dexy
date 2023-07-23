#pragma once

namespace Dexy {

/// @brief Output data to MCP482x DAC via SPI
namespace SpiDac {

/// @brief DAC output data type
/// @details 16 bit unsigned value. If the DAC precision is less than 16 bits, the
/// bottom bits are ignored.
using dacdata_t = uint16_t;

/// @brief DAC output "zero" value is 0 V (ground) which is midway between the
/// min and max values.
constexpr dacdata_t dacdataZero = mid_value<SpiDac::dacdata_t>();

/// @brief Initialization - must be called at startup
void init();

/// @brief Set the next DAC output value; it will be output at the next timer interrupt
/// @param value 
void setOutput(dacdata_t value);

/// @brief Wait until the previous pending output value has been sent to the DAC
void waitForOutputSent();

/// @brief Timer interrupt handler - Output wave data to the DAC
void onOutputTimer();

} } // namespace SpiDac
