
BOARD_NAME = HY-STM32_100P
BOARDS += $(BOARD_NAME)
## configure BOARD
HSE_VALUE = 8000000
DEVICE = stm32f103ve
ifeq ($(USE_DRIVER_LWIP_NET), 1)
USE_DRIVER_ENC28J60_PHY = 1
USE_DRIVER_MII_RMII_PHY = 0
endif

