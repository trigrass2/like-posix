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
 * SD Card SDIO driver.
 *
 * This is a heavily modified version of the STM32 SD Card drivers for STM32F1/F4,
 * merged and modified for brevity and slight performance improvement.
 *
 * - supports DMA mode only
 * - does use interrupts, but only to set flags used for polled checking at this time
 * - supports 512 byte blocksize only
 * - SDC V1, MMC card support untested
 * - SDC V2.x, SDHC tested
 * - loosely emulates the original API by ST, with some modifications
 *
 * @file
 * @{
 */

#include <unistd.h>
#include <sys/time.h>
#include "sdcard.h"
#include "sdio_cmd.h"
#include "sdio_hw_defs.h"
#include "cutensils.h"

#if USE_THREAD_AWARE_SDIO
#pragma message("using thread aware SDIO")
#else
#pragma message("using polled SDIO")
#endif

/**
 * @defgroup sd_card_sdio SD Card SDIO
 *
 * This file provides all the SD Card driver firmware functions.
 *
 * @{
 */

/**
  * @brief  SDIO data timeout
  */
#define SDIO_DATATIMEOUT                  ((uint32_t)0x000FFFFF*32)

/**
  * @brief  SDIO data timeout
  */
#define SDIO_WAITTIMEOUT                500

/**
  * @brief  SDIO Static flags, TimeOut, FIFO Address
  */

#define SDIO_STATIC_FLAGS               ((uint32_t)0x000005FF)
#define SDIO_CMD0TIMEOUT                500

#define SD_ALLZERO                      ((uint32_t)0x00000000)
#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO                     ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES                ((uint32_t)0x00000020)

#if USE_LOGGER
static logger_t sdiolog;
#endif

typedef struct {
    bool dma_xfer_end;
    SD_Error sdio_xfer_error;
    bool sdio_xfer_multi_block;
    uint8_t card_type;
    uint8_t rca;
    uint32_t csd_table[4];
    uint32_t cid_table[4];
}sdio_state_t;

volatile sdio_state_t sdio_state = {
    .dma_xfer_end = false,
    .sdio_xfer_error = SD_ACTIVE,
    .sdio_xfer_multi_block = false,
    .card_type = SDIO_UNKNOWN_CARD_TYPE,
    .rca = 0,
};

SD_Error SD_InitializeCards(void);
SD_Error SD_EnableWideBusOperation(uint32_t WideMode);
SD_Error SD_SelectDeselect(uint32_t addr);
SD_Error SD_StopTransfer(void);


static void SD_IO_DeInit(void);
static void SD_IO_Init(void);

static void SD_LowLevel_DMA_TxConfig(uint32_t *BufferSRC, uint32_t BufferSize);
static void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST, uint32_t BufferSize);
static void SD_NVIC_Configuration(void);

static SD_Error CmdError(void);
static SD_Error CmdResp1Error(uint8_t cmd);
static SD_Error CmdResp7Error(void);
static SD_Error CmdResp3Error(void);
static SD_Error CmdResp2Error(void);
static SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca);
static SD_Error SDEnWideBus(FunctionalState NewState);
static SD_Error IsCardProgramming(uint8_t *pstatus);


/**
 * SDIO_CmdInitStructure.SDIO_Wait,
 * SDIO_CmdInitStructure.SDIO_CPSM
 *
 * are not to be modified by the driver.
 */
static volatile SDIO_CmdInitTypeDef SDIO_CmdInitStructure = {
    .SDIO_Wait = SDIO_Wait_No,
    .SDIO_CPSM = SDIO_CPSM_Enable,
};

/**
 * gets a timestamp in ms
 */
static inline uint32_t gettime_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (1000 * tv.tv_sec) + (tv.tv_usec/1000);
}

/**
 * send a command to the sdcard - the response must be read differently
 * depending on the context of the command.
 */
static inline void sdio_send_cmd(uint32_t arg, uint32_t cmd, uint32_t resp)
{
    SDIO_CmdInitStructure.SDIO_Argument = arg;
    SDIO_CmdInitStructure.SDIO_CmdIndex = cmd;
    SDIO_CmdInitStructure.SDIO_Response = resp;
    SDIO_SendCommand((SDIO_CmdInitTypeDef*)&SDIO_CmdInitStructure);
}

/**
 * SDIO_DataInitStructure.SDIO_DataTimeOut,
 * SDIO_DataInitStructure.SDIO_TransferMode,
 * SDIO_DataInitStructure.SDIO_DPSM,
 * SDIO_DataInitStructure.SDIO_DataBlockSize
 *
 * are not to be modified by the driver.
 */
static volatile SDIO_DataInitTypeDef SDIO_DataInitStructure = {
    .SDIO_DataTimeOut = SDIO_DATATIMEOUT,
    .SDIO_TransferMode = SDIO_TransferMode_Block,
    .SDIO_DPSM = SDIO_DPSM_Enable,
    .SDIO_DataBlockSize = (uint32_t)(9 << 4), // 9<<4 is special, sets up for 512Bytes/block
};

/**
 * initialise a data transfer.
 */
static inline void sdio_init_data(uint32_t length, uint32_t direction)
{
    SDIO_DataInitStructure.SDIO_DataLength = length;
    SDIO_DataInitStructure.SDIO_TransferDir = direction;
    SDIO_DataConfig((SDIO_DataInitTypeDef*)&SDIO_DataInitStructure);
}

/**
 * initialise the SDIO peripheral.
 */
static void sdio_init_peripheral(uint8_t clock_div, uint32_t bus_width)
{
    SDIO_InitTypeDef SDIO_InitStructure = {
        .SDIO_ClockDiv = clock_div,
        .SDIO_ClockEdge = SDIO_ClockEdge_Rising,
        .SDIO_ClockBypass = SDIO_ClockBypass_Disable,
        .SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable,
        .SDIO_BusWide = bus_width,
        .SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable,
    };
    SDIO_Init(&SDIO_InitStructure);
}

/**
  * @brief  Initializes the SD Card and put it into StandBy State (Ready for data transfer).
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_Init(SD_CardInfo* sdcardinfo)
{
    SD_Error errorstatus;

    log_init(&sdiolog, "sdio");

    SD_NVIC_Configuration();
    SD_IO_Init();
    errorstatus = SD_PowerON();

    if(errorstatus == SD_OK)
    {
        errorstatus = SD_InitializeCards();

        if(errorstatus == SD_OK)
        {
            /*!< Configure the SDIO peripheral */
            /*!< SDIOCLK = HCLK, SDIO_CK = HCLK/(2 + SDIO_TRANSFER_CLK_DIV) */
            sdio_init_peripheral(SDIO_TRANSFER_CLK_DIV, SDIO_BusWide_1b);

            // Read CSD/CID MSD registers
            errorstatus = SD_GetCardInfo(sdcardinfo);

            // select card
            if(errorstatus == SD_OK)
                errorstatus = SD_SelectDeselect((uint32_t) (sdcardinfo->RCA << 16));

            // 4bit bus mode
            if(errorstatus == SD_OK)
                errorstatus = SD_EnableWideBusOperation(SDIO_BusWide_4b);

            // Set Block Size for Card
            sdio_send_cmd((uint32_t)SD_SECTOR_SIZE, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short);
            errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
        }
    }
    return errorstatus;
}

