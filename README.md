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