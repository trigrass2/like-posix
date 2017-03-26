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

/**
 * STM32 SPI Driver
 *
 * provides basic SPI IO functions, as well as a high level dev IO driver that enables
 * SPI access via system calls.
 *
 * - supports spi's on STM32F1 and STM32F4 devices: spi1,2,3
 *
 * relies upon a config file spi_config.h.
 * see stm32-device-support/board/board.bsp/spi_config.h.
 *
 * Supports the like-posix device backend API. When compiled together with USE_LIKEPOSIX set to 1,
 * the following posix functions are available for spiS:
 *
 * open, close, read, write, fstat, stat, isatty, tcgetattr, tcsetattr, cfgetispeed,
 * cfgetospeed, cfsetispeed, cfsetospeed, tcdrain, tcflow, tcflush
 *
 * baudrate and timeout settings are supported by tcgetattr/tcsetattr.
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
#include "logger.h"


#if USE_LIKEPOSIX

static int spi_enable_rx_ioctl(dev_ioctl_t* dev);
static int spi_enable_tx_ioctl(dev_ioctl_t* dev);
static int spi_open_ioctl(dev_ioctl_t* dev);
static int spi_close_ioctl(dev_ioctl_t* dev);
static int spi_ioctl(dev_ioctl_t* dev);
static volatile dev_ioctl_t* spi_dev_ioctls[NUM_ONCHIP_SPIS];

#endif

#if USE_FREERTOS

#define spi_async_wait_rx_sem_create(spi_ioctl)				do {\
																spi_ioctl->rx_expect = 0;\
																spi_ioctl->rx_sem = xSemaphoreCreateBinary(); \
																assert_true(spi_ioctl->rx_sem);\
															} while(0)
#define spi_async_wait_rx(spi_ioctl, timeout)				xSemaphoreTake(spi_ioctl->rx_sem, timeout)

#else

#define spi_async_wait_rx_sem_create(spi_ioctl)				(void)spi_ioctl
#define spi_async_wait_rx(spi_ioctl, timeout)				(void)spi_ioctl; (void)timeout

#endif


static void spi_init_ss_gpio(SPI_HANDLE_t spi);

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
SPI_HANDLE_t spi_create_polled(SPI_TypeDef* spi, bool enable, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width, uint32_t baudrate)
{
    SPI_HANDLE_t spih = spi_init_device(spi, enable, baudrate, bit_order, clock_phase, clock_polarity, data_width);
    spi_init_interrupt(spih, SPI_INTERRUPT_PRIORITY, false);
    spi_init_gpio(spih);
    spi_init_ss_gpio(spih);
	return spih;
}
/**
 * call this function to initialize an SPI port in interrupt driven mode.

 * @param spi is the SPI peripheral to initialize.
 * @param enable - set to true when using in polled mode. when the device file is specified,
 *        set to false - the device is enabled automatically when the file is opened.
 * @param bit_order Eg SPI_FIRSTBIT_MSB
 * @param clock_phase Eg SPI_PHASE_1EDGE
 * @param clock_polarity Eg SPI_POLARITY_LOW
 * @param data_width Eg SPI_DATASIZE_8BIT
 * @param   baudrate is the baudrate to set.
 * @param   buffersize is the number of slots to initialize.
 * @retval	returns the SPI handle, or SPI_INVALID_HANDLE on error.
 */
SPI_HANDLE_t spi_create_async(SPI_TypeDef* spi, bool enable, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width, uint32_t baudrate, uint32_t buffersize)
{
    SPI_HANDLE_t spih = spi_init_device(spi, enable, baudrate, bit_order, clock_phase, clock_polarity, data_width);

	spi_init_gpio(spih);
	spi_init_ss_gpio(spih);

	spi_ioctl_t* spi_ioctl = get_spi_ioctl(spih);

	spi_ioctl->rxfifo = vfifo_create(buffersize);
	spi_ioctl->txfifo = vfifo_create(buffersize);
	assert_true(spi_ioctl->rxfifo);
	assert_true(spi_ioctl->txfifo);

	spi_async_wait_rx_sem_create(spi_ioctl);

	spi_init_interrupt(spih, SPI_INTERRUPT_PRIORITY, enable);

	if(enable) {
		spi_enable_rx_int(spi_ioctl);
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
		spi_disable_tx_int(spi_ioctl);
		sent = vfifo_put_block(spi_ioctl->txfifo, data, length);

		if(sent > 0) {
			spi_enable_tx_int(spi_ioctl);
		}
	}
	return sent;
}