#if FAMILY == STM32F1

/**
  * @brief  Initializes the SDIO interface.
  */
static void SD_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    SD_WP_Card_Detect_init();

    GPIO_InitStructure.GPIO_Pin =  SD_CARD_CK_PIN | SD_CARD_D0_PIN | SD_CARD_D1_PIN | SD_CARD_D2_PIN | SD_CARD_D3_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(SD_CARD_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  SD_CARD_CMD_PIN;
    GPIO_Init(SD_CARD_CMD_PORT, &GPIO_InitStructure);

    /*!< Enable the SDIO AHB Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, ENABLE);
    /*!< Enable the DMA2 Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
}

/**
  * @brief  DeInitializes the SDIO interface.
  */
static void SD_IO_DeInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // deinit the SDIO peripheral
    SDIO_ClockCmd(DISABLE);
    SDIO_SetPowerState(SDIO_PowerState_OFF);
    SDIO_DeInit();
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, DISABLE);
    // deinit the SDIO gpio pins
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_CK_PIN | SD_CARD_D0_PIN | SD_CARD_D1_PIN | SD_CARD_D2_PIN | SD_CARD_D3_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(SD_CARD_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_CMD_PIN;
    GPIO_Init(SD_CARD_CMD_PORT, &GPIO_InitStructure);
}

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Tx request.
  * @param  BufferSRC: pointer to the source buffer
  * @param  BufferSize: buffer size
  */
static void SD_LowLevel_DMA_TxConfig(uint32_t *BufferSRC, uint32_t BufferSize)
{
    DMA_InitTypeDef DMA_InitStructure;

    DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);

    DMA_Cmd(DMA2_Channel4, DISABLE);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_ADDRESS;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)BufferSRC;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = BufferSize / 4;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel4, &DMA_InitStructure);
    DMA_ITConfig(DMA2_Channel4, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA2_Channel4, ENABLE);
}

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
  * @param  BufferDST: pointer to the destination buffer
  * @param  BufferSize: buffer size
  */
static void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST, uint32_t BufferSize)
{
    DMA_InitTypeDef DMA_InitStructure;

    DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);

    DMA_Cmd(DMA2_Channel4, DISABLE);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_ADDRESS;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)BufferDST;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = BufferSize / 4;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel4, &DMA_InitStructure);
    DMA_ITConfig(DMA2_Channel4, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA2_Channel4, ENABLE);
}

void SD_SDIO_DMA_IRQHANDLER(void)
{
    if(DMA_GetITStatus(DMA2_IT_TC4) == SET)
    {
        sdio_state.dma_xfer_end = true;
        DMA_ClearITPendingBit(DMA2_IT_TC4);
    }
}

#elif FAMILY == STM32F4
/**
  * @brief  Initializes the SDIO interface.
  */
static void SD_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    SD_WP_Card_Detect_init();

    GPIO_InitStructure.GPIO_Pin =  SD_CARD_CK_PIN | SD_CARD_D0_PIN | SD_CARD_D1_PIN | SD_CARD_D2_PIN | SD_CARD_D3_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_Init(SD_CARD_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  SD_CARD_CMD_PIN;
    GPIO_Init(SD_CARD_CMD_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(SD_CARD_PORT, SD_CARD_CK_PINSOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SD_CARD_PORT, SD_CARD_D0_PINSOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SD_CARD_PORT, SD_CARD_D1_PINSOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SD_CARD_PORT, SD_CARD_D2_PINSOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SD_CARD_PORT, SD_CARD_D3_PINSOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SD_CARD_CMD_PORT, SD_CARD_CMD_PINSOURCE, GPIO_AF_SDIO);

    /* Enable the SDIO APB2 Clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE);
    /* Enable the DMA2 Clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
}

/**
  * @brief  DeInitializes the SDIO interface.
  */
static void SD_IO_DeInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // deinit the SDIO peripheral
    SDIO_ClockCmd(DISABLE);
    SDIO_SetPowerState(SDIO_PowerState_OFF);
    SDIO_DeInit();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, DISABLE);
    // deinit the SDIO gpio pins
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_CK_PIN | SD_CARD_D0_PIN | SD_CARD_D1_PIN | SD_CARD_D2_PIN | SD_CARD_D3_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(SD_CARD_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin =  SD_CARD_CMD_PIN;
    GPIO_Init(SD_CARD_CMD_PORT, &GPIO_InitStructure);
}

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Tx request.
  * @param  BufferSRC: pointer to the source buffer
  * @param  BufferSize: buffer size
  */
static void SD_LowLevel_DMA_TxConfig(uint32_t *BufferSRC, uint32_t BufferSize)
{
    DMA_InitTypeDef SDDMA_InitStructure;

    DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF | SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);

    /* DMA2 Stream3  or Stream6 disable */
    DMA_Cmd(SD_SDIO_DMA_STREAM, DISABLE);

    /* DMA2 Stream3  or Stream6 Config */
    DMA_DeInit(SD_SDIO_DMA_STREAM);

    SDDMA_InitStructure.DMA_Channel = SD_SDIO_DMA_CHANNEL;
    SDDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_ADDRESS;
    SDDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
    SDDMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    SDDMA_InitStructure.DMA_BufferSize = BufferSize / 4;
    SDDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    SDDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    SDDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    SDDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    SDDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    SDDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    SDDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    SDDMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    SDDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
    SDDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
    DMA_Init(SD_SDIO_DMA_STREAM, &SDDMA_InitStructure);
    DMA_ITConfig(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
    DMA_FlowControllerConfig(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);

    /* DMA2 Stream3  or Stream6 enable */
    DMA_Cmd(SD_SDIO_DMA_STREAM, ENABLE);
}

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
  * @param  BufferDST: pointer to the destination buffer
  * @param  BufferSize: buffer size
  */
void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST, uint32_t BufferSize)
{
    DMA_InitTypeDef SDDMA_InitStructure;

    DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF | SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);

    /* DMA2 Stream3  or Stream6 disable */
    DMA_Cmd(SD_SDIO_DMA_STREAM, DISABLE);

    /* DMA2 Stream3 or Stream6 Config */
    DMA_DeInit(SD_SDIO_DMA_STREAM);

    SDDMA_InitStructure.DMA_Channel = SD_SDIO_DMA_CHANNEL;
    SDDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_ADDRESS;
    SDDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferDST;
    SDDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    SDDMA_InitStructure.DMA_BufferSize = BufferSize / 4;
    SDDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    SDDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    SDDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    SDDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    SDDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    SDDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    SDDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    SDDMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    SDDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
    SDDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
    DMA_Init(SD_SDIO_DMA_STREAM, &SDDMA_InitStructure);
    DMA_ITConfig(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
    DMA_FlowControllerConfig(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);

    /* DMA2 Stream3 or Stream6 enable */
    DMA_Cmd(SD_SDIO_DMA_STREAM, ENABLE);
}

