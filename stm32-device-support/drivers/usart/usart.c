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
 * This file is part of the lollyjar project, <https://github.com/drmetal/lollyjar>
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
#include "base_usart.h"
#include "asserts.h"
#include "cutensils.h"
#include "system.h"


static USART_TypeDef* console_usart;

#if USE_LIKEPOSIX
static int usart_ioctl(dev_ioctl_t* dev);
static int usart_close_ioctl(dev_ioctl_t* dev);
static int usart_open_ioctl(dev_ioctl_t* dev);
static int usart_enable_tx_ioctl(dev_ioctl_t* dev);
static int usart_enable_rx_ioctl(dev_ioctl_t* dev);
usart_ioctl_t usart_ioctls[6];
dev_ioctl_t* usart_dev_ioctls[6];
#endif




/**
 * write a character to the console usart.
 */
void phy_putc(char c)
{
	USART_HandleTypeDef husart;
	husart.Instance = console_usart;
    if(console_usart)
    {

        while(!__HAL_USART_GET_FLAG(&husart, USART_FLAG_TXE));
        console_usart->DR =  c;
    }
}

/**
 * get a character from the console usart.
 */
char phy_getc(void)
{
	USART_HandleTypeDef husart;
	husart.Instance = console_usart;
    if(console_usart)
    {
        while(!__HAL_USART_GET_FLAG(&husart, USART_FLAG_RXNE));
        return (char)console_usart->DR;
    }
    return -1;
}

/**
 * initialize the specified USART device.
 *
 * **note:** this function is called by usart_init.
 */
void usart_init_device(USART_TypeDef* usart, bool enable, usart_mode_t mode)
{
    USART_HandleTypeDef husart = {
    		.Instance = usart,
			.Init = {
					.BaudRate = USART_DEFAULT_BAUDRATE,
					.WordLength = USART_WORDLENGTH_8B,
					.StopBits = USART_STOPBITS_1,
					.Parity = USART_PARITY_NONE,
					.Mode = USART_MODE_TX_RX,
					.CLKPolarity = USART_POLARITY_LOW,
					.CLKPhase = USART_PHASE_1EDGE,
					.CLKLastBit = USART_LASTBIT_ENABLE
			},
			.pTxBuffPtr = NULL,
			.TxXferSize = 0,
			.TxXferCount = 0,
			.pRxBuffPtr = NULL,
			.RxXferSize = 0,
			.RxXferCount = 0,
			.hdmatx = NULL,
			.hdmarx = NULL,
			.Lock = HAL_UNLOCKED,
			.State = HAL_USART_STATE_RESET,
			.ErrorCode = 0
    };

	switch(mode)
	{
		case USART_ONEWIRE:
			husart.Init.Mode = USART_MODE_TX_RX;
		break;

		case USART_FULLDUPLEX:
		default:
			husart.Init.Mode = USART_MODE_TX_RX;
		break;
	}

    if(enable)
    	HAL_USART_Init(&husart);
    else
    	HAL_USART_DeInit(&husart);
}

void HAL_USART_MspInit(USART_HandleTypeDef *husart)
{
	if(husart->Instance == USART1)
		__HAL_RCC_USART1_CLK_ENABLE();
	if(husart->Instance == USART2)
		__HAL_RCC_USART2_CLK_ENABLE();
	if(husart->Instance == USART3)
		__HAL_RCC_USART3_CLK_ENABLE();
	if(husart->Instance == UART4)
		__HAL_RCC_UART4_CLK_ENABLE();
	if(husart->Instance == UART5)
		__HAL_RCC_UART5_CLK_ENABLE();
#if FAMILY == STM32F4
	if(husart->Instance == USART6)
		__HAL_RCC_USART6_CLK_ENABLE();
#endif
}

