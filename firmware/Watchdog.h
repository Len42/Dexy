// Watchdog Task - Report an error if tasks get backlogged

#pragma once

namespace Dexy {

/// @brief Task to report an error if tasks get backlogged
/// @details This Task should be last in the taskList.
/// @todo Add a hardware watchdog (reset in petTheDog()) to catch hard hangs
/// @see Core0::taskList
class Watchdog : public Tasks::Task
{
public:
    static constexpr unsigned taskInterval = 100'000;
    unsigned intervalMicros() const override { return taskInterval; }

    IN_FLASH("Watchdog")
    void init() override { petTheDog(); }

    IN_FLASH("Watchdog")
    void execute() override
    {
        if (timeIsReached(get_absolute_time(), tTimeout)) {
            // barkbarkbark
            Error::set<Error::Err::Watchdog>();
        }
        petTheDog();
    }

    /// @brief Reset the watchdog timeout
    /// @details This is called in execute() but long-running tasks should
    /// call it explicitly.
    static void petTheDog() { tTimeout = make_timeout_time_us(timeoutInterval); }

private:
    /// @brief Watchdog timeout interval
    static  constexpr unsigned timeoutInterval = 2 * taskInterval;

    /// @brief Timeout deadline
    static inline absolute_time_t tTimeout = from_us_since_boot_constexpr(0);
};

}
