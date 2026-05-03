#pragma once

#include <stdint.h>

// Send a 4-bit input vector to the FPGA, return the classification result.
// Two-byte SPI transaction: TX = {input_nibble, 0x00}; RX = {x, result_byte}.
uint8_t bnn_infer(uint8_t input_nibble);
