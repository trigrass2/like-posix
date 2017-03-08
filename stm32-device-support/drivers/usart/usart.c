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
 * @defgroup usart STM32 USART Driver
 *
 * provides basic USART IO functions, as well as a high level dev IO driver that enables
 * USART access via system calls.
 *
 * - supports USARTS on STM32F1 and STM32F4 devices: USART1,2,3,6 and UART4,5,7,8
 *
 * relies upon a config file usart_config.h,
 *
 * Eg:
 *
\code

#ifndef USART_CONFIG_H_
#define USART_CONFIG_H_

#define USART_DEFAULT_BAUDRATE 115200

#define CONSOLE_USART		USART3

#define USART1_FULL_REMAP 1
#define USART2_FULL_REMAP 1
#define USART3_FULL_REMAP 1
#define USART3_PARTIAL_REMAP 0

#endif // USART_CONFIG_H_

\endcode
 *
 *
 * Supports the like-posix device backend API. When compiled together with USE_LIKEPOSIX set to 1,
 * the following posix functions are available for USARTS:
 *
 * open, close, read, write, fstat, stat, isatty, tcgetattr, tcsetattr, cfgetispeed,
 * cfgetospeed, cfsetispeed, cfsetospeed, tcdrain, tcflow, tcflush
 *
 * baudrate and timeout settings are supported by tcgetattr/tcsetattr.
 *
 * @file usart.c
 * @{
 */

#if USE_LIKEPOSIX
#include <termios.h>
#include "syscalls.h"
#endif

#include "usart.h"
#include "usart_it.h"
#include "base_usart.h"
#include "asserts.h"
#include "cutensils.h"
#include "system.h"

#if USE_LIKEPOSIX
static int usart_ioctl(dev_ioctl_t* dev);
static int usart_close_ioctl(dev_ioctl_t* dev);
static int usart_open_ioctl(dev_ioctl_t* dev);
static int usart_enable_tx_ioctl(dev_ioctl_t* dev);
static int usart_enable_rx_ioctl(dev_ioctl_t* dev);
dev_ioctl_t* usart_dev_ioctls[8];
#endif

#if USE_FREERTOS

#define usart_async_wait_rx_sem_create(usart_ioctl)				do {\
																usart_ioctl->rx_expect = 0;\
																usart_ioctl->rx_sem = xSemaphoreCreateBinary(); \
																assert_true(usart_ioctl->rx_sem);\
															} while(0)
#define usart_async_wait_rx(usart_ioctl, timeout)				xSemaphoreTake(usart_ioctl->rx_sem, timeout)

#else

#define usart_async_wait_rx_sem_create(usart_ioctl)				(void)usart_ioctl
#define usart_async_wait_rx(usart_ioctl, timeout)				(void)usart_ioctl; (void)timeout; 1

#endif

USART_HANDLE_t stdio_usarth;

/**
 * initialize the the specified USART in polled mode.
 *
 * @param	usart is the usart to init (USART1,2,3,6 UART4,5)
 * @param	enable, when set to true enables the peripheral. use this option when using the peripehral
 * 			raw, not as a device.
 * 			set to false when using as a device - it will be enabled when the open()
 * 			function is invoked on its device file.
 * @param   mode is the mode to setup, select from usart_mode_t.
 * @param   baudrate is the baudrate to set.
 * @retval	returns true if the operation succeeded, false otherwise.
 */
USART_HANDLE_t usart_create_polled(USART_TypeDef* usart, bool enable, usart_mode_t mode, uint32_t baudrate)
{
	USART_HANDLE_t usarth = usart_init_device(usart, enable, mode, baudrate);
	usart_init_interrupt(usarth, USART_INTERRUPT_PRIORITY, false);
    usart_init_gpio(usarth);
    return usarth;
}

