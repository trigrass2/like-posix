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


#if USE_LIKEPOSIX
#include <termios.h>
#include "syscalls.h"
#endif

#include <stddef.h>
#include <string.h>
#include "board_config.h"
#include "spi.h"
#include "spi_it.h"
#include "base_spi.h"
#include "cutensils.h"


#if USE_LIKEPOSIX
int spi_enable_rx_ioctl(dev_ioctl_t* dev);
int spi_enable_tx_ioctl(dev_ioctl_t* dev);
int spi_open_ioctl(dev_ioctl_t* dev);
int spi_close_ioctl(dev_ioctl_t* dev);
int spi_ioctl(dev_ioctl_t* dev);

static dev_ioctl_t* spi_dev_ioctls[NUM_ONCHIP_SPIS];
#endif


/**
 * call this function to initialize an SPI port in polled mode.
 *
 * @param spi is the SPI peripheral to initialize.
 * @param enable - set to true when using in polled mode. when the device file is specified,
 *        set to false - the device is enabled automatically when the file is opened.
 * @param baudrate is the baudrate to set.
 * @param bit_order Eg SPI_FIRSTBIT_MSB
 * @param clock_phase Eg SPI_PHASE_1EDGE
 * @param clock_polarity Eg SPI_POLARITY_LOW
 * @param data_width Eg SPI_DATASIZE_8BIT
 */
SPI_HANDLE_t spi_create_polled(SPI_TypeDef* spi, bool enable, uint32_t baudrate, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width)
{
    SPI_HANDLE_t spih = spi_init_device(spi, enable, baudrate, bit_order, clock_phase, clock_polarity, data_width);
    spi_init_gpio(spih);
    spi_init_ss_gpio(spih);
	spi_init_interrupt(spih, SPI_INTERRUPT_PRIORITY, false);
	return spih;
}
/**
 * call this function to initialize an SPI port in interrupt driven mode.

 * @param spi is the SPI peripheral to initialize.
 * @param enable - set to true when using in polled mode. when the device file is specified,
 *        set to false - the device is enabled automatically when the file is opened.
 * @param baudrate is the baudrate to set.
 * @param bit_order Eg SPI_FIRSTBIT_MSB
 * @param clock_phase Eg SPI_PHASE_1EDGE
 * @param clock_polarity Eg SPI_POLARITY_LOW
 * @param data_width Eg SPI_DATASIZE_8BIT
 * @param buffersize is the number of fifo slots to initialize.
 */
SPI_HANDLE_t spi_create_async(SPI_TypeDef* spi, bool enable, uint32_t baudrate, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width, uint32_t buffersize)
{
    SPI_HANDLE_t spih = SPI_INVALID_HANDLE;

    uint8_t* fifomem = malloc((2 * sizeof(vfifo_t)) + (2 * buffersize * sizeof(vfifo_primitive_t)));
    if(fifomem) {
    	spih = spi_init_device(spi, enable, baudrate, bit_order, clock_phase, clock_polarity, data_width);
        spi_init_gpio(spih);
        spi_init_ss_gpio(spih);

    	spi_ioctl_t* spi_ioctl = get_spi_ioctl(spih);
		spi_ioctl->rxfifo = (vfifo_t*)fifomem;
		spi_ioctl->txfifo = (vfifo_t*)(fifomem + sizeof(vfifo_t) + (buffersize * sizeof(vfifo_primitive_t)));
		vfifo_init(spi_ioctl->rxfifo, spi_ioctl->rxfifo + sizeof(vfifo_t), buffersize);
		vfifo_init(spi_ioctl->txfifo, spi_ioctl->txfifo + sizeof(vfifo_t), buffersize);

		spi_init_interrupt(spih, SPI_INTERRUPT_PRIORITY, enable);
    }

	return spih;
}

/**
 * sets the NSS pin to a logic 0 (NSS pin is configured in spi_config.h for the board)
 */
