#pragma once

// Pin map for the BNN board (matches the schematic).

#define PIN_UART_TX        0    // -> FPGA UART_RX (pin 25)
#define PIN_UART_RX        1    // <- FPGA UART_TX (pin 27)

#define PIN_SPI_SCK        2    // shared SPI bus: SCK to FPGA flash + FPGA pin 15
#define PIN_SPI_MOSI       3    // shared: MOSI / FPGA SI (flash DI + FPGA pin 17)
#define PIN_SPI_MISO       4    // shared: MISO / FPGA SO (flash DO + FPGA pin 14)
#define PIN_FPGA_CS        5    // FPGA pin 26 — data SPI CS (also doubles as SS during slave-SPI config)
#define PIN_FLASH_CS       6    // FPGA flash CS (also FPGA pin 16 — SPI_SS during slave-SPI config)
#define PIN_FPGA_CLK       7    // PWM out -> FPGA pin 35 (global buffer)

#define PIN_FPGA_CRESET    8    // -> FPGA CRESET_B (pin 8), active low
#define PIN_FPGA_CDONE     9    // <- FPGA CDONE (pin 7)

#define PIN_LED_R         10    // active low (RGB common anode)
#define PIN_LED_G         11
#define PIN_LED_B         12
#define PIN_USER_BTN      13    // active low, has external 10k pull-up

#define SPI_INST          spi0  // SPI0 owns GPIO2-5
