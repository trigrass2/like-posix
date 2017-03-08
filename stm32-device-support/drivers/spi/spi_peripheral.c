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
 * This file is part of the like-posix project, <https://github.com/drmetal/like-posix>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */


#include <stddef.h>
#include "board_config.h"
#include "spi.h"
#include "spi_it.h"
#include "base_spi.h"
#include "cutensils.h"
#include "spi_peripheral.h"

static spi_ioctl_t spi_ioctls[NUM_ONCHIP_SPIS];

SPI_HANDLE_t spi_init_device(SPI_TypeDef* spi, bool enable, uint32_t baudrate, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width)
{
    SPI_HandleTypeDef hspi;
    SPI_HANDLE_t spih = get_spi_handle(spi);
    spi_ioctl_t* spi_ioctl = get_spi_ioctl(spih);

	log_syslog(NULL, "init spi%d", spih);
	assert_true(spih != SPI_INVALID_HANDLE);

	spi_ioctl->spi = spi;
	spi_ioctl->baudrate = baudrate;
	spi_ioctl->bit_order = bit_order;
	spi_ioctl->clock_phase = clock_phase;
	spi_ioctl->clock_polarity = clock_polarity;
	spi_ioctl->data_width = data_width;

	// init SPI peripheral
	hspi.Instance = spi_ioctl->spi;
	hspi.Init.FirstBit = spi_ioctl->bit_order; 			            // SPI_FirstBit_MSB or SPI_FirstBit_LSB
	hspi.Init.CLKPhase = spi_ioctl->clock_phase; 					  // SPI_CPHA_1Edge or SPI_CPHA_2Edge
	hspi.Init.CLKPolarity = spi_ioctl->clock_polarity; 				// SPI_CPOL_Low or SPI_CPOL_High
	hspi.Init.DataSize = spi_ioctl->data_width;
	hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256; 	// SPI_BAUDRATEPRESCALER_2 to SPI_BAUDRATEPRESCALER_256
	hspi.Init.Mode = SPI_MODE_MASTER;
	hspi.Init.Direction = SPI_DIRECTION_2LINES;
	hspi.Init.NSS = SPI_NSS_SOFT;
	hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi.Init.CRCPolynomial = 1;

    if(enable)
    {
        if(hspi.Instance == SPI1){
            __HAL_RCC_SPI1_CLK_ENABLE();
        }
        else if(hspi.Instance == SPI2) {
            __HAL_RCC_SPI2_CLK_ENABLE();
        }
        else if(hspi.Instance == SPI3) {
            __HAL_RCC_SPI3_CLK_ENABLE();
        }
        HAL_SPI_Init(&hspi);
        __HAL_SPI_ENABLE(&hspi);
    }
    else
    {
        HAL_SPI_DeInit(&hspi);
        if(hspi.Instance == SPI1){
            __HAL_RCC_SPI1_CLK_DISABLE();
        }
        else if(hspi.Instance == SPI2){
            __HAL_RCC_SPI2_CLK_DISABLE();
        }
        else if(hspi.Instance == SPI3){
            __HAL_RCC_SPI3_CLK_DISABLE();
        }
        __HAL_SPI_DISABLE(&hspi);
    }

	spi_set_baudrate(spih, spi_ioctl->baudrate);

	return spih;
}

