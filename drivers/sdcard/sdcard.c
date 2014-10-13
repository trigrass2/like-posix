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
 * @addtogroup sdfs
 *
 * basic SD Card management, anything generic to all cards lives here.
 *
 * @file
 * @{
 */

#include "sdcard.h"

#if FAMILY == STM32F1

void SD_WP_Card_Detect_init(void)
{
#if defined(SD_CARD_PRES_PORT) || defined(SD_CARD_WP_PORT) || defined(SD_CARD_NPRES_PORT) || defined(SD_CARD_NWP_PORT)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
#endif

#ifdef SD_CARD_PRES_PORT
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_PRES_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(SD_CARD_PRES_PORT, &GPIO_InitStructure);
#endif

#ifdef SD_CARD_NPRES_PORT
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_NPRES_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(SD_CARD_NPRES_PORT, &GPIO_InitStructure);
#endif

#ifdef SD_CARD_WP_PORT
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_WP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(SD_CARD_WP_PORT, &GPIO_InitStructure);
#endif

#ifdef SD_CARD_NWP_PORT
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_NWP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(SD_CARD_NWP_PORT, &GPIO_InitStructure);
#endif
}

#elif FAMILY == STM32F4

void SD_WP_Card_Detect_init(void)
{
#if defined(SD_CARD_PRES_PORT) || defined(SD_CARD_WP_PORT) || defined(SD_CARD_NPRES_PORT) || defined(SD_CARD_NWP_PORT)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType =  GPIO_OType_OD;
#endif

#ifdef SD_CARD_PRES_PORT
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_PRES_PIN;
    GPIO_InitStructure.GPIO_PuPd =  GPIO_PuPd_DOWN;
    GPIO_Init(SD_CARD_PRES_PORT, &GPIO_InitStructure);
#endif

#ifdef SD_CARD_NPRES_PORT
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_NPRES_PIN;
    GPIO_InitStructure.GPIO_PuPd =  GPIO_PuPd_UP;
    GPIO_Init(SD_CARD_NPRES_PORT, &GPIO_InitStructure);
#endif

#ifdef SD_CARD_WP_PORT
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_WP_PIN;
    GPIO_InitStructure.GPIO_PuPd =  GPIO_PuPd_DOWN;
    GPIO_Init(SD_CARD_WP_PORT, &GPIO_InitStructure);
#endif

#ifdef SD_CARD_NWP_PORT
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_NWP_PIN;
    GPIO_InitStructure.GPIO_PuPd =  GPIO_PuPd_UP;
    GPIO_Init(SD_CARD_NWP_PORT, &GPIO_InitStructure);
#endif
}

#endif


uint8_t SD_Detect(void)
{
#if defined(SD_CARD_PRES_PORT)
    if (GPIO_ReadInputDataBit(SD_CARD_PRES_PORT, SD_CARD_PRES_PIN))
        return SD_PRESENT;
    return SD_NOT_PRESENT;
#elif defined(SD_CARD_NPRES_PORT)
    if (GPIO_ReadInputDataBit(SD_CARD_NPRES_PORT, SD_CARD_NPRES_PIN))
        return SD_NOT_PRESENT;
    return SD_PRESENT;
#else
    return SD_PRESENT;
#endif
}

static uint8_t _disk_status =  SD_NOT_PRESENT;

void set_diskstatus(uint8_t state)
{
	_disk_status = state;
}

uint8_t get_diskstatus()
{
	return _disk_status;
}

uint8_t SD_WPDetect(void)
{
#if defined(SD_CARD_WP_PORT)
   if(GPIO_ReadInputDataBit(SD_CARD_WP_PORT, SD_CARD_WP_PIN))
       return SD_WRITE_PROTECTED;
   return SD_NOT_WRITE_PROTECTED;
#elif defined(SD_CARD_NWP_PORT)
   if(GPIO_ReadInputDataBit(SD_CARD_NWP_PORT, SD_CARD_NWP_PIN))
       return SD_NOT_WRITE_PROTECTED;
   return SD_WRITE_PROTECTED;
#else
   return SD_NOT_WRITE_PROTECTED;
#endif
}

/**
 * @}
 */
