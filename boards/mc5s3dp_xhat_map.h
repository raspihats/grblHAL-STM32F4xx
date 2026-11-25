/*
  mc5s3dp_xhat_map.h - driver code for STM32F4xx ARM processors

  Board map for the MC5S3DP-XHAT 5-axis motion control board (STM32F411RCT6,
  25 MHz crystal). Pin assignments match the board's original grbl-Mega-5X port:
  https://github.com/raspihats/MC5S3DP-XHAT-GRBL-5X
  Build and usage notes: MC5S3DP-XHAT.md in the repository root.

  Part of grblHAL

  grblHAL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  grblHAL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with grblHAL. If not, see <http://www.gnu.org/licenses/>.
*/

/* Pin assignments (board net names in brackets):
 *
 *  Motor       Step           Direction      Enable         Min limit      Max limit
 *  X           PC14 [X_STP]   PC15 [X_DIR]   PC13 [X_EN]    PC12 [X_MIN]   PB7  [X_MAX]
 *  Y           PA4  [Y_STP]   PA6  [Y_DIR]   PA3  [Y_EN]    PD2  [Y_MIN]   PB8  [Y_MAX]
 *  Z           PC4  [Z_STP]   PC5  [Z_DIR]   PA7  [Z_EN]    PB3  [Z_MIN]   PB9  [Z_MAX]
 *  M3 (A)      PB1  [A_STP]   PB2  [A_DIR]   PB0  [A_EN]    PB4  [A_MIN]   PC0  [A_MAX] -> aux input
 *  M4 (B)      PB12 [B_STP]   PB15 [B_DIR]   PB10 [B_EN]    PB6  [B_MIN]   PC1  [B_MAX] -> aux input
 *
 *  Inputs:  PB13 [DI_0] Reset/E-stop    PB14 [DI_1] Feed hold    PA15 [DI_2] Cycle start
 *           PC10 [DI_3] Safety door     PC11 [DI_4] Probe        PA5 [FAULT] Motor fault
 *  Outputs: PA11 [DQ_0] Aux P0   PA8 [DQ_1] Aux P1   PC9 [DQ_2] Aux P2   PC8 [DQ_3] Aux P3
 *           PC7 [DQ_4] Aux P4   PC6 [DQ_5] Aux P5   PA12 [ST_LED] Aux P6
 *           (no spindle by default; XHAT_SPINDLE=1/2 makes DQ_2 spindle PWM (TIM3_CH4), 2 also DQ_3/DQ_4
 *           spindle enable/direction; DQ_0/DQ_1 are coolant with COOLANT_ENABLE)
 *  Analog:  PA0 [THERM_0], PA1 [THERM_1] (PA2 [THERM_2] unused: driver supports two analog inputs)
 *  Serial:  USART1, PA9 TX / PA10 RX
 *
 *  Every input uses a different pin number, so there are no EXTI line conflicts.
 *  USB is not available: PA11/PA12 (USB D-/D+) are used as outputs on this board.
 */

#if N_ABC_MOTORS > 2
#error "Axis configuration is not supported!"
#endif

#if USB_SERIAL_CDC
#error "MC5S3DP-XHAT has no USB: PA11/PA12 are used for coolant flood and the status LED. Use the UART."
#endif

#define BOARD_NAME "MC5S3DP-XHAT"
#define HAS_BOARD_INIT              // boards/mc5s3dp_xhat.c: load outputs off on reset, E-stop, alarm, sleep
#define BOARD_URL "https://github.com/raspihats/MC5S3DP-XHAT-GRBL-5X"

#define SERIAL_PORT                 1       // GPIOA: TX = 9, RX = 10

// Define step pulse output pins.
#define X_STEP_PORT                 GPIOC
#define X_STEP_PIN                  14
#define Y_STEP_PORT                 GPIOA
#define Y_STEP_PIN                  4
#define Z_STEP_PORT                 GPIOC
#define Z_STEP_PIN                  4
#define STEP_OUTMODE                GPIO_BITBAND

// Define step direction output pins.
#define X_DIRECTION_PORT            GPIOC
#define X_DIRECTION_PIN             15
#define Y_DIRECTION_PORT            GPIOA
#define Y_DIRECTION_PIN             6
#define Z_DIRECTION_PORT            GPIOC
#define Z_DIRECTION_PIN             5
#define DIRECTION_OUTMODE           GPIO_BITBAND