void spi_clear_ss(SPI_HANDLE_t spih)
{
    switch(spih)
    {
#ifdef SPI1_NSS_PIN
		case SPI1_HANDLE:
			HAL_GPIO_WritePin(SPI1_NSS_PORT, SPI1_NSS_PIN, GPIO_PIN_RESET);
		break;
#else
#pragma message "SPI1 NSS pin not configured"
#endif

#ifdef SPI2_NSS_PIN
		case SPI2_HANDLE:
			HAL_GPIO_WritePin(SPI2_NSS_PORT, SPI2_NSS_PIN, GPIO_PIN_RESET);
		break;
#else
#pragma message "SPI2 NSS pin not configured"
#endif

#ifdef SPI3_NSS_PIN
		case SPI3_HANDLE:
			HAL_GPIO_WritePin(SPI3_NSS_PORT, SPI3_NSS_PIN, GPIO_PIN_RESET);
		break;
#else
#pragma message "SPI3 NSS pin not configured"
#endif
		default:
			assert_true(0);
		break;
    }
}

/**
 * sets the NSS pin to a logic 1 (NSS pin is configured in spi_config.h for the board)
 */
void spi_set_ss(SPI_HANDLE_t spih)
{
    switch(spih)
    {
#ifdef SPI1_NSS_PIN
		case SPI1_HANDLE:
			HAL_GPIO_WritePin(SPI1_NSS_PORT, SPI1_NSS_PIN, GPIO_PIN_SET);
		break;
#else
#pragma message "SPI1 NSS pin not configured"
#endif

#ifdef SPI2_NSS_PIN
		case SPI2_HANDLE:
			HAL_GPIO_WritePin(SPI2_NSS_PORT, SPI2_NSS_PIN, GPIO_PIN_SET);
		break;
#else
#pragma message "SPI2 NSS pin not configured"
#endif

#ifdef SPI3_NSS_PIN
		case SPI3_HANDLE:
			HAL_GPIO_WritePin(SPI3_NSS_PORT, SPI3_NSS_PIN, GPIO_PIN_SET);
		break;
#else
#pragma message "SPI3 NSS pin not configured"
#endif
		default:
			assert_true(0);
		break;
    }
}

void spi_init_ss_gpio(SPI_HANDLE_t spih)
{
    GPIO_InitTypeDef gpio_init;
    GPIO_TypeDef* gpio = NULL;
    memset(&gpio_init, 0, sizeof(GPIO_InitTypeDef));

    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_HIGH;

    switch(spih)
    {
#ifdef SPI1_NSS_PIN
		case SPI1_HANDLE:
			gpio_init.Pin = SPI1_NSS_PIN;
			gpio = SPI1_NSS_PORT;
		break;
#else
#pragma message "SPI1 NSS pin not configured"
#endif

#ifdef SPI2_NSS_PIN
		case SPI2_HANDLE:
			gpio_init.Pin = SPI2_NSS_PIN;
			gpio = SPI2_NSS_PORT;
		break;
#else
#pragma message "SPI2 NSS pin not configured"
#endif

#ifdef SPI3_NSS_PIN
		case SPI3_HANDLE:
			gpio_init.Pin = SPI3_NSS_PIN;
			gpio = SPI3_NSS_PORT;
		break;
#else
#pragma message "SPI3 NSS pin not configured"
#endif
		default:
			assert_true(0);
		break;
    }

	HAL_GPIO_Init(gpio, &gpio_init);
}

uint8_t spi_transfer_polled(SPI_HANDLE_t spih, uint8_t data)
{
	spi_tx(spih, data);
	return spi_rx(spih);
}

int32_t spi_put_async(SPI_HANDLE_t spih, const uint8_t* data, int32_t length)
{
	int32_t sent = 0;
	if(length) {
		spi_ioctl_t* spi_ioctl = get_spi_ioctl(spih);

		if(spi_ioctl->sending) {
			sent = vfifo_put_block(spi_ioctl->txfifo, data, length);
		}
		else {
			spi_ioctl->sending = true;
			sent = vfifo_put_block(spi_ioctl->txfifo, data, length);
			spi_enable_tx_int(spih);
		}
	}
	return sent;
}

