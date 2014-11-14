/*
 * Copyright (c) 2015 Michael Stuart.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the stm32-device-support project, <https://github.com/drmetal/stm32-device-support>
 *
 * based heavily on the code by Zizzle at: https://github.com/Zizzle/stm32_freertos_example/tree/master/drivers
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "tsc2046.h"


#define TSC2046_START  0x80
#define TSC2046_CH_X  0x50
#define TSC2046_CH_Y  0x10
#define TSC2046_READ_X  (TSC2046_START | TSC2046_CH_X)
#define TSC2046_READ_Y  (TSC2046_START | TSC2046_CH_Y)
#define TSC2046_MAX_X  4096
#define TSC2046_MAX_Y  4096

#if TSC2046_FLIP_Y
#define TSC2046_Y_OFFSET 239
#define TSC2046_PIX_Y  -240
#else
#define TSC2046_Y_OFFSET 0
#define TSC2046_PIX_Y  240
#endif
#if TSC2046_FLIP_X
#define TSC2046_X_OFFSET 319
#define TSC2046_PIX_X  -320
#else
#define TSC2046_X_OFFSET 0
#define TSC2046_PIX_X  320
#endif

#define tsc2046_select()      GPIO_ResetBits(TSC2046_NCS_PORT, TSC2046_NCS_PIN)
#define tsc2046_deselect()    GPIO_SetBits(TSC2046_NCS_PORT, TSC2046_NCS_PIN)
#define tsc2046_irq()         GPIO_ReadInputDataBit(TSC2046_IRQ_PORT, TSC2046_IRQ_PIN)
/**
 * http://e2e.ti.com/support/other_analog/touch/f/750/t/177249.aspx
 * ignore busy signal, is redundant and not documented in:
 * http://www.ti.com/lit/ds/symlink/tsc2046.pdf
 */
#define tsc2046_busy()        GPIO_ReadInputDataBit(TSC2046_BUSY_PORT, TSC2046_BUSY_PIN)


static void tsc2046_write(uint8_t Data);
static uint8_t tsc2046_read(void);


void tsc2046_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // configure SPI port
   // enable spi clock
    if((TSC2046_SPI_CLOCK == RCC_APB1Periph_SPI2)||(TSC2046_SPI_CLOCK == RCC_APB1Periph_SPI3))
        RCC_APB1PeriphClockCmd(TSC2046_SPI_CLOCK, ENABLE);
    else if (TSC2046_SPI_CLOCK == RCC_APB2Periph_SPI1)
        RCC_APB2PeriphClockCmd(TSC2046_SPI_CLOCK, ENABLE);
    else
        assert_true(0);

    SPI_InitTypeDef spi_init =
    {
        SPI_Direction_2Lines_FullDuplex,
        SPI_Mode_Master,
        SPI_DataSize_8b,
        SPI_CPOL_Low,
        SPI_CPHA_1Edge,
        SPI_NSS_Soft,
        TSC2046_SPI_PRESC,
        SPI_FirstBit_MSB,
        1
    };

    SPI_Init(TSC2046_SPI_PERIPH, &spi_init);
    SPI_Cmd(TSC2046_SPI_PERIPH, ENABLE);

    // configure IO's
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // input, pullup
#if FAMILY==STM32F1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
#elif FAMILY==STM32F4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
#endif
//    GPIO_InitStructure.GPIO_Pin = TSC2046_BUSY_PIN;
//    GPIO_Init(TSC2046_BUSY_PORT, &GPIO_InitStructure);

    // input, floating
#if FAMILY==STM32F1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#elif FAMILY==STM32F4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#endif
    GPIO_InitStructure.GPIO_Pin = TSC2046_IRQ_PIN;
    GPIO_Init(TSC2046_IRQ_PORT, &GPIO_InitStructure);

#if FAMILY==STM32F1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#elif FAMILY==STM32F4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#endif

    GPIO_InitStructure.GPIO_Pin = TSC2046_MISO_PIN;
    GPIO_Init(TSC2046_MISO_PORT, &GPIO_InitStructure);
#if FAMILY==STM32F4
    GPIO_PinAFConfig(TSC2046_MISO_PORT, TSC2046_MISO_PINSOURCE, TSC2046_AF_CONFIG);
#endif

    // output push pull
#if FAMILY==STM32F1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#elif FAMILY==STM32F4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#endif
    GPIO_InitStructure.GPIO_Pin = TSC2046_NCS_PIN;
    GPIO_Init(TSC2046_NCS_PORT, &GPIO_InitStructure);

    // output push pull alternate function

#if FAMILY==STM32F1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
#elif FAMILY==STM32F4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#endif

    GPIO_InitStructure.GPIO_Pin = TSC2046_SCK_PIN;
    GPIO_Init(TSC2046_SCK_PORT, &GPIO_InitStructure);
#if FAMILY==STM32F4
    GPIO_PinAFConfig(TSC2046_SCK_PORT, TSC2046_SCK_PINSOURCE, TSC2046_AF_CONFIG);
#endif

    GPIO_InitStructure.GPIO_Pin = TSC2046_MOSI_PIN;
    GPIO_Init(TSC2046_MOSI_PORT, &GPIO_InitStructure);
#if FAMILY==STM32F4
    GPIO_PinAFConfig(TSC2046_MOSI_PORT, TSC2046_MOSI_PINSOURCE, TSC2046_AF_CONFIG);
#endif
}

void tsc2046_write(uint8_t Data)
{
  while(SPI_I2S_GetFlagStatus(TSC2046_SPI_PERIPH, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(TSC2046_SPI_PERIPH, Data);
}

uint8_t tsc2046_read(void)
{
  while(SPI_I2S_GetFlagStatus(TSC2046_SPI_PERIPH, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(TSC2046_SPI_PERIPH, 0);
  while (SPI_I2S_GetFlagStatus(TSC2046_SPI_PERIPH, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(TSC2046_SPI_PERIPH);
}

int16_t tsc2046_x()
{
    int16_t x;
    if(tsc2046_irq() == SET)
        return -1;

    tsc2046_select();
    tsc2046_write(TSC2046_READ_X);
    x = ((tsc2046_read() & 0x7F) << 8);
    x |= tsc2046_read();
    x >>= 3;
    x = ((x * TSC2046_PIX_X) / TSC2046_MAX_X) + TSC2046_X_OFFSET;
    tsc2046_deselect();

    return x;
}

int16_t tsc2046_y()
{
    int16_t y;    
    if(tsc2046_irq() == SET)
        return -1;

    tsc2046_select();
    tsc2046_write(TSC2046_READ_Y);
    y = ((tsc2046_read() & 0x7F) << 8);
    y |= tsc2046_read();
    y >>= 3;
    y = ((y * TSC2046_PIX_Y) / TSC2046_MAX_Y) + TSC2046_Y_OFFSET;
    tsc2046_deselect();

    return y;
}
