// Simulation testbench: drives all 16 possible 4-bit inputs through the BNN
// engine and prints the predicted class. Use with `make sim`.

`timescale 1ns/1ps

module tb_bnn;
    reg  [3:0] input_vec;
    wire [1:0] class_out;

    bnn_engine dut (
        .input_vec(input_vec),
        .class_out(class_out)
    );

    integer i;
    initial begin
        $dumpfile("tb_bnn.vcd");
        $dumpvars(0, tb_bnn);
        for (i = 0; i < 16; i = i + 1) begin
            input_vec = i[3:0];
            #10;
            $display("input=%b -> class=%0d", input_vec, class_out);
        end
        $finish;
    end
endmodule
