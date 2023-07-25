cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# PICO_SDK_PATH is assumed to be set in the environment
#set(PICO_SDK_PATH "C:/Dev/Pico/pico-sdk")

include(pico_sdk_import.cmake)

set(PICO_SDK_VERSION_REQUIRED "1.4.0")
if (${PICO_SDK_VERSION_STRING} VERSION_LESS PICO_SDK_VERSION_REQUIRED)
    message(WARNING "Pico SDK version ${PICO_SDK_VERSION_REQUIRED}+ is required")
endif()

project(Dexy C CXX ASM)

# Set the type of the target dev board
#set(PICO_BOARD "adafruit_feather_rp2040")
set(PICO_BOARD "adafruit_kb2040")

# Initialise the Pico SDK
pico_sdk_init()

# Add executable
list(APPEND PROJ_CPP_FILES "${PROJECT_SOURCE_DIR}/main.cpp")
add_executable(Dexy ${PROJ_CPP_FILES} ssd1306.c)
pico_set_program_name(Dexy "Dexy")
#pico_set_program_version(Dexy "0.0") # No - this will be set by MakeVersionFile & Dexy

set(CMD_PYTHON "py") # for Windows
#set(CMD_PYTHON "/usr/local/bin/python3.11") # for macOS, Linux

# Update version info file from the latest git tag
list(APPEND VERSION_FILES "${PROJECT_SOURCE_DIR}/Version.h" "${PROJECT_SOURCE_DIR}/doc/Doxyfile.doxy")
list(TRANSFORM VERSION_FILES APPEND ".template" OUTPUT_VARIABLE VERSION_TEMPLATES)
target_sources(Dexy PRIVATE ${VERSION_FILES})
set(VERSION_TEMP_FILE "${PROJECT_BINARY_DIR}/version-temp")
set(VERSION_INFO_FILE "${PROJECT_BINARY_DIR}/version-info")
add_custom_target(MakeVersionFile
    COMMAND git describe --tags --always --dirty >${VERSION_TEMP_FILE}
    COMMAND ${CMD_PYTHON} ${PROJECT_SOURCE_DIR}/update-version-info.py
        ${VERSION_INFO_FILE}
        ${VERSION_TEMP_FILE}
)
add_dependencies(Dexy MakeVersionFile)
add_custom_command(
    OUTPUT ${VERSION_FILES}
    DEPENDS ${VERSION_INFO_FILE} ${VERSION_TEMPLATES}
    COMMAND ${CMD_PYTHON} ${PROJECT_SOURCE_DIR}/make-version-file.py
        ${VERSION_INFO_FILE}
        ${VERSION_FILES}
)

# Convert binary data files to C headers (until C++ gets #embed)
list(APPEND BINARY_FILES "${PROJECT_SOURCE_DIR}/../patches/default.dexy")
list(TRANSFORM BINARY_FILES APPEND ".h" OUTPUT_VARIABLE BINARY_INC_FILES)
target_sources(Dexy PRIVATE ${BINARY_INC_FILES})
add_custom_command(
    OUTPUT ${BINARY_INC_FILES}
    DEPENDS ${BINARY_FILES}
    COMMAND ${CMD_PYTHON} ${PROJECT_SOURCE_DIR}/make-binary-inc-file.py ${BINARY_FILES}
)

# Set compiler options
target_compile_options(Dexy PUBLIC "-O3")
# KLUDGE: -fwhole-program makes it do more inlining and fewer redundant out-of-line functions.
# Maybe LTO would work better? Unfortunately, -flto doesn't work with the Pi Pico SDK.
set_source_files_properties(${PROJ_CPP_FILES}
    PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wconversion -Wshadow -Wctad-maybe-unsupported -Wsuggest-attribute=noreturn -fwhole-program")
# Save compiler temp files. Assembler output will be in *.s.
target_compile_options(Dexy PUBLIC "-save-temps")
target_link_options(Dexy PRIVATE "LINKER:--no-warn-rwx-segments")

# Enable stdio via UART and/or USB
pico_enable_stdio_uart(Dexy 0)
pico_enable_stdio_usb(Dexy 1)

# Default stage-2 boot file (despite what it says in the Adafruit Feather board file)
set(PICO_DEFAULT_BOOT_STAGE2_FILE "${CMAKE_CURRENT_LIST_DIR}/boot2_w25qs080.S")

# Add link libraries
target_link_libraries(Dexy
    pico_stdlib
    pico_multicore
    hardware_interp
    hardware_spi
    hardware_i2c
    hardware_pwm
    hardware_adc
)

# Run executable from RAM instead of flash
set(COPY_TO_RAM 1)
target_compile_definitions(Dexy PUBLIC COPY_TO_RAM=${COPY_TO_RAM})
if (COPY_TO_RAM)
    pico_set_binary_type(Dexy copy_to_ram)
endif()

# Create a .uf2 executable file
pico_add_extra_outputs(Dexy)
