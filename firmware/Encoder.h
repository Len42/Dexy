#pragma once

namespace Dexy {

/// @brief Interface to a rotary encoder
/// @details Multiple instances of this class could be defined using different
/// GPIO pins, but for this module only a single instance is defined - see
/// getInstance().
class Encoder
{
public:
    /// @brief Initialize an encoder connected to the given GPIO pins
    /// @param pinAIn Encoder input pin A
    /// @param pinBIn Encoder input pin B
    /// @param pinSwitchIn Encoder pushbutton input pin
    void init(unsigned pinAIn, unsigned pinBIn, unsigned pinSwitchIn);

    /// @brief Get the change in the encoder's position since the last time this
    /// was called
    /// @return The change in encoder position. Positive for clockwise, negative
    /// for counter-clockwise.
    int getChange();

    /// @brief Check if the encoder's pushbutton switch has been pressed since
    /// the last time this was called
    /// @return true if pressed, false if not
    bool checkSwitch();

    /// @brief The GPIO pin to which the encoder's pin A is connected
    /// @return Pin number
    unsigned getPinA() const { return pinA; }
 
    /// @brief The GPIO pin to which the encoder's pin B is connected
    /// @return Pin number
    unsigned getPinB() const { return pinB; }

    /// @brief The GPIO pin to which the encoder's pushbutton is connected
    /// @return Pin number
    unsigned getPinSwitch() const { return pinSwitch; }

    /// @brief Interrupt handler for pin A
    /// @param events 
    void onEncoderAInterrupt(uint32_t events);

    /// @brief Interrupt handler for pin B
    /// @param events 
    void onEncoderBInterrupt(uint32_t events);

    /// @brief Interrupt handler for the pushbutton switch
    /// @param events 
    void onSwitchInterrupt(uint32_t events);

    /// @brief Get the single rotary encoder instance
    /// @return Encoder
    static Encoder& getInstance();

    /// @brief States for the state machine that tracks the encoder movements
    enum class State { start, cw1, plus, cw2, ccw1, minus, ccw2 };

private:
    void updateState();
    void pressSwitch();

private:
    unsigned pinA = 0;
    unsigned pinB = 0;
    unsigned pinSwitch = 0;
    bool switchPressed = false;
    int increment = 0;
    State state = State::start;
};

}
