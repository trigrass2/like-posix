

release: OPT = 2
release: all

small: OPT = s
small: all

debug: CFLAGS += -g
debug: OPT = 0
debug: all

DATE := $(shell date +"%F_%T")

## Supported boards:
# HY-STM32_100P
# stm32f4_discovery
# uemb1
# uemb4
# rrcv1
# rrcv2
BOARD ?= none

## Project name
PROJECT_NAME ?= main

## version string (override with an environment variable set in your build system)
PROJECT_VERSION ?= $(DATE)

#use assertions
USE_FULL_ASSERT ?= 1

# debug exceptions
DEBUG_PRINTF_EXCEPTIONS ?= 0

# route stdout to CONSOLE USART
USE_STDIO_USART ?= 0

## Optimization level,
# can be 0, 1, 2, 3, s
OPT ?= 2

## to use many of the following options, FreeRTOS is required.
# set to 1 to enable freertos
USE_FREERTOS ?= 0

## there are some small alternatives to the standard libraries
# set to 1 to enable those (may reduce code size by several 10's of kb)
USE_MINSTDLIBS ?= 0

## there is a small configuration file utility
# set to 1 to enable
USE_CONFPARSE ?= 0

## there is a logging facility
# set to 1 to enable, set to 0 to disable
# depends on USE_LARGE_SYSCALLS set to 1, if files/devices are to be written
USE_LOGGER ?= 0
USE_LOGGER_TIMESTAMP ?= 1

## to use many of the facilities, posix style IO is required.
# set to 1 to enable
USE_POSIX_STYLE_IO ?= 0

## socket utilities from the nutensils project
# set to 1 to enable, set to 0 to disable
USE_SOCK_UTILS ?= 0

## HTTP utilities from the nutensils project
# set to 1 to enable, set to 0 to disable
USE_HTTP_UTILS ?= 0

## Threaded server module from the nutensils project
# set to 1 to enable, set to 0 to disable
USE_THREADED_SERVER ?= 0

## HTTP server module from the nutensils project
# set to 1 to enable, set to 0 to disable
USE_HTTP_SERVER ?= 0

## Shell server module from the nutensils project
# set to 1 to enable, set to 0 to disable
USE_SHELL ?= 0

## graphics module from the graphics project
# set to 1 to enable, set to 0 to disable
USE_GRAPHICS ?= 0
USE_GRAPHIC_WIDGET_TOUCH_KEY ?= 0
USE_GRAPHIC_WIDGET_PANEL_METER ?= 0
USE_GRAPHIC_WIDGET_STATUSBAR ?= 0

## DRIVERS
# set thse to 0 to remove from the build, 1 to include in the build
# standalone drivers
USE_DRIVER_RTC ?= 0
USE_DRIVER_SYSTEM_TIMER ?= 0
USE_DRIVER_DS1820 ?= 0
USE_DRIVER_LEDS ?= 0
USE_DRIVER_USART ?= 0
# LCD and touch panel go together
USE_DRIVER_LCD ?= 0
USE_DRIVER_TOUCH_PANEL ?= 0
# Networking. PHY selection is set in board.mk
USE_DRIVER_UIP_NET ?= 0
USE_DRIVER_LWIP_NET ?= 0
# FAT Filesystem and SD Card driver selection go together
USE_DRIVER_SDCARD_SPI ?= 0
USE_DRIVER_SDCARD ?= 0
USE_DRIVER_FAT_FILESYSTEM ?= 0
# SDIO mode, polled or thread aware
USE_THREAD_AWARE_SDCARD_DRIVER ?= 0

# the location of this stuff WRT the project
STM32DEVSUPPORTDIR ?= ..

## specify location of the project: https://github.com/drmetal/stm32-build-env
BUILD_ENV_DIR ?= $(STM32DEVSUPPORTDIR)/stm32-build-env
## specify location of the project: https://github.com/drmetal/stm32-device-support
DEVICE_SUPPORT_DIR ?= $(STM32DEVSUPPORTDIR)/stm32-device-support
## specify location of the project: https://github.com/drmetal/cutensils
CUTENSILS_DIR ?= $(STM32DEVSUPPORTDIR)/cutensils
## specify location of the project: https://github.com/drmetal/nutensils
NUTENSILS_DIR ?= $(STM32DEVSUPPORTDIR)/nutensils
## specify location of the project: https://github.com/drmetal/FatFs
FATFS_DIR ?= $(STM32DEVSUPPORTDIR)/FatFs
## specify location of the project: https://github.com/drmetal/like-posix
LIKEPOSIX_DIR ?= $(STM32DEVSUPPORTDIR)/like-posix
## specify location of the project: https://github.com/drmetal/freertos
FREERTOS_DIR ?= $(STM32DEVSUPPORTDIR)/freertos
## specify location of the project: https://github.com/drmetal/LwIP
LWIP_DIR ?= $(STM32DEVSUPPORTDIR)/LwIP
## specify location of the project: https://github.com/drmetal/minstdlibs
MINSTDLIBSDIR ?= $(STM32DEVSUPPORTDIR)/minstdlibs
## specify location of the project: https://github.com/drmetal/graphics
GRAPHICSDIR ?= $(STM32DEVSUPPORTDIR)/graphics

## relocate application
# set appropriately if there is a bootloader in the low flash pages
APP_ADDRESS_OFFSET ?= 0x0000

# include the makefile that collects all modules together
include $(STM32DEVSUPPORTDIR)/stm32-build-env/collect.mk