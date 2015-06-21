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

/**
 * @addtogroup usart
 *
 * @file usart_it.c
 * @{
 */

#include "usart_it.h"
#include "board_config.h"
#include "asserts.h"
#if USE_POSIX_STYLE_IO
#include "syscalls.h"
#endif

/**
 * lives in usart.c
 */
extern void* usart_dev_ioctls[6];

/**
  * @brief	function called by the USART receive register not empty interrupt.
  * 		the USART RX register contents are inserted into the RX FIFO.
  */
inline void usart_rx_isr(USART_TypeDef* usart, void* usart_dev)
{
	if(USART_GetITStatus(usart, USART_IT_RXNE) == SET)
	{
#if USE_POSIX_STYLE_IO
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR(((dev_ioctl_t*)usart_dev)->pipe.read, (char*)&(usart->DR), &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else
        (void)usart_dev;
		// todo - fifo put
#endif
		USART_ClearITPendingBit(usart, USART_IT_RXNE);
	}
}

/**
  * @brief	function called by the USART transmit register empty interrupt.
  * 		data is sent from USART till no data is left in the tx fifo.
  */
inline void usart_tx_isr(USART_TypeDef* usart, void* usart_dev)
{
	if(USART_GetITStatus(usart, USART_IT_TXE) == SET)
	{
#if USE_POSIX_STYLE_IO
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if(xQueueReceiveFromISR(((dev_ioctl_t*)usart_dev)->pipe.write, (char*)&(usart->DR), &xHigherPriorityTaskWoken) == pdFALSE)
			USART_ITConfig(usart, USART_IT_TXE, DISABLE);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else
		(void)usart_dev;
        // todo - fifo get
#endif
	}
}


/**
  * @brief  This function handles USART1 interrupt.
  */
void USART1_IRQHandler(void)
{
	assert_true(usart_dev_ioctls[0]);
	usart_rx_isr(USART1, usart_dev_ioctls[0]);
	usart_tx_isr(USART1, usart_dev_ioctls[0]);
}

/**
  * @brief  This function handles USART2 interrupt.
  */
void USART2_IRQHandler(void)
{
	assert_true(usart_dev_ioctls[1]);
	usart_rx_isr(USART2, usart_dev_ioctls[1]);
	usart_tx_isr(USART2, usart_dev_ioctls[1]);
}

#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32F4XX)
/**
  * @brief  This function handles USART3 interrupt.
  */
void USART3_IRQHandler(void)
{
	assert_true(usart_dev_ioctls[2]);
	usart_rx_isr(USART3, usart_dev_ioctls[2]);
	usart_tx_isr(USART3, usart_dev_ioctls[2]);
}

/**
  * @brief  This function handles UART4 interrupt.
  */
void UART4_IRQHandler(void)
{
	assert_true(usart_dev_ioctls[3]);
	usart_rx_isr(UART4, usart_dev_ioctls[3]);
	usart_tx_isr(UART4, usart_dev_ioctls[3]);
}

/**
  * @brief  This function handles UART5 interrupt.
  */
void UART5_IRQHandler(void)
{
	assert_true(usart_dev_ioctls[4]);
	usart_rx_isr(UART5, usart_dev_ioctls[4]);
	usart_tx_isr(UART5, usart_dev_ioctls[4]);
}

#if FAMILY == STM32F4
/**
  * @brief  This function handles USART6 interrupt.
  */
 void USART6_IRQHandler(void)
 {
	assert_true(usart_dev_ioctls[5]);
 	usart_rx_isr(USART6, usart_dev_ioctls[5]);
 	usart_tx_isr(USART6, usart_dev_ioctls[5]);
 }
#endif
#endif




/**
 * @}
 */
