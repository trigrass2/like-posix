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
 * This file is part of the Appleseed project, <https://github.com/drmetal/appleseed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#if USE_LIKEPOSIX
#include "syscalls.h"
#endif

#include <stddef.h>
#include "spi.h"
#include "spi_it.h"
#include "base_spi.h"
#include "cutensils.h"


#if USE_LIKEPOSIX
static int spi_ioctl(dev_ioctl_t* dev);
static int spi_close_ioctl(dev_ioctl_t* dev);
static int spi_open_ioctl(dev_ioctl_t* dev);
static int spi_enable_tx_ioctl(dev_ioctl_t* dev);
static int spi_enable_rx_ioctl(dev_ioctl_t* dev);
#endif

static int8_t get_spi_devno(SPI_TypeDef* spi);
// used in the interrupt handlers....
void* spi_dev_ioctls[NUM_ONCHIP_SPIS];

static void spi_init_device(SPI_TypeDef* spi, bool enable);
static void spi_init_gpio(SPI_TypeDef* spi);
static void spi_init_interrupt(SPI_TypeDef* spi, uint8_t priority, bool enable);


/**
 * call this function to initialize an SPI port in polled mode,
 * or to install an SPI port as a device file (requires USE_LIKEPOSIX=1 in the Makefile).
 *
 * when installed as device file, posix system calls amy be made including open, close, read, write, etc.
 * termios functions are also available (tcgetattr, tcsetattr, etc)
 * file stream functions are also available (fputs, fgets, etc)
 *
 * @param spi is the SPI peripheral to initialize.
 * @param filename is the device file name to install as Eg "/dev/ttySPI0".
 *        if set to NULL, the device may be used in polled mode only.
 * @param enable - set to true when using in polled mode. when the device file is specified,
 *        set to false - the device is enabled automatically when the file is opened.
 */
bool spi_init(SPI_TypeDef* spi, char* filename, bool enable)
{
    bool ret = true;
    int8_t spi_devno = get_spi_devno(spi);

    log_syslog(NULL, "init spi%d", spi_devno+1);

    assert_true(spi_devno != -1);

    if(filename)
    {
#if USE_LIKEPOSIX
        // installed SPI can only work with interrupt enabled
        spi_init_interrupt(spi, SPI_INTERRUPT_PRIORITY, true);
        spi_dev_ioctls[spi_devno] = (void*)install_device(filename,
                                                        spi,
                                                        spi_enable_rx_ioctl,
                                                        spi_enable_tx_ioctl,
                                                        spi_open_ioctl,
                                                        spi_close_ioctl,
                                                        spi_ioctl);
#else
        // todo - init fifo's and interrupts for freertos independent version
//        spi_init_interrupt(spi, SPI_INTERRUPT_PRIORITY, true);
        spi_dev_ioctls[spi_devno] = NULL;
#endif
        ret = spi_dev_ioctls[spi_devno] != NULL;
        log_syslog(NULL, "install spi%d: %s", spi_devno+1, ret ? "successful" : "failed");
    }

    spi_init_gpio(spi);
    spi_init_device(spi, enable);

    return ret;
}

/**
 * asserts the NSS pin for the specified SPI peripheral (NSS pin is configured in spi_config.h for the board)
 */
void spi_assert_nss(SPI_TypeDef* spi)
{
#ifdef SPI1_NSS_PIN
	if(spi == SPI1)
	    HAL_GPIO_WritePin(SPI1_NSS_PORT, SPI1_NSS_PIN, GPIO_PIN_RESET);
#endif
#ifdef SPI2_NSS_PIN
	if(spi == SPI2)
	    HAL_GPIO_WritePin(SPI2_NSS_PORT, SPI2_NSS_PIN, GPIO_PIN_RESET);
#endif
#ifdef SPI3_NSS_PIN
	if(spi == SPI3)
	    HAL_GPIO_WritePin(SPI3_NSS_PORT, SPI3_NSS_PIN, GPIO_PIN_RESET);
#endif
}

