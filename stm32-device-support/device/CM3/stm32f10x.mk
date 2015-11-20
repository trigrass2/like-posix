
STM32_CM3_DIR = $(DEVICE_SUPPORT_DIR)/device/CM3

SOURCE += $(STM32_CM3_DIR)/system_stm32f1xx.c

SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc_ex.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_can.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cec.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_crc.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dac.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dac_ex.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_eth.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_hcd.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2s.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_irda.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_iwdg.c
#SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_msp_template.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_nand.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_nor.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pccard.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pcd.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pcd_ex.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc_ex.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_sd.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_smartcard.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi_ex.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_sram.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_usart.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_wwdg.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_fsmc.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_sdmmc.c
SOURCE += $(STM32_CM3_DIR)/cube_drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_usb.c


ifeq ($(USE_DRIVER_MII_RMII_PHY), 1)
SOURCE += $(STM32_CM3_DIR)/ethernet/stm32_eth.c
endif