void SD_SDIO_DMA_IRQHANDLER(void)
{
    if(DMA2->LISR & SD_SDIO_DMA_FLAG_TCIF)
    {
        sdio_state.dma_xfer_end = true;
        DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TCIF|SD_SDIO_DMA_FLAG_FEIF);
    }
}

#endif

/**
  * @brief  Configures SDIO IRQ channel.
  */
static void SD_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SDCARD_IT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = SD_SDIO_DMA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SDCARD_IT_PRIORITY+1;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  DeInitializes the SDIO interface.
  */
void SD_DeInit(void)
{
    SD_IO_DeInit();
}

/**
  * @brief  Returns the current card's status.
  * @param  pcardstatus: pointer to the buffer that will contain the SD card
  *         status (Card Status register).
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_QueryStatus(SDCardState* cardstatus)
{
    SD_Error errorstatus;

    sdio_send_cmd((uint32_t)sdio_state.rca << 16, SD_CMD_SEND_STATUS, SDIO_Response_Short);
    errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);

    if(errorstatus == SD_OK)
        *cardstatus = (SDIO_GetResponse(SDIO_RESP1) >> 9)&0x0F;
    else
        *cardstatus = SD_CARD_ERROR;

    return errorstatus;
}

/**
  * @brief  Enquires cards about their operating voltage and configures
  *   clock controls.
  *
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_PowerON(void)
{
    SD_Error errorstatus = SD_OK;
    uint32_t resp;
    uint32_t acmdarg;
    uint32_t timer = 0;
    uint32_t wait = 10000; // microseconds

    /*!< Power ON Sequence -----------------------------------------------------*/
    /*!< Configure the SDIO peripheral */
    /*!< SDIOCLK = HCLK, SDIO_CK = HCLK/(2 + SDIO_INIT_CLK_DIV) */
    /*!< SDIO_CK for initialization should not exceed 400 KHz */
    sdio_init_peripheral(SDIO_INIT_CLK_DIV, SDIO_BusWide_1b);

    /*!< Set Power State to ON */
    SDIO_SetPowerState(SDIO_PowerState_ON);

    /*!< Enable SDIO Clock */
    SDIO_ClockCmd(ENABLE);

    /*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
    /*!< No CMD response required */
    sdio_send_cmd(0x0, SD_CMD_GO_IDLE_STATE, SDIO_Response_No);
    errorstatus = CmdError();

    if(errorstatus != SD_OK)
        return errorstatus;

    /*!< CMD8: SEND_IF_COND ----------------------------------------------------*/
    /*!< Send CMD8 to verify SD card interface operating condition */
    /*!< Argument: - [31:12]: Reserved (shall be set to '0')
           - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
           - [7:0]: Check Pattern (recommended 0xAA) */
    /*!< CMD Response: R7 */
    sdio_send_cmd(SD_CHECK_PATTERN, SDIO_SEND_IF_COND, SDIO_Response_Short);
    errorstatus = CmdResp7Error();

    log_syslog(&sdiolog, "cmd8: err=%d", errorstatus);

    if(errorstatus == SD_OK)
    {
        sdio_state.card_type = SDIO_STD_CAPACITY_SD_CARD_V2_0;
        acmdarg = SD_VOLTAGE_WINDOW_SD | SD_HIGH_CAPACITY;
    }
    else
    {
        sdio_state.card_type = SDIO_STD_CAPACITY_SD_CARD_V1_1;
        acmdarg = SD_VOLTAGE_WINDOW_SD;
    }

    while(timer < 2000000)
    {
        sdio_send_cmd(0x00, SD_CMD_APP_CMD, SDIO_Response_Short);
        CmdResp1Error(SD_CMD_APP_CMD);
        sdio_send_cmd(acmdarg, SD_CMD_SD_APP_OP_COND, SDIO_Response_Short);
        errorstatus = CmdResp3Error();
        resp = SDIO_GetResponse(SDIO_RESP1);
        log_syslog(&sdiolog, "cmd41: err=%d, resp=%#08x", errorstatus, resp);
        if(resp & 0x80000000)
        {
            errorstatus = SD_OK;
            break;
        }
        else
            errorstatus = SD_INVALID_VOLTRANGE;
        usleep(wait);
        timer += wait;
        wait *= 2;
    }

    if(resp & SD_HIGH_CAPACITY)
        sdio_state.card_type = SDIO_HIGH_CAPACITY_SD_CARD;

  return errorstatus;
}

/**
  * @brief  Turns the SDIO output signals off.
  *
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_PowerOFF(void)
{
    SDIO_SetPowerState(SDIO_PowerState_OFF);
    return SD_OK;
}

/**
  * @brief  Intialises all cards or single card as the case may be Card(s) come
  *         into standby state.
  *
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_InitializeCards(void)
{
    SD_Error errorstatus = SD_OK;
    uint16_t rca = 0x01;

    if(SDIO_GetPowerState() == SDIO_PowerState_OFF)
        return SD_REQUEST_NOT_APPLICABLE;

    if(SDIO_SECURE_DIGITAL_IO_CARD != sdio_state.card_type)
    {
        /*!< Send CMD2 ALL_SEND_CID */
        sdio_send_cmd(0x0, SD_CMD_ALL_SEND_CID, SDIO_Response_Long);
        errorstatus = CmdResp2Error();

        if(SD_OK != errorstatus)
            return errorstatus;

        sdio_state.cid_table[0] = SDIO_GetResponse(SDIO_RESP1);
        sdio_state.cid_table[1] = SDIO_GetResponse(SDIO_RESP2);
        sdio_state.cid_table[2] = SDIO_GetResponse(SDIO_RESP3);
        sdio_state.cid_table[3] = SDIO_GetResponse(SDIO_RESP4);
    }
    if( (SDIO_STD_CAPACITY_SD_CARD_V1_1 == sdio_state.card_type) ||
        (SDIO_STD_CAPACITY_SD_CARD_V2_0 == sdio_state.card_type) ||
        (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == sdio_state.card_type) ||
        (SDIO_HIGH_CAPACITY_SD_CARD == sdio_state.card_type))
    {
        /*!< Send CMD3 SET_REL_ADDR with argument 0 */
        /*!< SD Card publishes its RCA. */
        sdio_send_cmd(0x00, SD_CMD_SET_REL_ADDR, SDIO_Response_Short);
        errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);

        if(SD_OK != errorstatus)
            return errorstatus;
    }

    if(SDIO_SECURE_DIGITAL_IO_CARD != sdio_state.card_type)
    {
        sdio_state.rca = rca;

        /*!< Send CMD9 SEND_CSD with argument as card's RCA */
        sdio_send_cmd((uint32_t)(rca << 16), SD_CMD_SEND_CSD, SDIO_Response_Long);
        errorstatus = CmdResp2Error();

        if(SD_OK != errorstatus)
            return errorstatus;

        sdio_state.csd_table[0] = SDIO_GetResponse(SDIO_RESP1);
        sdio_state.csd_table[1] = SDIO_GetResponse(SDIO_RESP2);
        sdio_state.csd_table[2] = SDIO_GetResponse(SDIO_RESP3);
        sdio_state.csd_table[3] = SDIO_GetResponse(SDIO_RESP4);
    }

    return errorstatus;
}

