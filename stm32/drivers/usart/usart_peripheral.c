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


#include "usart.h"
#include "base_usart.h"
#include "asserts.h"
#include "logger.h"
#include "device.h"

// for targets that have no USART_CR1_OVER8 bit, fake the value
#ifndef USART_CR1_OVER8
#define USART_CR1_OVER8 ((uint32_t)0x0000)
#endif


static volatile usart_ioctl_t usart_ioctls[8];

USART_HANDLE_t usart_init_device(USART_TypeDef* usart, bool enable, usart_mode_t mode, uint32_t baudrate)
{
	USART_HANDLE_t usarth = get_usart_handle(usart);
    usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);

	assert_true(usarth != USART_INVALID_HANDLE);

    usart_ioctl->baudrate = baudrate;
    usart_ioctl->mode = mode;
    usart_ioctl->usart = usart;

    if(enable)
    {
        if(usarth == USART1_HANDLE) {
            __HAL_RCC_USART1_CLK_ENABLE();
        }
        else if(usarth == USART2_HANDLE) {
            __HAL_RCC_USART2_CLK_ENABLE();
        }
        else if(usarth == USART3_HANDLE) {
            __HAL_RCC_USART3_CLK_ENABLE();
        }
#ifdef UART4
        else if(usarth == UART4_HANDLE) {
            __HAL_RCC_UART4_CLK_ENABLE();
        }
#endif
#ifdef UART5
        else if(usarth == UART5_HANDLE) {
            __HAL_RCC_UART5_CLK_ENABLE();
        }
#endif
#ifdef USART6
        else if(usarth == USART6_HANDLE) {
            __HAL_RCC_USART6_CLK_ENABLE();
        }
#endif
#ifdef UART7
        else if(usarth == UART7_HANDLE) {
            __HAL_RCC_UART7_CLK_ENABLE();
        }
#endif
#ifdef UART8
        else if(usarth == UART8_HANDLE) {
            __HAL_RCC_UART8_CLK_ENABLE();
        }
#endif

		usart_ioctl->usart->CR3 = 0;
        usart_ioctl->usart->CR1 = USART_CR1_UE;
        usart_ioctl->usart->CR1 |= USART_CR1_RE|USART_CR1_TE;
        usart_ioctl->usart->CR1 |= USART_CR1_OVER8;
		usart_ioctl->usart->CR2 = 0;
		usart_set_baudrate(usarth, baudrate);
		usart_ioctl->usart->GTPR = 0;
    }
    else
    {
        if(usarth == USART1_HANDLE) {
            __HAL_RCC_USART1_CLK_DISABLE();
        }
        else if(usarth == USART2_HANDLE) {
            __HAL_RCC_USART2_CLK_DISABLE();
        }
        else if(usarth == USART3_HANDLE) {
            __HAL_RCC_USART3_CLK_DISABLE();
        }
#ifdef UART4
        else if(usarth == UART4_HANDLE) {
            __HAL_RCC_UART4_CLK_DISABLE();
        }
#endif
#ifdef UART5
        else if(usarth == UART5_HANDLE) {
            __HAL_RCC_UART5_CLK_DISABLE();
        }
#endif
#ifdef USART6
        else if(usarth == USART6_HANDLE) {
            __HAL_RCC_USART6_CLK_DISABLE();
        }
#endif
#ifdef UART7
        else if(usarth == UART7_HANDLE) {
        	__HAL_RCC_UART7_CLK_DISABLE();
        }
#endif
#ifdef UART8
        else if(usarth == UART8_HANDLE) {
        	__HAL_RCC_UART8_CLK_DISABLE();
        }
#endif
    }

    return usarth;
}

/**
 * initialize the IO pins for the specified USART.
 *
 * **note:** pin mapping is set in serial_config.h.
 *
 * **note:** this function is called by usart_init.
 */
