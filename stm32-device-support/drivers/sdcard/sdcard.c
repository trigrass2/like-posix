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
 * - SDC MMC card support untested
 * - SDC V1, V2.x, SDHC tested
 * - loosely emulates the original API by ST, with some modifications
 *
 * @file
 * @{
 */

#include <unistd.h>
#include <sys/time.h>
#include "board_config.h"
#include "sdcard_config.h"
#include "sdcard.h"
#include "cutensils.h"

#ifndef SDCARD_DRIVER_MODE
#error  SDCARD_DRIVER_MODE must be defined - normally in stm32-device-support/board/xx/sdcard_config.h
#endif

#if SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_4BIT
#pragma message "SDCARD driver building in 4bit SDIO mode"
#elif SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_1BIT
#pragma message "SDCARD driver building in 1bit SDIO mode"
#elif SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SPI
#pragma message "SDCARD driver building in SPI mode"
#else
#error sdcard driver building in an invalid mode...
#endif

#if USE_DRIVER_LEDS
#include "leds.h"
#pragma message "SDCARD driver building with activity LED support"
#ifndef DISK_ACTIVITY_LED
#pragma message "warning: DISK_ACTIVITY_LED not defined for this board - set in led_config.h"
#endif
#else
#pragma message "SDCARD driver building without activity LED support"
#endif


typedef struct {
#if USE_THREAD_AWARE_SDCARD_DRIVER
    QueueHandle_t transfer_end;
#else
    bool transfer_end;
#endif
    SD_Error transfer_error;
    SD_Error sdio_dma_error;
    bool transfer_multiblock;
    uint8_t card_type;
    uint32_t rca;				///< holds the card RCA shifted up by 16 bits. used by the driver only. the true RCA is stored in SD_CardInfo sdcardinfo.RCA
    bool ccc_erase;
}sdcard_state_t;

volatile sdcard_state_t sdcard_state = {
#if USE_THREAD_AWARE_SDCARD_DRIVER
    .transfer_end = NULL,
#else
    .transfer_end = false,
#endif
    .transfer_error = SD_ACTIVE,
    .sdio_dma_error = SD_ACTIVE,
    .transfer_multiblock = false,
    .card_type = SDIO_UNKNOWN_CARD_TYPE,
    .rca = 0,
	.ccc_erase = false
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

#if USE_LOGGER
const char* sderrstr[] = {
     "null code error",
         /**
           * @brief  SDIO specific error defines
           */
     "SD_CMD_CRC_FAIL", 		///< Command response received (but CRC check failed)
     "SD_DATA_CRC_FAIL", 		///< Data bock sent/received (CRC check Failed)
     "SD_CMD_RSP_TIMEOUT", 		///< Command response timeout
     "SD_DATA_TIMEOUT", 		///< Data time out
     "SD_TX_UNDERRUN", 		///< Transmit FIFO under-run
     "SD_RX_OVERRUN", 		///< Receive FIFO over-run
     "SD_START_BIT_ERR", 		///< Start bit not detected on all data signals in widE bus mode
     "SD_CMD_OUT_OF_RANGE", 		///< CMD's argument was out of range.
     "SD_ADDR_MISALIGNED", 		///< Misaligned address
     "SD_BLOCK_LEN_ERR", 		///< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length
     "SD_ERASE_SEQ_ERR", 		///< An error in the sequence of erase command occurs.
     "SD_BAD_ERASE_PARAM", 		///< An Invalid selection for erase groups
     "SD_WRITE_PROT_VIOLATION", 		///< Attempt to program a write protect block
     "SD_LOCK_UNLOCK_FAILED", 		///< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card
     "SD_COM_CRC_FAILED", 		///< CRC check of the previous command failed
     "SD_ILLEGAL_CMD", 		///< Command is not legal for the card state
     "SD_CARD_ECC_FAILED", 		///< Card internal ECC was applied but failed to correct the data
     "SD_CC_ERROR", 		///< Internal card controller error
     "SD_GENERAL_UNKNOWN_ERROR", 		///< General or Unknown error
     "SD_STREAM_READ_UNDERRUN", 		///< The card could not sustain data transfer in stream read operation.
     "SD_STREAM_WRITE_OVERRUN", 		///< The card could not sustain data programming in stream mode
     "SD_CID_CSD_OVERWRITE", 		///< CID/CSD overwrite error
     "SD_WP_ERASE_SKIP", 		///< only partial address space was erased
     "SD_CARD_ECC_DISABLED", 		///< Command has been executed without using internal ECC
     "SD_ERASE_RESET", 		///< Erase sequence was cleared before executing because an out of erase sequence command was received
     "SD_AKE_SEQ_ERROR", 		///< Error in sequence of authentication.
     "SD_INVALID_VOLTRANGE",
     "SD_ADDR_OUT_OF_RANGE",
     "SD_SWITCH_ERROR",
     "SD_SDIO_DISABLED",
     "SD_SDIO_FUNCTION_BUSY",
     "SD_SDIO_FUNCTION_FAILED",
     "SD_SDIO_UNKNOWN_FUNCTION",

         /**
           * @brief  Standard error defines
           */
     "SD_INTERNAL_ERROR",
     "SD_NOT_CONFIGURED",
     "SD_REQUEST_PENDING",
     "SD_REQUEST_NOT_APPLICABLE",
     "SD_INVALID_PARAMETER",
     "SD_UNSUPPORTED_FEATURE",
     "SD_UNSUPPORTED_HW",
     "SD_ERROR",
     "SD_OK",
     "SD_ACTIVE",
     "SD_IDLE",
     "SD_DATA_WRITE_ERROR",
     "SD_DATA_READ_ERROR",
     "SD_BUSY_TIMEOUT",
};
#endif

static logger_t sdlog;
static uint8_t disk_status =  SD_NOT_PRESENT;

static void SD_WP_Card_Detect_init(void);

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


void set_diskstatus(uint8_t state)
{
	disk_status = state;
}

uint8_t get_diskstatus()
{
	return disk_status;
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

#if SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_4BIT || SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_1BIT

#include "sdio_hw_defs.h"
#include "sdio_cmd.h"

#if USE_THREAD_AWARE_SDCARD_DRIVER
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


SD_Error SD_InitializeCards(SD_CardInfo* cardinfo);
SD_Error SD_SetBusOperation(uint32_t width, uint32_t clockdiv);
SD_Error SD_SelectDeselect();
SD_Error SD_StopTransfer(void);


static void SD_IO_DeInit(void);
static void SD_IO_Init(void);

static void SD_LowLevel_DMA_TxConfig(uint32_t *BufferSRC, uint32_t BufferSize);
static void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST, uint32_t BufferSize);
static void SD_NVIC_Configuration(void);

static SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo, uint32_t* csd_table, uint32_t* cid_table);
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
    SD_Error sderr;

    log_init(&sdlog, "sdio");

#if USE_THREAD_AWARE_SDCARD_DRIVER
    sdcard_state.transfer_end = xQueueCreate(2, 1);
    assert_true(sdcard_state.transfer_end);
#endif

    SD_NVIC_Configuration();
    SD_IO_Init();
    sderr = SD_PowerON();

    if(sderr != SD_OK)
        return sderr;

    sderr = SD_InitializeCards(sdcardinfo);

    if(sderr != SD_OK)
        return sderr;

    // select card
    sderr = SD_SelectDeselect();

    if(sderr != SD_OK)
        return sderr;

    // high speed, set bus mode
    if(sderr == SD_OK)
#if SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_4BIT
        sderr = SD_SetBusOperation(SDIO_BusWide_4b, SDIO_TRANSFER_CLK_DIV);
#elif SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_1BIT
    sderr = SD_SetBusOperation(SDIO_BusWide_1b, SDIO_TRANSFER_CLK_DIV);
#endif

    if(sderr != SD_OK)
        return sderr;

    // Set Block Size for Card
    sdio_send_cmd((uint32_t)SD_SECTOR_SIZE, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short);
    sderr = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

    log_edebug(&sdlog, "cmd%d: %s", SD_CMD_SET_BLOCKLEN, sderrstr[sderr]);

    return sderr;
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
    DMA_ITConfig(DMA2_Channel4, DMA_IT_TC|DMA_IT_TE, ENABLE);
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
    DMA_ITConfig(DMA2_Channel4, DMA_IT_TC|DMA_IT_TE, ENABLE);
    DMA_Cmd(DMA2_Channel4, ENABLE);
}

// void SD_SDIO_DMA_IRQHANDLER(void)
// {
//     if(DMA_GetITStatus(DMA2_IT_TC4) == SET)
//     {
//         sdcard_state.transfer_end = true;
//         DMA_ClearITPendingBit(DMA2_IT_TC4);
//     }
// }