void spi_init_gpio(SPI_HANDLE_t spih)
{
    GPIO_InitTypeDef gpio_input_init;
    GPIO_InitTypeDef gpio_output_init;

    gpio_input_init.Mode = GPIO_MODE_AF_OD;
    gpio_input_init.Pull = GPIO_PULLUP;
    gpio_input_init.Speed = GPIO_SPEED_HIGH;

    gpio_output_init.Mode = GPIO_MODE_AF_PP;
    gpio_output_init.Pull = GPIO_NOPULL;
    gpio_output_init.Speed = GPIO_SPEED_HIGH;

	// config SPI clock, IO
    switch(spih)
	{
    	case SPI1_HANDLE:
#if FAMILY == STM32F1
#ifdef SPI1_REMAP
    		__HAL_AFIO_REMAP_SPI1_ENABLE();
#endif
#elif FAMILY == STM32F4
			gpio_output_init.Alternate = GPIO_AF5_SPI1;
			gpio_input_init.Alternate = GPIO_AF5_SPI1;
#endif

			gpio_input_init.Pin = SPI1_MISO_PIN;
			HAL_GPIO_Init(SPI1_PORT, &gpio_input_init);
			gpio_output_init.Pin = SPI1_MOSI_PIN;
			HAL_GPIO_Init(SPI1_PORT, &gpio_output_init);
			gpio_output_init.Pin = SPI1_SCK_PIN;
			HAL_GPIO_Init(SPI1_PORT, &gpio_output_init);
        break;

    	case SPI2_HANDLE:
#if FAMILY == STM32F4
			gpio_output_init.Alternate = GPIO_AF5_SPI2;
			gpio_input_init.Alternate = GPIO_AF5_SPI2;
#endif
			gpio_input_init.Pin = SPI2_MISO_PIN;
			HAL_GPIO_Init(SPI2_PORT, &gpio_input_init);
			gpio_output_init.Pin = SPI2_MOSI_PIN;
			HAL_GPIO_Init(SPI2_PORT, &gpio_output_init);
			gpio_output_init.Pin = SPI2_SCK_PIN;
			HAL_GPIO_Init(SPI2_PORT, &gpio_output_init);
		break;

    	case SPI3_HANDLE:
#if FAMILY == STM32F1
#ifdef SPI3_REMAP
        __HAL_AFIO_REMAP_SPI3_ENABLE();
#endif
#elif FAMILY == STM32F4
#if defined(STM32F407xx) || defined(STM32F417xx)
			gpio_output_init.Alternate = GPIO_AF6_SPI3;
			gpio_input_init.Alternate = GPIO_AF6_SPI3;
#else
			gpio_output_init.Alternate = GPIO_AF5_SPI3;
			gpio_input_init.Alternate = GPIO_AF5_SPI3;
#endif
#endif
			gpio_input_init.Pin = SPI3_MISO_PIN;
			HAL_GPIO_Init(SPI3_PORT, &gpio_input_init);
			gpio_output_init.Pin = SPI3_MOSI_PIN;
			HAL_GPIO_Init(SPI3_PORT, &gpio_output_init);
			gpio_output_init.Pin = SPI3_SCK_PIN;
			HAL_GPIO_Init(SPI3_PORT, &gpio_output_init);
        break;

    	default:
    		assert_true(0);
    	break;
	}
}

void spi_init_interrupt(SPI_HANDLE_t spih, uint8_t priority, bool enable)
{
    uint8_t irq = 0;

	SPI_HandleTypeDef hspi;
	hspi.Instance = get_spi_peripheral(spih);

	__HAL_SPI_DISABLE_IT(&hspi, SPI_IT_RXNE);
	__HAL_SPI_DISABLE_IT(&hspi, SPI_IT_TXE);
	__HAL_SPI_DISABLE_IT(&hspi, SPI_IT_ERR);

	if(spih == SPI1_HANDLE)
	    irq = SPI1_IRQn;
	else if(spih == SPI2_HANDLE)
	    irq = SPI2_IRQn;
	else if (spih == SPI3_HANDLE)
	    irq = SPI3_IRQn;
    else
        assert_true(0);

	if(enable)
    {
        HAL_NVIC_SetPriority(irq, priority, 0);
        HAL_NVIC_EnableIRQ(irq);
    }
    else
        HAL_NVIC_DisableIRQ(irq);
}

inline bool spi_rx_inwaiting(SPI_HANDLE_t spih)
{
	SPI_HandleTypeDef hspi;
	hspi.Instance = get_spi_peripheral(spih);
	return __HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_RXNE);
}

inline bool spi_tx_readytosend(SPI_HANDLE_t spih)
{
	SPI_HandleTypeDef hspi;
	hspi.Instance = get_spi_peripheral(spih);
	return __HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_TXE);
}

inline void spi_enable_rx_int(SPI_HANDLE_t spih)
{
	SPI_HandleTypeDef hspi;
	hspi.Instance = get_spi_peripheral(spih);
	__HAL_SPI_ENABLE_IT(&hspi, SPI_IT_RXNE);
}

inline void spi_enable_tx_int(SPI_HANDLE_t spih)
{
	SPI_HandleTypeDef hspi;
	hspi.Instance = get_spi_peripheral(spih);
	__HAL_SPI_ENABLE_IT(&hspi, SPI_IT_TXE);
}

inline void spi_disable_rx_int(SPI_HANDLE_t spih)
{
	SPI_HandleTypeDef hspi;
	hspi.Instance = get_spi_peripheral(spih);
	__HAL_SPI_DISABLE_IT(&hspi, SPI_IT_RXNE);
}

inline void spi_disable_tx_int(SPI_HANDLE_t spih)
{
	SPI_HandleTypeDef hspi;
	hspi.Instance = get_spi_peripheral(spih);
	__HAL_SPI_DISABLE_IT(&hspi, SPI_IT_TXE);
}

