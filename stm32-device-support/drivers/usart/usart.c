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
 *
 * Supports the like-posix device backend API. When compiled together with USE_POSIX_STYLE_IO set to 1,
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

#if USE_POSIX_STYLE_IO
#include <termios.h>
#include "syscalls.h"
#endif

#include "usart.h"
#include "base_usart.h"
#include "asserts.h"
#include "cutensils.h"
#include "system.h"

#define CR1_OVER8_Set             ((u16)0x8000)  /* USART OVER8 mode Enable Mask, used by usart_set_baudrate */
#define CR1_OVER8_Reset           ((u16)0x7FFF)  /* USART OVER8 mode Disable Mask, used by usart_set_baudrate */

static USART_TypeDef* console_usart;

#if USE_POSIX_STYLE_IO
static int usart_ioctl(dev_ioctl_t* dev);
static int usart_close_ioctl(dev_ioctl_t* dev);
static int usart_open_ioctl(dev_ioctl_t* dev);
static int usart_enable_tx_ioctl(dev_ioctl_t* dev);
static int usart_enable_rx_ioctl(dev_ioctl_t* dev);
#endif

void* usart_dev_ioctls[6];

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
 * **note:** this function is called by usart_init.
 */
void usart_init_device(USART_TypeDef* usart, FunctionalState enable)
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

    usart->CR1 &= ~CR1_OVER8_Set;

    USART_Init((USART_TypeDef*)usart, &usart_init);
    USART_Cmd((USART_TypeDef*)usart, enable);
}

/**
 * initialize the IO pins for the specified USART.
 *
 * **note:** pin mapping is set in serial_config.h.
 *
 * **note:** this function is called by usart_init.
 */
void usart_init_gpio(USART_TypeDef* usart)
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
 * initialize the the specified USART, interrupts, device file, and its GPIO pins.
 *
 * @param	usart is the usart to init (USART1,2,3,6 UART4,5)
 * @param	install, is the name of the file to install as. if specified,
 * 			creates a new file in the directory /dev, and installs the USART peripheral as a device.
 * 			if set to NULL, does not install the device, but simply configures the peripheral.
 * @param	enable, when set to true enables the peripheral. use this option when using the peripehral
 * 			raw, not as a device.
 * 			set to false when using as a device - it will be enabled when the open()
 * 			function is invoked on its device file.
 * @retval	returns true if the operation succeeded, false otherwise.
 */
bool usart_init(USART_TypeDef* usart, char* install, bool enable)
{
	bool ret = true;
	int8_t usart_devno = get_usart_devno(usart);

	log_syslog(NULL, "init usart%d", usart_devno);

	assert_true(usart_devno != -1);

    if(install)
    {
#if USE_POSIX_STYLE_IO
    	// installed USART can only work with interrupt enabled
    	usart_init_interrupt(usart, USART_INTERRUPT_PRIORITY, true);
    	usart_dev_ioctls[usart_devno] = (void*)install_device(install,
														usart,
														usart_enable_rx_ioctl,
														usart_enable_tx_ioctl,
														usart_open_ioctl,
														usart_close_ioctl,
														usart_ioctl);
#else
    	// todo - init fifo's - and interrupts
//        usart_init_interrupt(usart, USART_INTERRUPT_PRIORITY, true);
    	usart_dev_ioctls[usart_devno] = NULL;
#endif
    	ret = usart_dev_ioctls[usart_devno] != NULL;
        log_syslog(NULL, "install usart%d: %s", usart_devno, ret ? "successful" : "failed");
    }

    if(enable)
    {
    	// installed usarts are opened automatically... this is optional
        usart_init_gpio(usart);
        usart_init_device(usart, ENABLE);
    }

    return ret;
}


/**
 * initialize interrupts for the specified USART.
 *
 * **note:** this functuion is called by usart_init.
 */