void SD_SDIO_DMA_IRQHANDLER(void)
{
    if(DMA_GetITStatus(DMA2_IT_TC4) == SET)
    {
        DMA_ClearITPendingBit(DMA2_IT_TC4);
    }
    if(DMA_GetITStatus(DMA2_IT_TE4) == SET)
    {
        DMA_ClearITPendingBit(DMA2_IT_TE4);
        printf("transmission error\n");
    }

#if USE_THREAD_AWARE_SDCARD_DRIVER
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        uint8_t dummy;
        xQueueSendFromISR(sdcard_state.transfer_end, (const void*)&dummy, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else
        sdcard_state.transfer_end = true;
#endif
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
    DMA_ITConfig(SD_SDIO_DMA_STREAM, DMA_IT_TC|DMA_IT_TE|DMA_IT_FE, ENABLE);
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
    DMA_ITConfig(SD_SDIO_DMA_STREAM, DMA_IT_TC|DMA_IT_TE|DMA_IT_FE, ENABLE);
    DMA_FlowControllerConfig(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);

    /* DMA2 Stream3 or Stream6 enable */
    DMA_Cmd(SD_SDIO_DMA_STREAM, ENABLE);
}

//void SD_SDIO_DMA_IRQHANDLER(void)
//{
//    if(DMA2->LISR & SD_SDIO_DMA_FLAG_TCIF)
//    {
//        sdcard_state.transfer_end = true;
//        DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TCIF|SD_SDIO_DMA_FLAG_FEIF);
//    }
//}

void SD_SDIO_DMA_IRQHANDLER(void)
{
	if(DMA_GetITStatus(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_IT_TCIF) == SET)
	{
	    sdio_state.sdio_dma_error = SD_OK;
		DMA_ClearITPendingBit(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_IT_TCIF);
	}
	if(DMA_GetITStatus(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_IT_TEIF) == SET)
	{
	    sdio_state.sdio_dma_error = SD_DMA_TRANSMISSION_ERROR;
		DMA_ClearITPendingBit(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_IT_TEIF);
//		printf("transmission error\n");
		usleep(2000);
	}
	if(DMA_GetITStatus(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_IT_FEIF) == SET)
	{
	    sdio_state.sdio_dma_error = SD_DMA_FIFO_ERROR;
		DMA_ClearITPendingBit(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_IT_FEIF);
//		printf("fifo error\n");
		usleep(2000);
	}

	DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TCIF|SD_SDIO_DMA_FLAG_FEIF|SD_SDIO_DMA_FLAG_TEIF);

#if USE_THREAD_AWARE_SDCARD_DRIVER
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t dummy;
    xQueueSendFromISR(sdcard_state.transfer_end, (const void*)&dummy, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else
    sdcard_state.transfer_end = true;
#endif
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
    SD_Error sderr;

    sdio_send_cmd(sdcard_state.rca, SD_CMD_SEND_STATUS, SDIO_Response_Short);
    sderr = CmdResp1Error(SD_CMD_SEND_STATUS);

    if(sderr == SD_OK)
        *cardstatus = (SDIO_GetResponse(SDIO_RESP1) >> 9)&0x0F;
    else
        *cardstatus = SD_CARD_ERROR;

    return sderr;
}

/**
  * @brief  Enquires cards about their operating voltage and configures
  *   clock controls.
  *
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_PowerON(void)
{
    SD_Error sderr = SD_OK;
    uint8_t i;
    uint32_t resp;
    uint32_t acmdarg;
    uint32_t timer = 0;
    uint32_t wait = 50000; // microseconds

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
    for(i = 0; i < 10; i++)
    {
        sdio_send_cmd(0x0, SD_CMD_GO_IDLE_STATE, SDIO_Response_No);
        // wait for cmd sent
        while(SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) == RESET);
        SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    }

    /*!< CMD8: SEND_IF_COND ----------------------------------------------------*/
    /*!< Send CMD8 to verify SD card interface operating condition */
    /*!< Argument: - [31:12]: Reserved (shall be set to '0')
           - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
           - [7:0]: Check Pattern (recommended 0xAA) */
    /*!< CMD Response: R7 */
    sdio_send_cmd(SD_CHECK_PATTERN, SDIO_SEND_IF_COND, SDIO_Response_Short);
    sderr = CmdResp7Error();

    log_edebug(&sdlog, "cmd%d: %s", SDIO_SEND_IF_COND, sderrstr[sderr]);

    if(sderr == SD_OK)
    {
        sdcard_state.card_type = SDIO_STD_CAPACITY_SD_CARD_V2_0;
        acmdarg = SD_VOLTAGE_WINDOW_SD | SD_HIGH_CAPACITY;
    }
    else
    {
        sdcard_state.card_type = SDIO_STD_CAPACITY_SD_CARD_V1_1;
        acmdarg = SD_VOLTAGE_WINDOW_SD;
    }

    while(timer < 2000000)
    {
        sdio_send_cmd(0x00, SD_CMD_APP_CMD, SDIO_Response_Short);
        CmdResp1Error(SD_CMD_APP_CMD);
        sdio_send_cmd(acmdarg, SD_CMD_SD_APP_OP_COND, SDIO_Response_Short);
        sderr = CmdResp3Error();
        resp = SDIO_GetResponse(SDIO_RESP1);
        log_edebug(&sdlog, "cmd%d: %s", SD_CMD_SD_APP_OP_COND, sderrstr[sderr]);
        log_edebug(&sdlog, "resp: %#08x", resp);
        if(resp & 0x80000000)
        {
            sderr = SD_OK;
            break;
        }
        else
            sderr = SD_INVALID_VOLTRANGE;
        usleep(wait);
        timer += wait;
        wait *= 2;
    }

    if(resp & SD_HIGH_CAPACITY)
        sdcard_state.card_type = SDIO_HIGH_CAPACITY_SD_CARD;

  return sderr;
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
SD_Error SD_InitializeCards(SD_CardInfo* sdcardinfo)
{
    SD_Error sderr = SD_OK;
    uint16_t rca = 0x01;
    uint32_t cid_table[4] = {0, 0, 0, 0};
    uint32_t csd_table[4] = {0, 0, 0, 0};

    if(SDIO_GetPowerState() == SDIO_PowerState_OFF)
        return SD_REQUEST_NOT_APPLICABLE;

    if(SDIO_SECURE_DIGITAL_IO_CARD != sdcard_state.card_type)
    {
        /*!< Send CMD2 ALL_SEND_CID */
        sdio_send_cmd(0x0, SD_CMD_ALL_SEND_CID, SDIO_Response_Long);
        sderr = CmdResp2Error();

        log_edebug(&sdlog, "cmd%d: %s", SD_CMD_ALL_SEND_CID, sderrstr[sderr]);

        if(SD_OK != sderr)
            return sderr;

        cid_table[0] = SDIO_GetResponse(SDIO_RESP1);
        cid_table[1] = SDIO_GetResponse(SDIO_RESP2);
        cid_table[2] = SDIO_GetResponse(SDIO_RESP3);
        cid_table[3] = SDIO_GetResponse(SDIO_RESP4);
    }
    if( (SDIO_STD_CAPACITY_SD_CARD_V1_1 == sdcard_state.card_type) ||
        (SDIO_STD_CAPACITY_SD_CARD_V2_0 == sdcard_state.card_type) ||
        (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == sdcard_state.card_type) ||
        (SDIO_HIGH_CAPACITY_SD_CARD == sdcard_state.card_type))
    {
        /*!< Send CMD3 SET_REL_ADDR with argument 0 */
        /*!< SD Card publishes its RCA. */
        sdio_send_cmd(0x00, SD_CMD_SET_REL_ADDR, SDIO_Response_Short);
        sderr = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);

        log_edebug(&sdlog, "cmd%d: %s", SD_CMD_SET_REL_ADDR, sderrstr[sderr]);
        log_edebug(&sdlog, "rca: %d", rca);

        if(SD_OK != sderr)
            return sderr;
    }

    if(SDIO_SECURE_DIGITAL_IO_CARD != sdcard_state.card_type)
    {
        sdcard_state.rca = rca << 16;

        /*!< Send CMD9 SEND_CSD with argument as card's RCA */
        sdio_send_cmd((uint32_t)(rca << 16), SD_CMD_SEND_CSD, SDIO_Response_Long);
        sderr = CmdResp2Error();

        log_edebug(&sdlog, "cmd%d: %s", SD_CMD_SEND_CSD, sderrstr[sderr]);

        if(SD_OK != sderr)
            return sderr;

        csd_table[0] = SDIO_GetResponse(SDIO_RESP1);
        csd_table[1] = SDIO_GetResponse(SDIO_RESP2);
        csd_table[2] = SDIO_GetResponse(SDIO_RESP3);
        csd_table[3] = SDIO_GetResponse(SDIO_RESP4);

        if(SD_OK != sderr)
            return sderr;
    }

    // Interpret CSD/CID MSD registers
    sderr = SD_GetCardInfo(sdcardinfo, csd_table, cid_table);

    return sderr;
}

