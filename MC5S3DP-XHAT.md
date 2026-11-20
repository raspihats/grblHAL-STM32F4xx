# grblHAL for the MC5S3DP-XHAT board

This branch (`mc5s3dp-xhat` of [raspihats/grblHAL-STM32F4xx](https://github.com/raspihats/grblHAL-STM32F4xx))
adds support for the **MC5S3DP-XHAT** 5-axis motion control board to the grblHAL STM32F4xx driver.
Everything else is unchanged upstream [grblHAL/STM32F4xx](https://github.com/grblHAL/STM32F4xx).

The board's original firmware, a port of grbl-Mega-5X, lives in
[raspihats/MC5S3DP-XHAT-GRBL-5X](https://github.com/raspihats/MC5S3DP-XHAT-GRBL-5X).
This board map uses the same pin assignments.

> **Status:** builds cleanly; **not yet tested on hardware.** Check the items in
> [First power-up](#first-power-up) before connecting a machine.

## Board

| | |
|---|---|
| MCU | STM32F411RCT6 (LQFP64, 256 KB flash, 128 KB RAM) |
| Crystal | 25 MHz HSE, system clock 96 MHz |
| Axes | 5: X, Y, Z, A (motor M3), B (motor M4) |
| Host connection | USART1 (PA9 TX, PA10 RX), 115200 baud, 8N1 |
| Settings storage | Internal flash, sector 1 (the board has no EEPROM chip) |
| USB | Not available: PA11/PA12 are used as outputs on this board |

## Files added by this branch

| File | Purpose |
|---|---|
| `boards/mc5s3dp_xhat_map.h` | Board pin map |
| `Inc/driver.h` | Registers `BOARD_MC5S3DP_XHAT` (one `#elif`) |
| `STM32F411RCTX_FLASH.ld` | Linker script for the 256 KB part (upstream only has the 512 KB F411CE) |
| `StartupF411/startup_stm32f411xe.s` | F411 startup file |
| `Makefile` | Build with plain `arm-none-eabi-gcc`, no PlatformIO or STM32CubeIDE |
| `MC5S3DP-XHAT.md` | This document |

## Building

Requirements: `arm-none-eabi-gcc` (tested with 10.3-2021.07) and `make`.

```sh
git clone -b mc5s3dp-xhat --recurse-submodules git@github.com:raspihats/grblHAL-STM32F4xx.git
cd grblHAL-STM32F4xx
make
```

Output goes to `build/MC5S3DP_XHAT/`: `grblHAL.elf`, `grblHAL.hex`, `grblHAL.bin`, `grblHAL.map`.
The firmware uses about 202 KB of the 224 KB code space.

| Command | What it does |
|---|---|
| `make` | Build |
| `make flash` | Program with OpenOCD through an ST-Link (SWD on PA13/PA14) |
| `make stflash` | Program with `st-flash` |
| `make clean` | Delete `build/MC5S3DP_XHAT/` |

Optimisation is `-Os` by default. `-O2` (`make OPT="-O2 -g"`) is about 20 KB larger and leaves only
about 3 KB free.

### Build options

The Makefile defines `OVERRIDE_MY_MACHINE`, so `Inc/my_machine.h` is **ignored**. Options are set
on the `make` command line instead:

- `BOARD_DEFS` holds the board defaults. Currently `-DSAFETY_DOOR_ENABLE=1`, to match the original
  firmware. Setting `BOARD_DEFS` on the command line replaces these defaults.
- `EXTRA_DEFS` adds options on top, for example:

```sh
make EXTRA_DEFS="-DMOTOR_FAULT_ENABLE=1"   # use the FAULT input as motor fault
make EXTRA_DEFS="-DESTOP_ENABLE=0"         # DI_0 as a plain reset instead of E-stop
```

Run `make clean` (or `make -B`) after changing options, because the object files don't track them.

Options set by the Makefile: `-DBOARD_MC5S3DP_XHAT`, `-DN_AXIS=5`, `-DUSB_SERIAL_CDC=0`,
`-DSTM32F411xE`, `-DHSE_VALUE=25000000`.

Resulting defaults: probe, safety door, E-stop, feed hold and cycle start inputs enabled; spindle
PWM, enable and direction; flood and mist coolant; motor fault input disabled.

## Flashing

Flash the `.hex` or `.elf`. Avoid the `.bin`: it starts at `0x08000000` and covers the settings
sector with padding, which erases the stored `$` settings.

Flash layout (`STM32F411RCTX_FLASH.ld`):

| Address | Size | Content |
|---|---|---|
| `0x08000000` | 16 KB | Vector table (sector 0) |
| `0x08004000` | 16 KB | grblHAL settings (sector 1) |
| `0x08008000` | 224 KB | Code and constants (sectors 2-5) |

Settings are kept in RAM and written to flash only when the machine is idle or in alarm.

## Pin map

Board net names (from the original CubeMX project) are given in brackets.

### Motors

| Motor | Step | Direction | Enable | Limit (min) | Limit (max) |
|---|---|---|---|---|---|
| X | PC14 `X_STP` | PC15 `X_DIR` | PC13 `X_EN` | PC12 `X_MIN` | PB7 `X_MAX` |
| Y | PA4 `Y_STP` | PA6 `Y_DIR` | PA3 `Y_EN` | PD2 `Y_MIN` | PB8 `Y_MAX` |
| Z | PC4 `Z_STP` | PC5 `Z_DIR` | PA7 `Z_EN` | PB3 `Z_MIN` | PB9 `Z_MAX` |
| M3 (A) | PB1 `A_STP` | PB2 `A_DIR` | PB0 `A_EN` | PB4 `A_MIN` | aux input (see below) |
| M4 (B) | PB12 `B_STP` | PB15 `B_DIR` | PB10 `B_EN` | PB6 `B_MIN` | aux input (see below) |

grblHAL supports max limit switches for X, Y and Z only, so `A_MAX` and `B_MAX` are mapped as
general-purpose aux inputs.

### Inputs

| Pin | Net | Function |
|---|---|---|
| PB13 | `DI_0` | Reset / E-stop |
| PB14 | `DI_1` | Feed hold |
| PA15 | `DI_2` | Cycle start |
| PC10 | `DI_3` | Safety door |
| PC11 | `DI_4` | Probe |
| PA5 | `FAULT` | Motor fault with `MOTOR_FAULT_ENABLE=1`, otherwise aux input |
| PC0 | `A_MAX` | Aux input |
| PC1 | `B_MAX` | Aux input |

Every input uses a different pin number, so no two inputs share an EXTI interrupt line.

### Outputs

| Pin | Net | Function |
|---|---|---|
| PC9 | `DQ_2` | Spindle PWM (TIM3 channel 4; frequency set with `$33`, default 5 kHz) |
| PC8 | `DQ_3` | Spindle enable |
| PC7 | `DQ_4` | Spindle direction |
| PA11 | `DQ_0` | Coolant flood (M8) |
| PA8 | `DQ_1` | Coolant mist (M7) |
| PC6 | `DQ_5` | Aux output |
| PA12 | `ST_LED` | Aux output (status LED) |

Aux outputs not claimed by the spindle or coolant can be switched with `M62`-`M65`.

### Analog inputs

| Pin | Net | Function |
|---|---|---|
| PA0 | `THERM_0` | Analog aux input 0 (read with `M66`) |
| PA1 | `THERM_1` | Analog aux input 1 |
| PA2 | `THERM_2` | Not mapped: the driver supports two analog aux inputs |

### Other pins

| Pin | Function |
|---|---|
| PA9 / PA10 | USART1 TX / RX (host) |
| PA13 / PA14 | SWDIO / SWCLK (programming and debugging) |
| PH0 / PH1 | 25 MHz crystal |

Send `$pins` to list the pin assignments of the running firmware, including the port numbers that
`M62`-`M66` use.

## Coming from the original grbl-Mega-5X firmware

- **Settings are not carried over.** grblHAL uses a different settings format and location. Save the
  output of `$$` before flashing and re-enter the values afterwards.
- **`DI_0` is an E-stop.** grblHAL enables E-stop by default: activating the input latches an alarm
  until it is released and the controller is reset. The original firmware treated it as a soft
  reset. Build with `EXTRA_DEFS="-DESTOP_ENABLE=0"` for the old behaviour.
- **Hard limits are available.** The original firmware was built with `DISABLE_HW_LIMITS`; in
  grblHAL, hard limits are enabled with `$21=1`.
- **Spindle PWM frequency** defaults to 5 kHz (`$33`); the original firmware ran at a fixed 24.4 kHz.

## First power-up

Check these against the board before connecting motors or a spindle:

1. `$I` reports board `MC5S3DP-XHAT`, and `$pins` matches the tables above.
2. Stepper enable polarity: `$4`. The original firmware drove the enable pins low to enable the drivers.
3. Limit, probe and control input polarity: `$5`, `$6`, `$14`. Inputs use internal pull-ups (switch to GND).
4. Spindle PWM, enable and direction, and coolant outputs, with the spindle disconnected.
5. E-stop on `DI_0`, then `$X` / reset to clear it.

## Keeping the branch up to date

```sh
git remote add upstream git@github.com:grblHAL/STM32F4xx.git   # once
git fetch upstream
git merge upstream/master        # on the mc5s3dp-xhat branch
git submodule update --init --recursive
make clean && make
```

Upstream changes to `Inc/driver.h` can conflict with the one added `#elif`; keep both sides.
If upstream adds or removes source folders, update `SRC_DIRS` in the `Makefile`.
