namespace Dexy { namespace Core0 {

/// @brief Task to report any errors that have been logged
/// @note DEBUG_CHECK_ERRORS must be set for any errors to be logged
class ReportErrorsTask : public Tasks::Task
{
public:
    unsigned intervalMicros() const override { return 1'000'000; }
    void IN_FLASH("Core0") init() override {}
    void IN_FLASH("Core0") execute() override {
        if (Error::anySet()) {
            Error::dumpAllSetErrors();
            Error::clearAll();
        }
    }
};

/// @brief Define the list of tasks to execute
/// @details Any tasks not currently required (e.g. for debugging) can be commented out.
/// @see Tasks::TaskList
IN_FLASH("taskList")
static constexpr Tasks::TaskList<
    ReportErrorsTask,
    //TestTasks::OutputAdc,
    //TestTasks::PitchCv,
    //TestTasks::PitchCvHisto,
    //TestTasks::PitchCvAverage,
    //TestTasks::MonitorTemp,
    SerialIO::SerialIOTask,
    UI::UITask,
    Watchdog // should be last
> taskList;

// main

[[noreturn]] IN_FLASH("Core0")
void main()
{
    dputs("Core 0 start");
    dassert(get_core_num() == 0, WrongCore);

    AdcInput::init();

    IrqDispatch::initCore0();

    // Enable timer interrupt
    // #kludge This "timer" interrupt is actually a GPIO interrupt software-
    // triggered by core 1 because we can't have both cores handling the same
    // PWM timer interrupt.
    gpio_init(Gpio::pinCore0Timer);
    gpio_set_dir(Gpio::pinCore0Timer, GPIO_IN);
    gpio_pull_down(Gpio::pinCore0Timer);
    gpio_set_irq_enabled(Gpio::pinCore0Timer, GPIO_IRQ_EDGE_RISE, true);

    Encoder::getInstance().init(Gpio::pinEncoderA, Gpio::pinEncoderB, Gpio::pinEncoderSw);

    Display::init();

    // Initialize all the Tasks and run them forever
    taskList.initAll();
    while (true) {
        taskList.runAll();
    }
}

void onTimerInterrupt()
{
    // Handle the analog CV inputs
    AdcInput::readAll();
    static AdcInput::adcBuffer_t adcBuf;
    AdcInput::getCurrentValues(&adcBuf);
    AdcInput::adcResult_t adcPitch = adcBuf[Gpio::adcInputPitch];
    // Average adcPitch with the previous value to provide some extra filtering
    static AdcInput::adcResult_t adcPitchPrev = 0;
    adcPitchPrev = getAndSet(adcPitch, AdcInput::adcResult_t((adcPitch + adcPitchPrev) / 2));
    Synth::setNotePitch(AdcInput::getIncrementForAdcValue(adcPitch));
    Synth::setTimbreMod(AdcInput::getTimbreModForAdcValue(adcBuf[Gpio::adcInputTimbre]));
    // TODO: set level & rate scaling values here?
}

} } // namespace Core0