// Define stepper driver enable/disable output pins.
#define X_ENABLE_PORT               GPIOC
#define X_ENABLE_PIN                13
#define Y_ENABLE_PORT               GPIOA
#define Y_ENABLE_PIN                3
#define Z_ENABLE_PORT               GPIOA
#define Z_ENABLE_PIN                7

// Define homing/hard limit switch input pins.
#define X_LIMIT_PORT                GPIOC
#define X_LIMIT_PIN                 12
#define Y_LIMIT_PORT                GPIOD
#define Y_LIMIT_PIN                 2
#define Z_LIMIT_PORT                GPIOB
#define Z_LIMIT_PIN                 3
// The max limit inputs (J5) are off by default. J5 only has VS_OUT, no GND, so with the internal pull-ups a
// switch there can never read as active. And if max inputs are defined, grblHAL homes in the positive
// direction ($23 bit clear) on the *max* input, so homing would ignore the J4 switches and run into the end
// stop. The machine homes positive on the J4 (min) switches, which needs these left undefined.
#ifdef MC5S3DP_MAX_LIMITS
#define X_LIMIT_PORT_MAX            GPIOB
#define X_LIMIT_PIN_MAX             7
#define Y_LIMIT_PORT_MAX            GPIOB
#define Y_LIMIT_PIN_MAX             8
#define Z_LIMIT_PORT_MAX            GPIOB
#define Z_LIMIT_PIN_MAX             9
#endif
#define LIMIT_INMODE                GPIO_BITBAND

// Define ganged axis or A axis step pulse and step direction output pins.
#if N_ABC_MOTORS > 0
#define M3_AVAILABLE
#define M3_STEP_PORT                GPIOB
#define M3_STEP_PIN                 1
#define M3_DIRECTION_PORT           GPIOB
#define M3_DIRECTION_PIN            2
#define M3_ENABLE_PORT              GPIOB
#define M3_ENABLE_PIN               0
#define M3_LIMIT_PORT               GPIOB
#define M3_LIMIT_PIN                4
#endif

// Define ganged axis or B axis step pulse and step direction output pins.
#if N_ABC_MOTORS > 1
#define M4_AVAILABLE
#define M4_STEP_PORT                GPIOB
#define M4_STEP_PIN                 12
#define M4_DIRECTION_PORT           GPIOB
#define M4_DIRECTION_PIN            15
#define M4_ENABLE_PORT              GPIOB
#define M4_ENABLE_PIN               10
#define M4_LIMIT_PORT               GPIOB
#define M4_LIMIT_PIN                6
#endif

// The vacuum pump (OD2) is on/off only, so by default there is no spindle and all of DQ_0..DQ_5 are plain aux
// outputs whose M64/M65 port number equals the OD number. XHAT_SPINDLE selects a spindle instead:
//   0 (default) no spindle; P0..P5 = OD0..OD5, P6 = ST_LED
//   1           spindle PWM only on DQ_2 (M3 S../M5)
//   2           spindle PWM on DQ_2, enable on DQ_3, direction on DQ_4
#ifndef XHAT_SPINDLE
#define XHAT_SPINDLE                0
#endif
#undef DRIVER_SPINDLE_ENABLE
#if XHAT_SPINDLE == 2
#define DRIVER_SPINDLE_ENABLE       (SPINDLE_ENA|SPINDLE_PWM|SPINDLE_DIR)
#elif XHAT_SPINDLE == 1
#define DRIVER_SPINDLE_ENABLE       SPINDLE_PWM
#else
#define DRIVER_SPINDLE_ENABLE       0
#endif

// Free aux outputs get M64/M65 port numbers P0, P1, ... in this order, skipping pins claimed by the spindle or
// coolant (so with a spindle, the outputs after DQ_2 move down by one or three).
#define AUXOUTPUT0_PORT             GPIOA // DQ_0, OD0, valve 1 (coolant flood if COOLANT_ENABLE)
#define AUXOUTPUT0_PIN              11
#define AUXOUTPUT1_PORT             GPIOA // DQ_1, OD1, valve 2 (coolant mist if COOLANT_ENABLE)
#define AUXOUTPUT1_PIN              8
#define AUXOUTPUT2_PORT             GPIOC // DQ_2, OD2, vacuum pump (spindle PWM, TIM3_CH4, if XHAT_SPINDLE)
#define AUXOUTPUT2_PIN              9
#define AUXOUTPUT3_PORT             GPIOC // DQ_3, OD3 (spindle enable if XHAT_SPINDLE == 2)
#define AUXOUTPUT3_PIN              8
#define AUXOUTPUT4_PORT             GPIOC // DQ_4, OD4 (spindle direction if XHAT_SPINDLE == 2)
#define AUXOUTPUT4_PIN              7
#define AUXOUTPUT5_PORT             GPIOC // DQ_5, OD5
#define AUXOUTPUT5_PIN              6
#define AUXOUTPUT6_PORT             GPIOA // ST_LED
#define AUXOUTPUT6_PIN              12