void usart_init_gpio(USART_HANDLE_t usarth)
{
	usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
	GPIO_InitTypeDef gpio_input_init;
	GPIO_InitTypeDef gpio_output_init;

	switch(usart_ioctl->mode)
	{
		case USART_ONEWIRE:
#if FAMILY==STM32F1
			gpio_input_init.Mode = GPIO_MODE_INPUT;
#elif FAMILY==STM32F4
			gpio_input_init.Mode = GPIO_MODE_AF_OD;
#endif
			gpio_input_init.Pull = GPIO_PULLUP;
			gpio_input_init.Speed = GPIO_SPEED_MEDIUM;
			gpio_output_init.Mode = GPIO_MODE_AF_OD;
			gpio_output_init.Pull = GPIO_PULLUP;
			gpio_output_init.Speed = GPIO_SPEED_MEDIUM;
		break;

		case USART_FULLDUPLEX:
#if FAMILY==STM32F1
			gpio_input_init.Mode = GPIO_MODE_INPUT;
#elif FAMILY==STM32F4
			gpio_input_init.Mode = GPIO_MODE_AF_OD;
#endif
			gpio_input_init.Pull = GPIO_PULLUP;
			gpio_input_init.Speed = GPIO_SPEED_MEDIUM;
			gpio_output_init.Mode = GPIO_MODE_AF_PP;
			gpio_output_init.Pull = GPIO_NOPULL;
			gpio_output_init.Speed = GPIO_SPEED_MEDIUM;
		break;

		default:
			assert_true(0);
		break;
	}

    if(usarth == USART1_HANDLE)
    {
#if FAMILY == STM32F1
#ifdef USART1_REMAP
    	__HAL_AFIO_REMAP_USART1_ENABLE();
#endif
#elif FAMILY == STM32F4
    	gpio_output_init.Alternate = GPIO_AF7_USART1;
    	gpio_input_init.Alternate = GPIO_AF7_USART1;
#endif
    	gpio_output_init.Pin = USART1_TX_PIN;
    	gpio_input_init.Pin = USART1_RX_PIN;
    	HAL_GPIO_Init(USART1_PORT, &gpio_output_init);
    	HAL_GPIO_Init(USART1_PORT, &gpio_input_init);
    }
    else if(usarth == USART2_HANDLE)
    {
#if FAMILY == STM32F1
#ifdef USART2_REMAP
        __HAL_AFIO_REMAP_USART2_ENABLE();
#endif
#elif FAMILY == STM32F4
    	gpio_output_init.Alternate = GPIO_AF7_USART2;
    	gpio_input_init.Alternate = GPIO_AF7_USART2;
#endif
    	gpio_output_init.Pin = USART2_TX_PIN;
    	gpio_input_init.Pin = USART2_RX_PIN;
    	HAL_GPIO_Init(USART2_PORT, &gpio_output_init);
    	HAL_GPIO_Init(USART2_PORT, &gpio_input_init);
    }
    else if(usarth == USART3_HANDLE)
    {
#if FAMILY == STM32F1
#ifdef USART3_REMAP
    	__HAL_AFIO_REMAP_USART3_ENABLE();
#elif defined(USART3_PARTIAL_REMAP)
    	__HAL_AFIO_REMAP_USART3_PARTIAL();
#endif
#elif FAMILY == STM32F4
    	gpio_output_init.Alternate = GPIO_AF7_USART3;
    	gpio_input_init.Alternate = GPIO_AF7_USART3;
#endif
    	gpio_output_init.Pin = USART3_TX_PIN;
    	gpio_input_init.Pin = USART3_RX_PIN;
    	HAL_GPIO_Init(USART3_PORT, &gpio_output_init);
    	HAL_GPIO_Init(USART3_PORT, &gpio_input_init);
    }
    else if(usarth == UART4_HANDLE)
    {
#if FAMILY == STM32F4
    	gpio_output_init.Alternate = GPIO_AF8_UART4;
    	gpio_input_init.Alternate = GPIO_AF8_UART4;
#endif
    	gpio_output_init.Pin = UART4_TX_PIN;
    	gpio_input_init.Pin = UART4_RX_PIN;
    	HAL_GPIO_Init(UART4_PORT, &gpio_output_init);
    	HAL_GPIO_Init(UART4_PORT, &gpio_input_init);
    }
    else if(usarth == UART5_HANDLE)
    {
#if FAMILY == STM32F4
    	gpio_output_init.Alternate = GPIO_AF8_UART5;
    	gpio_input_init.Alternate = GPIO_AF8_UART5;
#endif
    	gpio_output_init.Pin = UART5_TX_PIN;
    	gpio_input_init.Pin = UART5_RX_PIN;
    	HAL_GPIO_Init(UART5_TX_PORT, &gpio_output_init);
    	HAL_GPIO_Init(UART5_RX_PORT, &gpio_input_init);
    }
#ifdef USART6
    else if(usarth == USART6_HANDLE)
    {
#if FAMILY == STM32F4
    	gpio_output_init.Alternate = GPIO_AF8_USART6;
    	gpio_input_init.Alternate = GPIO_AF8_USART6;
#endif
    	gpio_output_init.Pin = USART6_TX_PIN;
    	gpio_input_init.Pin = USART6_RX_PIN;
    	HAL_GPIO_Init(USART6_PORT, &gpio_output_init);
    	HAL_GPIO_Init(USART6_PORT, &gpio_input_init);
    }
#endif
#ifdef UART7
    else if(usarth == UART7_HANDLE)
    {
#if FAMILY == STM32F4
    	gpio_output_init.Alternate = GPIO_AF8_UART7;
    	gpio_input_init.Alternate = GPIO_AF8_UART7;
#endif
    	gpio_output_init.Pin = UART7_TX_PIN;
    	gpio_input_init.Pin = UART7_RX_PIN;
    	HAL_GPIO_Init(UART7_TX_PORT, &gpio_output_init);
    	HAL_GPIO_Init(UART7_RX_PORT, &gpio_input_init);
    }
#endif
#ifdef UART8
    else if(usarth == UART8_HANDLE)
    {
#if FAMILY == STM32F4
    	gpio_output_init.Alternate = GPIO_AF8_UART8;
    	gpio_input_init.Alternate = GPIO_AF8_UART8;
#endif
    	gpio_output_init.Pin = UART8_TX_PIN;
    	gpio_input_init.Pin = UART8_RX_PIN;
    	HAL_GPIO_Init(UART8_TX_PORT, &gpio_output_init);
    	HAL_GPIO_Init(UART8_RX_PORT, &gpio_input_init);
    }
#endif
}

