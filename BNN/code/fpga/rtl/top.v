// Top-level module for BNN inference accelerator.
// Wires the SPI slave to the combinational BNN engine.
//
// Protocol over SPI (CPOL=0, CPHA=0, MSB-first, 8-bit bytes):
//   Byte 0 (MOSI): 4-bit binarized input vector in lower nibble (upper nibble ignored).
//   Byte 1 (MOSI): dummy 0x00. MISO returns the result byte: lower 2 bits = class (0..2).
// CS must be deasserted between transactions.

module top (
    input  wire clk,        // clock from RP2040 (GPIO7 -> pin 35)
    input  wire spi_cs,     // pin 26
    input  wire spi_sck,    // pin 15
    input  wire spi_mosi,   // pin 17
    output wire spi_miso,   // pin 14
    input  wire uart_rx,    // pin 25 (unused in this design, reserved)
    output wire uart_tx     // pin 27 (unused in this design, reserved)
);

    // Pass UART straight through as idle-high so the line is defined.
    assign uart_tx = 1'b1;

    wire [3:0] input_vec;
    wire [7:0] result;

    spi_slave u_spi (
        .clk      (clk),
        .spi_cs   (spi_cs),
        .spi_sck  (spi_sck),
        .spi_mosi (spi_mosi),
        .spi_miso (spi_miso),
        .input_vec(input_vec),
        .result   (result)
    );

    wire [1:0] class_out;

    bnn_engine u_bnn (
        .input_vec(input_vec),
        .class_out(class_out)
    );

    assign result = {6'b000000, class_out};

endmodule