void HAL_USART_MspDeInit(USART_HandleTypeDef *husart)
{
	if(husart->Instance == USART1)
		__HAL_RCC_USART1_CLK_DISABLE();
	if(husart->Instance == USART2)
		__HAL_RCC_USART2_CLK_DISABLE();
	if(husart->Instance == USART3)
		__HAL_RCC_USART3_CLK_DISABLE();
	if(husart->Instance == UART4)
		__HAL_RCC_UART4_CLK_DISABLE();
	if(husart->Instance == UART5)
		__HAL_RCC_UART5_CLK_DISABLE();
#if FAMILY == STM32F4
	if(husart->Instance == USART6)
		__HAL_RCC_USART6_CLK_DISABLE();
#endif
}

/**
 * initialize the IO pins for the specified USART.
 *
 * **note:** pin mapping is set in serial_config.h.
 *
 * **note:** this function is called by usart_init.
 */
void usart_init_gpio(USART_TypeDef* usart, usart_mode_t mode)
{
	GPIO_InitTypeDef GPIO_InitStructure_rx;
	GPIO_InitTypeDef GPIO_InitStructure_tx;

	switch(mode)
	{
		case USART_ONEWIRE:
			GPIO_InitStructure_rx.Mode = GPIO_MODE_INPUT;
			GPIO_InitStructure_rx.Pull = GPIO_PULLUP;
			GPIO_InitStructure_rx.Speed = GPIO_SPEED_MEDIUM;
			GPIO_InitStructure_tx.Mode = GPIO_MODE_AF_OD;
			GPIO_InitStructure_tx.Pull = GPIO_PULLUP;
			GPIO_InitStructure_tx.Speed = GPIO_SPEED_MEDIUM;
		break;

		case USART_FULLDUPLEX:
		default:
			GPIO_InitStructure_rx.Mode = GPIO_MODE_INPUT;
			GPIO_InitStructure_rx.Pull = GPIO_PULLUP;
			GPIO_InitStructure_rx.Speed = GPIO_SPEED_MEDIUM;
			GPIO_InitStructure_tx.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStructure_tx.Pull = GPIO_NOPULL;
			GPIO_InitStructure_tx.Speed = GPIO_SPEED_MEDIUM;
		break;
	}

    if(usart == USART1)
    {
#if FAMILY == STM32F1
#ifdef USART1_REMAP
    	__HAL_AFIO_REMAP_USART1_ENABLE();
#endif
#elif FAMILY == STM32F4
    	GPIO_InitStructure_tx.Alternate = GPIO_AF7_USART1;
    	GPIO_InitStructure_rx.Alternate = GPIO_AF7_USART1;
#endif
    	GPIO_InitStructure_tx.Pin = USART1_TX_PIN;
    	GPIO_InitStructure_rx.Pin = USART1_RX_PIN;
    	HAL_GPIO_Init(USART1_PORT, &GPIO_InitStructure_tx);
    	HAL_GPIO_Init(USART1_PORT, &GPIO_InitStructure_rx);
    }
    else if(usart == USART2)
    {
#if FAMILY == STM32F1
#ifdef USART2_REMAP
        __HAL_AFIO_REMAP_USART2_ENABLE();
#endif
#elif FAMILY == STM32F4
    	GPIO_InitStructure_tx.Alternate = GPIO_AF7_USART2;
    	GPIO_InitStructure_rx.Alternate = GPIO_AF7_USART2;
#endif
    	GPIO_InitStructure_tx.Pin = USART2_TX_PIN;
    	GPIO_InitStructure_rx.Pin = USART2_RX_PIN;
    	HAL_GPIO_Init(USART2_PORT, &GPIO_InitStructure_tx);
    	HAL_GPIO_Init(USART2_PORT, &GPIO_InitStructure_rx);
    }
    else if(usart == USART3)
    {
#if FAMILY == STM32F1
#ifdef USART3_REMAP
    	__HAL_AFIO_REMAP_USART3_ENABLE();
#elif defined(USART3_PARTIAL_REMAP)
    	__HAL_AFIO_REMAP_USART3_PARTIAL();
#endif
#elif FAMILY == STM32F4
    	GPIO_InitStructure_tx.Alternate = GPIO_AF7_USART3;
    	GPIO_InitStructure_rx.Alternate = GPIO_AF7_USART3;
#endif
    	GPIO_InitStructure_tx.Pin = USART3_TX_PIN;
    	GPIO_InitStructure_rx.Pin = USART3_RX_PIN;
    	HAL_GPIO_Init(USART3_PORT, &GPIO_InitStructure_tx);
    	HAL_GPIO_Init(USART3_PORT, &GPIO_InitStructure_rx);
    }

    else if(usart == UART4)
    {
#if FAMILY == STM32F4
    	GPIO_InitStructure_tx.Alternate = GPIO_AF8_UART4;
    	GPIO_InitStructure_rx.Alternate = GPIO_AF8_UART4;
#endif
    	GPIO_InitStructure_tx.Pin = UART4_TX_PIN;
    	GPIO_InitStructure_rx.Pin = UART4_RX_PIN;
    	HAL_GPIO_Init(UART4_PORT, &GPIO_InitStructure_tx);
    	HAL_GPIO_Init(UART4_PORT, &GPIO_InitStructure_rx);
    }

    else if(usart == UART5)
    {
#if FAMILY == STM32F4
    	GPIO_InitStructure_tx.Alternate = GPIO_AF8_UART5;
    	GPIO_InitStructure_rx.Alternate = GPIO_AF8_UART5;
#endif
    	GPIO_InitStructure_tx.Pin = UART5_TX_PIN;
    	GPIO_InitStructure_rx.Pin = UART5_RX_PIN;
    	HAL_GPIO_Init(UART5_TX_PORT, &GPIO_InitStructure_tx);
    	HAL_GPIO_Init(UART5_RX_PORT, &GPIO_InitStructure_rx);
    }
#if FAMILY == STM32F4
    else if(usart == USART6)
    {
    	GPIO_InitStructure_tx.Alternate = GPIO_AF8_USART6;
    	GPIO_InitStructure_rx.Alternate = GPIO_AF8_USART6;
    	GPIO_InitStructure_tx.Pin = USART6_TX_PIN;
    	GPIO_InitStructure_rx.Pin = USART6_RX_PIN;
    	HAL_GPIO_Init(USART6_PORT, &GPIO_InitStructure_tx);
    	HAL_GPIO_Init(USART6_PORT, &GPIO_InitStructure_rx);
    }
#endif
}

