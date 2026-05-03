# Build script for the iCE40UP5K BNN bitstream.
#   make            -> bitstream.bin (Yosys + nextpnr-ice40 + icepack)
# 	make sim        -> Icarus Verilog simulation of the BNN engine
#   make clean

PROJ      := bnn
DEVICE    := up5k
PACKAGE   := sg48
TOP       := top

RTL_DIR   := rtl
PCF       := constraints/pins.pcf
SOURCES   := $(RTL_DIR)/top.v $(RTL_DIR)/bnn_engine.v $(RTL_DIR)/spi_slave.v
BUILD_DIR := build

YOSYS    ?= yosys
NEXTPNR  ?= nextpnr-ice40
ICEPACK  ?= icepack
IVERILOG ?= iverilog
VVP      ?= vvp

all: $(BUILD_DIR)/$(PROJ).bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(PROJ).json: $(SOURCES) $(RTL_DIR)/weights.vh | $(BUILD_DIR)
	$(YOSYS) -q -p "read_verilog -I $(RTL_DIR) $(SOURCES); synth_ice40 -top $(TOP) -json $@"

$(BUILD_DIR)/$(PROJ).asc: $(BUILD_DIR)/$(PROJ).json $(PCF)
	$(NEXTPNR) --$(DEVICE) --package $(PACKAGE) --json $< --pcf $(PCF) --asc $@

$(BUILD_DIR)/$(PROJ).bin: $(BUILD_DIR)/$(PROJ).asc
	$(ICEPACK) $< $@
	@echo "==> Bitstream ready: $@ ($$(wc -c < $@) bytes)"

# ---- Simulation ----
sim: $(BUILD_DIR)/tb_bnn.vvp
	cd $(BUILD_DIR) && $(VVP) tb_bnn.vvp

$(BUILD_DIR)/tb_bnn.vvp: testbench/tb_bnn.v $(RTL_DIR)/bnn_engine.v $(RTL_DIR)/weights.vh | $(BUILD_DIR)
	$(IVERILOG) -o $@ -I $(RTL_DIR) testbench/tb_bnn.v $(RTL_DIR)/bnn_engine.v

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all sim clean