/**
 * deasserts the NSS pin for the specified SPI peripheral (NSS pin is configured in spi_config.h for the board)
 */
void spi_deassert_nss(SPI_TypeDef* spi)
{
#ifdef SPI1_NSS_PIN
    if(spi == SPI1)
        HAL_GPIO_WritePin(SPI1_NSS_PORT, SPI1_NSS_PIN, GPIO_PIN_SET);
#endif
#ifdef SPI2_NSS_PIN
    if(spi == SPI2)
        HAL_GPIO_WritePin(SPI2_NSS_PORT, SPI2_NSS_PIN, GPIO_PIN_SET);
#endif
#ifdef SPI3_NSS_PIN
    if(spi == SPI3)
        HAL_GPIO_WritePin(SPI3_NSS_PORT, SPI3_NSS_PIN, GPIO_PIN_SET);
#endif
}

/**
 * sends and receives one byte on the specified SPI peripheral.
 */
uint8_t spi_transfer(SPI_TypeDef* spi, uint8_t data)
{
    SPI_HandleTypeDef hspi;
    hspi.Instance = spi;
    while(!__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_TXE));
    spi->DR =  data;
    while(!__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_RXNE));
    return spi->DR;
}

/**
 * sets the prescaler value of the specified SPI peripheral (can be SPI_BAUDRATEPRESCALER_2/4/8/16/32/64/128/256)
 */
void spi_set_prescaler(SPI_TypeDef* spi, uint16_t presc)
{
    SPI_HandleTypeDef hspi;
    hspi.Instance = spi;
	while(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_BSY));
	spi->CR1 &= ~SPI_CR1_BR;
	spi->CR1 |= presc;
}

/**
 * sets the baudrate of the SPI port to the nearest (or lower) possible baudrate to that specified, in Hz.
 */
void spi_set_baudrate(SPI_TypeDef* spi, uint32_t br)
{
    uint32_t integerdivider;
    uint16_t presc;

    if(spi == SPI1)
        integerdivider = HAL_RCC_GetPCLK2Freq() / br;
    else
        integerdivider = HAL_RCC_GetPCLK1Freq() / br;

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

    spi_set_prescaler(spi, presc);
}

/**
 * returns the baudrate of the SPI port in Hz.
 */