/**
  * @brief  Returns information about specific card.
  * @param  cardinfo: pointer to a SD_CardInfo structure that contains all SD card
  *         information.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
    SD_Error errorstatus = SD_OK;
    uint8_t tmp = 0;

    cardinfo->CardType = (uint8_t)sdio_state.card_type;
    cardinfo->RCA = (uint16_t)sdio_state.rca;

    /*!< Byte 0 */
    tmp = (uint8_t)((sdio_state.csd_table[0] & 0xFF000000) >> 24);
    cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;
    cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;
    cardinfo->SD_csd.Reserved1 = tmp & 0x03;

    /*!< Byte 1 */
    tmp = (uint8_t)((sdio_state.csd_table[0] & 0x00FF0000) >> 16);
    cardinfo->SD_csd.TAAC = tmp;

    /*!< Byte 2 */
    tmp = (uint8_t)((sdio_state.csd_table[0] & 0x0000FF00) >> 8);
    cardinfo->SD_csd.NSAC = tmp;

    /*!< Byte 3 */
    tmp = (uint8_t)(sdio_state.csd_table[0] & 0x000000FF);
    cardinfo->SD_csd.MaxBusClkFrec = tmp;

    /*!< Byte 4 */
    tmp = (uint8_t)((sdio_state.csd_table[1] & 0xFF000000) >> 24);
    cardinfo->SD_csd.CardComdClasses = tmp << 4;

    /*!< Byte 5 */
    tmp = (uint8_t)((sdio_state.csd_table[1] & 0x00FF0000) >> 16);
    cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;
    cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;

    /*!< Byte 6 */
    tmp = (uint8_t)((sdio_state.csd_table[1] & 0x0000FF00) >> 8);
    cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;
    cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;
    cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
    cardinfo->SD_csd.Reserved2 = 0; /*!< Reserved */

    if((sdio_state.card_type == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (sdio_state.card_type == SDIO_STD_CAPACITY_SD_CARD_V2_0))
    {
        cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;

        /*!< Byte 7 */
        tmp = (uint8_t)(sdio_state.csd_table[1] & 0x000000FF);
        cardinfo->SD_csd.DeviceSize |= (tmp) << 2;

        /*!< Byte 8 */
        tmp = (uint8_t)((sdio_state.csd_table[2] & 0xFF000000) >> 24);
        cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;

        cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
        cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);

        /*!< Byte 9 */
        tmp = (uint8_t)((sdio_state.csd_table[2] & 0x00FF0000) >> 16);
        cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
        cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
        cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
        /*!< Byte 10 */
        tmp = (uint8_t)((sdio_state.csd_table[2] & 0x0000FF00) >> 8);
        cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;

        // card capacity in blocks
        cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) ;
        cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
        /**
        * from wikipedia:
        * Later versions state (at Section 4.3.2) that a 2 GB SDSC card shall set its READ_BL_LEN (and WRITE_BL_LEN) to indicate 1024 bytes,
        * so that the above computation correctly reports the card's capacity; but that, for consistency,
        * the host device shall not request (by CMD16) block lengths over 512 bytes
        */
        if(sdio_state.card_type == SDIO_STD_CAPACITY_SD_CARD_V2_0)
            cardinfo->CardCapacity *= 2;

        cardinfo->CardBlockSize = SD_SECTOR_SIZE;
    }
    else if(sdio_state.card_type == SDIO_HIGH_CAPACITY_SD_CARD)
    {
        /*!< Byte 7 */
        tmp = (uint8_t)(sdio_state.csd_table[1] & 0x000000FF);
        cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16;

        /*!< Byte 8 */
        tmp = (uint8_t)((sdio_state.csd_table[2] & 0xFF000000) >> 24);

        cardinfo->SD_csd.DeviceSize |= (tmp << 8);

        /*!< Byte 9 */
        tmp = (uint8_t)((sdio_state.csd_table[2] & 0x00FF0000) >> 16);

        cardinfo->SD_csd.DeviceSize |= (tmp);

        /*!< Byte 10 */
        tmp = (uint8_t)((sdio_state.csd_table[2] & 0x0000FF00) >> 8);

        // card capacity in blocks
        cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) * 1024;
        cardinfo->CardBlockSize = SD_SECTOR_SIZE;
    }

    cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
    cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;

    /*!< Byte 11 */
    tmp = (uint8_t)(sdio_state.csd_table[2] & 0x000000FF);
    cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
    cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);

    /*!< Byte 12 */
    tmp = (uint8_t)((sdio_state.csd_table[3] & 0xFF000000) >> 24);
    cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
    cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
    cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
    cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;

    /*!< Byte 13 */
    tmp = (uint8_t)((sdio_state.csd_table[3] & 0x00FF0000) >> 16);
    cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
    cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.Reserved3 = 0;
    cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);

    /*!< Byte 14 */
    tmp = (uint8_t)((sdio_state.csd_table[3] & 0x0000FF00) >> 8);
    cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
    cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
    cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
    cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
    cardinfo->SD_csd.ECC = (tmp & 0x03);

    /*!< Byte 15 */
    tmp = (uint8_t)(sdio_state.csd_table[3] & 0x000000FF);
    cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
    cardinfo->SD_csd.Reserved4 = 1;


    /*!< Byte 0 */
    tmp = (uint8_t)((sdio_state.cid_table[0] & 0xFF000000) >> 24);
    cardinfo->SD_cid.ManufacturerID = tmp;

    /*!< Byte 1 */
    tmp = (uint8_t)((sdio_state.cid_table[0] & 0x00FF0000) >> 16);
    cardinfo->SD_cid.OEM_AppliID = tmp << 8;

    /*!< Byte 2 */
    tmp = (uint8_t)((sdio_state.cid_table[0] & 0x000000FF00) >> 8);
    cardinfo->SD_cid.OEM_AppliID |= tmp;

    /*!< Byte 3 */
    tmp = (uint8_t)(sdio_state.cid_table[0] & 0x000000FF);
    cardinfo->SD_cid.ProdName1 = tmp << 24;

    /*!< Byte 4 */
    tmp = (uint8_t)((sdio_state.cid_table[1] & 0xFF000000) >> 24);
    cardinfo->SD_cid.ProdName1 |= tmp << 16;

    /*!< Byte 5 */
    tmp = (uint8_t)((sdio_state.cid_table[1] & 0x00FF0000) >> 16);
    cardinfo->SD_cid.ProdName1 |= tmp << 8;

    /*!< Byte 6 */
    tmp = (uint8_t)((sdio_state.cid_table[1] & 0x0000FF00) >> 8);
    cardinfo->SD_cid.ProdName1 |= tmp;

    /*!< Byte 7 */
    tmp = (uint8_t)(sdio_state.cid_table[1] & 0x000000FF);
    cardinfo->SD_cid.ProdName2 = tmp;

    /*!< Byte 8 */
    tmp = (uint8_t)((sdio_state.cid_table[2] & 0xFF000000) >> 24);
    cardinfo->SD_cid.ProdRev = tmp;

    /*!< Byte 9 */
    tmp = (uint8_t)((sdio_state.cid_table[2] & 0x00FF0000) >> 16);
    cardinfo->SD_cid.ProdSN = tmp << 24;

    /*!< Byte 10 */
    tmp = (uint8_t)((sdio_state.cid_table[2] & 0x0000FF00) >> 8);
    cardinfo->SD_cid.ProdSN |= tmp << 16;

    /*!< Byte 11 */
    tmp = (uint8_t)(sdio_state.cid_table[2] & 0x000000FF);
    cardinfo->SD_cid.ProdSN |= tmp << 8;

    /*!< Byte 12 */
    tmp = (uint8_t)((sdio_state.cid_table[3] & 0xFF000000) >> 24);
    cardinfo->SD_cid.ProdSN |= tmp;

    /*!< Byte 13 */
    tmp = (uint8_t)((sdio_state.cid_table[3] & 0x00FF0000) >> 16);
    cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
    cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;

    /*!< Byte 14 */
    tmp = (uint8_t)((sdio_state.cid_table[3] & 0x0000FF00) >> 8);
    cardinfo->SD_cid.ManufactDate |= tmp;

    /*!< Byte 15 */
    tmp = (uint8_t)(sdio_state.cid_table[3] & 0x000000FF);
    cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
    cardinfo->SD_cid.Reserved2 = 1;

    return errorstatus;
}