/**
  * @brief  Returns information about specific card.
  * @param  cardinfo: pointer to a SD_CardInfo structure that contains all SD card
  *         information.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo, uint32_t* csd_table, uint32_t* cid_table)
{
    SD_Error sderr = SD_OK;
    uint8_t tmp = 0;

    cardinfo->CardType = (uint8_t)sdcard_state.card_type;
    cardinfo->RCA = (uint16_t)(sdcard_state.rca>>16);

    /*!< Byte 0 */
    tmp = (uint8_t)((csd_table[0] & 0xFF000000) >> 24);
    cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;
    cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;
    cardinfo->SD_csd.Reserved1 = tmp & 0x03;

    /*!< Byte 1 */
    tmp = (uint8_t)((csd_table[0] & 0x00FF0000) >> 16);
    cardinfo->SD_csd.TAAC = tmp;

    /*!< Byte 2 */
    tmp = (uint8_t)((csd_table[0] & 0x0000FF00) >> 8);
    cardinfo->SD_csd.NSAC = tmp;

    /*!< Byte 3 */
    tmp = (uint8_t)(csd_table[0] & 0x000000FF);
    cardinfo->SD_csd.MaxBusClkFrec = tmp;

    /*!< Byte 4 */
    tmp = (uint8_t)((csd_table[1] & 0xFF000000) >> 24);
    cardinfo->SD_csd.CardComdClasses = tmp << 4;

    /*!< Byte 5 */
    tmp = (uint8_t)((csd_table[1] & 0x00FF0000) >> 16);
    cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;
    cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;

    /*!< Byte 6 */
    tmp = (uint8_t)((csd_table[1] & 0x0000FF00) >> 8);
    cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;
    cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;
    cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
    cardinfo->SD_csd.Reserved2 = 0; /*!< Reserved */

    if((sdcard_state.card_type == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (sdcard_state.card_type == SDIO_STD_CAPACITY_SD_CARD_V2_0))
    {
        cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;

        /*!< Byte 7 */
        tmp = (uint8_t)(csd_table[1] & 0x000000FF);
        cardinfo->SD_csd.DeviceSize |= (tmp) << 2;

        /*!< Byte 8 */
        tmp = (uint8_t)((csd_table[2] & 0xFF000000) >> 24);
        cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;

        cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
        cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);

        /*!< Byte 9 */
        tmp = (uint8_t)((csd_table[2] & 0x00FF0000) >> 16);
        cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
        cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
        cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
        /*!< Byte 10 */
        tmp = (uint8_t)((csd_table[2] & 0x0000FF00) >> 8);
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
        if(sdcard_state.card_type == SDIO_STD_CAPACITY_SD_CARD_V2_0)
            cardinfo->CardCapacity *= 2;

        cardinfo->CardBlockSize = SD_SECTOR_SIZE;
    }
    else if(sdcard_state.card_type == SDIO_HIGH_CAPACITY_SD_CARD)
    {
        /*!< Byte 7 */
        tmp = (uint8_t)(csd_table[1] & 0x000000FF);
        cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16;

        /*!< Byte 8 */
        tmp = (uint8_t)((csd_table[2] & 0xFF000000) >> 24);

        cardinfo->SD_csd.DeviceSize |= (tmp << 8);

        /*!< Byte 9 */
        tmp = (uint8_t)((csd_table[2] & 0x00FF0000) >> 16);

        cardinfo->SD_csd.DeviceSize |= (tmp);

        /*!< Byte 10 */
        tmp = (uint8_t)((csd_table[2] & 0x0000FF00) >> 8);

        // card capacity in blocks
        cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) * 1024;
        cardinfo->CardBlockSize = SD_SECTOR_SIZE;
    }

    cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
    cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;

    /*!< Byte 11 */
    tmp = (uint8_t)(csd_table[2] & 0x000000FF);
    cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
    cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);

    /*!< Byte 12 */
    tmp = (uint8_t)((csd_table[3] & 0xFF000000) >> 24);
    cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
    cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
    cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
    cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;

    /*!< Byte 13 */
    tmp = (uint8_t)((csd_table[3] & 0x00FF0000) >> 16);
    cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
    cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.Reserved3 = 0;
    cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);

    /*!< Byte 14 */
    tmp = (uint8_t)((csd_table[3] & 0x0000FF00) >> 8);
    cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
    cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
    cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
    cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
    cardinfo->SD_csd.ECC = (tmp & 0x03);

    /*!< Byte 15 */
    tmp = (uint8_t)(csd_table[3] & 0x000000FF);
    cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
    cardinfo->SD_csd.Reserved4 = 1;


    /*!< Byte 0 */
    tmp = (uint8_t)((cid_table[0] & 0xFF000000) >> 24);
    cardinfo->SD_cid.ManufacturerID = tmp;

    /*!< Byte 1 */
    tmp = (uint8_t)((cid_table[0] & 0x00FF0000) >> 16);
    cardinfo->SD_cid.OEM_AppliID = tmp << 8;

    /*!< Byte 2 */
    tmp = (uint8_t)((cid_table[0] & 0x000000FF00) >> 8);
    cardinfo->SD_cid.OEM_AppliID |= tmp;

    /*!< Byte 3 */
    tmp = (uint8_t)(cid_table[0] & 0x000000FF);
    cardinfo->SD_cid.ProdName1 = tmp << 24;

    /*!< Byte 4 */
    tmp = (uint8_t)((cid_table[1] & 0xFF000000) >> 24);
    cardinfo->SD_cid.ProdName1 |= tmp << 16;

    /*!< Byte 5 */
    tmp = (uint8_t)((cid_table[1] & 0x00FF0000) >> 16);
    cardinfo->SD_cid.ProdName1 |= tmp << 8;

    /*!< Byte 6 */
    tmp = (uint8_t)((cid_table[1] & 0x0000FF00) >> 8);
    cardinfo->SD_cid.ProdName1 |= tmp;

    /*!< Byte 7 */
    tmp = (uint8_t)(cid_table[1] & 0x000000FF);
    cardinfo->SD_cid.ProdName2 = tmp;

    /*!< Byte 8 */
    tmp = (uint8_t)((cid_table[2] & 0xFF000000) >> 24);
    cardinfo->SD_cid.ProdRev = tmp;

    /*!< Byte 9 */
    tmp = (uint8_t)((cid_table[2] & 0x00FF0000) >> 16);
    cardinfo->SD_cid.ProdSN = tmp << 24;

    /*!< Byte 10 */
    tmp = (uint8_t)((cid_table[2] & 0x0000FF00) >> 8);
    cardinfo->SD_cid.ProdSN |= tmp << 16;

    /*!< Byte 11 */
    tmp = (uint8_t)(cid_table[2] & 0x000000FF);
    cardinfo->SD_cid.ProdSN |= tmp << 8;

    /*!< Byte 12 */
    tmp = (uint8_t)((cid_table[3] & 0xFF000000) >> 24);
    cardinfo->SD_cid.ProdSN |= tmp;

    /*!< Byte 13 */
    tmp = (uint8_t)((cid_table[3] & 0x00FF0000) >> 16);
    cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
    cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;

    /*!< Byte 14 */
    tmp = (uint8_t)((cid_table[3] & 0x0000FF00) >> 8);
    cardinfo->SD_cid.ManufactDate |= tmp;

    /*!< Byte 15 */
    tmp = (uint8_t)(cid_table[3] & 0x000000FF);
    cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
    cardinfo->SD_cid.Reserved2 = 1;

    // card command class 'erase' supported
    sdcard_state.ccc_erase = ((csd_table[1] >> 20) & SD_CCCC_ERASE) != 0;

    return sderr;
}

/**
  * @brief  Enables wide bus opeartion for the requeseted card if supported by
  *         card.
  * @param  width: Specifies the SD card wide bus mode.
  *   This parameter can be one of the following values:
  *     @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
  *     @arg SDIO_BusWide_4b: 4-bit data transfer
  *     @arg SDIO_BusWide_1b: 1-bit data transfer
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_SetBusOperation(uint32_t width, uint32_t clockdiv)
{
    SD_Error sderr = SD_UNSUPPORTED_FEATURE;

    /*!< MMC Card doesn't support this feature */

    if((sdcard_state.card_type == SDIO_STD_CAPACITY_SD_CARD_V1_1) ||
       (sdcard_state.card_type == SDIO_STD_CAPACITY_SD_CARD_V2_0) ||
       (sdcard_state.card_type == SDIO_HIGH_CAPACITY_SD_CARD))
    {
        if(width == SDIO_BusWide_4b)
            sderr = SDEnWideBus(ENABLE);
        else if(width == SDIO_BusWide_1b)
            sderr = SDEnWideBus(DISABLE);

        if(sderr == SD_OK)
            sdio_init_peripheral(clockdiv, width);
    }

    return sderr;
}

