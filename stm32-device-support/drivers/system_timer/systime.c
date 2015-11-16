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


#include "board_config.h"
#include "systime.h"

#include "systime_config.h"

static volatile unsigned long system_seconds;

#define SYSTIMER_TICK_RATE          10000
#define SYSTIMER_TV_TICK_RATE		1000000
#define SYSTIMER_PRESCALER			((SYSTIMER_BUS_CLOCK / SYSTIMER_TICK_RATE) - 1)
#define SYSTIMER_OVERFLOW			(SYSTIMER_TICK_RATE - 1)


void init_systime()
{
    system_seconds = 0;
    if((SYSTIMER_PERIPH == TIM2)||(SYSTIMER_PERIPH == TIM3)||(SYSTIMER_PERIPH == TIM4)||(SYSTIMER_PERIPH == TIM5))
        RCC_APB1PeriphClockCmd(SYSTIMER_CLOCK, ENABLE);
    else if((SYSTIMER_PERIPH == TIM1)||(SYSTIMER_PERIPH == TIM8))
        RCC_APB2PeriphClockCmd(SYSTIMER_CLOCK, ENABLE);
    else
        assert_true(0);

    TIM_TimeBaseInitTypeDef timer_init =
    {
            SYSTIMER_PRESCALER,								// prescaler value
            TIM_CounterMode_Up,				// counter mode
            SYSTIMER_OVERFLOW,  							// period reload value
            TIM_CKD_DIV1,		      		// clock divider value (1, 2 or 4) (has no effect on OC/PWM)
            0						      	// repetition counter
    };
    TIM_TimeBaseInit(SYSTIMER_PERIPH, &timer_init);

    NVIC_InitTypeDef timer_nvic =
    {
        SYSTIMER_IRQ,
        SYSTIMER_INT_PRIORITY,
        0,
        ENABLE
    };
    NVIC_Init(&timer_nvic);
    TIM_ITConfig(SYSTIMER_PERIPH, TIM_IT_Update, ENABLE);
    TIM_Cmd(SYSTIMER_PERIPH, ENABLE);
}

void SYSTIMER_INTERRUPT_HANDLER()
{
    TIM_ClearITPendingBit(SYSTIMER_PERIPH, TIM_IT_Update);
    system_seconds++;
}

void get_hw_time(unsigned long* secs, unsigned long* usecs)
{
    unsigned long seconds, useconds;
    __disable_irq();
    seconds = system_seconds;
    useconds = SYSTIMER_PERIPH->CNT * (SYSTIMER_TV_TICK_RATE / SYSTIMER_TICK_RATE);
    __enable_irq();
    *secs = seconds;
    *usecs = useconds;
}

unsigned long get_hw_time_ms()
{
    unsigned long seconds, useconds;
    __disable_irq();
    seconds = system_seconds;
    useconds = SYSTIMER_PERIPH->CNT * (SYSTIMER_TV_TICK_RATE / SYSTIMER_TICK_RATE);
    __enable_irq();
    return (seconds*1000) + (useconds/1000);
}

void set_hw_time(unsigned long secs, unsigned long usecs)
{
    __disable_irq();
    system_seconds = secs;
    SYSTIMER_PERIPH->CNT = usecs / (SYSTIMER_TV_TICK_RATE / SYSTIMER_TICK_RATE);
    __enable_irq();
}