/**
  * @brief  Enables wide bus opeartion for the requeseted card if supported by
  *         card.
  * @param  WideMode: Specifies the SD card wide bus mode.
  *   This parameter can be one of the following values:
  *     @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
  *     @arg SDIO_BusWide_4b: 4-bit data transfer
  *     @arg SDIO_BusWide_1b: 1-bit data transfer
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_EnableWideBusOperation(uint32_t WideMode)
{
    SD_Error errorstatus = SD_OK;

    /*!< MMC Card doesn't support this feature */
    if(SDIO_MULTIMEDIA_CARD == sdio_state.card_type)
        return SD_UNSUPPORTED_FEATURE;
    else if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == sdio_state.card_type) ||
            (SDIO_STD_CAPACITY_SD_CARD_V2_0 == sdio_state.card_type) ||
            (SDIO_HIGH_CAPACITY_SD_CARD == sdio_state.card_type))
    {
        if(SDIO_BusWide_8b == WideMode)
            return SD_UNSUPPORTED_FEATURE;
        else if(SDIO_BusWide_4b == WideMode)
        {
            errorstatus = SDEnWideBus(ENABLE);
            if(SD_OK == errorstatus)
                sdio_init_peripheral(SDIO_TRANSFER_CLK_DIV, SDIO_BusWide_4b);
        }
        else
        {
            errorstatus = SDEnWideBus(DISABLE);
            if(SD_OK == errorstatus)
                sdio_init_peripheral(SDIO_TRANSFER_CLK_DIV, SDIO_BusWide_1b);
        }
    }

    return errorstatus;
}

/**
  * @brief  Selects od Deselects the corresponding card.
  * @param  addr: Address of the Card to be selected.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_SelectDeselect(uint32_t addr)
{
    SD_Error errorstatus = SD_OK;
    sdio_send_cmd(addr, SD_CMD_SEL_DESEL_CARD, SDIO_Response_Short);
    errorstatus = CmdResp1Error(SD_CMD_SEL_DESEL_CARD);
    return errorstatus;
}


/**
  * @brief  Allows to read one block from a specified address in a card. The Data
  *         transfer can be managed by DMA mode or Polling mode. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.            
  * @param  readbuff: pointer to the buffer that will contain the received data
  * @param  sector: Address from where data are to be read.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_ReadBlock(uint8_t *readbuff, uint32_t sector)
{
    SD_Error errorstatus = SD_OK;

    if(!readbuff)
        return SD_INVALID_PARAMETER;

    sdio_state.sdio_xfer_error = SD_ACTIVE;
    sdio_state.sdio_xfer_multi_block = false;
    SDIO->DCTRL = 0x0;

    if(sdio_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
        sector *= SD_SECTOR_SIZE;

    sdio_init_data(SD_SECTOR_SIZE, SDIO_TransferDir_ToSDIO);

    /*!< Send CMD17 READ_SINGLE_BLOCK */
    sdio_send_cmd((uint32_t)sector, SD_CMD_READ_SINGLE_BLOCK, SDIO_Response_Short);

    errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);

    if(errorstatus != SD_OK)
        return errorstatus;

    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    SDIO_DMACmd(ENABLE);
    SD_LowLevel_DMA_RxConfig((uint32_t *)readbuff, SD_SECTOR_SIZE);

    return errorstatus;
}

/**
  * @brief  Allows to read blocks from a specified address  in a card.  The Data
  *         transfer can be managed by DMA mode or Polling mode. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.   
  * @param  readbuff: pointer to the buffer that will contain the received data.
  * @param  sector: Address from where data are to be read.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_ReadMultiBlocks(uint8_t *readbuff, uint32_t sector, uint32_t NumberOfBlocks)
{
    SD_Error errorstatus = SD_OK;
    uint32_t length = NumberOfBlocks * SD_SECTOR_SIZE;

    if(!readbuff ||
       (length > SD_MAX_DATA_LENGTH) ||
       (NumberOfBlocks <= 1)) {
        return SD_INVALID_PARAMETER;
    }

    sdio_state.sdio_xfer_error = SD_ACTIVE;
    sdio_state.sdio_xfer_multi_block = true;
    SDIO->DCTRL = 0x0;

    if(sdio_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
        sector *= SD_SECTOR_SIZE;

    sdio_init_data(length, SDIO_TransferDir_ToSDIO);

    /*!< Send CMD18 READ_MULT_BLOCK with argument data address */
    sdio_send_cmd((uint32_t)sector, SD_CMD_READ_MULT_BLOCK, SDIO_Response_Short);
    errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);

    if(errorstatus != SD_OK)
        return errorstatus;

    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    SDIO_DMACmd(ENABLE);
    SD_LowLevel_DMA_RxConfig((uint32_t *)readbuff, length);

    return errorstatus;
}