/**
 * initialize the the specified USART in interrupt driven mode.
 *
 * @param	usart is the usart to init (USART1,2,3,6 UART4,5)
 * @param	enable, when set to true enables the peripheral. use this option when using the peripehral
 * 			raw, not as a device.
 * 			set to false when using as a device - it will be enabled when the open()
 * 			function is invoked on its device file.
 * @param   mode is the mode to setup, select from usart_mode_t.
 * @param   buffersize is the number of slots to initialize.
 * @param   baudrate is the baudrate to set.
 * @retval	returns true if the operation succeeded, false otherwise.
 */
USART_HANDLE_t usart_create_async(USART_TypeDef* usart, bool enable, usart_mode_t mode, uint32_t baudrate, uint32_t buffersize)
{
	USART_HANDLE_t usarth = USART_INVALID_HANDLE;
	uint8_t* fifomem = malloc(2 * (sizeof(vfifo_t) + (buffersize * sizeof(vfifo_primitive_t))));
	if(fifomem) {
		usarth = usart_init_device(usart, enable, mode, baudrate);
		usart_init_gpio(usarth);

		usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
		usart_ioctl->rxfifo = (vfifo_t*)fifomem;
		usart_ioctl->txfifo = (vfifo_t*)(fifomem + sizeof(vfifo_t) + (buffersize * sizeof(vfifo_primitive_t)));
		vfifo_init(usart_ioctl->rxfifo, usart_ioctl->rxfifo + sizeof(vfifo_t), buffersize);
		vfifo_init(usart_ioctl->txfifo, usart_ioctl->txfifo + sizeof(vfifo_t), buffersize);

		usart_init_interrupt(usarth, USART_INTERRUPT_PRIORITY, enable);
	}
    return usarth;
}


void usart_set_stdio_usart(int usarth)
{
	stdio_usarth = usarth;
}

void usart_stdio_tx(const char data)
{
	usart_tx(stdio_usarth, (const uint8_t)data);
}

char usart_stdio_rx()
{
	return usart_rx(stdio_usarth);
}

int32_t usart_put_async(USART_HANDLE_t usarth, const uint8_t* data, int32_t length)
{
	int32_t sent = 0;
	if(length) {
		usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
		sent = vfifo_put_block(usart_ioctl->txfifo, data, length);

		if(sent > 0) {
			usart_ioctl->sending = true;
			usart_enable_tx_int(usarth);
		}
	}
	return sent;
}

int32_t usart_get_async(USART_HANDLE_t usarth, uint8_t* data, int32_t length, uint32_t timeout)
{
	int32_t recvd = 0;
	if(length) {
		usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
		usart_ioctl->rx_expect = length;
		usart_async_wait_rx(usart_ioctl, timeout);
		recvd = vfifo_get_block(usart_ioctl->rxfifo, (void*)data, length);
	}
	return recvd;
}




#if USE_LIKEPOSIX

/**
 * call this function to install an USART port as a device file (requires USE_LIKEPOSIX=1 in the Makefile).
 *
 * when installed as device file, posix system calls may be made including open, close, read, write, etc.
 * termios functions are also available (tcgetattr, tcsetattr, etc)
 * file stream functions are also available (fputs, fgets, etc)
 *
 * @param usart is the USART peripheral to initialize.
 * @param filename is the device file name to install as Eg "/dev/ttyUSART0".
 *        if set to NULL, the device may be used in polled mode only.
 * @param enable - set to true when using in polled mode. when the device file is specified,
 *        set to false - the device is enabled automatically when the file is opened.
 * @param   mode is the mode to setup, select from usart_mode_t.
 * @param baudrate is the baudrate to set.
 */
USART_HANDLE_t usart_create_dev(char* filename, USART_TypeDef* usart, bool enable, usart_mode_t mode, uint32_t baudrate)
{
    USART_HANDLE_t usarth = USART_INVALID_HANDLE;

	usarth = usart_init_device(usart, enable, mode, baudrate);
    usart_init_gpio(usarth);

	usart_dev_ioctls[usarth] = (void*)install_device(filename,
													usarth,
													usart_enable_rx_ioctl,
													usart_enable_tx_ioctl,
													usart_open_ioctl,
													usart_close_ioctl,
													usart_ioctl);

	log_syslog(NULL, "install usart%d: %s", usarth, usart_dev_ioctls[usarth] ? "successful" : "failed");

	if(usart_dev_ioctls[usarth]) {
		usart_init_interrupt(usarth, USART_INTERRUPT_PRIORITY, enable);
	}

    assert_true(usart_dev_ioctls[usarth]);

    return usarth;
}

