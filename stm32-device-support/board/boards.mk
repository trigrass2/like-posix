
## default board dir is in the appleseed DEVICE_SUPPORT_DIR location
# override BOARDDIR in custom board configuration board.mk
BOARDDIR ?= $(DEVICE_SUPPORT_DIR)/board

## default supported BOARDS
# override BOARDS in custom board configuration board.mk
BOARDS ?= 
BOARDS += stm32f4_discovery HY-STM32_100P uemb1 uemb4 rrcv2 rrcv1

## test BOARD againt BOARDS
ifeq ($(filter $(BOARD),$(BOARDS)), )
$(error board '$(BOARD)' not supported. supported boards: $(BOARDS))
endif

## Select DEVICE and FAMILY, by BOARD

## configure BOARD
include $(BOARDDIR)/$(BOARD).bsp/board.mk

## if no network drivers have been enabled then set PHY selection to none
USE_DRIVER_ENC28J60_PHY ?= 0
USE_DRIVER_MII_RMII_PHY ?= 0

## configure BOARD SOURCE and CFLAGS
CFLAGS += -I$(DEVICE_SUPPORT_DIR)/board
CFLAGS += -I$(BOARDDIR)/$(BOARD).bsp
SOURCE += $(BOARDDIR)/$(BOARD).bsp/board_config.c


