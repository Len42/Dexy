namespace Dexy {

/// @brief Timeout for multicore_lockout_start_timeout_us()
constexpr unsigned timeoutUs = 1'000;

void Lockout::initCore1()
{
    dassert(get_core_num() == 1, WrongCore);
    multicore_lockout_victim_init();
}

Lockout::Lockout()
{
    // Lockout core 1 https://news.blr.com/app/uploads/sites/2/2019/11/Lockout-Tagout.jpg
    dassert(get_core_num() == 0, WrongCore);
    locked = multicore_lockout_start_timeout_us(timeoutUs);
    if (!locked) {
        Error::set<Error::Err::Lockout>();
    }
    // Disable interrupts
    savedInterrupts = save_and_disable_interrupts();
}

Lockout::~Lockout()
{
    // Restore interrupts
    restore_interrupts(savedInterrupts);
    // Unlock core 1
    if (locked) {
        dassert(get_core_num() == 0, WrongCore);
        if (!multicore_lockout_end_timeout_us(timeoutUs)) {
            Error::set<Error::Err::Lockout>();
        }
    }
}

}
