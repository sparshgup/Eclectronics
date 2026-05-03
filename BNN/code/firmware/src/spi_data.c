#include "spi_data.h"
#include "board.h"

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

uint8_t bnn_infer(uint8_t input_nibble) {
    uint8_t tx[2] = { (uint8_t)(input_nibble & 0x0F), 0x00 };
    uint8_t rx[2] = { 0, 0 };

    gpio_put(PIN_FPGA_CS, 0);
    sleep_us(1);
    spi_write_read_blocking(SPI_INST, tx, rx, 2);
    sleep_us(1);
    gpio_put(PIN_FPGA_CS, 1);

    return rx[1] & 0x03;
}
