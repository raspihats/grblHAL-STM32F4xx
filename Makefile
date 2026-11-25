# Makefile build for grblHAL STM32F4xx with arm-none-eabi-gcc (no PlatformIO / CubeIDE).
#
# Currently set up for the MC5S3DP-XHAT board (STM32F411RCT6, 25 MHz HSE, USART1).
# See MC5S3DP-XHAT.md for the pin map, build options and flashing notes.
#
#   make                build build/<BOARD>/grblHAL.elf, .hex, .bin, .map
#   make flash          program with OpenOCD (ST-Link)
#   make stflash        program with st-flash
#   make clean
#
# Build options normally set in Inc/my_machine.h can be passed with EXTRA_DEFS, e.g.
#   make EXTRA_DEFS="-DPROBE_ENABLE=1 -DSAFETY_DOOR_ENABLE=1"

BOARD    ?= MC5S3DP_XHAT
MCU_DEF  ?= -DSTM32F411xE -DHSE_VALUE=25000000
LDSCRIPT ?= STM32F411RCTX_FLASH.ld
STARTUP  ?= StartupF411/startup_stm32f411xe.s
N_AXIS   ?= 5

BUILDDIR = build/$(BOARD)
TARGET   = grblHAL

PREFIX  ?= arm-none-eabi-
CC       = $(PREFIX)gcc
OBJCOPY  = $(PREFIX)objcopy
SIZE     = $(PREFIX)size

CPU  = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
# Board defaults, matching the original grbl-Mega-5X firmware (safety door input on DI_3).
BOARD_DEFS ?= -DSAFETY_DOOR_ENABLE=1 -DCOOLANT_ENABLE=0
DEFS = $(MCU_DEF) -DUSE_HAL_DRIVER -DOVERRIDE_MY_MACHINE -DBOARD_$(BOARD)= \
       -DN_AXIS=$(N_AXIS) -DUSB_SERIAL_CDC=0 $(BOARD_DEFS) $(EXTRA_DEFS)

INCLUDES = -I. -IInc -Iboards \
           -IDrivers/CMSIS/Include \
           -IDrivers/CMSIS/Device/ST/STM32F4xx/Include \
           -IDrivers/STM32F4xx_HAL_Driver/Inc \
           -IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy

OPT    ?= -Os -g    # -O2 is ~20 KB larger and leaves only ~3 KB of the 224 KB code space
CFLAGS  = $(CPU) $(DEFS) $(INCLUDES) $(OPT) -std=gnu11 -Wall \
          -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS = $(CPU) -T$(LDSCRIPT) -specs=nano.specs -specs=nosys.specs \
          -Wl,--gc-sections -Wl,-Map=$(BUILDDIR)/$(TARGET).map \
          -Wl,-u,_printf_float -Wl,-u,_scanf_float
LIBS    = -lm

# grblHAL core, driver and plugins. Disabled plugins compile to nothing.
# eeprom/ (external I2C EEPROM drivers) is left out: the MC5S3DP-XHAT has no EEPROM chip,
# settings are stored in internal flash (FLASH_ENABLE).
SRC_DIRS = Src grbl spindle motors trinamic keypad laser odometer fans plugins \
           embroidery bluetooth encoder plasma openpnp
C_SOURCES = $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.c)) boards/mc5s3dp_xhat.c \
            $(filter-out %_template.c,$(wildcard Drivers/STM32F4xx_HAL_Driver/Src/*.c))
OBJECTS = $(addprefix $(BUILDDIR)/,$(C_SOURCES:.c=.o) $(STARTUP:.s=.o))

OPENOCD ?= openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

.PHONY: all clean flash stflash size

all: $(BUILDDIR)/$(TARGET).elf $(BUILDDIR)/$(TARGET).hex $(BUILDDIR)/$(TARGET).bin size

$(BUILDDIR)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CPU) -c $< -o $@

$(BUILDDIR)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

$(BUILDDIR)/$(TARGET).hex: $(BUILDDIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILDDIR)/$(TARGET).bin: $(BUILDDIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

size: $(BUILDDIR)/$(TARGET).elf
	$(SIZE) $<

flash: $(BUILDDIR)/$(TARGET).elf
	$(OPENOCD) -c "program $< verify reset exit"

stflash: $(BUILDDIR)/$(TARGET).hex
	st-flash --reset --format ihex write $<

clean:
	rm -rf $(BUILDDIR)

-include $(OBJECTS:.o=.d)
