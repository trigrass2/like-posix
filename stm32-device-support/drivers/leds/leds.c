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

/**
 * @defgroup leds Led Driver
 *
 * Simple LED Driver.
 *
 * - suppoprts multiple LEDs
 * - supports an led flasher task :)
 *
 * relies upon a config file, led_config.h.
 *
 * Eg:
\code


#define LED_FLASHER_TASK_PRIORITY 2
#define LED_FLASHER_TASK_STACK	  0

#define LED1 	0
#define LED2 	1
#define LED3    2
#define LED3    3

// commonly used macros..
#define ERROR_LED LED3
#define BOOT_LED LED2

#define LED_PORT_MAP	{ \
	{GPIOE, GPIO_Pin_15}, \
    {GPIOE, GPIO_Pin_14}, \
	{GPIOE, GPIO_Pin_13}, \
	{GPIOE, GPIO_Pin_12}, \
}

\endcode
 *
 * @file leds.c
 * @{
 */

#include <unistd.h>
#include "leds.h"

#if USE_FREERTOS
static void led_flash_task(void *pvParameters);
static TaskHandle_t led_task = NULL;
#endif
static led_t leds[] = LED_PORT_MAP;
static uint8_t flashing_led = 0;

void init_leds()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;

    for(uint8_t i = 0; i < sizeof(leds)/sizeof(led_t); i++)
    {
        GPIO_InitStructure.Pin = leds[i].pin;
        HAL_GPIO_Init(leds[i].port, &GPIO_InitStructure);
        HAL_GPIO_WritePin(leds[i].port, leds[i].pin, GPIO_PIN_RESET);
    }
}

void flash_led(uint8_t led)
{
    flashing_led = led;

#if USE_FREERTOS
    if(led_task != NULL)
        vTaskDelete(led_task);
    xTaskCreate( led_flash_task,
                 "led_flasher",
                 configMINIMAL_STACK_SIZE+LED_FLASHER_TASK_STACK,
                 &flashing_led,
                 tskIDLE_PRIORITY+LED_FLASHER_TASK_PRIORITY,
                 &led_task);
#endif
}

void toggle_led(uint8_t led)
{
    HAL_GPIO_TogglePin(leds[led].port, leds[led].pin);
}

void set_led(uint8_t led)
{
    HAL_GPIO_WritePin(leds[led].port, leds[led].pin, GPIO_PIN_SET);
}

void clear_led(uint8_t led)
{
    HAL_GPIO_WritePin(leds[led].port, leds[led].pin, GPIO_PIN_RESET);
}

void switch_led(uint8_t led, bool state)
{
    HAL_GPIO_WritePin(leds[led].port, leds[led].pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

#if USE_FREERTOS
void led_flash_task(void *pvParameters)
{
    uint8_t led = *(uint8_t*)pvParameters;
    clear_led(led);
    for (;;)
    {
        usleep(1000000);
        toggle_led(led);
        usleep(60000);
        toggle_led(led);
    }
}
#endif

/**
 * @}
 */
