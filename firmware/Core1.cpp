namespace Dexy { namespace Core1 {

[[noreturn]]
static void synthLoop();

constexpr unsigned gateInterruptFlags = (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);

[[noreturn]] IN_FLASH("Core1")
void main()
{
    dputs("Core 1 start");
    dassert(get_core_num() == 1, WrongCore);

    Lockout::initCore1();

    Synth::init();

    SpiDac::init();

    IrqDispatch::initCore1();

    // Set up a GPIO pin with an interrupt handler for the gate input
    gpio_init(Gpio::pinGateIn);
    gpio_set_dir(Gpio::pinGateIn, GPIO_IN);
    gpio_disable_pulls(Gpio::pinGateIn); // no pull-up or pull-down wanted on this pin
    gpio_set_irq_enabled(Gpio::pinGateIn, gateInterruptFlags, true);

    // Generate audio output samples, forever
    synthLoop();
}

/// @brief Main synthesizer loop
///
/// Generates audio samples which will be output by onTimerInterrupt()
[[noreturn]] __attribute__((noinline))
static void synthLoop()
{
    for (;;) {
        // Generate the next output value
        SpiDac::dacdata_t output =
            SpiDac::dacdata_t(int32_t(Synth::genNextOutput()) + SpiDac::dacdataZero);

        // Wait until the previous output sample has been consumed, then set
        // the new one to be output next. (see SpiDac::onOutputTimer)
        SpiDac::waitForOutputSent();
        SpiDac::setOutput(output);
    }
}

void onTimerInterrupt()
{
    // Output a waveform sample to the DAC
    SpiDac::onOutputTimer();
}

void onGateInterrupt(uint32_t events)
{
    dassert((events & gateInterruptFlags) != 0, WrongIrqEvent);
    if (events & GPIO_IRQ_EDGE_RISE) {
        Synth::gateStart();
    }
    if (events & GPIO_IRQ_EDGE_FALL) {
        Synth::gateStop();
    }
}

} } // namespace Core1
