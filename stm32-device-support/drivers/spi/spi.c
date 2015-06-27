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
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#if USE_POSIX_STYLE_IO
#include "syscalls.h"
#endif
#include "spi.h"
#include "spi_it.h"
#include "base_spi.h"
#include "cutensils.h"


#if USE_POSIX_STYLE_IO
static int spi_ioctl(dev_ioctl_t* dev);
static int spi_close_ioctl(dev_ioctl_t* dev);
static int spi_open_ioctl(dev_ioctl_t* dev);
static int spi_enable_tx_ioctl(dev_ioctl_t* dev);
static int spi_enable_rx_ioctl(dev_ioctl_t* dev);
static int8_t get_spi_devno(SPI_TypeDef* spi);
// used in the interrupt handlers....
void* spi_dev_ioctls[NUM_ONCHIP_SPIS];
#endif

static void spi_init_device(SPI_TypeDef* spi, FunctionalState enable);
static void spi_init_gpio(SPI_TypeDef* spi);
static void spi_init_interrupt(SPI_TypeDef* spi, uint8_t priority, FunctionalState enable);


/**
 * call this function to initialize an SPI port in polled mode,
 * or to install an SPI port as a device file (requires USE_POSIX_STYLE_IO=1 in the Makefile).
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

    log_syslog(NULL, "init spi%d", spi_devno);

    assert_true(spi_devno != -1);

    if(filename)
    {
#if USE_POSIX_STYLE_IO
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
        log_syslog(NULL, "install spi%d: %s", spi_devno, ret ? "successful" : "failed");
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
		GPIO_ResetBits(SPI1_NSS_PORT, SPI1_NSS_PIN);
#endif
#ifdef SPI2_NSS_PIN
	if(spi == SPI2)
		GPIO_ResetBits(SPI2_NSS_PORT, SPI2_NSS_PIN);
#endif
#ifdef SPI3_NSS_PIN
	if(spi == SPI3)
		GPIO_ResetBits(SPI3_NSS_PORT, SPI3_NSS_PIN);
#endif
}

/**
 * deasserts the NSS pin for the specified SPI peripheral (NSS pin is configured in spi_config.h for the board)
 */
void spi_deassert_nss(SPI_TypeDef* spi)
{
#ifdef SPI1_NSS_PIN
	if(spi == SPI1)
		GPIO_SetBits(SPI1_NSS_PORT, SPI1_NSS_PIN);
#endif
#ifdef SPI2_NSS_PIN
	if(spi == SPI2)
		GPIO_SetBits(SPI2_NSS_PORT, SPI2_NSS_PIN);
#endif
#ifdef SPI3_NSS_PIN
	if(spi == SPI3)
		GPIO_SetBits(SPI3_NSS_PORT, SPI3_NSS_PIN);
#endif
}

/**
 * sends and receives one byte on the specified SPI peripheral.
 */
