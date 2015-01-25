
## DEVICE_SUPPORT_DIR
# the root directory of this project (where stm32_device_support.mk is) 
# must be defined in the variable DEVICE_SUPPORT_DIR, relative to the 
# directory where thw project make file is.


CFLAGS += -I$(DEVICE_SUPPORT_DIR)
include $(DEVICE_SUPPORT_DIR)/board/board.mk
include $(DEVICE_SUPPORT_DIR)/device/device.mk

###############################################################
# 
###############################################################
SYSTEMDIR = $(DEVICE_SUPPORT_DIR)/system

CFLAGS += -I$(SYSTEMDIR)
CFLAGS += -DDEBUG_PRINTF_EXCEPTIONS=$(DEBUG_PRINTF_EXCEPTIONS)

SOURCE += $(SYSTEMDIR)/system.c
SOURCE += $(SYSTEMDIR)/asserts.c
SOURCE += $(SYSTEMDIR)/hardware_exception.c
SOURCE += $(SYSTEMDIR)/services.c
ifeq ($(USE_FREERTOS), 1)
SOURCE += $(SYSTEMDIR)/stackoverflow.c
endif

###############################################################
# Drivers defined here must build for all targets
###############################################################

DRIVERSDIR = $(DEVICE_SUPPORT_DIR)/drivers

CFLAGS += -I$(DRIVERSDIR)

## System Clock timer
CFLAGS += -DUSE_DRIVER_SYSTEM_TIMER=$(USE_DRIVER_SYSTEM_TIMER)
ifeq ($(USE_DRIVER_SYSTEM_TIMER), 1)
SOURCE += $(DRIVERSDIR)/system_timer/systime.c
CFLAGS += -I$(DRIVERSDIR)/system_timer
endif

## LEDS
CFLAGS += -DUSE_DRIVER_LEDS=$(USE_DRIVER_LEDS)
ifeq ($(USE_DRIVER_LEDS), 1)
CFLAGS += -I$(DRIVERSDIR)/leds
SOURCE += $(DRIVERSDIR)/leds/leds.c
endif

## LCD
CFLAGS += -DUSE_DRIVER_LCD=$(USE_DRIVER_LCD)
ifeq ($(USE_DRIVER_LCD), 1)

ifneq ($(USE_FREERTOS), 1) 
$(error to use the LCD driver, USE_FREERTOS must be set to 1)
endif

CFLAGS += -I$(DRIVERSDIR)/lcd
SOURCE += $(DRIVERSDIR)/lcd/lcd.c
SOURCE += $(DRIVERSDIR)/touch_panel/tsc2046.c
SOURCE += $(DRIVERSDIR)/touch_panel/touch_panel.c
CFLAGS += -I $(DRIVERSDIR)/touch_panel
CFLAGS += -I $(DRIVERSDIR)/lcd
endif

## USART
CFLAGS += -DUSE_DRIVER_USART=$(USE_DRIVER_USART)
CFLAGS += -DUSE_STDIO_USART=$(USE_STDIO_USART)
ifeq ($(USE_DRIVER_USART), 1)
CFLAGS += -I$(DRIVERSDIR)/usart
SOURCE += $(DRIVERSDIR)/usart/usart.c
SOURCE += $(DRIVERSDIR)/usart/usart_it.c
endif

## SD Card, SDIO
CFLAGS += -DUSE_DRIVER_SDCARD=$(USE_DRIVER_SDCARD)
CFLAGS += -DUSE_THREAD_AWARE_SDCARD_DRIVER=$(USE_THREAD_AWARE_SDCARD_DRIVER)
ifeq ($(USE_DRIVER_SDCARD), 1)

ifneq ($(USE_DRIVER_SYSTEM_TIMER), 1) 
$(error to use the SDIO driver, USE_DRIVER_SYSTEM_TIMER must be set to 1)
endif

SOURCE += $(DRIVERSDIR)/sdcard/sdcard.c
SOURCE += $(DRIVERSDIR)/sdcard/sdfs.c
CFLAGS += -I$(DRIVERSDIR)/sdcard
endif

## Networking

CFLAGS += -DUSE_DRIVER_LWIP_NET=$(USE_DRIVER_LWIP_NET)
ifeq ($(USE_DRIVER_LWIP_NET), 1)

ifneq ($(USE_FREERTOS), 1) 
$(error to use posix style IO, USE_FREERTOS must be set to 1)
endif

CFLAGS += -I$(DRIVERSDIR)/net_lwip
CFLAGS += -I$(DRIVERSDIR)/net_lwip/netif

SOURCE += $(DRIVERSDIR)/net_lwip/net.c
SOURCE += $(DRIVERSDIR)/net_lwip/netconf.c
SOURCE += $(DRIVERSDIR)/net_lwip/netif/ethernetif.c
SOURCE += $(DRIVERSDIR)/net_lwip/arch/sys_arch.c

endif

## Ethernet PHY, ENC28J60
CFLAGS += -DUSE_DRIVER_ENC28J60_PHY=$(USE_DRIVER_ENC28J60_PHY)
ifeq ($(USE_DRIVER_ENC28J60_PHY), 1)
SOURCE += $(DRIVERSDIR)/ethernet/enc28j60.c
CFLAGS += -I$(DRIVERSDIR)/ethernet
CFLAGS += -DCHECKSUM_BY_HARDWARE=0
endif

## Ethernet PHY, BCM5241
CFLAGS += -DUSE_DRIVER_MII_RMII_PHY=$(USE_DRIVER_MII_RMII_PHY)
ifeq ($(USE_DRIVER_MII_RMII_PHY), 1)
SOURCE += $(DRIVERSDIR)/ethernet/MII_RMII.c
CFLAGS += -I$(DRIVERSDIR)/ethernet
CFLAGS += -DCHECKSUM_BY_HARDWARE=1
endif

## ADC streaming driver
CFLAGS += -DUSE_DRIVER_ADC_STREAM=$(USE_DRIVER_ADC_STREAM)
ifeq ($(USE_DRIVER_ADC_STREAM), 1)
SOURCE += $(DRIVERSDIR)/stream/adc/adc_stream.c
CFLAGS += -I$(DRIVERSDIR)/stream/adc
CFLAGS += -I$(DRIVERSDIR)/stream
endif

## DAC streaming driver
CFLAGS += -DUSE_DRIVER_DAC_STREAM=$(USE_DRIVER_DAC_STREAM)
ifeq ($(USE_DRIVER_DAC_STREAM), 1)
SOURCE += $(DRIVERSDIR)/stream/dac/dac_stream.c
CFLAGS += -I$(DRIVERSDIR)/stream/dac
CFLAGS += -I$(DRIVERSDIR)/stream
endif

###############################################################
# Include drivers for specific targets
###############################################################

## Configure drivers for FAMILY

ifeq ($(FAMILY), STM32F1)

endif

ifeq ($(FAMILY), STM32F4)

endif

