# Binary Neural Network Inference Accelerator PCB

Built a custom 2-layer PCB that combines an RP2040 microcontroller with a Lattice iCE40UP5K FPGA. The goal was to run a small binary neural network (BNN) classifier on the FPGA in hardware. The RP2040 programs the FPGA, sends input data over SPI, gets back a classification result, and prints it over USB serial. An RGB LED shows which class was picked.

BNNs use XNOR and popcount instead of floating-point math, which maps well onto FPGA lookup tables. The demo targets the Iris dataset (4 inputs, 3 classes). The hardware doesn't care what model you use since the weights are just baked into the FPGA bitstream.

The design is based on the open-source pico-ice board by tinyVision.ai and the riffpga framework by Psychogenic Technologies.

## Layout

<img width="900" alt="bnn_layout" src="https://github.com/user-attachments/assets/7eb55e11-e31d-4bcc-9300-581a4d3f9caa" />

## PCB

<img width="900" alt="pcb" src="https://github.com/user-attachments/assets/1b573c05-3a9e-43e2-bf06-7539f0bf0512" />

## Schematics

<img width="900" alt="bnn_schematic_root" src="https://github.com/user-attachments/assets/8b689062-257d-4386-8adb-51e71be824fd" />

<img width="900" alt="bnn_schematic_power" src="https://github.com/user-attachments/assets/16220746-a829-492e-89d8-6d6375fd0da5" />

<img width="900" alt="bnn_schematic_rp2040" src="https://github.com/user-attachments/assets/6e486bbc-ba83-4989-82e5-448a7e1e13d3" />

<img width="900" alt="bnn_schematic_ice40up5k" src="https://github.com/user-attachments/assets/f3aa75de-e01d-41ac-a394-a4230f72e7f0" />

<img width="900" alt="bnn_schematic_io" src="https://github.com/user-attachments/assets/000cf4cf-ba2e-4416-8d78-a4adcb2466a7" />

