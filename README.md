stm32-build-env
===============

Overview
--------

this project compiles a bunch of makefiles and scripts that can be used to build STM32 projects.

Both STM32F1 and STM32F4 are supported.

Requires
--------

the minstdlibs project must be cloned int a directory "minstlibs"
at the same location where stm32_freertos lives.


Options in project Makefiles
----------------------------

There are a few options that may be specified in the project makefile. Here is an example.

``` make
#******************************************************************************
# Sample project makefile
# COPYRIGHT 2009 Mike Stuart
#******************************************************************************

## Supported boards:
# HY-STM32_100P
# stm32f4_discovery
# uemb1
# uemb2
BOARD = stm32f4_discovery

## location of the stm32 framework
FRAMEWORK_DIR = ../stm32_freertos

## Project name
PROJECT_NAME = stm32f4demo

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

## there are some small alternatives to the standard libraries
# set to 1 to enable those (may reduce code size by several 10's of kb)
USE_MINSTDLIBS = 0

## there is a small configuration file utility
# set to 1 to enable
USE_CONFPARSE = 0

## there is a logging facility
# set to 1 to enable, set to 0 to disable
# depends on USE_LARGE_SYSCALLS set to 1, if files/devices are to be written
USE_LOGGER = 0

## DRIVERS
# set thse to 0 to remove from the build, 1 to include in the build
# standalone drivers
USE_DRIVER_RTC = 0
USE_DRIVER_DS1820 = 0
USE_DRIVER_LEDS = 0
USE_DRIVER_SERIAL = 0
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

## be sure to include the framework makefile
include $(FRAMEWORK_DIR)/framework.mk
```
