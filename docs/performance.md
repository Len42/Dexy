# RP2040 Firmware Performance Considerations

The [RP2040 microcontroller](https://www.raspberrypi.com/documentation/microcontrollers/rp2040.html) in this module has to perform several tasks, some of which have real-time performance requirements. The RP2040 is able to perform these tasks, but without a lot of slack, so I had to make sure the code was as efficient as possible. In addition to commonly-used techniques for writing performant code, I used some specific optimizations for this project.

The time-critical tasks are:
- Audio output - Audio samples _must_ be output at a fixed rate (specified to be 49152 Hz). Any variation in the output rate will sound bad.
- Analog CV inputs - Two analog control voltages must be sampled by the ADC at audio (or "near-audio") rate. The exact sample rate is less critical for the input than the
output - fast enough is fast enough.
- Gate input - There is a digital input for note-start and note-stop signals. These signals should be handled fairly promptly - within a millisecond or so is OK.

## Two CPU Cores

The CPU in the RP2040 is a dual-core ARM Cortex M0+. In this application, one core is dedicated to generating and outputing the audio samples. Everything else, including reading the ADC inputs and various lower-priority tasks, is done by the other CPU core.

Timer interrupts are used to write output samples to the DAC and read input samples from the ADC at regular intervals. Gate signals are handled by a GPIO interrupt.

## Fixed-Point Arithmetic

The Cortex M0+ CPU does not have hardware to do floating-point arithmetic. Performing floating-point operations in software is much slower than doing it in hardware - far too slow for this application. But digital audio synthesis has to do a lot of mathematical calculations involving real numbers (e.g. calculating sine waves), which are usually represented in computer software as floating-point numbers.

One way to do artithmetic quickly on an integer CPU is to use a fixed-point representation for numbers. Fixed-point arithmetic is just integer arithmetic except you assume that the decimal point (or rather, the binary point) is at a specific position in the middle of the number. Care must be taken to scale numbers by a power of 2 (a simple bit-shift operation) to make sure they line up when adding or multiplying.

## Lookup Tables

Another way to do arithmetic quickly is to avoid doing it at run time. I used pre-computed lookup tables to do complex calculations such as sine and exponential functions. This gives fairly accurate results very quickly.

Complete lookup tables for 16-bit or 32-bit quantities would be too large, so smaller tables are used (512 elements) with simple linear interpolation to get more accurate results.

## Flash Memory Problems

In the course of developing this firmware, I ran into a performance problem related to flash memory. Everything worked fine at first, but when I added some code to the firmware, it caused the time-critical audio generation code to run slower, to the point that it couldn't generate output samples fast enough to keep up with the 49 kHz output rate. Strangely, the code that I added only ran at startup - the time-critical code had not changed at all!

### Getting Technical

The RP2040 uses two types of memory:
- 264 kB of static RAM on the chip - fast but small
- a few megabytes of flash memory on an external IC - slower but much larger, and read-only during normal operation

Code and read-only data are stored in flash. Variable data is stored in RAM.

In my firmware, the time-critical program code and lookup tables were stored in flash memory (by the default behaviour of the C++ compiler). Even though flash is slower than RAM, I had done some benchmark testing to verify that the time-critical code ran more than fast enough to do its job.

Therefore it seemed strange that adding code _that only ran once at startup_ should cause the audio generation code to run more slowly. In fact, I was able to show that just adding a few kilobytes of _unused_ static data to the firmware ("`static int buf[1000];`") would make it run slowly - but only if that extra data was stored in flash, not if it was in RAM.

It turned out that this inconsistent behaviour was caused by the RP2040's internal cache for flash memory. This is a 16 kB block of RAM which is used to cache data that is read from flash, to speed up memory access when the same flash memory addresses are read repeatedly. As my firmware grew in size, and particularly because it uses some large lookup tables, the 16 kB cache was no longer big enough to hold all of the frequently-used code and data. Any change to the firmware can cause things to be stored at different addresses in flash, which changes which bytes get evicted from the cache as the program runs (due to the way memory addresses are mapped into the cache), which can cause some time-critical code or data bytes to be loaded (slowly) from flash instead of (quickly) from the RAM cache.

There is enough of a speed difference between on-chip RAM and external flash memory that poor caching behaviour was enough to make the time-critical code that calculates the audio output samples too slow to do its job.

### Solving The Problem

Simply put, if flash memory is too slow then don't use it.

The [Raspberry Pi Pico C/C++ SDK](https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf) provides a setting to force all of the firmware to be copied from flash to RAM at boot time so that all of the code and data are in RAM. The problem with this is that there is much less RAM (264 kB) than flash (a few MB).

There is also a way to specify that particular functions and data should be stored in RAM or flash. I tried specifying that only the code and data in time-critical execution paths should be in RAM, leaving everything else in flash, but that didn't quite work - it was still having trouble running fast enough. I think that there was still some time-critical code hiding somewhere in flash memory - probably the interrupt handlers in the Pi Pico SDK.

The final solution was to configure the settings so that everything is loaded into RAM by default, but code and data that are _not_ time-critical are marked to be loaded into flash memory, to minimize the amount of RAM used. This works well and there are no more glitches in the audio output.