/**
  * @brief  Selects od Deselects the corresponding card.
  * @param  addr: Address of the Card to be selected.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_SelectDeselect()
{
    SD_Error sderr = SD_OK;
    sdio_send_cmd(sdcard_state.rca, SD_CMD_SEL_DESEL_CARD, SDIO_Response_Short);
    sderr = CmdResp1Error(SD_CMD_SEL_DESEL_CARD);
    log_edebug(&sdlog, "cmd%d: %s", SD_CMD_SEL_DESEL_CARD, sderrstr[sderr]);
    return sderr;
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
    SD_Error sderr = SD_OK;

    if(!readbuff)
        return SD_INVALID_PARAMETER;

    sdcard_state.transfer_error = SD_ACTIVE;
    sdio_state.sdio_dma_error = SD_ACTIVE;
    sdcard_state.transfer_multiblock = false;

    SDIO->DCTRL = 0x0;

    if(sdcard_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
        sector *= SD_SECTOR_SIZE;

    sdio_init_data(SD_SECTOR_SIZE, SDIO_TransferDir_ToSDIO);

    /*!< Send CMD17 READ_SINGLE_BLOCK */
    sdio_send_cmd((uint32_t)sector, SD_CMD_READ_SINGLE_BLOCK, SDIO_Response_Short);

    sderr = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);

    if(sderr != SD_OK)
        return sderr;

#ifdef DISK_ACTIVITY_LED
    set_led(DISK_ACTIVITY_LED);
#endif
    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    SDIO_DMACmd(ENABLE);
    SD_LowLevel_DMA_RxConfig((uint32_t *)readbuff, SD_SECTOR_SIZE);

    return sderr;
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
    SD_Error sderr = SD_OK;
    uint32_t length = NumberOfBlocks * SD_SECTOR_SIZE;

    if(!readbuff ||
       (length > SD_MAX_DATA_LENGTH) ||
       (NumberOfBlocks <= 1)) {
        return SD_INVALID_PARAMETER;
    }

    sdcard_state.transfer_error = SD_ACTIVE;
    sdio_state.sdio_dma_error = SD_ACTIVE;
    sdcard_state.transfer_multiblock = true;

    SDIO->DCTRL = 0x0;

    if(sdcard_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
        sector *= SD_SECTOR_SIZE;

    sdio_init_data(length, SDIO_TransferDir_ToSDIO);

    /*!< Send CMD18 READ_MULT_BLOCK with argument data address */
    sdio_send_cmd((uint32_t)sector, SD_CMD_READ_MULT_BLOCK, SDIO_Response_Short);
    sderr = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);

    if(sderr != SD_OK)
        return sderr;

#ifdef DISK_ACTIVITY_LED
    set_led(DISK_ACTIVITY_LED);
#endif
    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    SDIO_DMACmd(ENABLE);
    SD_LowLevel_DMA_RxConfig((uint32_t *)readbuff, length);

    return sderr;
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
    SD_Error sderr = SD_OK;

    if(!writebuff)
        return SD_INVALID_PARAMETER;

    sdcard_state.transfer_error = SD_ACTIVE;
    sdio_state.sdio_dma_error = SD_ACTIVE;
    sdcard_state.transfer_multiblock = false;

    SDIO->DCTRL = 0x0;

    if(sdcard_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
        sector *= SD_SECTOR_SIZE;

    /*!< Send CMD24 WRITE_SINGLE_BLOCK */
    sdio_send_cmd(sector, SD_CMD_WRITE_SINGLE_BLOCK, SDIO_Response_Short);
    sderr = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);

    if(sderr != SD_OK)
        return sderr;

#ifdef DISK_ACTIVITY_LED
    set_led(DISK_ACTIVITY_LED);
#endif

    sdio_init_data(SD_SECTOR_SIZE, SDIO_TransferDir_ToCard);

    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    SD_LowLevel_DMA_TxConfig((uint32_t *)writebuff, SD_SECTOR_SIZE);
    SDIO_DMACmd(ENABLE);

    return sderr;
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
    SD_Error sderr = SD_OK;
    uint32_t length = NumberOfBlocks * SD_SECTOR_SIZE;

    if(!writebuff ||
        (length > SD_MAX_DATA_LENGTH) ||
        (NumberOfBlocks <= 1)) {
        return SD_INVALID_PARAMETER;
    }

    sdcard_state.transfer_error = SD_ACTIVE;
    sdio_state.sdio_dma_error = SD_ACTIVE;
    sdcard_state.transfer_multiblock = true;

    SDIO->DCTRL = 0x0;

    if(sdcard_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
        sector *= SD_SECTOR_SIZE;

    /*!< To improve performance */
    sdio_send_cmd(sdcard_state.rca, SD_CMD_APP_CMD, SDIO_Response_Short);
    sderr = CmdResp1Error(SD_CMD_APP_CMD);

    if(sderr != SD_OK)
        return sderr;

    /*!< To improve performance */
    sdio_send_cmd((uint32_t)NumberOfBlocks, SD_CMD_SET_BLOCK_COUNT, SDIO_Response_Short);
    sderr = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);

    if(sderr != SD_OK)
        return sderr;

    /*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
    sdio_send_cmd((uint32_t)sector, SD_CMD_WRITE_MULT_BLOCK, SDIO_Response_Short);
    sderr = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);

    if(SD_OK != sderr)
        return sderr;

#ifdef DISK_ACTIVITY_LED
    set_led(DISK_ACTIVITY_LED);
#endif

    sdio_init_data(length, SDIO_TransferDir_ToCard);

    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    SDIO_DMACmd(ENABLE);
    SD_LowLevel_DMA_TxConfig((uint32_t *)writebuff, length);

    return sderr;
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
    SD_Error sderr = SD_OK;

#if USE_THREAD_AWARE_SDCARD_DRIVER
    (void)io_flag;
    uint8_t dummy;

    // wait for DMA then SDIO end
    xQueueReceive(sdio_state.transfer_end, &dummy, 1000/portTICK_PERIOD_MS);
    xQueueReceive(sdio_state.transfer_end, &dummy, 1000/portTICK_PERIOD_MS);
    sderr = sdio_state.transfer_error;
    if(sdio_state.transfer_error != SD_OK || sdio_state.sdio_dma_error != SD_OK)
    {
//    	printf("sdio error: dmaerr=%u, sdioerr=%u\n", sdio_state.sdio_dma_error, sdio_state.transfer_error);
    }
#else
    uint32_t timeout;
    timeout = gettime_ms() + SDIO_WAITTIMEOUT;

    while (!sdcard_state.transfer_end &&
            (sdcard_state.transfer_error == SD_ACTIVE) &&
            (gettime_ms() < timeout)){
         usleep(1000);
    }

    sdcard_state.transfer_end = false;

    timeout = gettime_ms() + SDIO_WAITTIMEOUT;

    while((SDIO->STA & io_flag) &&
            (gettime_ms() < timeout)){
         usleep(1000);
    }

    if((gettime_ms() >= timeout) && (sderr == SD_OK))
        sderr = SD_DATA_TIMEOUT;

    sderr = sdcard_state.transfer_error;
#endif

    SDIO_ClearFlag(SDIO_STATIC_FLAGS);

#ifdef DISK_ACTIVITY_LED
    clear_led(DISK_ACTIVITY_LED);
#endif

    return sderr;
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
    SD_Error sderr = SD_OK;
    sdio_send_cmd(0x0, SD_CMD_STOP_TRANSMISSION, SDIO_Response_Short);
    sderr = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
    return sderr;
}

/**
  * @brief  Allows to erase memory area specified for the given card.
  * @param  startaddr: the start sector number.
  * @param  endaddr: the end sector number.
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_Erase(uint32_t startaddr, uint32_t endaddr)
{
    SD_Error sderr = SD_OK;
    uint32_t delay = 0;
    __IO uint32_t maxdelay = 0;
    uint8_t cardstate = 0;

    /*!< Check if the card coomnd class supports erase command */
    if(!sdcard_state.ccc_erase)
    {
        sderr = SD_REQUEST_NOT_APPLICABLE;
        return sderr;
    }

    maxdelay = 120000 / ((SDIO->CLKCR & 0xFF) + 2);

    if(SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
    {
        sderr = SD_LOCK_UNLOCK_FAILED;
        return sderr;
    }

    if(sdcard_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
    {
        startaddr *= SD_SECTOR_SIZE;
        endaddr *= SD_SECTOR_SIZE;
    }

    /*!< According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
    if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == sdcard_state.card_type) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == sdcard_state.card_type) || (SDIO_HIGH_CAPACITY_SD_CARD == sdcard_state.card_type))
    {
        /*!< Send CMD32 SD_ERASE_GRP_START with argument as addr  */
        sdio_send_cmd(startaddr, SD_CMD_SD_ERASE_GRP_START, SDIO_Response_Short);

        sderr = CmdResp1Error(SD_CMD_SD_ERASE_GRP_START);
        if(sderr != SD_OK)
        {
            return sderr;
        }

        /*!< Send CMD33 SD_ERASE_GRP_END with argument as addr  */
        sdio_send_cmd(endaddr, SD_CMD_SD_ERASE_GRP_END, SDIO_Response_Short);

        sderr = CmdResp1Error(SD_CMD_SD_ERASE_GRP_END);
        if(sderr != SD_OK)
        {
            return sderr;
        }
    }

    /*!< Send CMD38 ERASE */
    sdio_send_cmd(0, SD_CMD_ERASE, SDIO_Response_Short);

    sderr = CmdResp1Error(SD_CMD_ERASE);

    if(sderr != SD_OK)
    {
        return sderr;
    }

    for (delay = 0; delay < maxdelay; delay++)
    {}

    /*!< Wait till the card is in programming state */
    sderr = IsCardProgramming(&cardstate);
    delay = SDIO_DATATIMEOUT;
    while ((delay > 0) && (sderr == SD_OK) && ((SD_CARD_PROGRAMMING == cardstate) || (SD_CARD_RECEIVING == cardstate)))
    {
        sderr = IsCardProgramming(&cardstate);
        delay--;
    }

    return sderr;
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

        if(sdcard_state.transfer_multiblock)
            sdcard_state.transfer_error = SD_StopTransfer();
        else
            sdcard_state.transfer_error = SD_OK;
    }

    if(SDIO_GetITStatus(SDIO_IT_DCRCFAIL) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_DCRCFAIL);
        sdcard_state.transfer_error = SD_DATA_CRC_FAIL;
    }

    if(SDIO_GetITStatus(SDIO_IT_DTIMEOUT) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_DTIMEOUT);
        sdcard_state.transfer_error = SD_DATA_TIMEOUT;
    }

    if(SDIO_GetITStatus(SDIO_IT_RXOVERR) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_RXOVERR);
        sdcard_state.transfer_error = SD_RX_OVERRUN;
    }

    if(SDIO_GetITStatus(SDIO_IT_TXUNDERR) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_TXUNDERR);
        sdcard_state.transfer_error = SD_TX_UNDERRUN;
    }

    if(SDIO_GetITStatus(SDIO_IT_STBITERR) != RESET)
    {
        SDIO_ClearITPendingBit(SDIO_IT_STBITERR);
        sdcard_state.transfer_error = SD_START_BIT_ERR;
    }

    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
                SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
                SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);

