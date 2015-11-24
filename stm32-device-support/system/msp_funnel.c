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
 * This file is part of the Appleseed project, <https://github.com/drmetal/appleseed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "board_config.h"

#if USE_DRIVER_SYSTEM_TIMER
#include "systime.h"
#endif

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1)
		__HAL_RCC_TIM1_CLK_ENABLE();
	if(htim->Instance == TIM2)
		__HAL_RCC_TIM2_CLK_ENABLE();
	if(htim->Instance == TIM3)
		__HAL_RCC_TIM3_CLK_ENABLE();
	if(htim->Instance == TIM4)
		__HAL_RCC_TIM4_CLK_ENABLE();
	if(htim->Instance == TIM5)
		__HAL_RCC_TIM5_CLK_ENABLE();
	if(htim->Instance == TIM6)
		__HAL_RCC_TIM6_CLK_ENABLE();
	if(htim->Instance == TIM7)
		__HAL_RCC_TIM7_CLK_ENABLE();
	if(htim->Instance == TIM8)
		__HAL_RCC_TIM8_CLK_ENABLE();
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1)
		__HAL_RCC_TIM1_CLK_DISABLE();
	if(htim->Instance == TIM2)
		__HAL_RCC_TIM2_CLK_DISABLE();
	if(htim->Instance == TIM3)
		__HAL_RCC_TIM3_CLK_DISABLE();
	if(htim->Instance == TIM4)
		__HAL_RCC_TIM4_CLK_DISABLE();
	if(htim->Instance == TIM5)
		__HAL_RCC_TIM5_CLK_DISABLE();
	if(htim->Instance == TIM6)
		__HAL_RCC_TIM6_CLK_DISABLE();
	if(htim->Instance == TIM7)
		__HAL_RCC_TIM7_CLK_DISABLE();
	if(htim->Instance == TIM8)
		__HAL_RCC_TIM8_CLK_DISABLE();
}