/**
  * @brief  Allows to write one block starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.      
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  sector: Address from where data are to be read.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_WriteBlock(const uint8_t *writebuff, uint32_t sector)
{
    SD_Error errorstatus = SD_OK;

    if(!writebuff)
        return SD_INVALID_PARAMETER;

    sdio_state.sdio_xfer_error = SD_ACTIVE;
    sdio_state.sdio_xfer_multi_block = false;
    SDIO->DCTRL = 0x0;

    if(sdio_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
        sector *= SD_SECTOR_SIZE;

    /*!< Send CMD24 WRITE_SINGLE_BLOCK */
    sdio_send_cmd(sector, SD_CMD_WRITE_SINGLE_BLOCK, SDIO_Response_Short);
    errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);

    if(errorstatus != SD_OK)
        return errorstatus;

    sdio_init_data(SD_SECTOR_SIZE, SDIO_TransferDir_ToCard);

    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    SD_LowLevel_DMA_TxConfig((uint32_t *)writebuff, SD_SECTOR_SIZE);
    SDIO_DMACmd(ENABLE);

    return errorstatus;
}

/**
  * @brief  Allows to write blocks starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode only. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.     
  * @param  sector: Address from where data are to be read.
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  NumberOfBlocks: number of blocks to be written.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_WriteMultiBlocks(const uint8_t *writebuff, uint32_t sector, uint32_t NumberOfBlocks)
{
    SD_Error errorstatus = SD_OK;
    uint32_t length = NumberOfBlocks * SD_SECTOR_SIZE;

    if(!writebuff ||
        (length > SD_MAX_DATA_LENGTH) ||
        (NumberOfBlocks <= 1)) {
        return SD_INVALID_PARAMETER;
    }

    sdio_state.sdio_xfer_error = SD_ACTIVE;
    sdio_state.sdio_xfer_multi_block = true;
    SDIO->DCTRL = 0x0;

    if(sdio_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
        sector *= SD_SECTOR_SIZE;

    /*!< To improve performance */
    sdio_send_cmd((uint32_t) (sdio_state.rca << 16), SD_CMD_APP_CMD, SDIO_Response_Short);
    errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

    if(errorstatus != SD_OK)
        return errorstatus;

    /*!< To improve performance */
    sdio_send_cmd((uint32_t)NumberOfBlocks, SD_CMD_SET_BLOCK_COUNT, SDIO_Response_Short);
    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);

    if(errorstatus != SD_OK)
        return errorstatus;

    /*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
    sdio_send_cmd((uint32_t)sector, SD_CMD_WRITE_MULT_BLOCK, SDIO_Response_Short);
    errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);

    if(SD_OK != errorstatus)
        return errorstatus;

    sdio_init_data(length, SDIO_TransferDir_ToCard);

    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    SDIO_DMACmd(ENABLE);
    SD_LowLevel_DMA_TxConfig((uint32_t *)writebuff, length);

    return errorstatus;
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished.
  *         This function should be called after SDIO_ReadMultiBlocks() function
  *         to insure that all data sent by the card are already transferred by
  *         the DMA controller.
  * @param  the sdio status flag to wait on... WAIT_WHILE_RX_ACTIVE or WAIT_WHILE_TX_ACTIVE
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_WaitIOOperation(sdio_wait_on_io_t io_flag)
{
    SD_Error errorstatus = SD_OK;
    uint32_t timeout;

    timeout = gettime_ms() + SDIO_WAITTIMEOUT;

    while (!sdio_state.dma_xfer_end &&
            (sdio_state.sdio_xfer_error == SD_ACTIVE) &&
            (gettime_ms() < timeout)){
         usleep(1000);
    }

    sdio_state.dma_xfer_end = false;

    timeout = gettime_ms() + SDIO_WAITTIMEOUT;

    while((SDIO->STA & io_flag) &&
            (gettime_ms() < timeout)){
         usleep(1000);
    }

    if((gettime_ms() >= timeout) && (errorstatus == SD_OK))
        errorstatus = SD_DATA_TIMEOUT;


    SDIO_ClearFlag(SDIO_STATIC_FLAGS);

    if(sdio_state.sdio_xfer_error != SD_OK)
        return sdio_state.sdio_xfer_error;

    return errorstatus;
}


/**
  * @brief  Gets the cuurent data transfer state.
  *
  * @retval SDTransferState: Data Transfer state.
  *   This value can be:
  *        - SD_TRANSFER_OK: No data transfer is acting
  *        - SD_TRANSFER_BUSY: Data transfer is acting
  */
SDTransferState SD_GetTransferState(void)
{
    if(SDIO->STA & (SDIO_FLAG_TXACT | SDIO_FLAG_RXACT))
        return SD_TRANSFER_BUSY;

    return SD_TRANSFER_OK;
}

/**
  * @brief  Aborts an ongoing data transfer.
  *
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_StopTransfer(void)
{
    SD_Error errorstatus = SD_OK;
    sdio_send_cmd(0x0, SD_CMD_STOP_TRANSMISSION, SDIO_Response_Short);
    errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
    return errorstatus;
}

/**
  * @brief  Allows to erase memory area specified for the given card.
  * @param  startaddr: the start sector number.
  * @param  endaddr: the end sector number.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_Erase(uint32_t startaddr, uint32_t endaddr)
{
    SD_Error errorstatus = SD_OK;
    uint32_t delay = 0;
    __IO uint32_t maxdelay = 0;
    uint8_t cardstate = 0;

    /*!< Check if the card coomnd class supports erase command */
    if(((sdio_state.csd_table[1] >> 20) & SD_CCCC_ERASE) == 0)
    {
        errorstatus = SD_REQUEST_NOT_APPLICABLE;
        return errorstatus;
    }

    maxdelay = 120000 / ((SDIO->CLKCR & 0xFF) + 2);

    if(SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
    {
        errorstatus = SD_LOCK_UNLOCK_FAILED;
        return errorstatus;
    }

    if(sdio_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
    {
        startaddr *= SD_SECTOR_SIZE;
        endaddr *= SD_SECTOR_SIZE;
    }

    /*!< According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
    if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == sdio_state.card_type) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == sdio_state.card_type) || (SDIO_HIGH_CAPACITY_SD_CARD == sdio_state.card_type))
    {
        /*!< Send CMD32 SD_ERASE_GRP_START with argument as addr  */
        sdio_send_cmd(startaddr, SD_CMD_SD_ERASE_GRP_START, SDIO_Response_Short);

        errorstatus = CmdResp1Error(SD_CMD_SD_ERASE_GRP_START);
        if(errorstatus != SD_OK)
        {
            return errorstatus;
        }

        /*!< Send CMD33 SD_ERASE_GRP_END with argument as addr  */
        sdio_send_cmd(endaddr, SD_CMD_SD_ERASE_GRP_END, SDIO_Response_Short);

        errorstatus = CmdResp1Error(SD_CMD_SD_ERASE_GRP_END);
        if(errorstatus != SD_OK)
        {
            return errorstatus;
        }
    }

    /*!< Send CMD38 ERASE */
    sdio_send_cmd(0, SD_CMD_ERASE, SDIO_Response_Short);

    errorstatus = CmdResp1Error(SD_CMD_ERASE);

    if(errorstatus != SD_OK)
    {
        return errorstatus;
    }

    for (delay = 0; delay < maxdelay; delay++)
    {}

    /*!< Wait till the card is in programming state */
    errorstatus = IsCardProgramming(&cardstate);
    delay = SDIO_DATATIMEOUT;
    while ((delay > 0) && (errorstatus == SD_OK) && ((SD_CARD_PROGRAMMING == cardstate) || (SD_CARD_RECEIVING == cardstate)))
    {
        errorstatus = IsCardProgramming(&cardstate);
        delay--;
    }

    return errorstatus;
}