#if USE_THREAD_AWARE_SDCARD_DRIVER
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t dummy;
    xQueueSendFromISR(sdcard_state.transfer_end, (const void*)&dummy, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
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
    SD_Error sderr = SD_OK;

    if(SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED;

    /*!< If wide bus operation to be enabled */
    if(NewState == ENABLE)
    {
        /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
        sdio_send_cmd(sdcard_state.rca, SD_CMD_APP_CMD, SDIO_Response_Short);
        sderr = CmdResp1Error(SD_CMD_APP_CMD);

        if(sderr != SD_OK)
            return sderr;

        /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
        sdio_send_cmd(0x2, SD_CMD_APP_SD_SET_BUSWIDTH, SDIO_Response_Short);
        sderr = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);

    }   /*!< If wide bus operation to be disabled */
    else
    {
        /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
        sdio_send_cmd(sdcard_state.rca, SD_CMD_APP_CMD, SDIO_Response_Short);
        sderr = CmdResp1Error(SD_CMD_APP_CMD);

        if(sderr != SD_OK)
            return sderr;

        /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
        sdio_send_cmd(0x00, SD_CMD_APP_SD_SET_BUSWIDTH, SDIO_Response_Short);
        sderr = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
    }

    log_edebug(&sdlog, "cmd%d: %s", SD_CMD_APP_SD_SET_BUSWIDTH, sderrstr[sderr]);

    return sderr;
}

/**
  * @brief  Checks if the SD card is in programming state.
  * @param  pstatus: pointer to the variable that will contain the SD card state.
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error IsCardProgramming(uint8_t *pstatus)
{
    SD_Error sderr = SD_OK;
    __IO uint32_t respR1 = 0;
    __IO uint32_t status = 0;

    sdio_send_cmd(sdcard_state.rca, SD_CMD_SEND_STATUS, SDIO_Response_Short);

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
        return sderr;

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

    return sderr;
}

#elif SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SPI

#include "spi.h"

#define R1_RESP_TIMEOUT             250
#define BUSY_RESP_TIMEOUT           250
#define ACTIVE_RESP_TIMEOUT         1000
#define DATA_RESP_TIMEOUT           100
#define TOKEN_RESP_TIMEOUT          100


/**
 * @brief SPI mode commands
 */
#define SD_CMD		0x40
#define SD_ACMD		0x80
#define SD_CMD0		(SD_CMD+0)		//GO_IDLE_STATE
#define SD_CMD1		(SD_CMD+1)		//SEND_OP_COND (MMC)
#define SD_CMD8		(SD_CMD+8)		//SEND_IF_COND
#define SD_CMD9		(SD_CMD+9)		//SEND_CSD
#define SD_CMD10	(SD_CMD+10)		//SEND_CID
#define SD_CMD12	(SD_CMD+12)		//STOP_TRANSMISSION
#define SD_CMD13	(SD_CMD+13)		//SD_STATUS
#define SD_CMD16	(SD_CMD+16)		//SET_BLOCKLEN
#define SD_CMD17	(SD_CMD+17)		//READ_SINGLE_BLOCK
#define SD_CMD18	(SD_CMD+18)		//READ_MULTIPLE_BLOCK
#define SD_CMD23	(SD_CMD+23)		//SET_BLOCK_COUNT (MMC)
#define SD_CMD24	(SD_CMD+24)		//WRITE_BLOCK
#define SD_CMD25	(SD_CMD+25)		//WRITE_MULTIPLE_BLOCK
#define SD_CMD27    (SD_CMD+27)  	//PROG_CSD
#define SD_CMD55	(SD_CMD+55)		//APP_CMD
#define SD_CMD58	(SD_CMD+58)		//READ_OCR
#define SD_ACMD13	(SD_CMD+SD_ACMD+13)	//SD_STATUS (SDC)
#define SD_ACMD23	(SD_CMD+SD_ACMD+23)	//SET_WR_BLK_ERASE_COUNT (SDC)
#define SD_ACMD41	(SD_CMD+SD_ACMD+41)	//SEND_OP_COND (SDC)

/**
 * @brief SPI mode Response R1 Bit coding
 */
#define SD_R1_BYTE			0
#define SD_R1_START			(1<<7)		// Start Bit, Always 0
#define SD_R1_PARAM_ERR		(1<<6)		// Parameter Error
#define SD_R1_ADDR_ERR		(1<<5)		// Address Error
#define SD_R1_ERASE_ERR		(1<<4)		// Erase Sequence Error
#define SD_R1_CRC_ERR		(1<<3)		// CRC Error
#define SD_R1_CMD_ERR		(1<<2)		// Illegal Command
#define SD_R1_ERASE_RST		(1<<1)		// Erase Reset
#define SD_R1_IDLE			(1<<0)		// In Idle State
#define SD_R1_ACTIVE		0			// In active State

/**
 * @brief SPI mode R3 response OCR register
 */
#define SD_R3_OCR_REG_BYTE_COUNT	4		// byte to use in R3
#define SD_R3_OCR_REG_BYTE			0		// byte to use in R3
#define SD_R3_OCR_REG_BIT_CCS		0x40	// card capacity status

/**
 * @brief SPI mode data tokens
 */
#define SD_DATA_START_TOKEN					0xFE
#define SD_DATA_MULTI_WRITE_START_TOKEN		0xFC
#define SD_DATA_MULTI_WRITE_STOP_TOKEN		0xFD
#define SD_DATA_RESPONSE_TOKEN				0x1F
#define SD_DATA_RESPONSE_ACCEPTED			0x05
#define SD_DATA_RESPONSE_CRC_ERR			0x0B
#define SD_DATA_RESPONSE_WRITE_ERR			0x0D
#define SD_CSD_CID_DATA_LENGTH				16
#define SD_DATA_CRC_LENGTH					2

/**
 * @brief the signal sent by the SD card when busy
 */
