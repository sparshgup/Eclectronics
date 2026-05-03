# BNN Inference Accelerator — Firmware

Companion code for the RP2040 + iCE40UP5K BNN board. Three components:

```
training/   Python — train + binarize a tiny MLP on Iris, emit weights.vh
fpga/       Verilog — combinational BNN engine + SPI slave for iCE40UP5K
firmware/   C (bare pico-sdk) — programs the FPGA, exposes a USB-serial CLI
```

## Build order

```
# 1. Train the model and export weights for the FPGA
cd training/
pip install -r requirements.txt
python train_bnn.py        # writes model.json
python export_weights.py   # writes ../fpga/rtl/weights.vh

# 2. Synthesize the FPGA bitstream
cd ../fpga/
make                       # writes build/bnn.bin

# 3. Build the RP2040 firmware (with bitstream embedded)
cd ../firmware/
mkdir -p build && cd build
cmake ..
make bitstream             # regenerates include/bitstream.h from fpga/build/bnn.bin
make                       # writes bnn.uf2
```

Drag `firmware/build/bnn.uf2` onto the RP2040 mass-storage device (BOOTSEL).

## Toolchain prerequisites

| Component         | Tools                                                                  |
|-------------------|------------------------------------------------------------------------|
| Python training   | `python>=3.10`, packages in `training/requirements.txt`                |
| FPGA bitstream    | `yosys`, `nextpnr-ice40`, `icepack` (apt: `yosys nextpnr-ice40 fpga-icestorm`) |
| FPGA simulation   | `iverilog`, `vvp` (optional, for `make sim`)                           |
| RP2040 firmware   | `arm-none-eabi-gcc` (with newlib), `cmake`, pico-sdk via `$PICO_SDK_PATH` |

### macOS toolchain setup (verified working)

```sh
# FPGA tools (Homebrew has all four)
brew install yosys icestorm nextpnr-ice40 icarus-verilog

# pico-sdk (clone with tinyusb submodule)
git clone --depth 1 -b 2.1.0 https://github.com/raspberrypi/pico-sdk ~/pico-sdk
git -C ~/pico-sdk submodule update --init --depth 1 lib/tinyusb

# ARM toolchain — DO NOT use Homebrew's arm-none-eabi-gcc, it ships
# without newlib and the build will fail with "cannot read spec file 'nosys.specs'".
# Use ARM's official tarball or the gcc-arm-embedded cask (needs sudo) instead:
curl -LO https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-darwin-arm64-arm-none-eabi.tar.xz
tar xf arm-gnu-toolchain-13.2.rel1-darwin-arm64-arm-none-eabi.tar.xz -C ~

# Add to ~/.zshrc (or export each session)
export PICO_SDK_PATH=$HOME/pico-sdk
export PICO_TOOLCHAIN_PATH=$HOME/arm-gnu-toolchain-13.2.rel1-darwin-arm64-arm-none-eabi
```

## Architecture (Iris)

```
4 binary inputs ── XNOR ──┐
                          ├── 16x popcount ──┬── threshold ──── 16 hidden bits
16 weight rows (4 bits) ──┘                  │
                                             │
16 hidden bits   ── XNOR ──┐                 │
                           ├── 3x popcount ──┴── argmax ──── 2-bit class
3 weight rows (16 bits) ───┘
```

Estimated resource use: ~250 LUTs, well under 5% of the iCE40UP5K.

## SPI protocol (RP2040 master ↔ FPGA slave)

Mode 0, MSB-first, 8 MHz default. Two-byte transactions, CS asserted across both:

| Byte | MOSI                            | MISO                       |
|------|---------------------------------|----------------------------|
| 0    | input nibble in lower 4 bits    | don't care                 |
| 1    | dummy `0x00`                    | result, lower 2 bits = class |

## USB-CDC CLI

After flashing, open the USB serial port (any baud — it's CDC):

```
> 0110             # run inference on input 0110
input=0b0110 -> class 1 (versicolor)
> d               # demo sweep across a few representative inputs
> h               # help
```

Pressing the **USER button** triggers the same demo sweep without needing a host.

## Pin map (RP2040 ↔ FPGA)

See [firmware/include/board.h](firmware/include/board.h) and [fpga/constraints/pins.pcf](fpga/constraints/pins.pcf). The SPI bus (GPIO2-4) is shared with the FPGA configuration flash; CS for flash and CS for the FPGA data interface are separate GPIOs.