/**
  * @brief  Allows to process all the interrupts that are high.
  *
  * @retval SD_Error: SD Card Error code.
  */
void SDIO_IRQHandler(void)
{
    if(SDIO_GetITStatus(SDIO_IT_DATAEND) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_DATAEND);

        if(sdio_state.sdio_xfer_multi_block)
            sdio_state.sdio_xfer_error = SD_StopTransfer();
        else
            sdio_state.sdio_xfer_error = SD_OK;
    }

    if(SDIO_GetITStatus(SDIO_IT_DCRCFAIL) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_DCRCFAIL);
        sdio_state.sdio_xfer_error = SD_DATA_CRC_FAIL;
    }

    if(SDIO_GetITStatus(SDIO_IT_DTIMEOUT) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_DTIMEOUT);
        sdio_state.sdio_xfer_error = SD_DATA_TIMEOUT;
    }

    if(SDIO_GetITStatus(SDIO_IT_RXOVERR) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_RXOVERR);
        sdio_state.sdio_xfer_error = SD_RX_OVERRUN;
    }

    if(SDIO_GetITStatus(SDIO_IT_TXUNDERR) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_TXUNDERR);
        sdio_state.sdio_xfer_error = SD_TX_UNDERRUN;
    }

    if(SDIO_GetITStatus(SDIO_IT_STBITERR) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_STBITERR);
        sdio_state.sdio_xfer_error = SD_START_BIT_ERR;
    }

    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
                SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
                SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
}

/**
  * @brief  Checks for error conditions for CMD0.
  *
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdError(void)
{
    uint32_t timeout = gettime_ms() + SDIO_CMD0TIMEOUT;

    while((gettime_ms() < timeout) &&
          (SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) == RESET)){
        // usleep(1000);
    }

    if(gettime_ms() >= timeout)
        return SD_CMD_RSP_TIMEOUT;

    SDIO_ClearFlag(SDIO_STATIC_FLAGS);

    return SD_OK;
}

/**
  * @brief  Checks for error conditions for R7 response.
  *
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp7Error(void)
{
    uint32_t status = SDIO->STA;
    uint32_t timeout = gettime_ms() + SDIO_CMD0TIMEOUT;

    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)) &&
            (gettime_ms() < timeout)){
        // usleep(1000);
        status = SDIO->STA;
    }

    if(status & SDIO_FLAG_CMDREND)
    {
        /*!< Card is SD V2.0 compliant */
        SDIO_ClearFlag(SDIO_FLAG_CMDREND);
        return SD_OK;
    }

    if((gettime_ms() >= timeout) || (status & SDIO_FLAG_CTIMEOUT))
    {
        /*!< Card is not V2.0 complient or card does not support the set voltage range */
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return SD_CMD_RSP_TIMEOUT;
    }

    return SD_OK;
}