#define SD_BUSY_SIGNAL	0x00

/**
 * @brief dummy byte.
 */
#define SD_DUMMY_BYTE   0xFF

static void SD_LowLevel_Init();
static void SD_LowLevel_DeInit();
void SD_SetSPIPrescaler(uint16_t presc);
static SD_Error SD_WaitForReady();
static SD_Error SD_WaitForToken(uint8_t token);
static SD_Error SD_WaitForDataResponse();
static SD_Error SD_WaitForR1Response();
static SD_Error SD_SendCmd(uint8_t cmd, uint32_t arg);
static SD_Error SD_ReadDataBlock(uint8_t* data, uint32_t length);
static SD_Error SD_WriteDataBlock(const uint8_t* data, uint32_t length, uint8_t token);

static SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo);


/**
  * @brief  DeInitializes the SD/SD communication.
  * @param  None
  * @retval None
  */
void SD_DeInit(void)
{
   SD_LowLevel_DeInit();
}

/**
  * @brief  Initializes the SD/SD communication.
  * @param  None
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_OK: Sequence succeed
  */
SD_Error SD_Init(SD_CardInfo* cardinfo)
{
    uint32_t timer;
	uint16_t i;
	uint8_t frame[6];
	SD_Error code = SD_UNSUPPORTED_HW;

	sdcard_state.transfer_end = true;

    log_init(&sdlog, "sdio");

    SD_LowLevel_Init();

	// 80 clock cycles with CS high
	spi_deassert_nss(SDCARD_SPI_PERIPHERAL);
	for(i = 0; i < 10; i++)
		spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);

	// assert CS low permanently
	spi_assert_nss(SDCARD_SPI_PERIPHERAL);

	// go to idle state (software reset)
	code = SD_SendCmd(SD_CMD0, 0);

	if(code != SD_IDLE)
		return code;

	// send interface condition (only supported by SDSCV2, SDHC, SDXC)
	code = SD_SendCmd(SD_CMD8, 0x01AA);

	if(code == SD_IDLE)
	{
		// card may be SDHC or SDXC or SDSCV2
		// read end of R7 response
		for(i = 0; i < 4; i++)
			frame[i] = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);

		// check voltage range return value from CMD8
		if((frame[2] != 1) || (frame[3] != 0xAA))
			return SD_INVALID_VOLTRANGE;

		code = SD_IDLE;
		timer = (uint32_t)gettime_ms() + ACTIVE_RESP_TIMEOUT;
		while((code != SD_ACTIVE) && ((uint32_t)gettime_ms() < timer))
			code = SD_SendCmd(SD_ACMD41, 0x40000000);

		if(code != SD_ACTIVE)
			return SD_CMD_RSP_TIMEOUT;

		// send CMD58
		code = SD_SendCmd(SD_CMD58, SD_R1_ACTIVE);
		if(code != SD_ACTIVE)
			return code;

		// get end of R3 response
		for(i = 0; i < SD_R3_OCR_REG_BYTE_COUNT; i++)
			frame[i] = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);

		if(frame[SD_R3_OCR_REG_BYTE] & SD_R3_OCR_REG_BIT_CCS)
			cardinfo->CardType = SDIO_HIGH_CAPACITY_SD_CARD;
		else
			cardinfo->CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0;
	}
	else
	{
		// card may be SDSC or MMC
		code = SD_SendCmd(SD_ACMD41, 0);
		if(code == SD_ACTIVE || code == SD_IDLE)
		{
			cardinfo->CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1;
			frame[0] = SD_ACMD41;
		}
		else
		{
			cardinfo->CardType = SDIO_MULTIMEDIA_CARD;
			frame[0] = SD_CMD1;
		}

		code = SD_IDLE;
		timer = (uint32_t)gettime_ms() + ACTIVE_RESP_TIMEOUT;
		while((code != SD_ACTIVE) && ((uint32_t)gettime_ms() < timer))
			code = SD_SendCmd(frame[0], 0);

		if(code != SD_ACTIVE)
			return SD_CMD_RSP_TIMEOUT;
	}

	if(code == SD_ACTIVE)
	{
		code = SD_SendCmd(SD_CMD16, SD_SECTOR_SIZE);
		if(code == SD_ACTIVE)
			code = SD_GetCardInfo(cardinfo);

		if(code == SD_OK)
		{
//			SD_SetSPIPrescaler(SPI_BaudRatePrescaler_2); // go for fast SPI operation
			sdcard_state.card_type = cardinfo->CardType;
		}
	}

	return code;
}

void SD_LowLevel_Init()
{
	spi_deassert_nss(SDCARD_SPI_PERIPHERAL);
	spi_init(SDCARD_SPI_PERIPHERAL, NULL, true);
}

void SD_LowLevel_DeInit()
{
	spi_deassert_nss(SDCARD_SPI_PERIPHERAL);
}

void SD_SetSPIPrescaler(uint16_t presc)
{
	spi_set_prescaler(SDCARD_SPI_PERIPHERAL, presc);
}

SD_Error SD_PowerON(void)
{
	return SD_OK;
}

SD_Error SD_PowerOFF(void)
{
	return SD_OK;
}

SDTransferState SD_GetTransferState(void)
{
	return SD_TRANSFER_OK;
}

SD_Error SD_Erase(uint32_t startaddr, uint32_t endaddr)
{
	assert_true(0);
	(void)startaddr;
	(void)endaddr;
	return SD_OK;
}

/**
  * @brief  Returns the current card's status.
  * @param  pcardstatus: pointer to the buffer that will contain the SD card
  *         status (Card Status register).
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_QueryStatus(SDCardState* cardstatus)
{
    SD_Error sderr = SD_OK;

    *cardstatus = SD_CARD_TRANSFER;
//
//    sdio_send_cmd(sdcard_state.rca, SD_CMD_SEND_STATUS, SDIO_Response_Short);
//    sderr = CmdResp1Error(SD_CMD_SEND_STATUS);
//
//    if(sderr == SD_OK)
//        *cardstatus = (SDIO_GetResponse(SDIO_RESP1) >> 9)&0x0F;
//    else
//        *cardstatus = SD_CARD_ERROR;
//
//
//    SD_Error SD_SendCmd(SD_CMD13, 0);

    return sderr;
}

SD_Error SD_WaitForReady()
{
    uint8_t response;
    uint32_t timer = (uint32_t)gettime_ms() + BUSY_RESP_TIMEOUT;
    if(!sdcard_state.transfer_end)
    {
        response = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
    	while((response == SD_BUSY_SIGNAL) && ((uint32_t)gettime_ms() < timer))
            response = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
    	if(response == SD_BUSY_SIGNAL)
            return SD_BUSY_TIMEOUT;
    }

    sdcard_state.transfer_end = true;
    return SD_OK;
}

SD_Error SD_WaitForToken(uint8_t token)
{
    uint8_t response = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
    uint32_t timer = (uint32_t)gettime_ms() + TOKEN_RESP_TIMEOUT;

    while((response != token) && ((uint32_t)gettime_ms() < timer))
        response = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);

    if(response != token)
        return SD_BUSY_TIMEOUT;

	return SD_OK;
}

SD_Error SD_WaitForDataResponse()
{
    uint8_t response;
    uint32_t timer = (uint32_t)gettime_ms() + DATA_RESP_TIMEOUT;

    while((uint32_t)gettime_ms() < timer)
    {
        response = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
        if(response & SD_DATA_RESPONSE_ACCEPTED)
            return SD_OK;
        else if(response & SD_DATA_RESPONSE_CRC_ERR)
            return SD_DATA_CRC_FAIL;
        else if(response & SD_DATA_RESPONSE_WRITE_ERR)
        	return SD_DATA_WRITE_ERROR;
    }

	return SD_ERROR;
}

SD_Error SD_WaitForR1Response()
{
    uint32_t timer = (uint32_t)gettime_ms() + R1_RESP_TIMEOUT;
    uint8_t response = SD_R1_START;
    SD_Error err;

    // poll for valid R1 response
    // response is 0 when card is *not* busy
    while((response & SD_R1_START) && ((uint32_t)gettime_ms() < timer))
        response = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);

    if(response & SD_R1_START) // Start Bit, Always 0
        err = SD_START_BIT_ERR;
    else if(response & SD_R1_PARAM_ERR)// Parameter Error
        err = SD_INVALID_PARAMETER;
    else if(response & SD_R1_ADDR_ERR)// Address Error
        err = SD_ADDR_OUT_OF_RANGE;
    else if(response & SD_R1_ERASE_ERR)// Erase Sequence Error
        err = SD_ERASE_SEQ_ERR;
    else if(response & SD_R1_CRC_ERR)// CRC Error
        err = SD_CMD_CRC_FAIL;
    else if(response & SD_R1_CMD_ERR)// Illegal Command
        err = SD_ILLEGAL_CMD;
    else if(response & SD_R1_ERASE_RST)// Erase Reset
        err = SD_ERASE_RESET;
    else if(response & SD_R1_IDLE)// card is OK and In Idle State
        err = SD_IDLE;
    else
        err = SD_ACTIVE;

    return err;
}

/**
  * @param  cmd: The user expected command to send to SD card.
  * @param  arg: The command argument.
  * @param  crc: The CRC.
 * @retval  one of the SD_Error codes.
  */
