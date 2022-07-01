# MCU name
MCU = STM32F072
BOARD = GENERIC_STM32_F072XB

# Bootloader selection
BOOTLOADER = stm32-dfu

# Wildcard to allow APM32 MCU
DFU_SUFFIX_ARGS = -v FFFF -p FFFF

SRC += led.c \
	  render_gif.c \
      satisfaction_encoder.c \
      satisfaction_oled.c

# Build Options
#   change yes to no to disable
#
BOOTMAGIC_ENABLE = yes      # Enable Bootmagic Lite
MOUSEKEY_ENABLE = no	# Mouse keys
EXTRAKEY_ENABLE = yes	# Audio control and System control
CONSOLE_ENABLE = no	# Console for debug
COMMAND_ENABLE = no    # Commands for debug and configuration
NKRO_ENABLE = yes           # Enable N-Key Rollover
ENCODER_ENABLE = yes
OLED_ENABLE = yes
OLED_DRIVER = SSD1306
WPM_ENABLE = yes

#BACKLIGHT_ENABLE = yes

DEFAULT_FOLDER = cannonkeys/satisfaction75/rev1

# Enter lower-power sleep mode when on the ChibiOS idle thread
OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE

# Enable link time optimizations
LTO_ENABLE = yes

