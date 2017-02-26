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
#if USE_LIKEPOSIX
#include "syscalls.h"
#endif

#if USE_LIKEPOSIX
/**
 * lives in usart.c
 */
extern dev_ioctl_t* usart_dev_ioctls[8];

/**
  * @brief	function called by the USART receive register not empty interrupt.
  * 		the USART RX register contents are inserted into the RX FIFO.
  */
inline void usart_rx_isr(dev_ioctl_t* dev)
{
	USART_HandleTypeDef husart;
	husart.Instance = ((usart_ioctl_t*)(dev->ctx))->usart;
	if(__HAL_USART_GET_IT_SOURCE(&husart, USART_IT_RXNE) && __HAL_USART_GET_FLAG(&husart, USART_FLAG_RXNE))
	{
		static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR(dev->pipe.read, (char*)&(husart.Instance->DR), &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

/**
  * @brief	function called by the USART transmit register empty interrupt.
  * 		data is sent from USART till no data is left in the tx fifo.
  */
inline void usart_tx_isr(dev_ioctl_t* dev)
{
	USART_HandleTypeDef husart;
	husart.Instance = ((usart_ioctl_t*)(dev->ctx))->usart;
	if(__HAL_USART_GET_IT_SOURCE(&husart, USART_IT_TXE) && __HAL_USART_GET_FLAG(&husart, USART_FLAG_TXE))
	{
		static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if(xQueueReceiveFromISR(dev->pipe.write, (char*)&(husart.Instance->DR), &xHigherPriorityTaskWoken) == pdFALSE)
			__HAL_USART_DISABLE_IT(&husart, USART_IT_TXE);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

#endif

/**
  * @brief  This function handles USART1 interrupt.
  */
void USART1_IRQHandler(void)
{
#if USE_LIKEPOSIX
	assert_true(usart_dev_ioctls[0]);
	usart_rx_isr(usart_dev_ioctls[0]);
	usart_tx_isr(usart_dev_ioctls[0]);
#endif
}

/**
  * @brief  This function handles USART2 interrupt.
  */
void USART2_IRQHandler(void)
{
#if USE_LIKEPOSIX
	assert_true(usart_dev_ioctls[1]);
	usart_rx_isr(usart_dev_ioctls[1]);
	usart_tx_isr(usart_dev_ioctls[1]);
#endif
}

/**
  * @brief  This function handles USART3 interrupt.
  */
void USART3_IRQHandler(void)
{
#if USE_LIKEPOSIX
	assert_true(usart_dev_ioctls[2]);
	usart_rx_isr(usart_dev_ioctls[2]);
	usart_tx_isr(usart_dev_ioctls[2]);
#endif
}

/**
  * @brief  This function handles UART4 interrupt.
  */
void UART4_IRQHandler(void)
{
#if USE_LIKEPOSIX
	assert_true(usart_dev_ioctls[3]);
	usart_rx_isr(usart_dev_ioctls[3]);
	usart_tx_isr(usart_dev_ioctls[3]);
#endif
}

/**
  * @brief  This function handles UART5 interrupt.
  */
void UART5_IRQHandler(void)
{
#if USE_LIKEPOSIX
	assert_true(usart_dev_ioctls[4]);
	usart_rx_isr(usart_dev_ioctls[4]);
	usart_tx_isr(usart_dev_ioctls[4]);
#endif
}

#ifdef USART6
/**
  * @brief  This function handles USART6 interrupt.
  */
 void USART6_IRQHandler(void)
 {
#if USE_LIKEPOSIX
	assert_true(usart_dev_ioctls[5]);
 	usart_rx_isr(usart_dev_ioctls[5]);
 	usart_tx_isr(usart_dev_ioctls[5]);
#endif
 }
#endif

#ifdef UART7
 /**
   * @brief  This function handles UART7 interrupt.
   */
 void UART7_IRQHandler(void)
 {
 #if USE_LIKEPOSIX
 	assert_true(usart_dev_ioctls[6]);
 	usart_rx_isr(usart_dev_ioctls[6]);
 	usart_tx_isr(usart_dev_ioctls[6]);
 #endif
 }
#endif

#ifdef UART8
 /**
   * @brief  This function handles UART8 interrupt.
   */
 void UART8_IRQHandler(void)
 {
 #if USE_LIKEPOSIX
 	assert_true(usart_dev_ioctls[7]);
 	usart_rx_isr(usart_dev_ioctls[7]);
 	usart_tx_isr(usart_dev_ioctls[7]);
 #endif
 }
#endif




/**
 * @}
 */
