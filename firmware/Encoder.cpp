namespace Dexy {

// Define a single rotary encoder.

/// @brief Define a single instance of the class.
static Encoder theEncoder;

Encoder& Encoder::getInstance() { return theEncoder; }

static constexpr unsigned encoderInterruptFlags = (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);

/// @brief Declare a CritSec to control access to the Encoder
using CritSecEncoder = CritSec<Encoder>;

IN_FLASH("Encoder")
void Encoder::init(unsigned pinAIn, unsigned pinBIn, unsigned pinSwitchIn)
{
    pinA = pinAIn;
    pinB = pinBIn;
    pinSwitch = pinSwitchIn;

    CritSecEncoder::init();

    // Set up GPIO pins for input from the rotary encoder.
    gpio_init(pinA);
    gpio_set_dir(pinA, GPIO_IN);
    gpio_pull_up(pinA);
    gpio_init(pinB);
    gpio_set_dir(pinB, GPIO_IN);
    gpio_pull_up(pinB);
    gpio_init(pinSwitch);
    gpio_set_dir(pinSwitch, GPIO_IN);
    gpio_pull_up(pinSwitch);

    // Initialize the encoder's current state
    updateState();

    // Enable interrupts.
    // Interrupt dispatch is done in IrqDispatch.
    gpio_set_irq_enabled(pinA, encoderInterruptFlags, true);
    gpio_set_irq_enabled(pinB, encoderInterruptFlags, true);
    gpio_set_irq_enabled(pinSwitch, encoderInterruptFlags, true);

    // There are some spurious interrupts on startup. Wait for them to be
    // handled and discard the results. #kludge
    sleep_us(100);
    getChange();
    checkSwitch();
    // Sometimes this has to be done multiple times? No idea why.
    // sleep_us(1000);
    // getChange();
    // checkSwitch();
    // sleep_us(1000);
}

IN_FLASH("Encoder")
int Encoder::getChange()
{
    CritSecEncoder cs;
    return getAndSet(increment, 0);
}

IN_FLASH("Encoder")
bool Encoder::checkSwitch()
{
    CritSecEncoder cs;
    return getAndSet(switchPressed, false);
}

/// @brief State table to handle quadrature encoder transitions
static constexpr Encoder::State stateTable[][2][2] = {
    /* start */ { { Encoder::State::start, Encoder::State::ccw1 }, { Encoder::State::cw1, Encoder::State::start } },
    /* cw1 */   { { Encoder::State::start, Encoder::State::start }, { Encoder::State::cw1, Encoder::State::plus } },
    /* plus */  { { Encoder::State::start, Encoder::State::cw2 }, { Encoder::State::cw1, Encoder::State::plus } },
    /* cw2 */   { { Encoder::State::start, Encoder::State::cw2 }, { Encoder::State::start, Encoder::State::plus } },
    /* ccw1 */  { { Encoder::State::start, Encoder::State::ccw1 }, { Encoder::State::start, Encoder::State::minus } },
    /* minus */ { { Encoder::State::start, Encoder::State::ccw1 }, { Encoder::State::ccw2, Encoder::State::minus } },
    /* ccw2 */  { { Encoder::State::start, Encoder::State::start }, { Encoder::State::ccw2, Encoder::State::minus } }
};

/// @brief Update the encoder's current state and record incremental changes.
/// @details Called from the interrupt handlers.
void Encoder::updateState()
{
    int A = !gpio_get(pinA); // negated because the switches pull to ground
    int B = !gpio_get(pinB);
    State statePrev = state;
    state = stateTable[int(state)][A][B];
    // Increment/decrement the count when passing through state "plus" or "minus"
    // in the correct direction
    int change = 0;
    if (state == State::plus && statePrev == State::cw1)
      change = +1;
    else if (state == State::minus && statePrev == State::ccw1)
      change = -1;
    CritSecEncoder cs;
    increment += change;
}

/// @brief Record a button press.
/// @details Called from the interrupt handler.
void Encoder::pressSwitch()
{
    CritSecEncoder cs;
    switchPressed = true;
}

void Encoder::onEncoderAInterrupt(uint32_t events)
{
    dassert((events & encoderInterruptFlags) != 0, WrongIrqEvent);
    updateState();
}

void Encoder::onEncoderBInterrupt(uint32_t events)
{
    dassert((events & encoderInterruptFlags) != 0, WrongIrqEvent);
    updateState();
}

void Encoder::onSwitchInterrupt(uint32_t events)
{
    dassert((events & encoderInterruptFlags) != 0, WrongIrqEvent);
    if (!gpio_get(pinSwitch)) // negated because the switch pulls to ground
        pressSwitch();
}

} // namespace Encoder