int8_t get_usart_devno(USART_TypeDef* usart)
{
	if(usart == USART1)
		return 0;
	else if(usart == USART2)
		return 1;
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
bool usart_init(USART_TypeDef* usart, char* install, bool enable, usart_mode_t mode)
{
	bool ret = true;
	int8_t usart_devno = get_usart_devno(usart);

	log_syslog(NULL, "init usart%d", usart_devno+1);

	assert_true(usart_devno != -1);

    if(install)
    {
#if USE_LIKEPOSIX
    	usart_ioctls[usart_devno].usart = usart;
    	usart_ioctls[usart_devno].mode = mode;
    	usart_dev_ioctls[usart_devno] = install_device(install,
    													&usart_ioctls[usart_devno],
														usart_enable_rx_ioctl,
														usart_enable_tx_ioctl,
														usart_open_ioctl,
														usart_close_ioctl,
														usart_ioctl);
    	// installed USART can only work with interrupt enabled
    	usart_init_interrupt(usart, USART_INTERRUPT_PRIORITY, true);
    	ret = usart_dev_ioctls[usart_devno] != NULL;
#else
    	ret = true;
#endif
        log_syslog(NULL, "install usart%d: %s", usart_devno+1, ret ? "successful" : "failed");
    }

    if(enable)
    {
    	// installed usarts are opened automatically... this is optional
        usart_init_gpio(usart, mode);
        usart_init_device(usart, true, mode);
    }

    return ret;
}


/**
 * initialize interrupts for the specified USART.
 *
 * **note:** this function is called by usart_init.
 */
void usart_init_interrupt(USART_TypeDef* usart, uint8_t priority, bool enable)
{
	uint8_t irq = 0;
	USART_HandleTypeDef husart;
	husart.Instance = usart;

	__HAL_USART_DISABLE_IT(&husart, USART_IT_RXNE);
	__HAL_USART_DISABLE_IT(&husart, USART_IT_TXE);
	__HAL_USART_DISABLE_IT(&husart, USART_IT_ERR);

	if(usart == USART1)
		irq = USART1_IRQn;
	else if(usart == USART2)
		irq = USART2_IRQn;
	else if (usart == USART3)
		irq = USART3_IRQn;
	else if (usart == UART4)
		irq = UART4_IRQn;
	else if (usart == UART5)
		irq = UART5_IRQn;
#if FAMILY == STM32F4
	else if (usart == USART6)
		irq = USART6_IRQn;
	else
		assert_true(0);
#endif

    if(enable)
    {
    	HAL_NVIC_SetPriority(irq, priority, 0);
    	HAL_NVIC_EnableIRQ(irq);
    }
    else
    	HAL_NVIC_DisableIRQ(irq);
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
#if FAMILY == STM32F4
	if((usart == USART1) || (usart == USART6))
#else
    if(usart == USART1)
#endif
		usart->BRR = USART_BRR(HAL_RCC_GetPCLK2Freq(), br);
	else
		usart->BRR = USART_BRR(HAL_RCC_GetPCLK1Freq(), br);
}

/**
 * reads the current baudrate.
 */
uint32_t usart_get_baudrate(USART_TypeDef* usart)
{
    uint32_t pclock;

#if FAMILY == STM32F4
    if(usart == USART1 || usart == USART6)
#else
    if(usart == USART1)
#endif
    	pclock = HAL_RCC_GetPCLK2Freq();
    else
    	pclock = HAL_RCC_GetPCLK1Freq();

    // multiply by 25 as 25 * max of 168MHz just avoids overflow and give better prescicon
    uint32_t div = (25 * (usart->BRR >> 4)) + ((25 * (usart->BRR & 0x000f))/16);
    return ((pclock * 25) / div) / 16;
}

#if USE_LIKEPOSIX
static int usart_enable_rx_ioctl(dev_ioctl_t* dev)
{
	USART_HandleTypeDef husart;
	husart.Instance = ((usart_ioctl_t*)(dev->ctx))->usart;
	__HAL_USART_ENABLE_IT(&husart, USART_IT_RXNE);
    return 0;
}

static int usart_enable_tx_ioctl(dev_ioctl_t* dev)
{
	USART_HandleTypeDef husart;
	husart.Instance = ((usart_ioctl_t*)(dev->ctx))->usart;
	__HAL_USART_ENABLE_IT(&husart, USART_IT_TXE);
    return 0;
}

static int usart_open_ioctl(dev_ioctl_t* dev)
{
    usart_ioctl_t* ioctl = (usart_ioctl_t*)(dev->ctx);
    usart_init_gpio(ioctl->usart, ioctl->mode);
    usart_init_device(ioctl->usart, true, ioctl->mode);
    return 0;
}

static int usart_close_ioctl(dev_ioctl_t* dev)
{
	usart_ioctl_t* ioctl = (usart_ioctl_t*)(dev->ctx);
	USART_HandleTypeDef husart;
	husart.Instance = ioctl->usart;
	__HAL_USART_DISABLE_IT(&husart, USART_IT_RXNE);
	__HAL_USART_DISABLE_IT(&husart, USART_IT_TXE);
    usart_init_device(husart.Instance, false, ioctl->mode);
    return 0;
}

static int usart_ioctl(dev_ioctl_t* dev)
{
    USART_TypeDef* usart = ((usart_ioctl_t*)(dev->ctx))->usart;

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

