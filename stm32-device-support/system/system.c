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
 * @defgroup system STM32 System Control
 *
 * A collection of random functions for target initialization and system setup.
 *
 * @file system.c
 * @{
 */

#include <stdlib.h>
#include <stdio.h>
//#include "misc.h"
#include "board_config.h"
#include "system.h"
#include "strutils.h"


static void set_resetflag(uint16_t rcc_flag, uint16_t resetflag, uint16_t* resetflags);

/**
 * linker script defines memory base and vector table offset values
 * Set the Vector Table base location at:
 * FLASH_BASE+___SVECTOR_OFFSET
 * set 16 levels of preemption priority, 0 levels of subpriority
 */
void configure_nvic()
{
    // happens in system_stm32fxxx.c
//	NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)&___SVECTOR_OFFSET);
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
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
    PWR_PVDTypeDef conf = {
            .PVDLevel = PWR_PVDLEVEL_6,
            .Mode = PWR_PVD_MODE_NORMAL
    };
	// enable power control system clock, set it up for 3.3V supply operation (2.8V brownout reset)
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_ConfigPVD(&conf);
	HAL_PWR_EnablePVD();
}

/**
 * bit of a hack and not really a good idea to use these kind of things.
 * delays for aproximately count milliseconds.
 */
void delay(volatile uint32_t count)
{
#if USE_DRIVER_SYSTEM_TIMER
    uint32_t t = get_hw_time_ms() + count;
    while(get_hw_time_ms() < t);
#else
    count *= SystemCoreClock/8960;
    while(count-- > 0);
#endif
}

/**
  * @brief  reset the processor by software...
  */
void soft_reset()
{
    clear_resetflags();
    HAL_NVIC_SystemReset();
}

/**
  * @brief  causes a hardfault by calling invalid function by pointer. used for testing exception handler.
  */
void fake_hardfault()
{
    function_pointer_t f = (function_pointer_t)0x12345678;
    f();
}

/**
 *
 * The STM32 device can have multiple reset sources flagged.
 *
 * @param rcc_flag is one of the RCC flags to check....
 *    @arg RCC_FLAG_HSIRDY: HSI oscillator clock ready
 *    @arg RCC_FLAG_HSERDY: HSE oscillator clock ready
 *    @arg RCC_FLAG_PLLRDY: main PLL clock ready
 *    @arg RCC_FLAG_PLLI2SRDY: PLLI2S clock ready
 *    @arg RCC_FLAG_LSERDY: LSE oscillator clock ready
 *    @arg RCC_FLAG_LSIRDY: LSI oscillator clock ready
 *    @arg RCC_FLAG_BORRST: POR/PDR or BOR reset
 *    @arg RCC_FLAG_PINRST: Pin reset
 *    @arg RCC_FLAG_PORRST: POR/PDR reset
 *    @arg RCC_FLAG_SFTRST: Software reset
 *    @arg RCC_FLAG_IWDGRST: Independent Watchdog reset
 *    @arg RCC_FLAG_WWDGRST: Window Watchdog reset
 *    @arg RCC_FLAG_LPWRRST: Low Power reset
 * @param resetflag is one of the following  enumerated values:
 *    @arg RESETFLAG_PINRST
 *    @arg RESETFLAG_PORRST
 *    @arg RESETFLAG_SFTRST
 *    @arg RESETFLAG_IWDGRST
 *    @arg RESETFLAG_WWDGRST
 * @param resetflags is a variable to store the result in.
 */
void set_resetflag(uint16_t rcc_flag, uint16_t resetflag, uint16_t* resetflags)
{
    if(__HAL_RCC_GET_FLAG(rcc_flag) == SET)
        *resetflags |= resetflag;
}

/**
 * clears all reset flags
 */
void clear_resetflags()
{
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

/**
 * @retval  returns the source(s) of the last reset.
 */
uint16_t get_resetflags()
{
    uint16_t resetflags = 0;

    set_resetflag(RCC_FLAG_SFTRST, RESETFLAG_SFTRST, &resetflags);
    set_resetflag(RCC_FLAG_PORRST, RESETFLAG_PORRST, &resetflags);
    set_resetflag(RCC_FLAG_PINRST, RESETFLAG_PINRST, &resetflags);
    set_resetflag(RCC_FLAG_IWDGRST, RESETFLAG_IWDGRST, &resetflags);
    set_resetflag(RCC_FLAG_WWDGRST, RESETFLAG_WWDGRST, &resetflags);
    set_resetflag(RCC_FLAG_LPWRRST, RESETFLAG_LPWRRST, &resetflags);

    return resetflags;
}

/**
 * @retval  returns true if the specified flag is set in the resetflags variable (populated by @ref get_resetflags)
 */
bool get_resetflag_state(uint16_t resetflags, uint16_t flag)
{
    return (bool)(resetflags&flag);
}

/**
 * @brief   set stack pointer and exectute from some address.
 */
void run_from(uint32_t address)
{
    __set_MSP(*(__IO uint32_t*)address);

    uint32_t startAddress = *(__IO uint32_t*)(address + 4);
    function_pointer_t runapp = (function_pointer_t)(startAddress);
    runapp();
}

/**
 * @brief   reads the device unique id, converts the 96bit number to a uint64_t.
 */
uint64_t get_device_uid()
{
    uint64_t serialnumber;
#if defined(STM32F10X_HD) || defined(STM32F10X_MD) || defined(STM32F10X_LD)
    serialnumber = *(__IO uint64_t*)(0x1FFFF7E8);
    serialnumber += *(__IO uint64_t*)(0x1FFFF7EC);
//  serialnumber += *(__IO uint64_t*)(0x1FFFF7F0);
#elif defined(STM32F10X_CL)
    // TODO - this isnt right?
    serialnumber = *(__IO uint64_t*)(0x1FFFB7E8);
    serialnumber += *(__IO uint64_t*)(0x1FFFB7EC);
//  serialnumber += *(__IO uint64_t*)(0x1FFFB7F0);
#elif defined(STM32F4XX)
    serialnumber = *(__IO uint64_t*)(0x1FFF7A10);
    serialnumber += *(__IO uint64_t*)(0x1FFF7A14);
//  serialnumber += *(__IO uint64_t*)(0x1FFF7A18);
#endif

    return serialnumber;
}

/**
 * @brief   reads the device unique id, converts it to a base32 string.
 * @param   str is some memory to put the uid string into including the null terminator.
 *          it should be a minimum of 14 characters long.
 */
void get_device_uid_string(uint8_t* str)
{
    ditoa(get_device_uid(), (char*)str, 32);
}

/**
 * @}
 */
