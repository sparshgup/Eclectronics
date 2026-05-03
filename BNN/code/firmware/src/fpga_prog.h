#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Initialise the GPIOs and SPI peripheral used for FPGA configuration and
// data exchange. Call once at startup before fpga_program().
void fpga_prog_init(void);

// Configure the FPGA over SPI in slave mode. Returns true if CDONE goes
// high within the timeout, false otherwise.
bool fpga_program(const uint8_t *bitstream, size_t length);

// Start a 12 MHz clock to the FPGA on PIN_FPGA_CLK using PWM.
void fpga_clock_start(uint32_t hz);
