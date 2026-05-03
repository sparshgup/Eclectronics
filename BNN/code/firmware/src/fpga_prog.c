#include "fpga_prog.h"
#include "board.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

// ---- iCE40 slave-SPI configuration sequence (Lattice TN1248) ----
//
// 1. CRESET_B = 0, SPI_SS = 0
// 2. Wait > 200 ns
// 3. CRESET_B = 1, wait > 1200 µs
// 4. 8 dummy clocks with SS high
// 5. SS low, clock in bitstream MSB-first
// 6. SS high, send >= 49 trailing clocks (we send 104 to be safe)
// 7. CDONE should be high

#define CONFIG_SPI_HZ   (8 * 1000 * 1000)
#define CDONE_TIMEOUT_MS 200

void fpga_prog_init(void) {
    // ---- Control pins ----
    gpio_init(PIN_FPGA_CRESET);
    gpio_set_dir(PIN_FPGA_CRESET, GPIO_OUT);
    gpio_put(PIN_FPGA_CRESET, 0);

    gpio_init(PIN_FPGA_CDONE);
    gpio_set_dir(PIN_FPGA_CDONE, GPIO_IN);
    gpio_pull_up(PIN_FPGA_CDONE);    // CDONE is open-drain on the FPGA

    gpio_init(PIN_FLASH_CS);
    gpio_set_dir(PIN_FLASH_CS, GPIO_OUT);
    gpio_put(PIN_FLASH_CS, 1);

    gpio_init(PIN_FPGA_CS);
    gpio_set_dir(PIN_FPGA_CS, GPIO_OUT);
    gpio_put(PIN_FPGA_CS, 1);

    // ---- SPI bus (used for both configuration and post-config data) ----
    spi_init(SPI_INST, CONFIG_SPI_HZ);
    spi_set_format(SPI_INST, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_SPI_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI_MISO, GPIO_FUNC_SPI);
}

bool fpga_program(const uint8_t *bitstream, size_t length) {
    // Step 1: assert CRESET and SS, deassert data CS.
    gpio_put(PIN_FPGA_CRESET, 0);
    gpio_put(PIN_FLASH_CS, 0);     // SS_B = 0 (selects slave-SPI mode)
    gpio_put(PIN_FPGA_CS, 1);
    sleep_us(2);

    // Step 2: release CRESET; wait for FPGA to enter config state.
    gpio_put(PIN_FPGA_CRESET, 1);
    sleep_ms(2);

    // Step 3: 8 dummy clocks with SS high.
    gpio_put(PIN_FLASH_CS, 1);
    uint8_t dummy = 0xFF;
    spi_write_blocking(SPI_INST, &dummy, 1);

    // Step 4: SS low, send the bitstream.
    gpio_put(PIN_FLASH_CS, 0);
    spi_write_blocking(SPI_INST, bitstream, length);
    gpio_put(PIN_FLASH_CS, 1);

    // Step 5: trailing clocks so the FPGA can finish initialisation.
    uint8_t trailing[14] = {0};   // 14 bytes = 112 clocks (>= 49 required)
    spi_write_blocking(SPI_INST, trailing, sizeof(trailing));

    // Step 6: poll CDONE.
    for (int i = 0; i < CDONE_TIMEOUT_MS; i++) {
        if (gpio_get(PIN_FPGA_CDONE)) return true;
        sleep_ms(1);
    }
    return false;
}

void fpga_clock_start(uint32_t hz) {
    gpio_set_function(PIN_FPGA_CLK, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN_FPGA_CLK);
    uint chan  = pwm_gpio_to_channel(PIN_FPGA_CLK);

    // PWM counter wraps at TOP, output toggles at TOP/2 -> 50% duty.
    uint32_t sys_hz = clock_get_hz(clk_sys);
    uint32_t top = sys_hz / hz;
    if (top < 2) top = 2;

    pwm_set_wrap(slice, top - 1);
    pwm_set_chan_level(slice, chan, top / 2);
    pwm_set_enabled(slice, true);
}