SD_Error SD_SendCmd(uint8_t cmd, uint32_t arg)
{
	SD_Error err;
	uint8_t frame[6];
	uint16_t i;

	// if the command was specified as ACMD, send CMD55
	if(cmd & SD_ACMD)
	{
		err = SD_SendCmd(SD_CMD55, 0);
		if(err != SD_ACTIVE && err != SD_IDLE)
			return err;
	}

	// set command and argument, strip ACMD flag, set crc
	frame[0] = cmd;
	frame[0] &= ~SD_ACMD;
	frame[1] = (uint8_t)(arg >> 24);
	frame[2] = (uint8_t)(arg >> 16);
	frame[3] = (uint8_t)(arg >> 8);
	frame[4] = (uint8_t)(arg);
	if(cmd == SD_CMD0)
		frame[5] = 0x95; 	// valid only with arg of 0x0000
	else if(cmd == SD_CMD8)
		frame[5] = 0x87; 	// valid only with arg of 0x01AA
	else
		frame[5] = 0x01;		// dummy CRC

	for(i = 0; i < sizeof(frame); i++)
		spi_transfer(SDCARD_SPI_PERIPHERAL, frame[i]);

	// discard CMD12 stuff byte
	if(cmd == SD_CMD12)
		spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);

    // poll for valid R1 response
    err = SD_WaitForR1Response();

    return err;
}

/**
  * @brief  Read the CSD and CID card registers.
  * @param  a pointer to the cardinfo structure.
  * @retval returns one of the SD_Error codes.
  */
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
  	SD_Error err;
  	uint8_t data_table[SD_CSD_CID_DATA_LENGTH];

  	/**
  	 * CSD register
  	 */
  	err = SD_SendCmd(SD_CMD9, 0);
  	if(err != SD_ACTIVE)
  		return err;
  	err = SD_ReadDataBlock(data_table, SD_CSD_CID_DATA_LENGTH);
  	if(err != SD_OK)
  		return err;

  	/*!< Byte 0 */
  	cardinfo->SD_csd.CSDStruct = (data_table[0] & 0xC0) >> 6;
  	cardinfo->SD_csd.SysSpecVersion = (data_table[0] & 0x3C) >> 2;
  	cardinfo->SD_csd.Reserved1 = data_table[0] & 0x03;

  	/*!< Byte 1 */
  	cardinfo->SD_csd.TAAC = data_table[1];

  	/*!< Byte 2 */
  	cardinfo->SD_csd.NSAC = data_table[2];

  	/*!< Byte 3 */
  	cardinfo->SD_csd.MaxBusClkFrec = data_table[3];

  	/*!< Byte 4 */
  	cardinfo->SD_csd.CardComdClasses = data_table[4] << 4;

  	/*!< Byte 5 */
  	cardinfo->SD_csd.CardComdClasses |= (data_table[5] & 0xF0) >> 4;
  	cardinfo->SD_csd.RdBlockLen = data_table[5] & 0x0F;

  	/*!< Byte 6 */
  	cardinfo->SD_csd.PartBlockRead = (data_table[6] & 0x80) >> 7;
  	cardinfo->SD_csd.WrBlockMisalign = (data_table[6] & 0x40) >> 6;
  	cardinfo->SD_csd.RdBlockMisalign = (data_table[6] & 0x20) >> 5;
  	cardinfo->SD_csd.DSRImpl = (data_table[6] & 0x10) >> 4;
  	cardinfo->SD_csd.Reserved2 = 0; /*!< Reserved */

  	if((cardinfo->CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (cardinfo->CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0))
  	{
  		cardinfo->SD_csd.DeviceSize = (data_table[6] & 0x03) << 10;

  		/*!< Byte 7 */
  		cardinfo->SD_csd.DeviceSize |= (data_table[7]) << 2;

  		/*!< Byte 8 */
  		cardinfo->SD_csd.DeviceSize |= (data_table[8] & 0xC0) >> 6;

  		cardinfo->SD_csd.MaxRdCurrentVDDMin = (data_table[8] & 0x38) >> 3;
  		cardinfo->SD_csd.MaxRdCurrentVDDMax = (data_table[8] & 0x07);

  		/*!< Byte 9 */
  		cardinfo->SD_csd.MaxWrCurrentVDDMin = (data_table[9] & 0xE0) >> 5;
  		cardinfo->SD_csd.MaxWrCurrentVDDMax = (data_table[9] & 0x1C) >> 2;
  		cardinfo->SD_csd.DeviceSizeMul = (data_table[9] & 0x03) << 1;

  		/*!< Byte 10 */
  		cardinfo->SD_csd.DeviceSizeMul |= (data_table[10] & 0x80) >> 7;

  	    // card capacity in blocks
  	    cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) ;
  	    cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
  	    /**
  		 * from wikipedia:
  		 * Later versions state (at Section 4.3.2) that a 2 GB SDSC card shall set its READ_BL_LEN (and WRITE_BL_LEN) to indicate 1024 bytes,
  		 * so that the above computation correctly reports the card's capacity; but that, for consistency,
  		 * the host device shall not request (by CMD16) block lengths over 512 bytes
  		 */
  		if(cardinfo->CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0)
  			cardinfo->CardCapacity *= 2;
  	    cardinfo->CardBlockSize = SD_SECTOR_SIZE;
  	}
  	else if(cardinfo->CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  	{
  		/*!< Byte 7 */
  		cardinfo->SD_csd.DeviceSize = (data_table[7] & 0x3F) << 16;

  		/*!< Byte 8 */
  		cardinfo->SD_csd.DeviceSize |= (data_table[8] << 8);

  		/*!< Byte 9 */
  		cardinfo->SD_csd.DeviceSize |= (data_table[9]);

  		/*!< Byte 10 */
  	    // card capacity in blocks
  		cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) * 1024;
  		cardinfo->CardBlockSize = SD_SECTOR_SIZE;
  	}

  	cardinfo->SD_csd.EraseGrSize = (data_table[10] & 0x40) >> 6;
  	cardinfo->SD_csd.EraseGrMul = (data_table[10] & 0x3F) << 1;

  	/*!< Byte 11 */
  	cardinfo->SD_csd.EraseGrMul |= (data_table[11] & 0x80) >> 7;
  	cardinfo->SD_csd.WrProtectGrSize = (data_table[11] & 0x7F);

  	/*!< Byte 12 */
  	cardinfo->SD_csd.WrProtectGrEnable = (data_table[12] & 0x80) >> 7;
  	cardinfo->SD_csd.ManDeflECC = (data_table[12] & 0x60) >> 5;
  	cardinfo->SD_csd.WrSpeedFact = (data_table[12] & 0x1C) >> 2;
  	cardinfo->SD_csd.MaxWrBlockLen = (data_table[12] & 0x03) << 2;

  	/*!< Byte 13 */
  	cardinfo->SD_csd.MaxWrBlockLen |= (data_table[13] & 0xC0) >> 6;
  	cardinfo->SD_csd.WriteBlockPaPartial = (data_table[13] & 0x20) >> 5;
  	cardinfo->SD_csd.Reserved3 = 0;
  	cardinfo->SD_csd.ContentProtectAppli = (data_table[13] & 0x01);

  	/*!< Byte 14 */
  	cardinfo->SD_csd.FileFormatGrouop = (data_table[14] & 0x80) >> 7;
  	cardinfo->SD_csd.CopyFlag = (data_table[14] & 0x40) >> 6;
  	cardinfo->SD_csd.PermWrProtect = (data_table[14] & 0x20) >> 5;
  	cardinfo->SD_csd.TempWrProtect = (data_table[14] & 0x10) >> 4;
  	cardinfo->SD_csd.FileFormat = (data_table[14] & 0x0C) >> 2;
  	cardinfo->SD_csd.ECC = (data_table[14] & 0x03);

  	/*!< Byte 15 */
  	cardinfo->SD_csd.CSD_CRC = (data_table[15] & 0xFE) >> 1;
  	cardinfo->SD_csd.Reserved4 = 1;

  	/**
  	 * CID register
  	 */
  	err = SD_SendCmd(SD_CMD10, 0);
  	if(err != SD_ACTIVE)
  		return err;
  	err = SD_ReadDataBlock(data_table, SD_CSD_CID_DATA_LENGTH);
  	if(err != SD_OK)
  		return err;

	/*!< Byte 0 */
	cardinfo->SD_cid.ManufacturerID = data_table[0];

	/*!< Byte 1 */
	cardinfo->SD_cid.OEM_AppliID = data_table[1] << 8;

	/*!< Byte 2 */
	cardinfo->SD_cid.OEM_AppliID |= data_table[2];

	/*!< Byte 3 */
	cardinfo->SD_cid.ProdName1 = data_table[3] << 24;

	/*!< Byte 4 */
	cardinfo->SD_cid.ProdName1 |= data_table[4] << 16;

	/*!< Byte 5 */
	cardinfo->SD_cid.ProdName1 |= data_table[5] << 8;

	/*!< Byte 6 */
	cardinfo->SD_cid.ProdName1 |= data_table[6];

	/*!< Byte 7 */
	cardinfo->SD_cid.ProdName2 = data_table[7];

	/*!< Byte 8 */
	cardinfo->SD_cid.ProdRev = data_table[8];

	/*!< Byte 9 */
	cardinfo->SD_cid.ProdSN = data_table[9] << 24;

	/*!< Byte 10 */
	cardinfo->SD_cid.ProdSN |= data_table[10] << 16;

	/*!< Byte 11 */
	cardinfo->SD_cid.ProdSN |= data_table[11] << 8;

	/*!< Byte 12 */
	cardinfo->SD_cid.ProdSN |= data_table[12];

	/*!< Byte 13 */
	cardinfo->SD_cid.Reserved1 |= (data_table[13] & 0xF0) >> 4;
	cardinfo->SD_cid.ManufactDate = (data_table[13] & 0x0F) << 8;

	/*!< Byte 14 */
	cardinfo->SD_cid.ManufactDate |= data_table[14];

	/*!< Byte 15 */
	cardinfo->SD_cid.CID_CRC = (data_table[15] & 0xFE) >> 1;
	cardinfo->SD_cid.Reserved2 = 1;

	return err;
}