/**
  * @brief  Checks for error conditions for R1 response.
  * @param  cmd: The sent command index.
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp1Error(uint8_t cmd)
{
    uint32_t status = SDIO->STA;
    uint32_t response_r1;

    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT))){
        // usleep(1000);
        status = SDIO->STA;
    }

    if(status & SDIO_FLAG_CTIMEOUT)
    {
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return SD_CMD_RSP_TIMEOUT;
    }
    else if(status & SDIO_FLAG_CCRCFAIL)
    {
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
        return SD_CMD_CRC_FAIL;
    }

    /*!< Check response received is of desired command */
    if(SDIO_GetCommandResponse() != cmd)
        return SD_ILLEGAL_CMD;


    SDIO_ClearFlag(SDIO_STATIC_FLAGS);

    /*!< We have received response, retrieve it for analysis  */
    response_r1 = SDIO_GetResponse(SDIO_RESP1);

    if((response_r1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
        return SD_OK;

    if(response_r1 & SD_OCR_ADDR_OUT_OF_RANGE)
        return SD_ADDR_OUT_OF_RANGE;

    if(response_r1 & SD_OCR_ADDR_MISALIGNED)
        return SD_ADDR_MISALIGNED;

    if(response_r1 & SD_OCR_BLOCK_LEN_ERR)
        return SD_BLOCK_LEN_ERR;

    if(response_r1 & SD_OCR_ERASE_SEQ_ERR)
        return SD_ERASE_SEQ_ERR;

    if(response_r1 & SD_OCR_BAD_ERASE_PARAM)
        return SD_BAD_ERASE_PARAM;

    if(response_r1 & SD_OCR_WRITE_PROT_VIOLATION)
        return SD_WRITE_PROT_VIOLATION;

    if(response_r1 & SD_OCR_LOCK_UNLOCK_FAILED)
        return SD_LOCK_UNLOCK_FAILED;

    if(response_r1 & SD_OCR_COM_CRC_FAILED)
        return SD_COM_CRC_FAILED;

    if(response_r1 & SD_OCR_ILLEGAL_CMD)
        return SD_ILLEGAL_CMD;

    if(response_r1 & SD_OCR_CARD_ECC_FAILED)
        return SD_CARD_ECC_FAILED;

    if(response_r1 & SD_OCR_CC_ERROR)
        return SD_CC_ERROR;

    if(response_r1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
        return SD_GENERAL_UNKNOWN_ERROR;

    if(response_r1 & SD_OCR_STREAM_READ_UNDERRUN)
        return SD_STREAM_READ_UNDERRUN;

    if(response_r1 & SD_OCR_STREAM_WRITE_OVERRUN)
        return SD_STREAM_WRITE_OVERRUN;

    if(response_r1 & SD_OCR_CID_CSD_OVERWRIETE)
        return SD_CID_CSD_OVERWRITE;

    if(response_r1 & SD_OCR_WP_ERASE_SKIP)
        return SD_WP_ERASE_SKIP;

    if(response_r1 & SD_OCR_CARD_ECC_DISABLED)
        return SD_CARD_ECC_DISABLED;

    if(response_r1 & SD_OCR_ERASE_RESET)
        return SD_ERASE_RESET;

    if(response_r1 & SD_OCR_AKE_SEQ_ERROR)
        return SD_AKE_SEQ_ERROR;

    return SD_OK;
}

/**
  * @brief  Checks for error conditions for R3 (OCR) response.
  *
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp3Error(void)
{
    uint32_t status = SDIO->STA;

    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT))){
        // usleep(1000);
        status = SDIO->STA;
    }

    if(status & SDIO_FLAG_CTIMEOUT)
    {
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return SD_CMD_RSP_TIMEOUT;
    }

    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    return SD_OK;
}

/**
  * @brief  Checks for error conditions for R2 (CID or CSD) response.
  *
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp2Error(void)
{
    uint32_t status = SDIO->STA;

    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND))){
        // usleep(1000);
        status = SDIO->STA;
    }

    if(status & SDIO_FLAG_CTIMEOUT)
    {
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return SD_CMD_RSP_TIMEOUT;
    }
    else if(status & SDIO_FLAG_CCRCFAIL)
    {
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
        return SD_CMD_CRC_FAIL;
    }

    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    return SD_OK;
}

/**
  * @brief  Checks for error conditions for R6 (RCA) response.
  * @param  cmd: The sent command index.
  * @param  prca: pointer to the variable that will contain the SD card relative
  *         address RCA.
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca)
{
    uint32_t status = SDIO->STA;
    uint32_t response_r1;

    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND))){
        // usleep(1000);
        status = SDIO->STA;
    }

    if(status & SDIO_FLAG_CTIMEOUT)
    {
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return SD_CMD_RSP_TIMEOUT;
    }
    else if(status & SDIO_FLAG_CCRCFAIL)
    {
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
        return SD_CMD_CRC_FAIL;
    }

    /*!< Check response received is of desired command */
    if(SDIO_GetCommandResponse() != cmd)
        return SD_ILLEGAL_CMD;


    SDIO_ClearFlag(SDIO_STATIC_FLAGS);

    /*!< We have received response, retrieve it.  */
    response_r1 = SDIO_GetResponse(SDIO_RESP1);

    if(SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
    {
        *prca = (uint16_t) (response_r1 >> 16);
        return SD_OK;
    }

    if(response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR)
        return SD_GENERAL_UNKNOWN_ERROR;

    if(response_r1 & SD_R6_ILLEGAL_CMD)
        return SD_ILLEGAL_CMD;

    if(response_r1 & SD_R6_COM_CRC_FAILED)
        return SD_COM_CRC_FAILED;

    return SD_OK;
}

/**
  * @brief  Enables or disables the SDIO wide bus mode.
  * @param  NewState: new state of the SDIO wide bus mode.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error SDEnWideBus(FunctionalState NewState)
{
    SD_Error errorstatus = SD_OK;

    if(SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED;

    /*!< If wide bus operation to be enabled */
    if(NewState == ENABLE)
    {
        /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
        sdio_send_cmd((uint32_t) sdio_state.rca << 16, SD_CMD_APP_CMD, SDIO_Response_Short);
        errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

        if(errorstatus != SD_OK)
            return errorstatus;

        /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
        sdio_send_cmd(0x2, SD_CMD_APP_SD_SET_BUSWIDTH, SDIO_Response_Short);
        return CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
    }   /*!< If wide bus operation to be disabled */
    else
    {
        /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
        sdio_send_cmd((uint32_t) sdio_state.rca << 16, SD_CMD_APP_CMD, SDIO_Response_Short);
        errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

        if(errorstatus != SD_OK)
            return errorstatus;

        /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
        sdio_send_cmd(0x00, SD_CMD_APP_SD_SET_BUSWIDTH, SDIO_Response_Short);
        return CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
    }
}

/**
  * @brief  Checks if the SD card is in programming state.
  * @param  pstatus: pointer to the variable that will contain the SD card state.
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error IsCardProgramming(uint8_t *pstatus)
{
    SD_Error errorstatus = SD_OK;
    __IO uint32_t respR1 = 0;
    __IO uint32_t status = 0;

    sdio_send_cmd((uint32_t) sdio_state.rca << 16, SD_CMD_SEND_STATUS, SDIO_Response_Short);

    status = SDIO->STA;
    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT))){
        // usleep(1000);
        status = SDIO->STA;
    }

    if(status & SDIO_FLAG_CTIMEOUT)
    {
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return SD_CMD_RSP_TIMEOUT;
    }
    else if(status & SDIO_FLAG_CCRCFAIL)
    {
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
        return SD_CMD_CRC_FAIL;
    }

    status = (uint32_t)SDIO_GetCommandResponse();

    /*!< Check response received is of desired command */
    if(status != SD_CMD_SEND_STATUS)
        return SD_ILLEGAL_CMD;

    SDIO_ClearFlag(SDIO_STATIC_FLAGS);

    /*!< We have received response, retrieve it for analysis  */
    respR1 = SDIO_GetResponse(SDIO_RESP1);

    /*!< Find out card status */
    *pstatus = (uint8_t) ((respR1 >> 9) & 0x0000000F);

    if((respR1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
        return errorstatus;

    if(respR1 & SD_OCR_ADDR_OUT_OF_RANGE)
        return SD_ADDR_OUT_OF_RANGE;

    if(respR1 & SD_OCR_ADDR_MISALIGNED)
        return SD_ADDR_MISALIGNED;

    if(respR1 & SD_OCR_BLOCK_LEN_ERR)
        return SD_BLOCK_LEN_ERR;

    if(respR1 & SD_OCR_ERASE_SEQ_ERR)
        return SD_ERASE_SEQ_ERR;

    if(respR1 & SD_OCR_BAD_ERASE_PARAM)
        return SD_BAD_ERASE_PARAM;

    if(respR1 & SD_OCR_WRITE_PROT_VIOLATION)
        return SD_WRITE_PROT_VIOLATION;

    if(respR1 & SD_OCR_LOCK_UNLOCK_FAILED)
        return SD_LOCK_UNLOCK_FAILED;

    if(respR1 & SD_OCR_COM_CRC_FAILED)
        return SD_COM_CRC_FAILED;

    if(respR1 & SD_OCR_ILLEGAL_CMD)
        return SD_ILLEGAL_CMD;

    if(respR1 & SD_OCR_CARD_ECC_FAILED)
        return SD_CARD_ECC_FAILED;

    if(respR1 & SD_OCR_CC_ERROR)
        return SD_CC_ERROR;

    if(respR1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
        return SD_GENERAL_UNKNOWN_ERROR;

    if(respR1 & SD_OCR_STREAM_READ_UNDERRUN)
        return SD_STREAM_READ_UNDERRUN;

    if(respR1 & SD_OCR_STREAM_WRITE_OVERRUN)
        return SD_STREAM_WRITE_OVERRUN;

    if(respR1 & SD_OCR_CID_CSD_OVERWRIETE)
        return SD_CID_CSD_OVERWRITE;

    if(respR1 & SD_OCR_WP_ERASE_SKIP)
        return SD_WP_ERASE_SKIP;

    if(respR1 & SD_OCR_CARD_ECC_DISABLED)
        return SD_CARD_ECC_DISABLED;

    if(respR1 & SD_OCR_ERASE_RESET)
        return SD_ERASE_RESET;

    if(respR1 & SD_OCR_AKE_SEQ_ERROR)
        return SD_AKE_SEQ_ERROR;

    return errorstatus;
}

/**
  * @}
  */
