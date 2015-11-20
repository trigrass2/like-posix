
BOARD_NAME = HY-STM32_100P
BOARDS += $(BOARD_NAME)
## configure BOARD
HSE_VALUE = 8000000
DEVICE = STM32F103xE
DATA_IN_ExtSRAM = 0
DATA_IN_ExtSDRAM = 0
ifeq ($(USE_DRIVER_LWIP_NET), 1)
USE_DRIVER_ENC28J60_PHY = 1
USE_DRIVER_MII_RMII_PHY = 0
endif

