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
 * STM32 USART Driver
 *
 * provides basic USART IO functions, as well as a high level dev IO driver that enables
 * USART access via system calls.
 *
 * - supports usart's on STM32F1 and STM32F4 devices: USART1,2,3,6 and UART4,5,7,8
 *
 * relies upon a config file usart_config.h.
 * see stm32-device-support/board/board.bsp/usart_config.h.
 *
 * Supports the like-posix device backend API. When compiled together with USE_LIKEPOSIX set to 1,
 * the following posix functions are available for USARTS:
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

#include "board_config.h"
#include "usart.h"
#include "usart_it.h"
#include "base_usart.h"
#include "asserts.h"
#include "cutensils.h"
#include "device.h"

#if USE_LIKEPOSIX

static int usart_ioctl(dev_ioctl_t* dev);
static int usart_close_ioctl(dev_ioctl_t* dev);
static int usart_open_ioctl(dev_ioctl_t* dev);
static int usart_enable_tx_ioctl(dev_ioctl_t* dev);
static int usart_enable_rx_ioctl(dev_ioctl_t* dev);
static volatile dev_ioctl_t* usart_dev_ioctls[8];

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
#define usart_async_wait_rx(usart_ioctl, timeout)				(void)usart_ioctl; (void)timeout

#endif

static volatile USART_HANDLE_t stdio_usarth = USART_INVALID_HANDLE;

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
 * @param   baudrate is the baudrate to set.
 * @param   buffersize is the number of slots to initialize.
 * @retval	returns the USART handle, or USART_INVALID_HANDLE on error.
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

		usart_async_wait_rx_sem_create(usart_ioctl);

		usart_init_interrupt(usarth, USART_INTERRUPT_PRIORITY, enable);

		if(enable) {
			usart_enable_rx_int(usart_ioctl);
		}
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
		usart_disable_tx_int(usart_ioctl);
		sent = vfifo_put_block(usart_ioctl->txfifo, data, length);

		if(sent > 0) {
			usart_enable_tx_int(usart_ioctl);
		}
	}
	return sent;
}