void usart_init_interrupt(USART_TypeDef* usart, uint8_t priority, FunctionalState enable)
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
 *
 * The baud rate is computed using the following formula:
 * - IntegerDivider = ((PCLKx) / (8 * (OVR8+1) * baudrate))
 * - FractionalDivider = ((IntegerDivider - IntegerDivider) * 8 * (OVR8+1)) + 0.5
 * Where OVR8 is the "oversampling by 8 mode" configuration bit in the CR1 register.
 */
void usart_set_baudrate(USART_TypeDef* usart, uint32_t br)
{
    uint32_t apbclock;
	uint32_t tmpreg;
	uint32_t integerdivider;
    uint32_t fractionaldivider;
    RCC_ClocksTypeDef RCC_ClocksStatus;

	RCC_GetClocksFreq(&RCC_ClocksStatus);

#if defined(STM32F10X_HD) || defined(STM32F10X_CL)
    if(usart == USART1)
#elif FAMILY == STM32F4
    if(usart == USART1 || usart == USART6)
#endif
        apbclock = RCC_ClocksStatus.PCLK2_Frequency;
    else
        apbclock = RCC_ClocksStatus.PCLK1_Frequency;

	// Determine the integer part
	integerdivider = ((25 * apbclock) / (4 * br)); // Integer part computing in case Oversampling mode is 16 Samples

	tmpreg = (integerdivider / 100) << 4;

	// Determine the fractional part
	fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

	// Implement the fractional part in the register
	tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);

	/* Write to USART BRR */
	usart->BRR = (uint16_t)tmpreg;
}

/**
 * reads the current baudrate.
 */
uint32_t usart_get_baudrate(USART_TypeDef* usart)
{
    uint32_t pclock;
    RCC_ClocksTypeDef RCC_ClocksStatus;
    RCC_GetClocksFreq(&RCC_ClocksStatus);

#if defined(STM32F10X_HD) || defined(STM32F10X_CL)
    if(usart == USART1)
#elif FAMILY == STM32F4
    if(usart == USART1 || usart == USART6)
#endif
        pclock = RCC_ClocksStatus.PCLK2_Frequency;
    else
        pclock = RCC_ClocksStatus.PCLK1_Frequency;

    // multiply by 25 as 25 * max of 168MHz just avoids overflow and give better prescicon
    uint32_t div = (25 * (usart->BRR >> 4)) + ((25 * (usart->BRR & 0x000f))/16);
    return ((pclock * 25) / div) / 16;
}

#if USE_POSIX_STYLE_IO
static int usart_enable_rx_ioctl(dev_ioctl_t* dev)
{
    USART_TypeDef* usart = (USART_TypeDef*)(dev->ctx);
    USART_ITConfig(usart, USART_IT_RXNE, ENABLE);
    return 0;
}

static int usart_enable_tx_ioctl(dev_ioctl_t* dev)
{
    USART_TypeDef* usart = (USART_TypeDef*)(dev->ctx);
    USART_ITConfig(usart, USART_IT_TXE, ENABLE);
    return 0;
}

static int usart_open_ioctl(dev_ioctl_t* dev)
{
    USART_TypeDef* usart = (USART_TypeDef*)(dev->ctx);
    usart_init_gpio(usart);
    usart_init_device(usart, ENABLE);
    return 0;
}

static int usart_close_ioctl(dev_ioctl_t* dev)
{
    USART_TypeDef* usart = (USART_TypeDef*)(dev->ctx);
    USART_ITConfig(usart, USART_IT_RXNE, DISABLE);
    USART_ITConfig(usart, USART_IT_TXE, DISABLE);
    usart_init_device(usart, DISABLE);
    return 0;
}

static int usart_ioctl(dev_ioctl_t* dev)
{
    USART_TypeDef* usart = (USART_TypeDef*)(dev->ctx);

    uint32_t baudrate = dev->termios->c_ispeed ?
            dev->termios->c_ispeed : dev->termios->c_ospeed;
    if(baudrate)
    {
        // set baudrate
        usart_set_baudrate(usart, baudrate);
    }
    else
    {
        // read baudrate
        dev->termios->c_ispeed = usart_get_baudrate(usart);
        dev->termios->c_ospeed = dev->termios->c_ispeed;
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
 * @}
 */

