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
 * @addtogroup usart
 *
 * @file usart_it.c
 * @{
 */

#include "usart_it.h"
#include "board_config.h"
#include "asserts.h"
#include "usart.h"
#include "syscalls.h"


/**
  * @brief	function called by the USART receive register not empty interrupt.
  * 		the USART RX register contents are inserted into the RX FIFO.
  */
inline void usart_rx_isr(USART_HANDLE_t usarth)
{
	usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
#if USE_LIKEPOSIX
	dev_ioctl_t* dev_ioctl = get_usart_device_ioctl(usarth);
#endif
	assert_true(usart_ioctl);

    USART_HandleTypeDef husart = {.Instance=usart_ioctl->usart};

	if(__HAL_USART_GET_IT_SOURCE(&husart, USART_IT_RXNE))
	{
#if USE_LIKEPOSIX
		if(dev_ioctl) {
			static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			xQueueSendFromISR(dev_ioctl->pipe.read, (char*)&husart.Instance->DR, &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			return;
		}
#endif
		vfifo_put(usart_ioctl->rxfifo, (void*)&usart_ioctl->usart->DR);
	}
}

/**
  * @brief	function called by the USART transmit register empty interrupt.
  * 		data is sent from USART till no data is left in the tx fifo.
  */
inline void usart_tx_isr(USART_HANDLE_t usarth)
{
	usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
#if USE_LIKEPOSIX
	dev_ioctl_t* dev_ioctl = get_usart_device_ioctl(usarth);
#endif
	assert_true(usart_ioctl);

    USART_HandleTypeDef husart = {.Instance=usart_ioctl->usart};

	if(__HAL_USART_GET_IT_SOURCE(&husart, USART_IT_TXE))
	{
#if USE_LIKEPOSIX
		if(dev_ioctl) {
			static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			if(xQueueReceiveFromISR(dev_ioctl->pipe.write, (char*)&husart.Instance->DR, &xHigherPriorityTaskWoken) == pdFALSE) {
				__HAL_USART_DISABLE_IT(&husart, USART_IT_TXE);
			}
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			return;
		}
#endif
		if(!vfifo_get(usart_ioctl->txfifo, (void*)&usart_ioctl->usart->DR)) {
			usart_ioctl->sending = false;
			usart_disable_tx_int(usarth);
		}
	}
}


/**
  * @brief  This function handles USART1 interrupt.
  */
void USART1_IRQHandler(void)
{
	usart_rx_isr(USART1_HANDLE);
	usart_tx_isr(USART1_HANDLE);
}

/**
  * @brief  This function handles USART2 interrupt.
  */
void USART2_IRQHandler(void)
{
	usart_rx_isr(USART2_HANDLE);
	usart_tx_isr(USART2_HANDLE);
}

/**
  * @brief  This function handles USART3 interrupt.
  */
void USART3_IRQHandler(void)
{
	usart_rx_isr(USART3_HANDLE);
	usart_tx_isr(USART3_HANDLE);
}

/**
  * @brief  This function handles UART4 interrupt.
  */
void UART4_IRQHandler(void)
{
	usart_rx_isr(UART4_HANDLE);
	usart_tx_isr(UART4_HANDLE);
}

/**
  * @brief  This function handles UART5 interrupt.
  */
void UART5_IRQHandler(void)
{
	usart_rx_isr(UART5_HANDLE);
	usart_tx_isr(UART5_HANDLE);
}

#ifdef USART6
/**
  * @brief  This function handles USART6 interrupt.
  */
 void USART6_IRQHandler(void)
 {
	 usart_rx_isr(USART6_HANDLE);
	 usart_tx_isr(USART6_HANDLE);
 }
#endif

#ifdef UART7
 /**
   * @brief  This function handles UART7 interrupt.
   */
 void UART7_IRQHandler(void)
 {
	 usart_rx_isr(UART7_HANDLE);
	 usart_tx_isr(UART7_HANDLE);
 }
#endif

#ifdef UART8
 /**
   * @brief  This function handles UART8 interrupt.
   */
 void UART8_IRQHandler(void)
 {
	 usart_rx_isr(UART8_HANDLE);
	 usart_tx_isr(UART8_HANDLE);
 }
#endif




/**
 * @}
 */