void usart_init_interrupt(USART_HANDLE_t usarth, uint8_t priority, bool enable)
{
	uint8_t irq = 0;
	usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);

	usart_disable_tx_int(usart_ioctl);
	usart_disable_rx_int(usart_ioctl);

	if(usarth == USART1_HANDLE)
		irq = USART1_IRQn;
	else if(usarth == USART2_HANDLE)
		irq = USART2_IRQn;
	else if (usarth == USART3_HANDLE)
		irq = USART3_IRQn;
#ifdef UART4
	else if (usarth == UART4_HANDLE)
		irq = UART4_IRQn;
#endif
#ifdef UART5
	else if (usarth == UART5_HANDLE)
		irq = UART5_IRQn;
#endif
#ifdef USART6
	else if (usarth == USART6_HANDLE)
		irq = USART6_IRQn;
#endif
#ifdef UART7
	else if (usarth == UART7_HANDLE)
		irq = UART7_IRQn;
#endif
#ifdef UART8
	else if (usarth == UART8_HANDLE)
		irq = UART8_IRQn;
#endif
	else
		assert_true(0);

    if(enable) {
    	HAL_NVIC_SetPriority(irq, priority, 0);
    	HAL_NVIC_EnableIRQ(irq);
    }
    else {
    	HAL_NVIC_DisableIRQ(irq);
    }
}

