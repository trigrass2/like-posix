stm32-device-support
====================

A collection of code that supports stm32fxxx devices

build options
------------------

 - DEVICE_SUPPORT_DIR
	 - the path to the stm32-device-support root directory, relative to the project makefile
 - BOARD
	 - the name of the board to build for - this must be one of those listed in board.mk->BOARDS
 - APP_ADDRESS_OFFSET		
 	- can be set to some other value to make the built application run from a location other than the flash origin. for example setting to 0x4000 will offset the application in flash by 16KBytes. The first 16KBytes could then be used for a bootloader or non volatile data space... defaults to **0x0000**.

 - USE_DRIVER_LWIP_NET
	 - for boards that include an ethernet device, this option can be set to 1 to enable the device specfic ethernet driver. 

submakefiles
-----------------

The user makefile should include the makefiles from this project:
```make
CFLAGS += -I$(DEVICE_SUPPORT_DIR)
include $(DEVICE_SUPPORT_DIR)/board/board.mk
include $(DEVICE_SUPPORT_DIR)/device/device.mk
```

The user makefile should call **buildlinkerscript** as part of its **all** target. this makes sure  the linker script exists, and matches the chip specified for the board, in board.mk.

Timers
------

Timers are used to support device drivers. It can be hard to work out if its safe to use a timer in the application. The drivers that use timers are:

|module	|file      |set in              |defaults to   |shared with|
|-------|----------|--------------------|--------------|-----------|
|systimer|systimer.c|board/<board>/systime_config.h|TIM2|lcd|
|adc_stream|adc_stream.c|<project>/adc_stream_config.h|TIM3||
|dac_stream|dac_stream.c|<project>/dac_stream_config.h|TIM6||
|i2s_stream|i2s_stream.c|<project>/i2s_stream_config.h|TIM5||
|ds1820|ds1820.c|<project>/ds1820_config.h|TIM4||
|lcd|lcd.c|<project>/board/<board>/lcd_config.h|TIM2|systimer|
|pwm|pwm.c|pwm_config.h|any left over||
