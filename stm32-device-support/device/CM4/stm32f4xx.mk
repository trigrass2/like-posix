
STM32_CM4_DIR = $(DEVICE_SUPPORT_DIR)/device/CM4

SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/misc.c
SOURCE += $(STM32_CM4_DIR)/system_stm32f4xx.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_adc.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_can.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_crc.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_cryp.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_dac.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_dbgmcu.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_dcmi.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_dma.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_exti.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_flash.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_fsmc.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_hash.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_gpio.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_i2c.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_iwdg.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_pwr.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_rcc.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_rng.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_rtc.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_sdio.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_spi.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_syscfg.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_tim.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_usart.c
SOURCE += $(STM32_CM4_DIR)/std_periph_drivers/src/stm32f4xx_wwdg.c

ifeq ($(USE_DRIVER_MII_RMII_PHY), 1)
SOURCE += $(STM32_CM4_DIR)/ethernet/stm32_eth.c
endif