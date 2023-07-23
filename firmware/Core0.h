#pragma once

namespace Dexy {

/// @brief Main routine for core 0
namespace Core0 {

/// @brief Main routine for this core
[[noreturn]]
void main();

/// @brief Timer interrupt handler for this core
/// @details Reads the analog inputs.
/// Timer frequency is the same as core 1.
void onTimerInterrupt();

} } // namespace Core0
