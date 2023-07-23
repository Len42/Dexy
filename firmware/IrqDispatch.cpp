namespace Dexy { namespace IrqDispatch {

static void onGpioInterrupt0(uint pin, uint32_t events);
static void onGpioInterrupt1(uint pin, uint32_t events);
static void onTimerInterrupt1();

IN_FLASH("IrqDispatch")
void initCore0()
{
    // Initialize the overall GPIO interrupt handler
    gpio_set_irq_callback(onGpioInterrupt0);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

IN_FLASH("IrqDispatch")
void initCore1()
{
    // Initialize the overall GPIO interrupt handler
    gpio_set_irq_callback(onGpioInterrupt1);
    irq_set_enabled(IO_IRQ_BANK0, true);

    // Set up a handler for PWM timer interrupts, running at the sample rate.
    // These settings must match the sample rate defined by SineWave::freqSample
    // as closely as possible.
    // PWM settings for freqSample = 49152 Hz (actually comes out to 49152.13 Hz):
    constexpr unsigned pwmWrap = 1564;
    constexpr uint8_t pwmClkDivInt = 1;
    constexpr uint8_t pwmClkDivFrac = 10;
    constexpr unsigned pwmPhaseCorrect = false;
    pwm_clear_irq(Gpio::pwmTimerSlice);
    pwm_set_irq_enabled(Gpio::pwmTimerSlice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, onTimerInterrupt1);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    // Initialize a PWM slice but don't connect it to a particular GPIO pin.
    // It's only used for its interrupts.
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, pwmWrap);
    pwm_config_set_clkdiv_int_frac(&config, pwmClkDivInt, pwmClkDivFrac);
    pwm_config_set_phase_correct(&config, pwmPhaseCorrect);
    pwm_init(Gpio::pwmTimerSlice, &config, /*start*/ true);
}

/// @brief GPIO interrupt handler/dispatcher for core 0
/// @param pin The pin that generated the interrupt
/// @param events The interrupt event(s)
static void onGpioInterrupt0(uint pin, uint32_t events)
{
    dassert(get_core_num() == 0, WrongCore);
    if (pin == Gpio::pinCore0Timer) {
        // Timer interrupt for core 0
        // #kludge This "timer" interrupt is actually a GPIO interrupt software-
        // triggered by core 1 because we can't have both cores handling the same
        // PWM timer interrupt.
        dassert((events & GPIO_IRQ_EDGE_RISE) != 0, WrongIrqEvent);
        Core0::onTimerInterrupt();
    } else {
        // Rotary encoder interrupts
        Encoder& enc = Encoder::getInstance();
        if (pin == enc.getPinA()) {
            enc.onEncoderAInterrupt(events);
        } else if (pin == enc.getPinB()) {
            enc.onEncoderBInterrupt(events);
        } else if (pin == enc.getPinSwitch()) {
            enc.onSwitchInterrupt(events);
        } else {
            Error::set<Error::Err::WrongIrqGpio>();
        }
    }
}

/// @brief GPIO interrupt handler/dispatcher for core 1
/// @param pin The pin that generated the interrupt
/// @param events The interrupt event(s)
static void onGpioInterrupt1(uint pin, uint32_t events)
{
    dassert(get_core_num() == 1, WrongCore);
    dassert(pin == Gpio::pinGateIn, WrongIrqGpio);
    // Gate interrupt handler
    Core1::onGateInterrupt(events);
}

/// @brief PWM timer interrupt handler for core 1
static void onTimerInterrupt1()
{
    // Clear the interrupt flag here, for consistency with the GPIO interrupt handlers
    // which have the interrupt flag cleared automatically by the SDK.
    pwm_clear_irq(Gpio::pwmTimerSlice);
    dassert(get_core_num() == 1, WrongCore);
    // Call the handler on core 1
    Core1::onTimerInterrupt();
    // Send an interrupt to core 0 (because we can't have both cores handling
    // a single PWM interrupt).
    gpio_set_irqover(Gpio::pinCore0Timer, GPIO_OVERRIDE_HIGH);
    gpio_set_irqover(Gpio::pinCore0Timer, GPIO_OVERRIDE_LOW);
}

} } // namespace IrqDispatch
