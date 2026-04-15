# RW612 Flash Unlock Tool

[中文](README_CN.md) | English

## Problem

Flashing firmware from **NXP RD-RW612-BGA** or **Quectel FCM363X-L/FCM365X** boards (which use Macronix flash) to modules with **Winbond/XMC flash** (FCMA62N/FCM363X/FGMH63X) causes flash lock issues due to incompatible status register configurations.

## Solution

This tool unlocks the flash by running from RAM and clearing the protection bits.

## Usage

### Method 1: Use Pre-built Binary

1. Download `flexspi_nor_dma_transfer.bin` from [Releases](../../releases)
2. Load to RAM using J-Link:

```bash
JLink
> connect
> device FCM363X
> si JTAG              # Use SWD for FCMA62N, JTAG for FCM363X/FGMH63X
> speed 4000
> loadbin flexspi_nor_dma_transfer.bin 0x20000000
> setpc 0x200012fc     # Check output.map if this address doesn't work
> go
```

3. Monitor serial output (115200 baud) for unlock status

### Method 2: Build from Source

**Recommended: Use Debug (build + run)**

1. Open project in **MCUXpresso for VS Code**
2. Select build configuration: **ram_debug** or **ram_release**
3. Click **Debug** button (builds and runs automatically)

**Alternative: Build only**

1. Open project in **MCUXpresso for VS Code**
2. Select build configuration: **ram_debug** or **ram_release**
3. Click **Build** button
4. Output: `armgcc/ram_debug/flexspi_nor_dma_transfer.bin`

## Notes

- Runs from RAM to safely modify flash
- Supports XMC flash (vendor ID 0x20)
- **Device**: Choose according to your board
  - FCMA62N → device FCMA62N, si SWD
  - FCM363X → device FCM363X, si JTAG
  - FGMH63X → device FGMH63X, si JTAG
- Check `armgcc/ram_debug/output.map` for Reset_Handler address if 0x200012fc doesn't work


