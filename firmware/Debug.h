// Debugging Flags

#pragma once

/// @brief Set or unset DEBUG_MORE to get more or less debugging code
#define DEBUG_MORE

// These ones are always on (or are they??):

/// @brief Log and report errors
#define DEBUG_CHECK_ERRORS

// The following are only turned on if DEBUG_MORE is set:
#ifdef DEBUG_MORE

/// @brief Send debugging output messages over the USB serial connection
#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#define dputs(str) puts(str)
#define dprintf(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define dputs(str)
#define dprintf(fmt, ...)
#endif

/// @brief Print debugging messages when patch updates are handled
#undef DEBUG_PRINT_PATCH_UPDATES

/// @brief LFO to generate timbre modulation for testing
#undef DEBUG_TEST_LFO

/// @brief Print synth algorithm definitions to serial output
#undef DEBUG_DUMP_ALGORITHMS

#endif // DEBUG_MORE
