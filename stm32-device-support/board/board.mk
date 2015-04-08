
BOARDDIR = $(DEVICE_SUPPORT_DIR)/board
DEVICEDIR = $(DEVICE_SUPPORT_DIR)/device

## Supported BOARDS
BOARDS = stm32f4_discovery HY-STM32_100P uemb1 uemb4 rrcv2 rrcv1

## test BOARD againt BOARDS
ifeq ($(filter $(BOARD),$(BOARDS)), )
$(error board '$(BOARD)' not supported. supported boards: $(BOARDS))
endif

## Select DEVICE and FAMILY, by BOARD
## configure BOARD

ifeq ($(BOARD),  stm32f4_discovery)
HSE_VALUE = 8000000
FAMILY = STM32F4
DEVICE = stm32f407vg
ifeq ($(USE_DRIVER_LWIP_NET), 1)
USE_DRIVER_ENC28J60_PHY = 1
endif
endif

ifeq ($(BOARD),  uemb1)
HSE_VALUE = 8000000
FAMILY = STM32F1
DEVICE = stm32f103ve
ifeq ($(USE_DRIVER_LWIP_NET), 1)
USE_DRIVER_ENC28J60_PHY = 1
endif
endif

ifeq ($(BOARD),  uemb4)
HSE_VALUE = 8000000
FAMILY = STM32F4
DEVICE = stm32f407ve
ifeq ($(USE_DRIVER_LWIP_NET), 1)
USE_DRIVER_ENC28J60_PHY = 1
endif
endif

ifeq ($(BOARD),  rrcv1)
HSE_VALUE = 25000000
FAMILY = STM32F1
DEVICE = stm32f107rc
ifeq ($(USE_DRIVER_LWIP_NET), 1)
USE_DRIVER_MII_RMII_PHY = 1
endif
endif

ifeq ($(BOARD),  rrcv2)
HSE_VALUE = 25000000
FAMILY = STM32F4
DEVICE = stm32f407vg
ifeq ($(USE_DRIVER_LWIP_NET), 1)
USE_DRIVER_MII_RMII_PHY = 1
endif
endif

ifeq ($(BOARD),  HY-STM32_100P)
HSE_VALUE = 8000000
FAMILY = STM32F1
DEVICE = stm32f103ve
ifeq ($(USE_DRIVER_LWIP_NET), 1)
USE_DRIVER_ENC28J60_PHY = 1
endif
endif

USE_DRIVER_ENC28J60_PHY ?= 0
USE_DRIVER_MII_RMII_PHY ?= 0

## configure BOARD SOURCE and CFLAGS
CFLAGS += -I$(BOARDDIR)
CFLAGS += -I$(BOARDDIR)/$(BOARD)
SOURCE += $(BOARDDIR)/$(BOARD)/board_config.c


