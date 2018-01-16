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

#include <string.h>
#include <stdint.h>
#include "adc_stream.h"

extern stream_t adc_stream;
extern stream_t adc_stream_buffer;


#if ADC_STREAM_SR_TIMER_UNIT != 0
static TIM_HandleTypeDef adc_stream_htim = {
    .Instance = ADC_STREAM_SR_TIMER,
    .Init = {
        .Prescaler = 0,
        .CounterMode = TIM_COUNTERMODE_UP,
        .Period = 0,
        .ClockDivision = TIM_CLOCKDIVISION_DIV1,
        .RepetitionCounter = 0
    },
    .Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED,
    .Lock = HAL_UNLOCKED,
    .State = HAL_TIM_STATE_RESET,
};
#endif

void adc_stream_set_samplerate(uint32_t samplerate)
{
#if ADC_STREAM_SR_TIMER_UNIT == 0
	(void)samplerate;
#else
	uint32_t clock = ADC_SR_TIMER_CLOCK_RATE;
#ifdef ADC_STREAM_SR_TIMER_OC_CHANNEL
	clock /= 2; // we use TIM_OCMODE_TOGGLE so got to halve the effective value
#endif
	stream_set_samplerate(&adc_stream, ADC_STREAM_SR_TIMER, clock, samplerate);
#endif
}

uint32_t adc_stream_get_samplerate()
{
#if ADC_STREAM_SR_TIMER_UNIT == 0
	return 0;
#else
    return stream_get_samplerate(&adc_stream);
#endif
}

void adc_stream_connect_service(stream_connection_t* interface, uint8_t stream_channel)
{
    stream_connect_service(interface, &adc_stream, stream_channel);
}

void adc_stream_init_local_io()
{
    uint8_t i;
    GPIO_TypeDef* stream_input_ports[ADC_STREAM_CHANNEL_COUNT] = ADC_STREAM_CHANNEL_PORTS;
    uint16_t stream_input_pins[ADC_STREAM_CHANNEL_COUNT] = ADC_STREAM_CHANNEL_PINS;
	GPIO_InitTypeDef gpio_input_init;
	gpio_input_init.Mode = GPIO_MODE_ANALOG;
	gpio_input_init.Pull = GPIO_NOPULL;
	gpio_input_init.Speed = GPIO_SPEED_LOW;
    // ADC IO pins
    for(i = 0; i < ADC_STREAM_CHANNEL_COUNT; i++)
    {
    	gpio_input_init.Pin = stream_input_pins[i];
    	HAL_GPIO_Init(stream_input_ports[i], &gpio_input_init);
    }
}

void adc_stream_init_samplerate_timer()
{
#if ADC_STREAM_SR_TIMER_UNIT != 0
    ADC_STREAM_SR_TIMER_CLOCK();

    adc_stream_htim.Init.Prescaler = ADC_SR_TIMER_PRESCALER-1;
    adc_stream_htim.Init.Period = (ADC_SR_TIMER_CLOCK_RATE / ADC_STREAM_DEFAULT_SAMPLERATE) - 1;

#ifdef ADC_STREAM_SR_TIMER_OC_CHANNEL
    HAL_StatusTypeDef ret = HAL_TIM_OC_Init(&adc_stream_htim);
	assert_true(ret == HAL_OK);
	TIM_OC_InitTypeDef sConfig;
    sConfig.OCMode = TIM_OCMODE_TOGGLE,
    sConfig.Pulse = 0;
    sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfig.OCFastMode = TIM_OCFAST_ENABLE;
    sConfig.OCIdleState = TIM_OCIDLESTATE_SET;
    sConfig.OCNIdleState = TIM_OCNIDLESTATE_SET;
    ret = HAL_TIM_OC_ConfigChannel(&adc_stream_htim, &sConfig, ADC_STREAM_SR_TIMER_OC_CHANNEL);
	assert_true(ret == HAL_OK);
#else
	HAL_StatusTypeDef ret = HAL_TIM_Base_Init(&adc_stream_htim);
	assert_true(ret == HAL_OK);
#endif

    TIM_MasterConfigTypeDef sMasterConfig;
    sMasterConfig.MasterOutputTrigger = ADC_STREAM_SR_TIMER_TRIGGER_OUT;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    ret = HAL_TIMEx_MasterConfigSynchronization(&adc_stream_htim, &sMasterConfig);
	assert_true(ret == HAL_OK);

    adc_stream_set_samplerate(ADC_STREAM_DEFAULT_SAMPLERATE);
#endif
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	assert_true(0);
}

void _adc_stream_start()
{
    adc_stream.buffer = NULL;
    //clear the buffer
    memset(adc_stream._buffer, ADC_STREAM_BUFFER_CLEAR_VALUE, sizeof(adc_stream_buffer));

#if ADC_STREAM_SR_TIMER_UNIT != 0
#ifdef ADC_STREAM_SR_TIMER_OC_CHANNEL
    HAL_StatusTypeDef ret = HAL_TIM_OC_Start(&adc_stream_htim, ADC_STREAM_SR_TIMER_OC_CHANNEL);
#else
    HAL_StatusTypeDef ret = HAL_TIM_Base_Start(&adc_stream_htim);
#endif
    assert_true(ret == HAL_OK);
#endif

    log_debug(&adc_stream.log, "started");
}

void _adc_stream_stop()
{
    adc_stream.buffer = NULL;

#if ADC_STREAM_SR_TIMER_UNIT != 0
#ifdef ADC_STREAM_SR_TIMER_OC_CHANNEL
    HAL_StatusTypeDef ret = HAL_TIM_OC_Stop(&adc_stream_htim, ADC_STREAM_SR_TIMER_OC_CHANNEL);
#else
    HAL_StatusTypeDef ret = HAL_TIM_Base_Stop(&adc_stream_htim);
#endif
    assert_true(ret == HAL_OK);
#endif

    log_debug(&adc_stream.log, "stream adc_stream stopped");
}
