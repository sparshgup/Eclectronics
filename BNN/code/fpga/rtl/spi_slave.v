// Minimal SPI slave (mode 0, MSB-first) for the BNN data path.
//
// Two-byte transaction:
//   Byte 0 (MOSI): 4-bit input vector in lower nibble, latched into input_vec.
//   Byte 1 (MOSI): dummy. MISO returns `result`.
//
// `result` is combinational off `input_vec`. There are many `clk` cycles
// between the last sck_fall of byte 0 and the first sck_rise of byte 1
// (sck is much slower than clk), so the new result is settled in time.

module spi_slave (
    input  wire       clk,
    input  wire       spi_cs,
    input  wire       spi_sck,
    input  wire       spi_mosi,
    output wire       spi_miso,
    output reg  [3:0] input_vec,
    input  wire [7:0] result
);

    // ---- Resync external SPI signals into clk domain ----
    reg [2:0] cs_sync, sck_sync;
    reg [1:0] mosi_sync;
    always @(posedge clk) begin
        cs_sync   <= {cs_sync[1:0],  spi_cs};
        sck_sync  <= {sck_sync[1:0], spi_sck};
        mosi_sync <= {mosi_sync[0],  spi_mosi};
    end

    wire cs_n     = cs_sync[2];
    wire sck_rise = (sck_sync[2:1] == 2'b01);
    wire sck_fall = (sck_sync[2:1] == 2'b10);
    wire mosi     = mosi_sync[1];

    // ---- Framing ----
    reg [2:0] bit_idx;
    reg       byte_idx;
    reg [7:0] rx_shift;
    reg [7:0] tx_shift;
    reg       load_tx_pending;

    // Drive MISO directly (single-slave bus — pin 14 is dedicated post-config).
    assign spi_miso = tx_shift[7];

    always @(posedge clk) begin
        if (cs_n) begin
            bit_idx         <= 3'd0;
            byte_idx        <= 1'b0;
            tx_shift        <= result;       // ready in case master starts reading immediately
            load_tx_pending <= 1'b0;
        end else begin
            if (sck_rise) begin
                rx_shift <= {rx_shift[6:0], mosi};
            end
            if (sck_fall) begin
                tx_shift <= {tx_shift[6:0], 1'b0};
                if (bit_idx == 3'd7) begin
                    bit_idx <= 3'd0;
                    if (byte_idx == 1'b0) begin
                        // End of byte 0: latch full nibble (rx_shift was updated on the rise).
                        input_vec       <= rx_shift[3:0];
                        byte_idx        <= 1'b1;
                        load_tx_pending <= 1'b1;
                    end
                end else begin
                    bit_idx <= bit_idx + 3'd1;
                end
            end
            // One cycle after input_vec is latched, the combinational result is
            // stable and we can capture it. Many idle clk cycles before next sck.
            if (load_tx_pending) begin
                tx_shift        <= result;
                load_tx_pending <= 1'b0;
            end
        end
    end

endmodule
