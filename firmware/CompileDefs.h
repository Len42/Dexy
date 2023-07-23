/// @brief Include all the .cpp files into one file and compile them in one unit
#define COMPILE_MONOLITHIC

/// @brief Functions & data that are not time-critical should be marked to load
/// into flash memory
/// @details Usage:
/// @code
/// IN_FLASH("Envelope")
/// void Envelope::init()
/// {
/// ...
/// }
/// @endcode
/// The group name is required, and should be based on the class or namespace
/// name. (It's not possible to use a single group name for everything marked
/// IN_FLASH due to linker behaviour.)
#define IN_FLASH(group) __in_flash(group)

#if !(defined(COPY_TO_RAM) && COPY_TO_RAM)
    #error "Must be compiled with pico_set_binary_type(Dexy copy_to_ram)"
#else
    #pragma message "Code & data will be loaded in RAM by default"
#endif
