#pragma once

namespace Dexy {

/// @brief Disable interrupts and stop core 1 while messing with flash
///
/// When writing to flash memory, it is necessary to disable interrupts and
/// stop the other core from running to ensure that flash is not being accessed.
/// This class wraps the multicore_lockout functions in the Pico SDK.
///
/// Usage
/// -----
/// @code
/// void core1Main()
/// {
///     Lockout::initCore1();
/// }
///
/// void core0WriteToFlash()
/// {
///     Lockout lockout; // disable interrupts and stop core 1 during flash programming
///     flash_range_erase(...);
///     flash_range_program(...);
/// }
/// @endcode
class Lockout
{
public:
    /// @brief Initialization - Must be called by core 1 at startup.
    static void initCore1();

    /// @brief Ctor (called on core 0) disables interrupts and pauses execution on core 1
    explicit Lockout();

    /// @brief Dtor unlocks core 1 and re-enables interrupts
    ~Lockout();

    // Move-only semantics
    Lockout(Lockout const&) = delete;
    Lockout& operator=(Lockout const&) = delete;
    Lockout(Lockout&&) noexcept = default;
    Lockout& operator=(Lockout&&) noexcept = default;

private:
    bool locked;                ///< Are interrupts and core 1 currently disabled?
    uint32_t savedInterrupts;   ///< Interrupt flags to be restored by ~Lockout()
};

}
