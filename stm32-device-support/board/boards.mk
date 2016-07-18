
## default board dir is in the lollyjar DEVICE_SUPPORT_DIR location
# override BOARDDIR in custom board configuration board.mk
BOARDDIR ?= $(DEVICE_SUPPORT_DIR)/board

## default supported BOARDS
# add to BOARDS in board configuration makefile, board.mk
BOARDS = 

## Select DEVICE and FAMILY, by BOARD

## configure BOARD
include $(BOARDDIR)/$(BOARD).bsp/board.mk

## test BOARD againt BOARDS
ifeq ($(filter $(BOARD),$(BOARDS)), )
$(error board '$(BOARD)' not supported. supported boards: $(BOARDS))
endif

## if no network drivers have been enabled then set PHY selection to none
USE_DRIVER_ENC28J60_PHY ?= 0
USE_DRIVER_MII_RMII_PHY ?= 0

ifeq ($(USE_DRIVER_ENC28J60_PHY), 1)
ifneq ($(USE_DRIVER_SPI), 1)
$(error when USE_DRIVER_ENC28J60_PHY=1,  USE_DRIVER_SPI must be set to 1, it is set to '$(USE_DRIVER_SPI)')
endif
endif

ifeq ($(DATA_IN_ExtSRAM), 1)
CFLAGS += -DDATA_IN_ExtSRAM
endif

ifeq ($(DATA_IN_ExtSDRAM), 1)
CFLAGS += -DDATA_IN_ExtSDRAM
endif

## configure BOARD SOURCE and CFLAGS
CFLAGS += -I$(DEVICE_SUPPORT_DIR)/board
CFLAGS += -I$(BOARDDIR)/$(BOARD).bsp
SOURCE += $(BOARDDIR)/$(BOARD).bsp/board_config.c


