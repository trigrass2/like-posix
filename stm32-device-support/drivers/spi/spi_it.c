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

#include "spi_it.h"
#include "base_spi.h"

#include "board_config.h"
#include "asserts.h"
#if USE_LIKEPOSIX
#include "syscalls.h"
#endif

/**
 * lives in spi.c
 */
extern void* spi_dev_ioctls[NUM_ONCHIP_SPIS];

/**
  * @brief	function called by the SPI receive register not empty interrupt.
  * 		the SPI RX register contents are inserted into the RX FIFO.
  */
inline void spi_rx_isr(SPI_TypeDef* spi, void* spi_dev)
{
	if(SPI_I2S_GetITStatus(spi, SPI_I2S_IT_RXNE) == SET)
	{
#if USE_LIKEPOSIX
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR(((dev_ioctl_t*)spi_dev)->pipe.read, (char*)&(spi->DR), &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else
        (void)spi_dev;
		// todo - fifo put
#endif
		SPI_I2S_ClearITPendingBit(spi, SPI_I2S_IT_RXNE);
	}
}

/**
  * @brief	function called by the SPI transmit register empty interrupt.
  * 		data is sent from SPI till no data is left in the tx fifo.
  */
inline void spi_tx_isr(SPI_TypeDef* spi, void* spi_dev)
{
	if(SPI_I2S_GetITStatus(spi, SPI_I2S_IT_TXE) == SET)
	{
#if USE_LIKEPOSIX
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if(xQueueReceiveFromISR(((dev_ioctl_t*)spi_dev)->pipe.write, (char*)&(spi->DR), &xHigherPriorityTaskWoken) == pdFALSE)
			SPI_I2S_ITConfig(spi, SPI_I2S_IT_TXE, DISABLE);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else
		(void)spi_dev;
        // todo - fifo get
#endif
	}
}


/**
  * @brief  This function handles SPI1 interrupt.
  */
void SPI1_IRQHandler(void)
{
	assert_true(spi_dev_ioctls[0]);
	spi_rx_isr(SPI1, spi_dev_ioctls[0]);
	spi_tx_isr(SPI1, spi_dev_ioctls[0]);
}

/**
  * @brief  This function handles SPI2 interrupt.
  */
void SPI2_IRQHandler(void)
{
	assert_true(spi_dev_ioctls[1]);
	spi_rx_isr(SPI2, spi_dev_ioctls[1]);
	spi_tx_isr(SPI2, spi_dev_ioctls[1]);
}

#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32F4XX)
/**
  * @brief  This function handles SPI3 interrupt.
  */
void SPI3_IRQHandler(void)
{
	assert_true(spi_dev_ioctls[2]);
	spi_rx_isr(SPI3, spi_dev_ioctls[2]);
	spi_tx_isr(SPI3, spi_dev_ioctls[2]);
}
#endif



