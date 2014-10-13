
STM32_CM3_DIR = $(DEVICE_SUPPORT_DIR)/device/CM3

SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/misc.c
SOURCE += $(STM32_CM3_DIR)/core_cm3.c
SOURCE += $(STM32_CM3_DIR)/system_stm32f10x.c
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_adc.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_bkp.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_can.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_crc.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_dac.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_dbgmcu.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_dma.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_exti.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_flash.c
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_fsmc.c
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_gpio.c
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_i2c.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_iwdg.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_pwr.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_rcc.c
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_rtc.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_sdio.c 
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_spi.c
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_tim.c
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_usart.c
SOURCE += $(STM32_CM3_DIR)/std_periph_drivers/src/stm32f10x_wwdg.c 

ifeq ($(USE_DRIVER_MII_RMII_PHY), 1)
SOURCE += $(STM32_CM3_DIR)/ethernet/stm32_eth.c
endif