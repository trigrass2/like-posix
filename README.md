stm32-build-env
===============

Overview
--------

this project compiles a bunch of makefiles and scripts that can be used to build STM32 projects.

Both STM32F1 and STM32F4 are supported.

Requires
--------

to use, clone the following projects into a new directory:

 - https://github.com/drmetal/stm32-build-env
 - https://github.com/drmetal/stm32-device-support
 - https://github.com/drmetal/like-posix
 - https://github.com/drmetal/freertos
 - https://github.com/drmetal/minstdlibs
 - https://github.com/drmetal/cutensils
 - https://github.com/drmetal/nutensils
 - https://github.com/drmetal/LwIP
 - https://github.com/drmetal/FatFs
 
 
Demos
-----

**Led Blinker**

https://github.com/drmetal/led-blink-demo.git

**File IO**

https://github.com/drmetal/file-io-demo.git


Options in project Makefiles
----------------------------

There are a few options that may be specified in the project makefile. Here is an example.

``` make

#******************************************************************************
# Sample project makefile
# COPYRIGHT 2015 Mike Stuart
#******************************************************************************

## Supported boards:
# HY-STM32_100P
# stm32f4_discovery
# uemb1
# uemb4
# rrcv1
# rrcv2
BOARD = stm32f4_discovery

## Project name
PROJECT_NAME = demoproject

## project source, add any new project source files
SOURCE =
SOURCE += main.c

## Debugging format:
# stabs
# dwarf-2
# gdb
DEBUG =

## Optimization level,
# can be 0, 1, 2, 3, s
OPT = 2

## there are some small alternatives to the standard libraries, the minstdlibs project
# set to 1 to enable those (may reduce code size by several 10's of kb)
USE_MINSTDLIBS = 0

## there is a small configuration file utility in cutensils project
# set to 1 to enable
USE_CONFPARSE = 0

## there is a logging facility in cutensils project, requires the like-posix project
# set to 1 to enable, set to 0 to disable
USE_LOGGER = 0

## to use many of the facilities, posix style IO is required.
# set to 1 to enable
USE_POSIX_STYLE_IO = 0

## socket utilities from the nutensils project
# set to 1 to enable, set to 0 to disable
USE_SOCK_UTILS = 0

## HTTP utilities from the nutensils project
# set to 1 to enable, set to 0 to disable
USE_HTTP_UTILS = 0

## DRIVERS
# set thse to 0 to remove from the build, 1 to include in the build
# standalone drivers
USE_DRIVER_RTC = 0
USE_DRIVER_DS1820 = 0
USE_DRIVER_LEDS = 0
USE_DRIVER_USART = 0
# LCD and touch panel go together
USE_DRIVER_LCD = 0
USE_DRIVER_TOUCH_PANEL = 0
# Networking. PHY selection is set in board.mk
USE_DRIVER_UIP_NET = 0
USE_DRIVER_LWIP_NET = 0
# FAT Filesystem and SD Card driver selection go together
USE_DRIVER_SDCARD_SPI = 0
USE_DRIVER_SDCARD_SDIO = 0
USE_DRIVER_FAT_FILESYSTEM = 0


## specify location of the project: https://github.com/drmetal/stm32-build-env
BUILD_ENV_DIR = ../stm32-build-env
## specify location of the project: https://github.com/drmetal/stm32-device-support
DEVICE_SUPPORT_DIR = ../stm32-device-support
## specify location of the project: https://github.com/drmetal/cutensils
CUTENSILS_DIR = ../cutensils
## specify location of the project: https://github.com/drmetal/nutensils
NUTENSILS_DIR = ../nutensils
## specify location of the project: https://github.com/drmetal/FatFs
FATFS_DIR = ../FatFs
## specify location of the project: https://github.com/drmetal/like-posix
LIKEPOSIX_DIR = ../like-posix
## specify location of the project: https://github.com/drmetal/freertos
FREERTOS_DIR = ../freertos
## specify location of the project: https://github.com/drmetal/LwIP
LWIP_DIR = ../LwIP
## specify location of the project: https://github.com/drmetal/minstdlibs
MINSTDLIBSDIR = ../minstdlibs

## relocate application
# set appropriately if there is a bootloader in the low flash pages
APP_ADDRESS_OFFSET = 0x0000

## be sure to include the other makefiles
include $(BUILD_ENV_DIR)/collect.mk
```
