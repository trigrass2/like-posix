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
#******************************************************************************

## this file lives where your project source is.
# build options are:
# make clean (remove all build artifacts)
# make all (default build)
# make release (no debug)
# make debug (debug enabled)
# to rebuild completely:
# make clean release

## to change settings from the command line:
# make release PROJECT_NAME="demo=project" BOARD="HY-STM32_100P"

## see setup.mk for a complete list of options.
# to set an option, define it here, set to an appriopriate value 

# set board if not specified on the command line
BOARD ?= HY-STM32_100P

# set project if not specified on the command line
PROJECT_NAME ?= demo-project

# specify project source
SOURCE = main.c

USE_MINSTDLIBS = 1
USE_LOGGER = 1
USE_POSIX_STYLE_IO = 1
USE_DRIVER_LEDS = 1
USE_DRIVER_USART = 1
USE_DRIVER_SDCARD_SDIO = 1
USE_DRIVER_FAT_FILESYSTEM = 1

## be sure to include the other makefiles
include ../stm32-build-env/setup.mk
```