int32_t spi_get_async(SPI_HANDLE_t spih, uint8_t* data, int32_t length)
{
	int32_t recvd = 0;
	if(length) {
		spi_ioctl_t* spi_ioctl = get_spi_ioctl(spih);
		recvd = vfifo_get_block(spi_ioctl->rxfifo, (void*)data, length);
	}
	return recvd;
}

#if USE_LIKEPOSIX

dev_ioctl_t* get_spi_device_ioctl(SPI_HANDLE_t spih)
{
	return spi_dev_ioctls[spih];
}

/**
 * call this function to install an SPI port as a device file (requires USE_LIKEPOSIX=1 in the Makefile).
 *
 * when installed as device file, posix system calls may be made including open, close, read, write, etc.
 * termios functions are also available (tcgetattr, tcsetattr, etc)
 * file stream functions are also available (fputs, fgets, etc)
 *
 * @param spi is the SPI peripheral to initialize.
 * @param filename is the device file name to install as Eg "/dev/ttySPI0".
 *        if set to NULL, the device may be used in polled mode only.
 * @param enable - set to true when using in polled mode. when the device file is specified,
 *        set to false - the device is enabled automatically when the file is opened.
 * @param baudrate is the baudrate to set,
 * @param bit_order Eg SPI_FIRSTBIT_MSB
 * @param clock_phase Eg SPI_PHASE_1EDGE
 * @param clock_polarity Eg SPI_POLARITY_LOW
 * @param data_width Eg SPI_DATASIZE_8BIT
 */
SPI_HANDLE_t spi_create_dev(char* filename, SPI_TypeDef* spi, bool enable, uint32_t baudrate, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width)
{
    SPI_HANDLE_t spih = SPI_INVALID_HANDLE;

	spih = spi_init_device(spi, enable, baudrate, bit_order, clock_phase, clock_polarity, data_width);
    spi_init_gpio(spih);
    spi_init_ss_gpio(spih);

	spi_dev_ioctls[spih] = (void*)install_device(filename,
													spih,
													spi_enable_rx_ioctl,
													spi_enable_tx_ioctl,
													spi_open_ioctl,
													spi_close_ioctl,
													spi_ioctl);

	log_syslog(NULL, "install spi%d: %s", spih, spi_dev_ioctls[spih] ? "successful" : "failed");

	if(spi_dev_ioctls[spih]) {
		spi_init_interrupt(spih, SPI_INTERRUPT_PRIORITY, enable);
	}
    assert_true(spi_dev_ioctls[spih]);

    return spih;
}

int spi_enable_rx_ioctl(dev_ioctl_t* dev)
{
	spi_enable_rx_int(dev->device_handle);
    return 0;
}

int spi_enable_tx_ioctl(dev_ioctl_t* dev)
{
	spi_enable_tx_int(dev->device_handle);
    return 0;
}

int spi_open_ioctl(dev_ioctl_t* dev)
{
    spi_ioctl_t* spi_ioctl = get_spi_ioctl(dev->device_handle);
    spi_init_gpio(dev->device_handle);
    spi_init_device(spi_ioctl->spi, true, spi_ioctl->baudrate, spi_ioctl->bit_order, spi_ioctl->clock_phase, spi_ioctl->clock_polarity, spi_ioctl->data_width);
    return 0;
}

int spi_close_ioctl(dev_ioctl_t* dev)
{
    spi_ioctl_t* spi_ioctl = get_spi_ioctl(dev->device_handle);
	spi_disable_rx_int(dev->device_handle);
	spi_disable_tx_int(dev->device_handle);
    spi_init_device(spi_ioctl->spi, false, spi_ioctl->baudrate, spi_ioctl->bit_order, spi_ioctl->clock_phase, spi_ioctl->clock_polarity, spi_ioctl->data_width);
    return 0;
}

int spi_ioctl(dev_ioctl_t* dev)
{
    uint32_t baudrate = dev->termios->c_ispeed ?
            dev->termios->c_ispeed : dev->termios->c_ospeed;
    if(baudrate)
    {
        // set baudrate
        spi_set_baudrate(dev->device_handle, baudrate);
    }
    else
    {
        // read baudrate
    	dev->termios->c_ospeed = dev->termios->c_ispeed = spi_get_baudrate(dev->device_handle);
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

