// Dexy main program

#include "Dexy.h"

#pragma message "Pico SDK " PICO_SDK_VERSION_STRING ", GCC " __VERSION__

/// @brief Main entry point
IN_FLASH("main")
int main()
{
    stdio_init_all();
    // KLUDGE: Wait to give time for USB comm port to appear.
    sleep_ms(500);
    dprintf("\nDexy version %s %s\n", Dexy::VersionInfo::getName(), Dexy::VersionInfo::date);

    Dexy::Patches::init();
    Dexy::Gpio::init();
    Dexy::Defer::init();

    // Start up the other core
    multicore_launch_core1(Dexy::Core1::main);

    // Run this core's process
    Dexy::Core0::main();

    __builtin_unreachable();
}

#ifdef COMPILE_MONOLITHIC
// .cpp files are all included here and compiled in one big pile.
#include "Error.cpp"
#include "Lockout.cpp"
#include "Flash.cpp"
#include "Patches.cpp"
#include "PatchChanges.cpp"
#include "Gpio.cpp"
#include "WaveTable.cpp"
#include "SineWave.cpp"
#include "AdcInput.cpp"
#include "Envelope.cpp"
#include "Operator.cpp"
#include "SpiDac.cpp"
#include "Synth.cpp"
#include "SerialIO.cpp"
#include "Display.cpp"
#include "Encoder.cpp"
#include "UI.cpp"
#include "IrqDispatch.cpp"
#include "Core0.cpp"
#include "Core1.cpp"
#endif // COMPILE_MONOLITHIC
