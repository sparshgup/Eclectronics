// Combinational BNN inference engine.
//
// Layer 1: 4-bit input, 16 hidden neurons.
//   For each neuron i in 0..15:
//     popcount(XNOR(input_vec, W1[i])) >= T1[i] -> hidden[i]
//
// Layer 2: 16-bit hidden, 3 output neurons.
//   For each class c in 0..2:
//     score[c] = popcount(XNOR(hidden, W2[c]))
//   class_out = argmax(score)
//
// Weights and thresholds come from training/export_weights.py.

module bnn_engine (
    input  wire [3:0] input_vec,
    output reg  [1:0] class_out
);

    `include "weights.vh"

    // ---- Layer 1: 4 inputs -> 16 hidden ----
    wire [15:0] hidden;
    genvar i;
    generate
        for (i = 0; i < 16; i = i + 1) begin : g_hidden
            wire [3:0] xnor_i = ~(input_vec ^ W1[i]);
            wire [2:0] pop_i  = xnor_i[0] + xnor_i[1] + xnor_i[2] + xnor_i[3];
            assign hidden[i] = (pop_i >= T1[i]);
        end
    endgenerate

    // ---- Layer 2: 16 hidden -> 3 class scores ----
    // popcount of 16 bits fits in 5 bits (max 16).
    wire [15:0] xnor_c0 = ~(hidden ^ W2_0);
    wire [15:0] xnor_c1 = ~(hidden ^ W2_1);
    wire [15:0] xnor_c2 = ~(hidden ^ W2_2);

    function [4:0] popcount16;
        input [15:0] x;
        integer k;
        begin
            popcount16 = 5'd0;
            for (k = 0; k < 16; k = k + 1)
                popcount16 = popcount16 + x[k];
        end
    endfunction

    wire [4:0] s0 = popcount16(xnor_c0);
    wire [4:0] s1 = popcount16(xnor_c1);
    wire [4:0] s2 = popcount16(xnor_c2);

    // ---- Argmax of three 5-bit values ----
    // Ties broken by lower class index.
    always @* begin
        if ((s0 >= s1) && (s0 >= s2))
            class_out = 2'd0;
        else if (s1 >= s2)
            class_out = 2'd1;
        else
            class_out = 2'd2;
    end

endmodule
