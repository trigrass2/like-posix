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
 * @defgroup system STM32 System Control
 *
 * A collection of random functions for target initialization and system setup.
 *
 * @file system.c
 * @{
 */

#include <stdio.h>
#include "misc.h"
#include "board_config.h"
#include "system.h"

volatile int32_t ITM_RxBuffer;

/**
 * @TODO use this to offset the application for use with bootloader
 * linker script defines memory base and vector table offset values
 * Set the Vector Table base location at:
 * FLASH_BASE+_isr_vectorsflash_offs
 * or:SRAM_BASE+_isr_vectorsram_offs
 * set 16 levels of preemption priority, 0 levels of subpriority
 */
// extern uint32_t _isr_vectorsflash_offs;
// void configure_nvic()
// {
// 	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
// 	NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)&_isr_vectorsflash_offs);
// }
void configure_nvic()
{
	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, VECT_TAB_OFFSET);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}

/**
 * @brief enables the FPU - cortex-m4 devices only.
 */
void enable_fpu()
{
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
#endif
}

/**
  * @brief  Enables brown-out detection and reset.
  * 		Brown out detection is set to 2.9V.
  */
void enable_bod()
{
	// enable power control system clock, set it up for 3.3V supply operation (2.9V brownout reset)
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

#if FAMILY == STM32F1
	PWR_PVDLevelConfig(PWR_PVDLevel_2V9);
#elif FAMILY == STM32F4
	PWR_PVDLevelConfig(PWR_PVDLevel_6);
#endif
	PWR_PVDCmd(ENABLE);
}

void delay(volatile uint32_t count)
{
    count *= SystemCoreClock/8960;
    while(count-- > 0);
}

/**
 * @}
 */
