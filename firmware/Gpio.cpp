// GPIO pin definitions & initialization

namespace Dexy { namespace Gpio {

IN_FLASH("Gpio")
void init()
{
    // Set up a GPIO pin to drive an LED, so we have something to look at.
    // Make it a PWM pin so we can fade the LED.
    // NOTE: Not supported on some boards that don't have a built-in LED
    if constexpr (pinLed != pinNone) {
        constexpr unsigned pwmLedSlice = pwm_gpio_to_slice_num_constexpr(pinLed);
        static_assert(pwmTimerSlice != pwmLedSlice, "Two PWMs must use different slices");
        gpio_init(pinLed);
        gpio_set_function(pinLed, GPIO_FUNC_PWM);
        pwm_config config = pwm_get_default_config();
        pwm_config_set_wrap(&config, 65535u);
        pwm_config_set_clkdiv(&config, 1.0f);
        pwm_config_set_phase_correct(&config, false);
        pwm_init(pwmLedSlice, &config, true/*start*/);
    }
}

} } // namespace Gpio
