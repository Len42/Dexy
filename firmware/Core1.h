#pragma once

namespace Dexy {
    
/// @brief Main routine for core #1
namespace Core1 {

/// @brief Main routine for this core
[[noreturn]]
void main();

/// @brief Timer interrupt handler for this core
/// @details Outputs the last audio sample calculated by synthLoop()
void onTimerInterrupt();

/// @brief GPIO interrupt handler for the gate input
/// @param events 
void onGateInterrupt(uint32_t events);

} } // namespace Core1
