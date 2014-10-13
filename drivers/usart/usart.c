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
 * This file is part of the stm32-device-support project, <https://github.com/drmetal/stm32-device-support>
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
 * - supports USARTS on STM32F1 and STM32F4 devices: USART1,2,3,6 and UART4,5
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
 * @file usart.c
 * @{
 */

#include "usart.h"
#include "base_usart.h"
#include "syscalls.h"
#include "asserts.h"
#include "cutensils.h"

#define CR1_OVER8_Set             ((u16)0x8000)  /* USART OVER8 mode Enable Mask, used by set_baudrate */
#define CR1_OVER8_Reset           ((u16)0x7FFF)  /* USART OVER8 mode Disable Mask, used by set_baudrate */

static USART_TypeDef* console_usart;
dev_ioctl_t* usart_dev_ioctls[6];

/**
 * write a character to the console usart.
 */
void phy_putc(char c)
{
    if(console_usart)
    {
        while(USART_GetFlagStatus((USART_TypeDef*)console_usart, USART_FLAG_TXE) == RESET);
        USART_SendData((USART_TypeDef*)console_usart, (uint16_t)c);
    }
}

/**
 * get a character from the console usart.
 */
char phy_getc(void)
{
    if(console_usart)
    {
        while(USART_GetFlagStatus((USART_TypeDef*)console_usart, USART_FLAG_RXNE) == RESET);
        return (char)USART_ReceiveData((USART_TypeDef*)console_usart);
    }
    return -1;
}

/**
 * initialize the specified USART device.
 *
 * **note:** this function is called by init_usart.
 */
void init_usart_device(USART_TypeDef* usart, FunctionalState enable)
{
	USART_InitTypeDef usart_init;

    if(usart == USART1)
        RCC_APB2PeriphClockCmd(USART1_CLOCK, enable);
    else if(usart == USART2)
        RCC_APB1PeriphClockCmd(USART2_CLOCK, enable);
#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32F4XX)
    else if (usart == USART3)
        RCC_APB1PeriphClockCmd(USART3_CLOCK, enable);
    else if (usart == UART4)
        RCC_APB1PeriphClockCmd(UART4_CLOCK, enable);
    else if (usart == UART5)
        RCC_APB1PeriphClockCmd(UART5_CLOCK, enable);
#if FAMILY == STM32F4
    else if (usart == USART6)
        RCC_APB2PeriphClockCmd(USART6_CLOCK, enable);
#endif
#endif

    usart_init.USART_BaudRate = USART_DEFAULT_BAUDRATE;
    usart_init.USART_WordLength = USART_WordLength_8b;
    usart_init.USART_StopBits = USART_StopBits_1;
    usart_init.USART_Parity = USART_Parity_No;
    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init((USART_TypeDef*)usart, &usart_init);
    USART_Cmd((USART_TypeDef*)usart, enable);
}

/**
 * initialize the IO pins for the specified USART.
 *
 * **note:** pin mapping is set in serial_config.h.
 *
 * **note:** this function is called by init_usart.
 */
void init_usart_gpio(USART_TypeDef* usart)
{
#if FAMILY == STM32F1
    GPIO_InitTypeDef rx_init = {
        .GPIO_Mode = GPIO_Mode_IN_FLOATING,
        .GPIO_Speed = GPIO_Speed_50MHz,
    };
    GPIO_InitTypeDef tx_init = {
        .GPIO_Mode = GPIO_Mode_AF_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
    };
#elif FAMILY == STM32F4
    GPIO_InitTypeDef rx_init = {
        .GPIO_Mode = GPIO_Mode_AF,
        .GPIO_Speed = GPIO_Speed_100MHz,
        .GPIO_OType = GPIO_OType_OD,
        .GPIO_PuPd = GPIO_PuPd_NOPULL,
    };
    GPIO_InitTypeDef tx_init = {
        .GPIO_Mode = GPIO_Mode_AF,
        .GPIO_Speed = GPIO_Speed_100MHz,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_PuPd = GPIO_PuPd_NOPULL,
    };
#endif

    if(usart == USART1)
    {
#if FAMILY == STM32F1
#ifdef USART1_REMAP
        GPIO_PinRemapConfig(USART1_REMAP, ENABLE);
#endif
#elif FAMILY == STM32F4
        GPIO_PinAFConfig(USART1_PORT, USART1_TX_PINSOURCE, GPIO_AF_USART1);
        GPIO_PinAFConfig(USART1_PORT, USART1_RX_PINSOURCE, GPIO_AF_USART1);
#endif
        tx_init.GPIO_Pin = USART1_TX_PIN;
        GPIO_Init(USART1_PORT, &tx_init);
        rx_init.GPIO_Pin = USART1_RX_PIN;
        GPIO_Init(USART1_PORT, &rx_init);
    }

    else if(usart == USART2)
    {
#if FAMILY == STM32F1
#ifdef USART2_REMAP
        GPIO_PinRemapConfig(USART2_REMAP, ENABLE);
#endif
#elif FAMILY == STM32F4
        GPIO_PinAFConfig(USART2_PORT, USART2_TX_PINSOURCE, GPIO_AF_USART2);
        GPIO_PinAFConfig(USART2_PORT, USART2_RX_PINSOURCE, GPIO_AF_USART2);
#endif
        tx_init.GPIO_Pin = USART2_TX_PIN;
        GPIO_Init(USART2_PORT, &tx_init);
        rx_init.GPIO_Pin = USART2_RX_PIN;
        GPIO_Init(USART2_PORT, &rx_init);
    }
#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32F4XX)
    else if (usart == USART3)
    {
#if FAMILY == STM32F1
#ifdef USART3_REMAP
        GPIO_PinRemapConfig(USART3_REMAP, ENABLE);
#endif
#elif FAMILY == STM32F4
        GPIO_PinAFConfig(USART3_PORT, USART3_TX_PINSOURCE, GPIO_AF_USART3);
        GPIO_PinAFConfig(USART3_PORT, USART3_RX_PINSOURCE, GPIO_AF_USART3);
#endif
        tx_init.GPIO_Pin = USART3_TX_PIN;
        GPIO_Init(USART3_PORT, &tx_init);
        rx_init.GPIO_Pin = USART3_RX_PIN;
        GPIO_Init(USART3_PORT, &rx_init);
    }

    else if (usart == UART4)
    {
#if FAMILY == STM32F4
        GPIO_PinAFConfig(UART4_PORT, UART4_TX_PINSOURCE, GPIO_AF_UART4);
        GPIO_PinAFConfig(UART4_PORT, UART4_RX_PINSOURCE, GPIO_AF_UART4);
#endif
        tx_init.GPIO_Pin = UART4_TX_PIN;
        GPIO_Init(UART4_PORT, &tx_init);
        rx_init.GPIO_Pin = UART4_RX_PIN;
        GPIO_Init(UART4_PORT, &rx_init);
    }

    else if (usart == UART5)
    {
#if FAMILY == STM32F4
        GPIO_PinAFConfig(UART5_TX_PORT, UART5_TX_PINSOURCE, GPIO_AF_UART5);
        GPIO_PinAFConfig(UART5_RX_PORT, UART5_RX_PINSOURCE, GPIO_AF_UART5);
#endif
        tx_init.GPIO_Pin = UART5_TX_PIN;
        GPIO_Init(UART5_TX_PORT, &tx_init);
        rx_init.GPIO_Pin = UART5_RX_PIN;
        GPIO_Init(UART5_RX_PORT, &rx_init);
    }

#if FAMILY == STM32F4
    else if (usart == USART6)
    {
        GPIO_PinAFConfig(USART6_PORT, USART6_TX_PINSOURCE, GPIO_AF_USART6);
        GPIO_PinAFConfig(USART6_PORT, USART6_RX_PINSOURCE, GPIO_AF_USART6);
        tx_init.GPIO_Pin = USART6_TX_PIN;
        GPIO_Init(USART6_PORT, &tx_init);
        rx_init.GPIO_Pin = USART6_RX_PIN;
        GPIO_Init(USART6_PORT, &rx_init);
    }
#endif
#endif
}


int usart_enable_rx_ioctl(void* ctx)
{
	USART_TypeDef* usart = (USART_TypeDef*)ctx;
	USART_ITConfig(usart, USART_IT_RXNE, ENABLE);
	return 0;
}

int usart_enable_tx_ioctl(void* ctx)
{
	USART_TypeDef* usart = (USART_TypeDef*)ctx;
	USART_ITConfig(usart, USART_IT_TXE, ENABLE);
	return 0;
}

int usart_open_ioctl(void* ctx)
{
	USART_TypeDef* usart = (USART_TypeDef*)ctx;
	init_usart_gpio(usart);
	init_usart_device(usart, ENABLE);
	return 0;
}

int usart_close_ioctl(void* ctx)
{
	USART_TypeDef* usart = (USART_TypeDef*)ctx;
	USART_ITConfig(usart, USART_IT_RXNE, DISABLE);
	USART_ITConfig(usart, USART_IT_TXE, DISABLE);
	init_usart_device(usart, DISABLE);
	return 0;
}


int8_t get_usart_devno(USART_TypeDef* usart)
{
	if(usart == USART1)
		return 0;
	else if(usart == USART2)
		return 1;
#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32F4XX)
	else if (usart == USART3)
		return 2;
	else if (usart == UART4)
		return 3;
	else if (usart == UART5)
		return 4;
#if FAMILY == STM32F4
	else if (usart == USART6)
		return 5;
#endif
#endif
	return -1;
}

/**
 * initialize the the specified USART, and its GOIO pins.
 * if async is set to rue, enables interrupts.
 */
bool init_usart(USART_TypeDef* usart, char* install_as, bool open)
{
	bool ret = true;
	int8_t usart_devno = get_usart_devno(usart);

	log_syslog(NULL, "init usart%d", usart_devno);

	assert_true(usart_devno != -1);

    if(install_as)
    {
    	// installed USART can only work with interrupt enabled
    	init_usart_interrupt(usart, USART_INTERRUPT_PRIORITY, true);

    	usart_dev_ioctls[usart_devno] = install_device(install_as,
														usart,
														usart_enable_rx_ioctl,
														usart_enable_tx_ioctl,
														usart_open_ioctl,
														usart_close_ioctl);
    	ret = usart_dev_ioctls[usart_devno] != NULL;
    	log_syslog(NULL, "install usart%d: %d", usart_devno, ret);
    }

    if(open)
    {
    	// installed usarts are opened automatically... this is optional
    	usart_open_ioctl(usart);
    }

    return ret;
}


/**
 * initialize interrupts for the specified USART.
 *
 * **note:** this functuion is called by init_usart.
 */
void init_usart_interrupt(USART_TypeDef* usart, uint8_t priority, FunctionalState enable)
{
	NVIC_InitTypeDef nvic_init;

	if(usart == USART1)
		nvic_init.NVIC_IRQChannel = USART1_IRQn;
	else if(usart == USART2)
		nvic_init.NVIC_IRQChannel = USART2_IRQn;
#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32F4XX)
	else if (usart == USART3)
		nvic_init.NVIC_IRQChannel = USART3_IRQn;
	else if (usart == UART4)
		nvic_init.NVIC_IRQChannel = UART4_IRQn;
	else if (usart == UART5)
		nvic_init.NVIC_IRQChannel = UART5_IRQn;
#if FAMILY == STM32F4
	else if (usart == USART6)
		nvic_init.NVIC_IRQChannel = USART6_IRQn;
#endif
#endif

	nvic_init.NVIC_IRQChannelPreemptionPriority = priority;
	nvic_init.NVIC_IRQChannelSubPriority = 0;
	nvic_init.NVIC_IRQChannelCmd = enable;
	NVIC_Init(&nvic_init);
}

/**
 * sets the console USART. this enables printf through the console USART.
 */
void set_console_usart(USART_TypeDef* usart)
{
	console_usart = usart;
}

/**
 * sets the usart baudrate.
 */
void set_baudrate(USART_TypeDef* usart, uint32_t br)
{
	uint32_t apbclock;
	uint32_t tmpreg;
	uint32_t integerdivider;
    uint32_t fractionaldivider;
    RCC_ClocksTypeDef RCC_ClocksStatus;

	RCC_GetClocksFreq(&RCC_ClocksStatus);
	if(usart == USART1 || usart == USART6)
		apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	else
		apbclock = RCC_ClocksStatus.PCLK1_Frequency;

	// Determine the integer part
	if (usart->CR1 & CR1_OVER8_Set)
		integerdivider = ((25 * apbclock) / (2 * br)); // Integer part computing in case Oversampling mode is 8 Samples
	else
		integerdivider = ((25 * apbclock) / (4 * br)); // Integer part computing in case Oversampling mode is 16 Samples

	tmpreg = (integerdivider / 100) << 4;

	// Determine the fractional part
	fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

	// Implement the fractional part in the register
	if (usart->CR1 & CR1_OVER8_Set)
		tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
	else
		tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);

	/* Write to USART BRR */
	usart->BRR = (uint16_t)tmpreg;
}


/**
 * @}
 */

