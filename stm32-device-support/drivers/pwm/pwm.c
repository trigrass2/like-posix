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

/**
 * @defgroup pwm PWM Driver
 *
 * Simple PWM Driver.
 *
 * - suppoprts multiple PWM's.
 * - if the same timer is used for multiple channels, they must be set at the same frequency and resolution.
 * - one timer supports up to 4 channels.
 * - to set up more than 4 channels, use multiple timers.
 * - when using multiple timers, different frequencies and resulmay be set.
 * - if the frequency and resolution cant be set up due to physical limitation of the timer, an assertion error is raised.
 *
 *
 * Eg:
\code

pwm_t pwm1 = {
    .timer=TIM3,
    .channel=TIM_CHANNEL_1,
    .port=GPIOA,
    .pin=GPIO_PIN_6
};

pwm_t pwm2 = {
    .timer=TIM3,
    .channel=TIM_CHANNEL_2,
    .port=GPIOA,
    .pin=GPIO_PIN_7
};

// init both PWM's at 1000Hz, duty resolution of 100 steps.
pwm_init(&pwm1, "pwm1", 1000, 100);
pwm_init(&pwm2, "pwm2", 1000, 100);

// set one channel to 50% duty, the other to 100%.
pwm_set_duty(&pwm1, 50);
pwm_set_duty(&pwm2, 99);
// this wont work, range for a resolution of 100 is 0-99
pwm_set_duty(&pwm2, 100);

\endcode
 *
 * @file pwm.c
 * @{
 */

#include "pwm.h"


void pwm_init(pwm_t* pwm, const char* name, uint32_t frequency, uint16_t resolution)
{
    uint32_t apbclock;
    uint32_t prescaler;

    log_init(&pwm->log, name);

    if(pwm->timer == TIM1 || pwm->timer == TIM8) {
        apbclock = HAL_RCC_GetPCLK2Freq();
    }
    else {
        apbclock = HAL_RCC_GetPCLK1Freq();
    }

    prescaler = apbclock / (frequency * resolution);

    log_debug(&pwm->log, "freq=%d, res=%d presc=%d", frequency, resolution, prescaler);

    assert_true(frequency <= apbclock/2);
    assert_true(prescaler > 0);

    pwm->resolution = resolution;

    TIM_HandleTypeDef htim;
    htim.Instance = pwm->timer;
    htim.Init.Prescaler=prescaler-1;
	htim.Init.CounterMode=TIM_COUNTERMODE_UP;
	htim.Init.Period=resolution-1;
	htim.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
	htim.Init.RepetitionCounter=0;

    TIM_OC_InitTypeDef oc_config;
	oc_config.OCMode = TIM_OCMODE_PWM1;
	oc_config.Pulse=0;
	oc_config.OCPolarity = TIM_OCPOLARITY_HIGH;
	oc_config.OCNPolarity = TIM_OCNPOLARITY_LOW;
	oc_config.OCFastMode = TIM_OCFAST_ENABLE;
	oc_config.OCIdleState = TIM_OCIDLESTATE_SET;
	oc_config.OCNIdleState = TIM_OCNIDLESTATE_SET;

    HAL_TIM_PWM_Init(&htim);
    HAL_TIM_PWM_ConfigChannel(&htim, &oc_config, pwm->channel);

    GPIO_InitTypeDef pwm_pin_config;
    pwm_pin_config.Mode = GPIO_MODE_AF_PP;
    pwm_pin_config.Speed = GPIO_SPEED_HIGH;
    pwm_pin_config.Pull = GPIO_NOPULL;

#if FAMILY==STM32F4
    switch((uint32_t)pwm->timer)
    {
        case (uint32_t)TIM1:
        pwm_pin_config.Alternate = GPIO_AF1_TIM1;
        break;
        case (uint32_t)TIM2:
        pwm_pin_config.Alternate = GPIO_AF1_TIM2;
        break;
        case (uint32_t)TIM3:
        pwm_pin_config.Alternate = GPIO_AF2_TIM3;
        break;
        case (uint32_t)TIM4:
        pwm_pin_config.Alternate = GPIO_AF2_TIM4;
        break;
        case (uint32_t)TIM5:
        pwm_pin_config.Alternate = GPIO_AF2_TIM5;
        break;
        case (uint32_t)TIM8:
        pwm_pin_config.Alternate = GPIO_AF3_TIM8;
        break;
#ifdef TIM9
        case (uint32_t)TIM9:
        pwm_pin_config.Alternate = GPIO_AF3_TIM9;
        break;
#endif
#ifdef TIM10
        case (uint32_t)TIM10:
        pwm_pin_config.Alternate = GPIO_AF3_TIM10;
        break;
#endif
#ifdef TIM11
        case (uint32_t)TIM11:
        pwm_pin_config.Alternate = GPIO_AF3_TIM11;
        break;
#endif
    }
#endif

    switch((uint32_t)pwm->timer)
    {
        case (uint32_t)TIM1:
        	__TIM1_CLK_ENABLE();
        break;
        case (uint32_t)TIM2:
        	__TIM2_CLK_ENABLE();
        break;
        case (uint32_t)TIM3:
        	__TIM3_CLK_ENABLE();
        break;
        case (uint32_t)TIM4:
        	__TIM4_CLK_ENABLE();
        break;
        case (uint32_t)TIM5:
        	__TIM5_CLK_ENABLE();
        break;
        case (uint32_t)TIM8:
        	__TIM8_CLK_ENABLE();
        break;
#ifdef __TIM9_CLK_ENABLE
        case (uint32_t)TIM9:
        	__TIM9_CLK_ENABLE();
        break;
#endif
#ifdef __TIM10_CLK_ENABLE
        case (uint32_t)TIM10:
        	__TIM10_CLK_ENABLE();
        break;
#endif
#ifdef __TIM11_CLK_ENABLE
        case (uint32_t)TIM11:
        	__TIM11_CLK_ENABLE();
        break;
#endif
    }

    pwm_pin_config.Pin = pwm->pin;
    HAL_GPIO_Init(pwm->port, &pwm_pin_config);

    HAL_TIM_PWM_Start(&htim, pwm->channel);
}

void pwm_set_duty(pwm_t* pwm, uint16_t duty)
{
    TIM_HandleTypeDef htim;
    htim.Instance = pwm->timer;
    if(duty <= pwm->resolution){
    	__HAL_TIM_SET_COMPARE(&htim, pwm->channel, duty);
    }
    else {
        log_error(&pwm->log, "duty %d out of range (0-%d)", duty, pwm->resolution);
    }
}

/**
 * @}
 */
