

debug: DEBUG=gdb
debug: OPT=0
debug: all

release: DEBUG=
release: OPT=2
release: all

## Supported boards:
# HY-STM32_100P
# stm32f4_discovery
# uemb1
# uemb4
# rrcv1
# rrcv2
BOARD ?= stm32f4_discovery

## Project name
PROJECT_NAME ?= main

## Debugging format:
# stabs
# dwarf-2
# gdb
DEBUG ?= 

## Optimization level,
# can be 0, 1, 2, 3, s
OPT ?= 2

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

## DRIVERS
# set thse to 0 to remove from the build, 1 to include in the build
# standalone drivers
USE_DRIVER_RTC ?= 0
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
USE_DRIVER_SDCARD_SDIO ?= 0
USE_DRIVER_FAT_FILESYSTEM ?= 0


## specify location of the project: https://github.com/drmetal/stm32-build-env
BUILD_ENV_DIR ?= ../stm32-build-env
## specify location of the project: https://github.com/drmetal/stm32-device-support
DEVICE_SUPPORT_DIR ?= ../stm32-device-support
## specify location of the project: https://github.com/drmetal/cutensils
CUTENSILS_DIR ?= ../cutensils
## specify location of the project: https://github.com/drmetal/nutensils
NUTENSILS_DIR ?= ../nutensils
## specify location of the project: https://github.com/drmetal/FatFs
FATFS_DIR ?= ../FatFs
## specify location of the project: https://github.com/drmetal/like-posix
LIKEPOSIX_DIR ?= ../like-posix
## specify location of the project: https://github.com/drmetal/freertos
FREERTOS_DIR ?= ../freertos
## specify location of the project: https://github.com/drmetal/LwIP
LWIP_DIR ?= ../LwIP
## specify location of the project: https://github.com/drmetal/minstdlibs
MINSTDLIBSDIR ?= ../minstdlibs

## relocate application
# set appropriately if there is a bootloader in the low flash pages
APP_ADDRESS_OFFSET ?= 0x0000

# include the makefile that collects all modules together
include ../stm32-build-env/collect.mk