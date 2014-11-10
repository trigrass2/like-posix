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

#include <stdio.h>
#include "board_config.h"
#include "systime.h"

static unsigned long seconds;

#define TIMER_INT_PRIORITY		2
#define TIMER_BUS_CLOCK			84000000
#define TIMER_TICK_RATE			10000
#define TV_TICK_RATE			1000000
#define PRESCALER				((TIMER_BUS_CLOCK / TIMER_TICK_RATE) - 1)
#define OVERFLOW				(TIMER_TICK_RATE - 1)

void init_systime()
{
	seconds = 0;

	// Audio input sample rate, select trigger
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef timer_init =
	{
			PRESCALER,								// prescaler value
			TIM_CounterMode_Up,				// counter mode
			OVERFLOW,  							// period reload value
			TIM_CKD_DIV1,		      		// clock divider value (1, 2 or 4) (has no effect on OC/PWM)
			0						      	// repetition counter
	};
	TIM_TimeBaseInit(TIM2, &timer_init);

	// configure interrupt for card control timer
	NVIC_InitTypeDef timer_nvic =
	{
		TIM2_IRQn,
		2,
		0,
		ENABLE
	};
	NVIC_Init(&timer_nvic);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler()
{
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	seconds++;
}

void get_hw_time(unsigned long* secs, unsigned long* usecs)
{
	*secs = seconds;
	*usecs = TIM2->CNT * (TV_TICK_RATE / TIMER_TICK_RATE);
}

void set_hw_time(unsigned long secs, unsigned long usecs)
{
	seconds = secs;
	TIM2->CNT = usecs / (TV_TICK_RATE / TIMER_TICK_RATE);
}
