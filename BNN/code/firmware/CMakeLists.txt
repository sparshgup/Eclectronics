cmake_minimum_required(VERSION 3.13)

# Pull in pico-sdk via the standard import shim. Set PICO_SDK_PATH in your
# environment, or keep pico_sdk_import.cmake next to this file.
include(pico_sdk_import.cmake)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico CACHE STRING "Board type")

project(bnn_firmware C CXX ASM)

pico_sdk_init()

add_executable(bnn
    src/main.c
    src/fpga_prog.c
    src/spi_data.c
    src/usb_serial.c
    src/led.c
)

target_include_directories(bnn PRIVATE
    include
    src
)

target_link_libraries(bnn
    pico_stdlib
    hardware_spi
    hardware_pwm
    hardware_gpio
    hardware_clocks
)

# USB CDC stdio, disable UART stdio (UART pins go to the FPGA).
pico_enable_stdio_usb(bnn 1)
pico_enable_stdio_uart(bnn 0)

pico_add_extra_outputs(bnn)

# Convenience: regenerate include/bitstream.h from the FPGA build output.
add_custom_target(bitstream
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/bin2c.py
            ${CMAKE_CURRENT_SOURCE_DIR}/../fpga/build/bnn.bin
            ${CMAKE_CURRENT_SOURCE_DIR}/include/bitstream.h
    COMMENT "Embedding fpga/build/bnn.bin into include/bitstream.h"
)