/**
  * @brief  Reads a block of data from the SD.
  * @param  readbuff: pointer to the buffer that receives the data read from the
  *                  SD.
  * @param  sector: sector number from where data are to be read.
  * @retval  returns SD_OK if successful, or one of the other SD_Error codes if not.
  */
SD_Error SD_ReadBlock(uint8_t *readbuff, uint32_t sector)//uint8_t* pBuffer, uint32_t sector, uint16_t BlockSize)
{
	SD_Error err;
	if(sdcard_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
		sector *= SD_SECTOR_SIZE;

    err = SD_WaitForReady();

    if(err == SD_OK)
    {
    	err = SD_SendCmd(SD_CMD17, sector);
    	if(err == SD_ACTIVE)
    		err = SD_ReadDataBlock(readbuff, SD_SECTOR_SIZE);
    }
	return err;
}

/**
  * @brief  Reads multiple block of data from the SD.
  * @param  readbuff: pointer to the buffer that receives the data read from the
  *                  SD.
  * @param  sector: sector number from where data are to be read.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval  returns SD_OK if successful, or one of the other SD_Error codes if not.
  */
SD_Error SD_ReadMultiBlocks(uint8_t *readbuff, uint32_t sector, uint32_t NumberOfBlocks)//uint8_t* pBuffer, uint32_t sector, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	SD_Error err;
	if(sdcard_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
		sector *= SD_SECTOR_SIZE;

    err = SD_WaitForReady();

    if(err == SD_OK)
    {
    	err = SD_SendCmd(SD_CMD18, sector);

    	if(err == SD_ACTIVE)
    		err = SD_OK;
    	while((err == SD_OK) && NumberOfBlocks--)
    	{
    		err = SD_ReadDataBlock(readbuff, SD_SECTOR_SIZE);
    		readbuff += SD_SECTOR_SIZE;
    	}
        if(err == SD_OK)
            err = SD_SendCmd(SD_CMD12, 0);
    }
	return err;
}

/**
 * reads a data block from the card.
 * waits for the token SD_DATA_START_TOKEN prior to reading data.
 *
 * @param   data is the memory to read into.
 * @param   length is the length of the read in bytes.
 * @retval  returns SD_OK if successful, or SD_CMD_RSP_TIMEOUT if not.
 */
SD_Error SD_ReadDataBlock(uint8_t* data, uint32_t length)
{
    if(SD_WaitForToken(SD_DATA_START_TOKEN) == SD_OK)
    {
        // read block
        while(length--)
        {
            *data = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
            data++;
        }
        // read crc
        spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
        spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
        return SD_OK;
    }
    return SD_CMD_RSP_TIMEOUT;
}

/**
  * @brief  Writes a block on the SD
  * @param  writebuf: pointer to the buffer containing the data to be written on
  *                  the SD.
  * @param  sector: sector number to where data are to be written.
  * @retval  returns SD_OK if successful, or one of the other SD_Error codes if not.
  */
SD_Error SD_WriteBlock(const uint8_t* writebuf, uint32_t sector)//, uint16_t BlockSize)
{
	SD_Error err;
	if(sdcard_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
		sector *= SD_SECTOR_SIZE;

    err = SD_WaitForReady();

    if(err == SD_OK)
    {
    	err = SD_SendCmd(SD_CMD24, sector);
    	if(err == SD_ACTIVE)
        {
            spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
    		err = SD_WriteDataBlock(writebuf, SD_SECTOR_SIZE, SD_DATA_START_TOKEN);
            spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
        }
    }
	return err;
}

/**
  * @brief  Writes many blocks on the SD
  * @param  writebuf: pointer to the buffer containing the data to be written on
  *                  the SD.
  * @param  sector: sector number to where data are to be written.
  * @param  NumberOfBlocks: number of blocks to be written.
  * @retval  returns SD_OK if successful, or one of the other SD_Error codes if not.
  */
SD_Error SD_WriteMultiBlocks(const uint8_t* writebuf, uint32_t sector, uint32_t NumberOfBlocks)
{
	SD_Error err = SD_ACTIVE;

    err = SD_WaitForReady();
    if(err == SD_OK)
    {

        if(sdcard_state.card_type != SDIO_HIGH_CAPACITY_SD_CARD)
            sector *= SD_SECTOR_SIZE;

        // specify pre erase number of blocks
        if(sdcard_state.card_type != SDIO_MULTIMEDIA_CARD)
            err = SD_SendCmd(SD_ACMD23, NumberOfBlocks);
        else
            err = SD_SendCmd(SD_CMD23, NumberOfBlocks);

    	if(err == SD_ACTIVE)
    		err = SD_SendCmd(SD_CMD25, sector);

    	if(err == SD_ACTIVE)
    	{
            spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
    		err = SD_OK;
    		while(err == SD_OK && NumberOfBlocks--)
    		{
                SD_WaitForReady();
    			err = SD_WriteDataBlock(writebuf, SD_SECTOR_SIZE, SD_DATA_MULTI_WRITE_START_TOKEN);
    			writebuf += SD_SECTOR_SIZE;
    			sector++;
    		}
            if(sdcard_state.card_type != SDIO_MULTIMEDIA_CARD)
            {
                SD_WaitForReady();
        		err = SD_WriteDataBlock(NULL, 0, SD_DATA_MULTI_WRITE_STOP_TOKEN);
                spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
            }
    	}
    }

	return err;
}

SD_Error SD_WaitIOOperation(sdio_wait_on_io_t io_flag)
{
	(void)io_flag;
	return SD_OK;
}

/**
 * writes a data block to the card.
 * waits for the busy signal to end before
 * sending a control token, then does the write operation.
 *
 * @param   data is the memory to write from into.
 * @param   length is the length of the write in bytes.
 * @param   token, may be:
 *              SD_DATA_START_TOKEN (write single block)
 *              SD_DATA_MULTI_WRITE_START_TOKEN (write multiple blocks)
 *              SD_DATA_MULTI_WRITE_STOP_TOKEN (stop multiple block write)
 * @retval  returns SD_OK if successful, or one of the other SD_Error codes if not.
 */
SD_Error SD_WriteDataBlock(const uint8_t* data, uint32_t length, uint8_t token)
{
    SD_Error err = SD_OK;

    // send control token
    spi_transfer(SDCARD_SPI_PERIPHERAL, token);
    if(token != SD_DATA_MULTI_WRITE_STOP_TOKEN)
    {
        // send data
        while(length--)
        {
            spi_transfer(SDCARD_SPI_PERIPHERAL, *data);
            data++;
        }
        // send crc
        spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
        spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);

        err = SD_WaitForDataResponse();
    }

    sdcard_state.transfer_end = false;

    return err;
}
#endif // SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SPI

/**
 * @}
 */