int32_t usart_get_async(USART_HANDLE_t usarth, uint8_t* data, int32_t length, uint32_t timeout)
{
	int32_t recvd = 0;
	int32_t inwaiting = 1; // dummy start value

	if(length) {
		usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
		usart_ioctl->rx_expect = length;

		while(usart_ioctl->rx_expect && inwaiting) {

			usart_disable_rx_int(usart_ioctl);
			recvd += vfifo_get_block(usart_ioctl->rxfifo, (void*)data, usart_ioctl->rx_expect);
			usart_ioctl->rx_expect = length - recvd;
			usart_enable_rx_int(usart_ioctl);

			if(usart_ioctl->rx_expect) {
				usart_async_wait_rx(usart_ioctl, timeout);
			}

			usart_disable_rx_int(usart_ioctl);
			inwaiting = vfifo_used_slots(usart_ioctl->rxfifo);
			usart_enable_rx_int(usart_ioctl);
		}
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
 * @param   mode is the mode to setup, select from usart_mode_t.
 * @param baudrate is the baudrate to set.
 */
USART_HANDLE_t usart_create_dev(const char* filename, USART_TypeDef* usart, usart_mode_t mode, uint32_t baudrate, uint32_t buffersize)
{
    USART_HANDLE_t usarth = USART_INVALID_HANDLE;

	usarth = usart_init_device(usart, true, mode, baudrate);
    usart_init_gpio(usarth);
	usart_init_interrupt(usarth, USART_INTERRUPT_PRIORITY, true);

	usart_dev_ioctls[usarth] = (void*)install_device(filename,
													usarth,
													usart_enable_rx_ioctl,
													usart_enable_tx_ioctl,
													usart_open_ioctl,
													usart_close_ioctl,
													usart_ioctl,
													buffersize);

	log_debug(NULL, "install usart%d: %s", usarth, usart_dev_ioctls[usarth] ? "successful" : "failed");

    assert_true(usart_dev_ioctls[usarth]);

    return usarth;
}

int usart_enable_rx_ioctl(dev_ioctl_t* dev)
{
	usart_ioctl_t* usart_ioctl = get_usart_ioctl(dev->device_handle);
	usart_enable_rx_int(usart_ioctl);
    return 0;
}

int usart_enable_tx_ioctl(dev_ioctl_t* dev)
{
	usart_ioctl_t* usart_ioctl = get_usart_ioctl(dev->device_handle);
	usart_enable_tx_int(usart_ioctl);
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
	usart_disable_rx_int(usart_ioctl);
	usart_disable_tx_int(usart_ioctl);
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
inline void _usart_isr(USART_HANDLE_t usarth)
{
	static BaseType_t receiving_task_has_woken = pdFALSE;
	static BaseType_t sending_task_has_woken = pdFALSE;
	USART_HandleTypeDef husart;
	usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
	husart.Instance = usart_ioctl->usart;

	uint32_t tmp1 = 0, tmp2 = 0;

	  tmp1 = __HAL_USART_GET_FLAG(&husart, USART_FLAG_PE);
	  tmp2 = __HAL_USART_GET_IT_SOURCE(&husart, USART_IT_PE);
	  /* USART parity error interrupt occurred -----------------------------------*/
	  if((tmp1 != RESET) && (tmp2 != RESET))
	  {
	    __HAL_USART_CLEAR_PEFLAG(&husart);
	    husart.ErrorCode |= HAL_USART_ERROR_PE;
	  }

	  tmp1 = __HAL_USART_GET_FLAG(&husart, USART_FLAG_FE);
	  tmp2 = __HAL_USART_GET_IT_SOURCE(&husart, USART_IT_ERR);
	  /* USART frame error interrupt occurred ------------------------------------*/
	  if((tmp1 != RESET) && (tmp2 != RESET))
	  {
	    __HAL_USART_CLEAR_FEFLAG(&husart);
	    husart.ErrorCode |= HAL_USART_ERROR_FE;
	  }

	  tmp1 = __HAL_USART_GET_FLAG(&husart, USART_FLAG_NE);
	  tmp2 = __HAL_USART_GET_IT_SOURCE(&husart, USART_IT_ERR);
	  /* USART noise error interrupt occurred ------------------------------------*/
	  if((tmp1 != RESET) && (tmp2 != RESET))
	  {
	    __HAL_USART_CLEAR_NEFLAG(&husart);
	    husart.ErrorCode |= HAL_USART_ERROR_NE;
	  }

	  tmp1 = __HAL_USART_GET_FLAG(&husart, USART_FLAG_ORE);
	  tmp2 = __HAL_USART_GET_IT_SOURCE(&husart, USART_IT_ERR);
	  /* USART Over-Run interrupt occurred ---------------------------------------*/
	  if((tmp1 != RESET) && (tmp2 != RESET))
	  {
	    __HAL_USART_CLEAR_OREFLAG(&husart);
	    husart.ErrorCode |= HAL_USART_ERROR_ORE;
	  }

	  if(husart.ErrorCode != HAL_USART_ERROR_NONE)
	  {
	    /* Set the USART state ready to be able to start again the process */
	    husart.State = HAL_USART_STATE_READY;

	    HAL_USART_ErrorCallback(&husart);
	  }

	  tmp1 = __HAL_USART_GET_FLAG(&husart, USART_FLAG_RXNE);
	  tmp2 = __HAL_USART_GET_IT_SOURCE(&husart, USART_IT_RXNE);
	  /* USART in mode Receiver --------------------------------------------------*/
	  if((tmp1 != RESET) && (tmp2 != RESET))
	  {
			if(usart_dev_ioctls[usarth]) {
				xQueueSendFromISR(usart_dev_ioctls[usarth]->pipe.read, (char*)&usart_ioctl->usart->DR, &receiving_task_has_woken);
				portYIELD_FROM_ISR(receiving_task_has_woken);
			}
	  }

	  tmp1 = __HAL_USART_GET_FLAG(&husart, USART_FLAG_TXE);
	  tmp2 = __HAL_USART_GET_IT_SOURCE(&husart, USART_IT_TXE);
	  /* USART in mode Transmitter -----------------------------------------------*/
	  if((tmp1 != RESET) && (tmp2 != RESET))
	  {
			if(xQueueReceiveFromISR(usart_dev_ioctls[usarth]->pipe.write, (char*)&usart_ioctl->usart->DR, &sending_task_has_woken) == pdFALSE) {
			      __HAL_USART_DISABLE_IT(&husart, USART_IT_TXE);
			}
			portYIELD_FROM_ISR(sending_task_has_woken);
	  }

//	  tmp1 = __HAL_USART_GET_FLAG(&husart, USART_FLAG_TC);
//	  tmp2 = __HAL_USART_GET_IT_SOURCE(husart, USART_IT_TC);
//	  /* USART in mode Transmitter (transmission end) ----------------------------*/
//	  if((tmp1 != RESET) && (tmp2 != RESET))
//	  {
//	    USART_EndTransmit_IT(husart);
//	  }


//#if USE_FREERTOS
//	static BaseType_t waiting_receiving_task_has_woken = pdFALSE;
//#endif
//#if USE_LIKEPOSIX
//	static BaseType_t receiving_task_has_woken = pdFALSE;
//	static BaseType_t sending_task_has_woken = pdFALSE;
//#endif
//	usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
//	assert_true(usart_ioctl);
//	uint8_t byte;
//
//	if(usart_rx_inwaiting(usart_ioctl) || usart_rx_overrun(usart_ioctl))
//	{
//		byte = usart_ioctl->usart->DR;
//
//#if USE_LIKEPOSIX
//		if(usart_dev_ioctls[usarth]) {
//			xQueueSendFromISR(usart_dev_ioctls[usarth]->pipe.read, (char*)&byte, &receiving_task_has_woken);
//			portYIELD_FROM_ISR(receiving_task_has_woken);
//		}
//		else
//#endif
//		{
//			vfifo_put(usart_ioctl->rxfifo, (void*)&byte);
//
//#if USE_FREERTOS
//			if(usart_ioctl->rx_expect && ((vfifo_used_slots(usart_ioctl->rxfifo) >= usart_ioctl->rx_expect) || vfifo_full(usart_ioctl->rxfifo))) {
//				xSemaphoreGiveFromISR(usart_ioctl->rx_sem, &waiting_receiving_task_has_woken);
//				portYIELD_FROM_ISR(waiting_receiving_task_has_woken);
//			}
//#endif
//		}
//	}
//
//	if(usart_tx_readytosend(usart_ioctl))
//	{
//#if USE_LIKEPOSIX
//		if(usart_dev_ioctls[usarth]) {
//			if(xQueueReceiveFromISR(usart_dev_ioctls[usarth]->pipe.write, (char*)&usart_ioctl->usart->DR, &sending_task_has_woken) == pdFALSE) {
//				usart_disable_tx_int(usart_ioctl);
//			}
//			portYIELD_FROM_ISR(sending_task_has_woken);
//		}
//		else
//#endif
//		{
//			if(!vfifo_get(usart_ioctl->txfifo, (void*)&usart_ioctl->usart->DR)) {
//				usart_disable_tx_int(usart_ioctl);
//			}
//		}
//	}
}