int usart_enable_rx_ioctl(dev_ioctl_t* dev)
{
	usart_enable_rx_int(dev->device_handle);
    return 0;
}

int usart_enable_tx_ioctl(dev_ioctl_t* dev)
{
	usart_enable_tx_int(dev->device_handle);
    return 0;
}

int usart_open_ioctl(dev_ioctl_t* dev)
{
    usart_ioctl_t* usart_ioctl = get_usart_ioctl(dev->device_handle);
    usart_init_gpio(dev->device_handle);
    usart_init_device(usart_ioctl->usart, true, usart_ioctl->mode, usart_ioctl->baudrate);
    return 0;
}

int usart_close_ioctl(dev_ioctl_t* dev)
{
    usart_ioctl_t* usart_ioctl = get_usart_ioctl(dev->device_handle);
	usart_disable_rx_int(dev->device_handle);
	usart_disable_tx_int(dev->device_handle);
    usart_init_device(usart_ioctl->usart, false, usart_ioctl->mode, usart_ioctl->baudrate);
    return 0;
}

static int usart_ioctl(dev_ioctl_t* dev)
{
    uint32_t baudrate = dev->termios->c_ispeed ?
            dev->termios->c_ispeed : dev->termios->c_ospeed;
    if(baudrate)
    {
        // set baudrate
        usart_set_baudrate(dev->device_handle, baudrate);
    }
    else
    {
        // read baudrate
    	dev->termios->c_ospeed = dev->termios->c_ispeed = usart_get_baudrate(dev->device_handle);
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
  * @brief	function called by the USART receive register not empty interrupt.
  * 		the USART RX register contents are inserted into the RX FIFO.
  */
inline bool _usart_rx_isr(USART_HANDLE_t usarth)
{
#if USE_FREERTOS
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
#endif
	usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
	assert_true(usart_ioctl);

	if(usart_rx_inwaiting(usarth))
	{
#if USE_LIKEPOSIX
		if(usart_dev_ioctls[usarth]) {
			xQueueSendFromISR(usart_dev_ioctls[usarth]->pipe.read, (char*)&usart_ioctl->usart->DR, &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			return true;
		}
#endif

		vfifo_put(usart_ioctl->rxfifo, (void*)&usart_ioctl->usart->DR);
#if USE_FREERTOS
		if((usart_ioctl->rx_expect > 0) && (vfifo_used_slots(usart_ioctl->rxfifo) >= usart_ioctl->rx_expect || vfifo_full(usart_ioctl->rxfifo))) {
			usart_ioctl->rx_expect = 0;
			xSemaphoreGiveFromISR(usart_ioctl->rx_sem, &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
#endif
		return true;
	}
	return false;
}

/**
  * @brief	function called by the USART transmit register empty interrupt.
  * 		data is sent from USART till no data is left in the tx fifo.
  */
inline bool _usart_tx_isr(USART_HANDLE_t usarth)
{
#if USE_LIKEPOSIX
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
#endif
	usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
	assert_true(usart_ioctl);

	if(usart_tx_readytosend(usarth))
	{
#if USE_LIKEPOSIX
		if(usart_dev_ioctls[usarth]) {
			if(xQueueReceiveFromISR(usart_dev_ioctls[usarth]->pipe.write, (char*)&usart_ioctl->usart->DR, &xHigherPriorityTaskWoken) == pdFALSE) {
				usart_disable_tx_int(usarth);
			}
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			return true;
		}
#endif
		if(!vfifo_get(usart_ioctl->txfifo, (void*)&usart_ioctl->usart->DR)) {
			usart_ioctl->sending = false;
			usart_disable_tx_int(usarth);
		}
		return true;
	}
	return false;
}

/**
 * @}
 */