SPI_HANDLE_t get_spi_handle(SPI_TypeDef* spi)
{
	if(spi == SPI1)
		return SPI1_HANDLE;
	else if(spi == SPI2)
		return SPI2_HANDLE;
	else if (spi == SPI3)
		return SPI3_HANDLE;

	assert_true(0);
	return SPI_INVALID_HANDLE;
}

SPI_TypeDef* get_spi_peripheral(SPI_HANDLE_t spih)
{
	if(spih == SPI1_HANDLE)
		return SPI1;
	else if(spih == SPI2_HANDLE)
		return SPI2;
	else if (spih == SPI3_HANDLE)
		return SPI3;

	assert_true(0);
	return NULL;
}

spi_ioctl_t* get_spi_ioctl(SPI_HANDLE_t spih)
{
	return &spi_ioctls[spih];
}

void spi_tx(SPI_HANDLE_t spih, const uint8_t data)
{
    SPI_HandleTypeDef hspi;
    SPI_TypeDef* spi = get_spi_peripheral(spih);
    hspi.Instance = spi;
    while(!__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_TXE));
    spi->DR = data;
}

uint8_t spi_rx(SPI_HANDLE_t spih)
{
    SPI_HandleTypeDef hspi;
    SPI_TypeDef* spi = get_spi_peripheral(spih);
    hspi.Instance = spi;
    while(!__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_RXNE));
    return spi->DR;
}


/**
 * sets the prescaler value of the specified SPI peripheral (can be SPI_BAUDRATEPRESCALER_2/4/8/16/32/64/128/256)
 */
void spi_set_prescaler(SPI_HANDLE_t spih, uint16_t presc)
{
    SPI_HandleTypeDef hspi;
    SPI_TypeDef* spi = get_spi_peripheral(spih);
    hspi.Instance = spi;
	while(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_BSY));
	spi->CR1 &= ~SPI_CR1_BR;
	spi->CR1 |= presc;
}

/**
 * sets the baudrate of the SPI port to the nearest (or lower) possible baudrate to that specified, in Hz.
 */
void spi_set_baudrate(SPI_HANDLE_t spih, uint32_t baudrate)
{
    uint32_t integerdivider;
    uint16_t presc;

    spi_ioctl_t* spi_ioctl = get_spi_ioctl(spih);
    spi_ioctl->baudrate = baudrate;

    if(spih == SPI1_HANDLE)
        integerdivider = HAL_RCC_GetPCLK2Freq() / baudrate;
    else
        integerdivider = HAL_RCC_GetPCLK1Freq() / baudrate;

    if(integerdivider > 128)
        presc = SPI_BAUDRATEPRESCALER_256;
    else if(integerdivider > 64)
        presc = SPI_BAUDRATEPRESCALER_128;
    else if(integerdivider > 32)
        presc = SPI_BAUDRATEPRESCALER_64;
    else if(integerdivider > 16)
        presc = SPI_BAUDRATEPRESCALER_32;
    else if(integerdivider > 8)
        presc = SPI_BAUDRATEPRESCALER_16;
    else if(integerdivider > 4)
        presc = SPI_BAUDRATEPRESCALER_8;
    else if(integerdivider > 2)
        presc = SPI_BAUDRATEPRESCALER_4;
    else
        presc = SPI_BAUDRATEPRESCALER_2;

    spi_set_prescaler(spih, presc);
}

/**
 * returns the baudrate of the SPI port in Hz.
 */
uint32_t spi_get_baudrate(SPI_HANDLE_t spih)
{
    uint32_t apbclock;
    SPI_TypeDef* spi = get_spi_peripheral(spih);

    if(spih == SPI1_HANDLE)
        apbclock = HAL_RCC_GetPCLK2Freq();
    else
        apbclock = HAL_RCC_GetPCLK1Freq();

    if(spi->CR1 & SPI_BAUDRATEPRESCALER_256)
        return apbclock / 256;
    else if(spi->CR1 & SPI_BAUDRATEPRESCALER_128)
        return apbclock / 128;
    else if(spi->CR1 & SPI_BAUDRATEPRESCALER_64)
        return apbclock / 64;
    else if(spi->CR1 & SPI_BAUDRATEPRESCALER_32)
        return apbclock / 32;
    else if(spi->CR1 & SPI_BAUDRATEPRESCALER_16)
        return apbclock / 16;
    else if(spi->CR1 & SPI_BAUDRATEPRESCALER_8)
        return apbclock / 8;
    else if(spi->CR1 & SPI_BAUDRATEPRESCALER_4)
        return apbclock / 4;
    else if(spi->CR1 & SPI_BAUDRATEPRESCALER_2)
        return apbclock / 2;
    return 0;
}

