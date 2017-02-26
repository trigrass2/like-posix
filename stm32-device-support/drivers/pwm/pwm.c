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
    .channel=TIM_Channel_1,
    .port=GPIOA,
    .pinsource=GPIO_PinSource6
};

pwm_t pwm2 = {
    .timer=TIM3,
    .channel=TIM_Channel_2,
    .port=GPIOA,
    .pinsource=GPIO_PinSource7
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
#if FAMILY == STM32F4
    uint8_t altfunc = 0;
#endif
    uint32_t apbclock;
    uint32_t prescaler;
    GPIO_InitTypeDef pwm_pin_config;
    RCC_ClocksTypeDef RCC_ClocksStatus;

    log_init(&pwm->log, name);

    RCC_GetClocksFreq(&RCC_ClocksStatus);
    if(pwm->timer == TIM1 || pwm->timer == TIM8)
        apbclock = RCC_ClocksStatus.PCLK2_Frequency;
    else
        apbclock = RCC_ClocksStatus.PCLK1_Frequency;

    prescaler = apbclock / (frequency * resolution);

    log_debug(&pwm->log, "freq=%d, res=%d presc=%d", frequency, resolution, prescaler);

    assert_true(frequency <= apbclock/2);
    assert_true(prescaler > 0);

    pwm->resolution = resolution;

    TIM_TimeBaseInitTypeDef timebase =
    {
            prescaler-1,                    // prescaler value
            TIM_CounterMode_Up,             // counter mode
            resolution-1,                             // period reload value
            TIM_CKD_DIV1,                   // clock divider value
            0                               //
    };

    TIM_OCInitTypeDef occonfig =
    {
            TIM_OCMode_PWM1,            // PWM mode
            TIM_OutputState_Enable,     // output state
            TIM_OutputState_Enable,     // complementary output state
            0,                          // capture compare pulse value
            TIM_OCPolarity_High,        // output polarity
            TIM_OCNPolarity_Low,        // complementary output polarity
            TIM_OCIdleState_Set,        // idle state
            TIM_OCIdleState_Set,        // complementary idle state
    };

    switch((uint32_t)pwm->timer)
    {
        case (uint32_t)TIM1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
#if FAMILY == STM32F4
            altfunc = GPIO_AF_TIM1;
#endif
        break;
        case (uint32_t)TIM2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
#if FAMILY == STM32F4
            altfunc = GPIO_AF_TIM2;
#endif
        break;
        case (uint32_t)TIM3:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
#if FAMILY == STM32F4
            altfunc = GPIO_AF_TIM3;
#endif
        break;
        case (uint32_t)TIM4:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
#if FAMILY == STM32F4
            altfunc = GPIO_AF_TIM4;
#endif
        break;
        case (uint32_t)TIM5:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
#if FAMILY == STM32F4
            altfunc = GPIO_AF_TIM5;
#endif
        break;
        case (uint32_t)TIM8:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
#if FAMILY == STM32F4
            altfunc = GPIO_AF_TIM8;
#endif
        break;
    }

    TIM_TimeBaseInit(pwm->timer, &timebase);

    switch(pwm->channel)
    {
        case TIM_Channel_1:
            TIM_OC1Init(pwm->timer, &occonfig);
            TIM_OC1PreloadConfig(pwm->timer, TIM_OCPreload_Enable);
        break;
        case TIM_Channel_2:
            TIM_OC2Init(pwm->timer, &occonfig);
            TIM_OC2PreloadConfig(pwm->timer, TIM_OCPreload_Enable);
        break;
        case TIM_Channel_3:
            TIM_OC3Init(pwm->timer, &occonfig);
            TIM_OC3PreloadConfig(pwm->timer, TIM_OCPreload_Enable);
        break;
        case TIM_Channel_4:
            TIM_OC4Init(pwm->timer, &occonfig);
            TIM_OC4PreloadConfig(pwm->timer, TIM_OCPreload_Enable);
        break;
    }

    TIM_ARRPreloadConfig(pwm->timer, ENABLE);
    TIM_Cmd(pwm->timer, ENABLE);

    pwm_pin_config.GPIO_Speed = GPIO_Speed_2MHz;
    pwm_pin_config.GPIO_Pin = (1 << pwm->pinsource);
#if FAMILY == STM32F1
    pwm_pin_config.GPIO_Mode = GPIO_Mode_AF_PP;
#elif FAMILY == STM32F4
    assert_true(altfunc);
    pwm_pin_config.GPIO_Mode = GPIO_Mode_AF;
    pwm_pin_config.GPIO_OType = GPIO_OType_PP;
    pwm_pin_config.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_PinAFConfig(pwm->port, pwm->pinsource, altfunc);
#endif
    GPIO_Init(pwm->port, &pwm_pin_config);
}

void pwm_set_duty(pwm_t* pwm, uint16_t duty)
{
    if(duty <= pwm->resolution)
    {
        switch(pwm->channel)
        {
            case TIM_Channel_1:
                TIM_SetCompare1(pwm->timer, duty);
            break;
            case TIM_Channel_2:
                TIM_SetCompare2(pwm->timer, duty);
            break;
            case TIM_Channel_3:
                TIM_SetCompare3(pwm->timer, duty);
            break;
            case TIM_Channel_4:
                TIM_SetCompare4(pwm->timer, duty);
            break;
        }
        log_debug(&pwm->log, "duty=%d", duty);
    }
    else
        log_error(&pwm->log, "duty %d out of range (0-%d)", duty, pwm->resolution);
}

/**
 * @}
 */