int32_t spi_get_async(SPI_HANDLE_t spih, uint8_t* data, int32_t length, uint32_t timeout)
{
	int32_t recvd = 0;
	int32_t inwaiting = 1; // dummy start value

	if(length) {
		spi_ioctl_t* spi_ioctl = get_spi_ioctl(spih);
		spi_ioctl->rx_expect = length;

		while(spi_ioctl->rx_expect && inwaiting) {

			spi_disable_rx_int(spi_ioctl);
			recvd += vfifo_get_block(spi_ioctl->rxfifo, (void*)data, spi_ioctl->rx_expect);
			spi_ioctl->rx_expect = length - recvd;
			spi_enable_rx_int(spi_ioctl);

			if(spi_ioctl->rx_expect) {
				spi_async_wait_rx(spi_ioctl, timeout);
			}

			spi_disable_rx_int(spi_ioctl);
			inwaiting = vfifo_used_slots(spi_ioctl->rxfifo);
			spi_enable_rx_int(spi_ioctl);
		}
	}
	return recvd;
}

#if USE_LIKEPOSIX

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
SPI_HANDLE_t spi_create_dev(const char* filename, SPI_TypeDef* spi, uint32_t bit_order, uint32_t clock_phase, uint32_t clock_polarity, uint32_t data_width, uint32_t baudrate, uint32_t buffersize)
{
    SPI_HANDLE_t spih = SPI_INVALID_HANDLE;

	spih = spi_init_device(spi, true, baudrate, bit_order, clock_phase, clock_polarity, data_width);
    spi_init_gpio(spih);
    spi_init_ss_gpio(spih);
	spi_init_interrupt(spih, SPI_INTERRUPT_PRIORITY, true);

	spi_dev_ioctls[spih] = (void*)install_device(filename,
													spih,
													spi_enable_rx_ioctl,
													spi_enable_tx_ioctl,
													spi_open_ioctl,
													spi_close_ioctl,
													spi_ioctl,
													buffersize);

	log_debug(NULL, "install spi%d: %s", spih, spi_dev_ioctls[spih] ? "successful" : "failed");

    assert_true(spi_dev_ioctls[spih]);

    return spih;
}

int spi_enable_rx_ioctl(dev_ioctl_t* dev)
{
	spi_ioctl_t* spi_ioctl = get_spi_ioctl(dev->device_handle);
	spi_enable_rx_int(spi_ioctl);
    return 0;
}

int spi_enable_tx_ioctl(dev_ioctl_t* dev)
{
	spi_ioctl_t* spi_ioctl = get_spi_ioctl(dev->device_handle);
	spi_enable_tx_int(spi_ioctl);
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
	spi_disable_rx_int(spi_ioctl);
	spi_disable_tx_int(spi_ioctl);
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

/**
  * @brief	function called by the SPI receive register not empty interrupt.
  * 		the SPI RX register contents are inserted into the RX FIFO.
  */
inline void _spi_isr(SPI_HANDLE_t spih)
{
#if USE_FREERTOS
	static BaseType_t waiting_receiving_task_has_woken = pdFALSE;
#endif
#if USE_LIKEPOSIX
	static BaseType_t receiving_task_has_woken = pdFALSE;
	static BaseType_t sending_task_has_woken = pdFALSE;
#endif
	spi_ioctl_t* spi_ioctl = get_spi_ioctl(spih);
	assert_true(spi_ioctl);
	uint16_t word;

	if(spi_rx_inwaiting(spi_ioctl))
	{
		word = spi_ioctl->spi->DR;

#if USE_LIKEPOSIX
		if(spi_dev_ioctls[spih]) {
			xQueueSendFromISR(spi_dev_ioctls[spih]->pipe.read, (char*)&word, &receiving_task_has_woken);
			portYIELD_FROM_ISR(receiving_task_has_woken);
		}
		else
#endif
		{
			vfifo_put(spi_ioctl->rxfifo, (void*)&word);

#if USE_FREERTOS
			if(spi_ioctl->rx_expect && ((vfifo_used_slots(spi_ioctl->rxfifo) >= spi_ioctl->rx_expect) || vfifo_full(spi_ioctl->rxfifo))) {
				xSemaphoreGiveFromISR(spi_ioctl->rx_sem, &waiting_receiving_task_has_woken);
				portYIELD_FROM_ISR(waiting_receiving_task_has_woken);
			}
#endif
		}
	}

	if(spi_tx_readytosend(spi_ioctl))
	{
#if USE_LIKEPOSIX
		if(spi_dev_ioctls[spih]) {
			if(xQueueReceiveFromISR(spi_dev_ioctls[spih]->pipe.write, (char*)&spi_ioctl->spi->DR, &sending_task_has_woken) == pdFALSE) {
				spi_disable_tx_int(spi_ioctl);
			}
			portYIELD_FROM_ISR(sending_task_has_woken);
		}
		else
#endif
		{
			if(!vfifo_get(spi_ioctl->txfifo, (void*)&spi_ioctl->spi->DR)) {
				spi_disable_tx_int(spi_ioctl);
			}
		}
	}
}