uint8_t spi_transfer(SPI_TypeDef* spi, uint8_t data)
{
  while(SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(spi, data);
  while(SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(spi);
}

/**
 * sets the prescaler value of the specified SPI peripheral (can be SPI_BaudRatePrescaler_2/4/8/16/32/64/128/256)
 */
void spi_set_prescaler(SPI_TypeDef* spi, uint16_t presc)
{
	while(SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_BSY) == SET);
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
    RCC_ClocksTypeDef RCC_ClocksStatus;

    RCC_GetClocksFreq(&RCC_ClocksStatus);

    if(spi == SPI1)
        integerdivider = RCC_ClocksStatus.PCLK2_Frequency / br;
    else
        integerdivider = RCC_ClocksStatus.PCLK1_Frequency / br;

    if(integerdivider > 128)
        presc = SPI_BaudRatePrescaler_256;
    else if(integerdivider > 64)
        presc = SPI_BaudRatePrescaler_128;
    else if(integerdivider > 32)
        presc = SPI_BaudRatePrescaler_64;
    else if(integerdivider > 16)
        presc = SPI_BaudRatePrescaler_32;
    else if(integerdivider > 8)
        presc = SPI_BaudRatePrescaler_16;
    else if(integerdivider > 4)
        presc = SPI_BaudRatePrescaler_8;
    else if(integerdivider > 2)
        presc = SPI_BaudRatePrescaler_4;
    else
        presc = SPI_BaudRatePrescaler_2;

    spi_set_prescaler(spi, presc);
}

/**
 * returns the baudrate of the SPI port in Hz.
 */
uint32_t spi_get_baudrate(SPI_TypeDef* spi)
{
    uint32_t apbclock;
    RCC_ClocksTypeDef RCC_ClocksStatus;

    RCC_GetClocksFreq(&RCC_ClocksStatus);

    if(spi == SPI1)
        apbclock = RCC_ClocksStatus.PCLK2_Frequency;
    else
        apbclock = RCC_ClocksStatus.PCLK1_Frequency;

    if(spi->CR1 & SPI_BaudRatePrescaler_256)
        return apbclock / 256;
    else if(spi->CR1 & SPI_BaudRatePrescaler_128)
        return apbclock / 128;
    else if(spi->CR1 & SPI_BaudRatePrescaler_64)
        return apbclock / 64;
    else if(spi->CR1 & SPI_BaudRatePrescaler_32)
        return apbclock / 32;
    else if(spi->CR1 & SPI_BaudRatePrescaler_16)
        return apbclock / 16;
    else if(spi->CR1 & SPI_BaudRatePrescaler_8)
        return apbclock / 8;
    else if(spi->CR1 & SPI_BaudRatePrescaler_4)
        return apbclock / 4;
    else if(spi->CR1 & SPI_BaudRatePrescaler_2)
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
#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32F4XX)
	else if (spi == SPI3)
		return 2;
#endif
	return -1;
}

void spi_init_device(SPI_TypeDef* spi, FunctionalState enable)
{
	SPI_InitTypeDef device_init;

	// config SPI clock, IO
	if(spi == SPI1)
		RCC_APB2PeriphClockCmd(SPI1_CLOCK, enable);
	else if(spi == SPI2)
		RCC_APB1PeriphClockCmd(SPI2_CLOCK, enable);
#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32F4XX)
	else if(spi == SPI3)
		RCC_APB1PeriphClockCmd(SPI3_CLOCK, enable);
#endif

	// init SPI peripheral
	device_init.SPI_FirstBit = SPI_FirstBit_MSB; 			// SPI_FirstBit_MSB or SPI_FirstBit_LSB
	device_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; 	// SPI_BaudRatePrescaler_2 to SPI_BaudRatePrescaler_256
	device_init.SPI_CPHA = SPI_CPHA_1Edge; 					// SPI_CPHA_1Edge or SPI_CPHA_2Edge
	device_init.SPI_CPOL = SPI_CPOL_Low; 					// SPI_CPOL_Low or SPI_CPOL_High
	device_init.SPI_DataSize = SPI_DataSize_8b;
	device_init.SPI_Mode = SPI_Mode_Master;
	device_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	device_init.SPI_NSS = SPI_NSS_Soft;

	SPI_Init(spi, &device_init);
	SPI_Cmd(spi, enable);
}

void spi_init_gpio(SPI_TypeDef* spi)
{
#if FAMILY == STM32F1
    GPIO_InitTypeDef input_init = {
        .GPIO_Mode = GPIO_Mode_IN_FLOATING,
        .GPIO_Speed = GPIO_Speed_50MHz,
    };
    GPIO_InitTypeDef altfunc_init = {
        .GPIO_Mode = GPIO_Mode_AF_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
    };
    GPIO_InitTypeDef output_init = {
        .GPIO_Mode = GPIO_Mode_Out_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
    };
#elif FAMILY == STM32F4
    GPIO_InitTypeDef input_init = {
        .GPIO_Mode = GPIO_Mode_AF,
        .GPIO_Speed = GPIO_Speed_100MHz,
        .GPIO_OType = GPIO_OType_OD,
        .GPIO_PuPd = GPIO_PuPd_NOPULL,
    };
    GPIO_InitTypeDef altfunc_init = {
        .GPIO_Mode = GPIO_Mode_AF,
        .GPIO_Speed = GPIO_Speed_100MHz,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_PuPd = GPIO_PuPd_NOPULL,
    };
    GPIO_InitTypeDef output_init = {
        .GPIO_Mode = GPIO_Mode_OUT,
        .GPIO_Speed = GPIO_Speed_100MHz,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_PuPd = GPIO_PuPd_NOPULL,
    };
#endif

	// config SPI clock, IO
	if(spi == SPI1)
	{
#if FAMILY == STM32F1
#ifdef SPI1_REMAP
		GPIO_PinRemapConfig(SPI1_REMAP, ENABLE);
#endif
#elif FAMILY == STM32F4
        GPIO_PinAFConfig(SPI1_PORT, SPI1_MISO_PINSOURCE, GPIO_AF_SPI1);
        GPIO_PinAFConfig(SPI1_PORT, SPI1_MOSI_PINSOURCE, GPIO_AF_SPI1);
        GPIO_PinAFConfig(SPI1_PORT, SPI1_SCK_PINSOURCE, GPIO_AF_SPI1);
#endif
        input_init.GPIO_Pin = SPI1_MISO_PIN;
        GPIO_Init(SPI1_PORT, &input_init);
        altfunc_init.GPIO_Pin = SPI1_MOSI_PIN;
        GPIO_Init(SPI1_PORT, &altfunc_init);
        altfunc_init.GPIO_Pin = SPI1_SCK_PIN;
        GPIO_Init(SPI1_PORT, &altfunc_init);
#ifdef SPI1_NSS_PIN
		output_init.GPIO_Pin = SPI1_NSS_PIN;
	    GPIO_Init(SPI1_NSS_PORT, &output_init);
#else
#pragma message "SPI1 NSS pin not configured"
#endif
	}
	else if(spi == SPI2)
	{
#if FAMILY == STM32F1
#ifdef SPI2_REMAP
		GPIO_PinRemapConfig(SPI2_REMAP, ENABLE);
#endif
#elif FAMILY == STM32F4
        GPIO_PinAFConfig(SPI2_PORT, SPI2_MISO_PINSOURCE, GPIO_AF_SPI2);
        GPIO_PinAFConfig(SPI2_PORT, SPI2_MOSI_PINSOURCE, GPIO_AF_SPI2);
        GPIO_PinAFConfig(SPI2_PORT, SPI2_SCK_PINSOURCE, GPIO_AF_SPI2);
#endif
        input_init.GPIO_Pin = SPI2_MISO_PIN;
        GPIO_Init(SPI2_PORT, &input_init);
        altfunc_init.GPIO_Pin = SPI2_MOSI_PIN;
        GPIO_Init(SPI2_PORT, &altfunc_init);
        altfunc_init.GPIO_Pin = SPI2_SCK_PIN;
        GPIO_Init(SPI2_PORT, &altfunc_init);
#ifdef SPI2_NSS_PIN
		output_init.GPIO_Pin = SPI2_NSS_PIN;
	    GPIO_Init(SPI2_NSS_PORT, &output_init);
#else
#pragma message "SPI2 NSS pin not configured"
#endif
	}

#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32F4XX)
	else if(spi == SPI3)
	{
#if FAMILY == STM32F1
#ifdef SPI3_REMAP
		GPIO_PinRemapConfig(SPI3_REMAP, ENABLE);
#endif
#elif FAMILY == STM32F4
        GPIO_PinAFConfig(SPI3_PORT, SPI3_MISO_PINSOURCE, GPIO_AF_SPI3);
        GPIO_PinAFConfig(SPI3_PORT, SPI3_MOSI_PINSOURCE, GPIO_AF_SPI3);
        GPIO_PinAFConfig(SPI3_PORT, SPI3_SCK_PINSOURCE, GPIO_AF_SPI3);
#endif
        input_init.GPIO_Pin = SPI3_MISO_PIN;
        GPIO_Init(SPI3_PORT, &input_init);
        altfunc_init.GPIO_Pin = SPI3_MOSI_PIN;
        GPIO_Init(SPI3_PORT, &altfunc_init);
        altfunc_init.GPIO_Pin = SPI3_SCK_PIN;
        GPIO_Init(SPI3_PORT, &altfunc_init);
#ifdef SPI3_NSS_PIN
		output_init.GPIO_Pin = SPI3_NSS_PIN;
	    GPIO_Init(SPI3_NSS_PORT, &output_init);
#else
#pragma message "SPI3 NSS pin not configured"
#endif
	}
#endif
}

void spi_init_interrupt(SPI_TypeDef* spi, uint8_t priority, FunctionalState enable)
{
	NVIC_InitTypeDef nvic_init;

	if(spi == SPI1)
		nvic_init.NVIC_IRQChannel = SPI1_IRQn;
	else if(spi == SPI2)
		nvic_init.NVIC_IRQChannel = SPI2_IRQn;
#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32F4XX)
	else if (spi == SPI3)
		nvic_init.NVIC_IRQChannel = SPI3_IRQn;
#endif

	nvic_init.NVIC_IRQChannelPreemptionPriority = priority;
	nvic_init.NVIC_IRQChannelSubPriority = 0;
	nvic_init.NVIC_IRQChannelCmd = enable;
	NVIC_Init(&nvic_init);
}

#if USE_POSIX_STYLE_IO
static int spi_enable_rx_ioctl(dev_ioctl_t* dev)
{
    SPI_TypeDef* spi = (SPI_TypeDef*)(dev->ctx);
    SPI_I2S_ITConfig(spi, SPI_I2S_IT_RXNE, ENABLE);
    return 0;
}

static int spi_enable_tx_ioctl(dev_ioctl_t* dev)
{
    SPI_TypeDef* spi = (SPI_TypeDef*)(dev->ctx);
    SPI_I2S_ITConfig(spi, SPI_I2S_IT_TXE, ENABLE);
    return 0;
}

static int spi_open_ioctl(dev_ioctl_t* dev)
{
    SPI_TypeDef* spi = (SPI_TypeDef*)(dev->ctx);
    spi_init_gpio(spi);
    spi_init_device(spi, ENABLE);
    return 0;
}

static int spi_close_ioctl(dev_ioctl_t* dev)
{
    SPI_TypeDef* spi = (SPI_TypeDef*)(dev->ctx);
    SPI_I2S_ITConfig(spi, SPI_I2S_IT_RXNE, DISABLE);
    SPI_I2S_ITConfig(spi, SPI_I2S_IT_TXE, DISABLE);
    spi_init_device(spi, DISABLE);
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
