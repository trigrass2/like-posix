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

#include "spi_it.h"
#include "base_spi.h"

#include "board_config.h"
#include "asserts.h"
#include "spi.h"
#include "syscalls.h"

/**
  * @brief	function called by the SPI receive register not empty interrupt.
  * 		the SPI RX register contents are inserted into the RX FIFO.
  */
inline void spi_rx_isr(SPI_HANDLE_t spih)
{
	spi_ioctl_t* spi_ioctl = get_spi_ioctl(spih);
#if USE_LIKEPOSIX
	dev_ioctl_t* dev_ioctl = get_spi_device_ioctl(spih);
#endif
	assert_true(spi_ioctl);

    SPI_HandleTypeDef hspi = {.Instance=spi_ioctl->spi};

	if(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_RXNE))
	{
#if USE_LIKEPOSIX
		if(dev_ioctl) {
			static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			xQueueSendFromISR(dev_ioctl->pipe.read, (char*)&(hspi.Instance->DR), &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			return;
		}
#endif
		vfifo_put(spi_ioctl->rxfifo, (void*)&spi_ioctl->spi->DR);
	}
}

/**
  * @brief	function called by the SPI transmit register empty interrupt.
  * 		data is sent from SPI till no data is left in the tx fifo.
  */
inline void spi_tx_isr(SPI_HANDLE_t spih)
{
	spi_ioctl_t* spi_ioctl = get_spi_ioctl(spih);
#if USE_LIKEPOSIX
	dev_ioctl_t* dev_ioctl = get_spi_device_ioctl(spih);
#endif
	assert_true(spi_ioctl);

    SPI_HandleTypeDef hspi = {.Instance=spi_ioctl->spi};

	if(__HAL_SPI_GET_FLAG(&hspi, SPI_FLAG_TXE))
	{
#if USE_LIKEPOSIX
		if(dev_ioctl) {
			static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			if(xQueueReceiveFromISR(dev_ioctl->pipe.write, (char*)&hspi.Instance->DR, &xHigherPriorityTaskWoken) == pdFALSE) {
				__HAL_SPI_DISABLE_IT(&hspi, SPI_IT_TXE);
			}
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			return;
		}
#endif
		if(!vfifo_get(spi_ioctl->txfifo, (void*)&spi_ioctl->spi->DR)) {
			spi_ioctl->sending = false;
			spi_disable_tx_int(spih);
		}
	}
}


/**
  * @brief  This function handles SPI1 interrupt.
  */
void SPI1_IRQHandler(void)
{
	spi_rx_isr(SPI1_HANDLE);
	spi_tx_isr(SPI1_HANDLE);
}

/**
  * @brief  This function handles SPI2 interrupt.
  */
void SPI2_IRQHandler(void)
{
	spi_rx_isr(SPI2_HANDLE);
	spi_tx_isr(SPI2_HANDLE);
}

/**
  * @brief  This function handles SPI3 interrupt.
  */
void SPI3_IRQHandler(void)
{
	spi_rx_isr(SPI3_HANDLE);
	spi_tx_isr(SPI3_HANDLE);
}



