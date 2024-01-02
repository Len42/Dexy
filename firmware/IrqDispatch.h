#pragma once

namespace Dexy {

/// @brief Interrupt handlers and dispatching
namespace IrqDispatch {

/// @brief Set up a handler/dispatcher for GPIO interrupts handled by core 0.
/// @details Specific pin interrupts are enabled in the appropriate places.
/// Must be called by core 0 at startup.
void initCore0();

/// @brief Set up a handler/dispatcher for GPIO and PWM interrupts handled by core 1.
/// @details Specific pin interrupts are enabled in the appropriate places.
/// Must be called by core 1 at startup.
void initCore1();

} } // namespace IrqDispatch
