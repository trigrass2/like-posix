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
 * @{
 *
 * @file    base_usart.h
 */


#ifndef BASE_USART_H_
#define BASE_USART_H_

#include "../../board/uemb4.bsp/usart_config.h"


#if USART1_FULL_REMAP
#define USART1_REMAP GPIO_Remap_USART1
#define USART1_PORT GPIOB
#define USART1_TX_PIN GPIO_Pin_6
#define USART1_RX_PIN GPIO_Pin_7
#define USART1_TX_PINSOURCE GPIO_PinSource6
#define USART1_RX_PINSOURCE GPIO_PinSource7
#else
#define USART1_PORT GPIOA
#define USART1_TX_PIN GPIO_Pin_9
#define USART1_RX_PIN GPIO_Pin_10
#define USART1_TX_PINSOURCE GPIO_PinSource9
#define USART1_RX_PINSOURCE GPIO_PinSource10
#endif
#define USART1_CLOCK RCC_APB2Periph_USART1

#if USART2_FULL_REMAP
#define USART2_REMAP GPIO_Remap_USART2
#define USART2_PORT GPIOD
#define USART2_TX_PIN GPIO_Pin_5
#define USART2_RX_PIN GPIO_Pin_6
#define USART2_TX_PINSOURCE GPIO_PinSource5
#define USART2_RX_PINSOURCE GPIO_PinSource6
#else
#define USART2_PORT GPIOA
#define USART2_TX_PIN GPIO_Pin_2
#define USART2_RX_PIN GPIO_Pin_3
#define USART2_TX_PINSOURCE GPIO_PinSource2
#define USART2_RX_PINSOURCE GPIO_PinSource3
#endif
#define USART2_CLOCK RCC_APB1Periph_USART2

#if USART3_FULL_REMAP
#define USART3_REMAP GPIO_FullRemap_USART3
#define USART3_PORT GPIOD
#define USART3_TX_PIN GPIO_Pin_8
#define USART3_RX_PIN GPIO_Pin_9
#define USART3_TX_PINSOURCE GPIO_PinSource8
#define USART3_RX_PINSOURCE GPIO_PinSource9
#elif USART3_PARTIAL_REMAP
#define USART3_REMAP GPIO_PartialRemap_USART3
#define USART3_PORT GPIOC
#define USART3_TX_PIN GPIO_Pin_10
#define USART3_RX_PIN GPIO_Pin_11
#define USART3_TX_PINSOURCE GPIO_PinSource10
#define USART3_RX_PINSOURCE GPIO_PinSource11
#else
#define USART3_PORT GPIOB
#define USART3_TX_PIN GPIO_Pin_10
#define USART3_RX_PIN GPIO_Pin_11
#define USART3_TX_PINSOURCE GPIO_PinSource10
#define USART3_RX_PINSOURCE GPIO_PinSource11
#endif
#define USART3_CLOCK RCC_APB1Periph_USART3

#define UART4_PORT GPIOC
#define UART4_TX_PIN GPIO_Pin_10
#define UART4_RX_PIN GPIO_Pin_11
#define UART4_TX_PINSOURCE GPIO_PinSource10
#define UART4_RX_PINSOURCE GPIO_PinSource11
#define UART4_CLOCK RCC_APB1Periph_UART4

#define UART5_RX_PORT GPIOD
#define UART5_TX_PORT GPIOC
#define UART5_TX_PIN GPIO_Pin_12
#define UART5_RX_PIN GPIO_Pin_2
#define UART5_TX_PINSOURCE GPIO_PinSource12
#define UART5_RX_PINSOURCE GPIO_PinSource2
#define UART5_CLOCK RCC_APB1Periph_UART5

#if FAMILY == STM32F4
#define USART6_PORT GPIOC
#define USART6_TX_PIN GPIO_Pin_6
#define USART6_RX_PIN GPIO_Pin_7
#define USART6_TX_PINSOURCE GPIO_PinSource6
#define USART6_RX_PINSOURCE GPIO_PinSource7
#define USART6_CLOCK RCC_APB2Periph_USART6
#endif

#endif /* BASE_USART_H_ */

/**
 * @}
 */