// Define driver spindle pins
#if DRIVER_SPINDLE_ENABLE & SPINDLE_PWM
#define SPINDLE_PWM_PORT            AUXOUTPUT2_PORT
#define SPINDLE_PWM_PIN             AUXOUTPUT2_PIN
#endif
#if DRIVER_SPINDLE_ENABLE & SPINDLE_ENA
#define SPINDLE_ENABLE_PORT         AUXOUTPUT3_PORT
#define SPINDLE_ENABLE_PIN          AUXOUTPUT3_PIN
#endif
#if DRIVER_SPINDLE_ENABLE & SPINDLE_DIR
#define SPINDLE_DIRECTION_PORT      AUXOUTPUT4_PORT
#define SPINDLE_DIRECTION_PIN       AUXOUTPUT4_PIN
#endif

// Define flood and mist coolant enable output pins.
#if COOLANT_ENABLE & COOLANT_FLOOD
#define COOLANT_FLOOD_PORT          AUXOUTPUT0_PORT
#define COOLANT_FLOOD_PIN           AUXOUTPUT0_PIN
#endif
#if COOLANT_ENABLE & COOLANT_MIST
#define COOLANT_MIST_PORT           AUXOUTPUT1_PORT
#define COOLANT_MIST_PIN            AUXOUTPUT1_PIN
#endif

#define AUXINPUT0_ANALOG_PORT       GPIOA // THERM_0
#define AUXINPUT0_ANALOG_PIN        0
#define AUXINPUT1_ANALOG_PORT       GPIOA // THERM_1
#define AUXINPUT1_ANALOG_PIN        1

#define AUXINPUT0_PORT              GPIOC // Safety door, DI_3
#define AUXINPUT0_PIN               10
#define AUXINPUT1_PORT              GPIOC // Probe, DI_4
#define AUXINPUT1_PIN               11
#define AUXINPUT2_PORT              GPIOB // Reset/E-stop, DI_0
#define AUXINPUT2_PIN               13
#define AUXINPUT3_PORT              GPIOB // Feed hold, DI_1
#define AUXINPUT3_PIN               14
#define AUXINPUT4_PORT              GPIOA // Cycle start, DI_2
#define AUXINPUT4_PIN               15
#define AUXINPUT5_PORT              GPIOA // Motor fault, FAULT
#define AUXINPUT5_PIN               5
#define AUXINPUT6_PORT              GPIOC // A_MAX
#define AUXINPUT6_PIN               0
#define AUXINPUT7_PORT              GPIOC // B_MAX
#define AUXINPUT7_PIN               1

// Define user-control controls (cycle start, reset, feed hold) input pins.
#if CONTROL_ENABLE & CONTROL_HALT
#define RESET_PORT                  AUXINPUT2_PORT
#define RESET_PIN                   AUXINPUT2_PIN
#endif
#if CONTROL_ENABLE & CONTROL_FEED_HOLD
#define FEED_HOLD_PORT              AUXINPUT3_PORT
#define FEED_HOLD_PIN               AUXINPUT3_PIN
#endif
#if CONTROL_ENABLE & CONTROL_CYCLE_START
#define CYCLE_START_PORT            AUXINPUT4_PORT
#define CYCLE_START_PIN             AUXINPUT4_PIN
#endif

#if PROBE_ENABLE
#define PROBE_PORT                  AUXINPUT1_PORT
#define PROBE_PIN                   AUXINPUT1_PIN
#endif

#if SAFETY_DOOR_ENABLE
#define SAFETY_DOOR_PORT            AUXINPUT0_PORT
#define SAFETY_DOOR_PIN             AUXINPUT0_PIN
#endif

#if MOTOR_FAULT_ENABLE
#define MOTOR_FAULT_PORT            AUXINPUT5_PORT
#define MOTOR_FAULT_PIN             AUXINPUT5_PIN
#endif

/* EOF */