USART_HANDLE_t get_usart_handle(USART_TypeDef* usart)
{
	if(usart == USART1)
		return USART1_HANDLE;
	else if(usart == USART2)
		return USART2_HANDLE;
	else if (usart == USART3)
		return USART3_HANDLE;
#ifdef UART4
	else if (usart == UART4)
		return UART4_HANDLE;
#endif
#ifdef UART5
	else if (usart == UART5)
		return UART5_HANDLE;
#endif
#ifdef USART6
	else if (usart == USART6)
		return USART6_HANDLE;
#endif
#ifdef UART7
	else if (usarth == UART7)
		return UART7_HANDLE;
#endif
#ifdef UART8
	else if (usarth == UART8)
		return UART8_HANDLE;
#endif

	assert_true(0);
	return USART_INVALID_HANDLE;
}

USART_TypeDef* get_usart_peripheral(USART_HANDLE_t usarth)
{
	if(usarth == USART1_HANDLE)
		return USART1;
	else if(usarth == USART2_HANDLE)
		return USART2;
	else if (usarth == USART3_HANDLE)
		return USART3;
#ifdef UART4
	else if (usarth == UART4_HANDLE)
		return UART4;
#endif
#ifdef UART5
	else if (usarth == UART5_HANDLE)
		return UART5;
#endif
#ifdef USART6
	else if (usarth == USART6_HANDLE)
		return USART6;
#endif
#ifdef UART7
	else if (usarth == UART7_HANDLE)
		return UART7;
#endif
#ifdef UART8
	else if (usarth == UART8_HANDLE)
		return UART8;
#endif

	assert_true(0);
	return NULL;
}

usart_ioctl_t* get_usart_ioctl(USART_HANDLE_t usarth)
{
	return (usart_ioctl_t*)&usart_ioctls[usarth];
}

#include <unistd.h>
/**
 * write a character to the console usart.
 */
void usart_tx(USART_HANDLE_t usarth, const uint8_t data)
{
	if(usarth != USART_INVALID_HANDLE){
		usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
		while(!usart_tx_readytosend(usart_ioctl));
		usart_ioctl->usart->DR =  data;
	}
}

uint8_t usart_rx(USART_HANDLE_t usarth)
{
	if(usarth != USART_INVALID_HANDLE){
		usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
		while(!usart_rx_inwaiting(usart_ioctl));
		return usart_ioctl->usart->DR;
	}
	return 0;
}

/**
 * sets the usart baudrate.
 *
 * The baud rate is computed using the following formula:
 * - IntegerDivider = ((PCLKx) / (8 * (OVR8+1) * baudrate))
 * - FractionalDivider = ((IntegerDivider - IntegerDivider) * 8 * (OVR8+1)) + 0.5
 * Where OVR8 is the "oversampling by 8 mode" configuration bit in the CR1 register.
 */
void usart_set_baudrate(USART_HANDLE_t usarth, uint32_t baudrate)
{
    USART_TypeDef* usart = get_usart_peripheral(usarth);

    usart_ioctl_t* usart_ioctl = get_usart_ioctl(usarth);
    usart_ioctl->baudrate = baudrate;

	if((usarth == USART1_HANDLE) || (usarth == USART6_HANDLE))
		usart->BRR = USART_BRR(HAL_RCC_GetPCLK2Freq(), baudrate);
	else
		usart->BRR = USART_BRR(HAL_RCC_GetPCLK1Freq(), baudrate);
}

/**
 * reads the current baudrate.
 */
uint32_t usart_get_baudrate(USART_HANDLE_t usarth)
{
    uint32_t pclock;
    USART_TypeDef* usart = get_usart_peripheral(usarth);

    if((usarth == USART1_HANDLE) || (usarth == USART6_HANDLE))
    	pclock = HAL_RCC_GetPCLK2Freq();
    else
    	pclock = HAL_RCC_GetPCLK1Freq();

    // multiply by 25 as 25 * max of 168MHz just avoids overflow and give better precision
    uint32_t div = (25 * (usart->BRR >> 4)) + ((25 * (usart->BRR & 0x000f))/16);
    return ((pclock * 25) / div) / 16;
}