uint32_t spi_get_baudrate(SPI_TypeDef* spi)
{
    uint32_t apbclock;

    if(spi == SPI1)
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


/**
 * private functions
 */

int8_t get_spi_devno(SPI_TypeDef* spi)
{
	if(spi == SPI1)
		return 0;
	else if(spi == SPI2)
		return 1;
	else if (spi == SPI3)
		return 2;

	return -1;
}

void spi_init_device(SPI_TypeDef* spi, bool enable)
{
    SPI_HandleTypeDef hspi;

	// init SPI peripheral
	hspi.Instance = spi;
	hspi.Init.FirstBit = SPI_FIRSTBIT_MSB; 			// SPI_FirstBit_MSB or SPI_FirstBit_LSB
	hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256; 	// SPI_BAUDRATEPRESCALER_2 to SPI_BAUDRATEPRESCALER_256
	hspi.Init.CLKPhase = SPI_PHASE_1EDGE; 					// SPI_CPHA_1Edge or SPI_CPHA_2Edge
	hspi.Init.CLKPolarity = SPI_POLARITY_LOW; 					// SPI_CPOL_Low or SPI_CPOL_High
	hspi.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi.Init.Mode = SPI_MODE_MASTER;
	hspi.Init.Direction = SPI_DIRECTION_2LINES;
	hspi.Init.NSS = SPI_NSS_SOFT;
	hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

    if(enable)
        HAL_SPI_Init(&hspi);
    else
        HAL_SPI_DeInit(&hspi);
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
    if(hspi->Instance == SPI1)
        __HAL_RCC_SPI1_CLK_ENABLE();
    else if(hspi->Instance == SPI2)
        __HAL_RCC_SPI2_CLK_ENABLE();
    else if(hspi->Instance == SPI3)
        __HAL_RCC_SPI3_CLK_ENABLE();
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
    if(hspi->Instance == SPI1)
        __HAL_RCC_SPI1_CLK_DISABLE();
    else if(hspi->Instance == SPI2)
        __HAL_RCC_SPI2_CLK_DISABLE();
    else if(hspi->Instance == SPI3)
        __HAL_RCC_SPI3_CLK_DISABLE();
}

void spi_init_gpio(SPI_TypeDef* spi)
{
    GPIO_InitTypeDef GPIO_InitStructure_rx;
    GPIO_InitTypeDef GPIO_InitStructure_tx;

    GPIO_InitStructure_rx.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure_rx.Pull = GPIO_PULLUP;
    GPIO_InitStructure_rx.Speed = GPIO_SPEED_HIGH;
#if FAMILY == STM32F4
        GPIO_InitStructure_rx.Alternate = 0;
#endif

    GPIO_InitStructure_tx.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure_tx.Pull = GPIO_NOPULL;
    GPIO_InitStructure_tx.Speed = GPIO_SPEED_HIGH;
#if FAMILY == STM32F4
        GPIO_InitStructure_tx.Alternate = 0;
#endif

	// config SPI clock, IO
	if(spi == SPI1)
	{
#ifdef SPI1_NSS_PIN
	    GPIO_InitStructure_tx.Pin = SPI1_NSS_PIN;
		HAL_GPIO_Init(SPI1_NSS_PORT, &GPIO_InitStructure_tx);
#else
#pragma message "SPI1 NSS pin not configured"
#endif

		GPIO_InitStructure_tx.Mode = GPIO_MODE_AF_PP;

#if FAMILY == STM32F1
#ifdef SPI1_REMAP
	    __HAL_AFIO_REMAP_SPI1_ENABLE();
#endif
#elif FAMILY == STM32F4
	    GPIO_InitStructure_tx.Alternate = GPIO_AF5_SPI1;
	    GPIO_InitStructure_rx.Alternate = GPIO_AF5_SPI1;
#endif

	    GPIO_InitStructure_rx.Pin = SPI1_MISO_PIN;
	    HAL_GPIO_Init(SPI1_PORT, &GPIO_InitStructure_rx);
        GPIO_InitStructure_tx.Pin = SPI1_MOSI_PIN;
        HAL_GPIO_Init(SPI1_PORT, &GPIO_InitStructure_tx);
        GPIO_InitStructure_tx.Pin = SPI1_SCK_PIN;
        HAL_GPIO_Init(SPI1_PORT, &GPIO_InitStructure_tx);
	}
	else if(spi == SPI2)
	{
#ifdef SPI2_NSS_PIN
        GPIO_InitStructure_tx.Pin = SPI2_NSS_PIN;
        HAL_GPIO_Init(SPI2_NSS_PORT, &GPIO_InitStructure_tx);
#else
#pragma message "SPI2 NSS pin not configured"
#endif

        GPIO_InitStructure_tx.Mode = GPIO_MODE_AF_PP;

#if FAMILY == STM32F4
        GPIO_InitStructure_tx.Alternate = GPIO_AF5_SPI2;
        GPIO_InitStructure_rx.Alternate = GPIO_AF5_SPI2;
#endif

        GPIO_InitStructure_rx.Pin = SPI2_MISO_PIN;
        HAL_GPIO_Init(SPI2_PORT, &GPIO_InitStructure_rx);
        GPIO_InitStructure_tx.Pin = SPI2_MOSI_PIN;
        HAL_GPIO_Init(SPI2_PORT, &GPIO_InitStructure_tx);
        GPIO_InitStructure_tx.Pin = SPI2_SCK_PIN;
        HAL_GPIO_Init(SPI2_PORT, &GPIO_InitStructure_tx);
	}
	else if(spi == SPI3)
	{
#ifdef SPI3_NSS_PIN
        GPIO_InitStructure_tx.Pin = SPI3_NSS_PIN;
        HAL_GPIO_Init(SPI3_NSS_PORT, &GPIO_InitStructure_tx);
#else
#pragma message "SPI3 NSS pin not configured"
#endif

        GPIO_InitStructure_tx.Mode = GPIO_MODE_AF_PP;

#if FAMILY == STM32F1
#ifdef SPI3_REMAP
        __HAL_AFIO_REMAP_SPI3_ENABLE();
#endif
#elif FAMILY == STM32F4
        GPIO_InitStructure_tx.Alternate = GPIO_AF5_SPI3;
        GPIO_InitStructure_rx.Alternate = GPIO_AF5_SPI3;
#endif

        GPIO_InitStructure_rx.Pin = SPI3_MISO_PIN;
        HAL_GPIO_Init(SPI3_PORT, &GPIO_InitStructure_rx);
        GPIO_InitStructure_tx.Pin = SPI3_MOSI_PIN;
        HAL_GPIO_Init(SPI3_PORT, &GPIO_InitStructure_tx);
        GPIO_InitStructure_tx.Pin = SPI3_SCK_PIN;
        HAL_GPIO_Init(SPI3_PORT, &GPIO_InitStructure_tx);
	}
}

void spi_init_interrupt(SPI_TypeDef* spi, uint8_t priority, bool enable)
{
    uint8_t irq = 0;

	if(spi == SPI1)
	    irq = SPI1_IRQn;
	else if(spi == SPI2)
	    irq = SPI2_IRQn;
	else if (spi == SPI3)
	    irq = SPI3_IRQn;
    else
        assert_true(0);

	if(enable)
    {
        HAL_NVIC_SetPriority(irq, 0, priority);
        HAL_NVIC_EnableIRQ(irq);
    }
    else
        HAL_NVIC_DisableIRQ(irq);
}

#if USE_LIKEPOSIX
static int spi_enable_rx_ioctl(dev_ioctl_t* dev)
{
    SPI_HandleTypeDef hspi;
    hspi.Instance = (SPI_TypeDef*)(dev->ctx);
    __HAL_SPI_ENABLE_IT(&hspi, SPI_IT_RXNE);
    return 0;
}

static int spi_enable_tx_ioctl(dev_ioctl_t* dev)
{
    SPI_HandleTypeDef hspi;
    hspi.Instance = (SPI_TypeDef*)(dev->ctx);
    __HAL_SPI_ENABLE_IT(&hspi, SPI_IT_TXE);
    return 0;
}

static int spi_open_ioctl(dev_ioctl_t* dev)
{
    SPI_TypeDef* spi = (SPI_TypeDef*)(dev->ctx);
    spi_init_gpio(spi);
    spi_init_device(spi, true);
    return 0;
}

static int spi_close_ioctl(dev_ioctl_t* dev)
{
    SPI_HandleTypeDef hspi;
    hspi.Instance = (SPI_TypeDef*)(dev->ctx);
    __HAL_SPI_DISABLE_IT(&hspi, SPI_IT_RXNE);
    __HAL_SPI_DISABLE_IT(&hspi, SPI_IT_TXE);
    spi_init_device(hspi.Instance, false);
    return 0;
}

static int spi_ioctl(dev_ioctl_t* dev)
{
    SPI_TypeDef* spi = (SPI_TypeDef*)(dev->ctx);

    uint32_t baudrate = dev->termios->c_ispeed ?
            dev->termios->c_ispeed : dev->termios->c_ospeed;
    if(baudrate)
    {
        // set baudrate
        spi_set_baudrate(spi, baudrate);
    }
    else
    {
        // read baudrate
        dev->termios->c_ispeed = spi_get_baudrate(spi);
        dev->termios->c_ospeed = dev->termios->c_ispeed;
    }

    if(dev->termios->c_cc[VTIME] > 0)
    {
        // set device timeout in units of 10ms
        dev->timeout = 10 * dev->termios->c_cc[VTIME];
    }
    else
    {
        dev->termios->c_cc[VTIME] = dev->timeout / 10;
    }

    // set character size

    // set character size

    // set parity

    // get parity

    // set parity polarity

    // get parity polarity

    // set stopbits

    // get stopbits


    return 0;
}

#endif
