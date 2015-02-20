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
 * @addtogroup pwm PWM Driver
 *
 * @{
 */

#ifndef DRIVERS_PWM_PWM_H_
#define DRIVERS_PWM_PWM_H_

#include <stdint.h>
#include "board_config.h"
#include "cutensils.h"

#define PWM_MAX_CHANNELS 4

typedef struct {
    TIM_TypeDef* timer;          ///< timer used for each channel, eg TIM3. same timer may be used in multiple positions. set to NULL to terminate.
    uint16_t channel;            ///< timer output compare channels, TIM_Channel_x, where x may be 1-4
    GPIO_TypeDef* port;          ///< GPIO port for each channel, eg GPIOA
    uint16_t pinsource;          ///< GPIO pinsource for each port, eg GPIO_PinSource6
    uint16_t resolution;         ///< the resolution of the pwm channel, in counts. set by the driver.
    logger_t log;
}pwm_t;

void pwm_init(pwm_t* pwm, const char* name, uint32_t frequency, uint16_t resolution);
void pwm_set_duty(pwm_t* pwm, uint16_t duty);

#endif /* DRIVERS_PWM_PWM_H_ */

/**
 * @}
 */
