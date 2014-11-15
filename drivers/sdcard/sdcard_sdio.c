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

#include "sdcard.h"

/**
 * @defgroup sd_card_sdio SD Card SDIO
 *
 * This file provides all the SD Card driver firmware functions.
 *
 * @{
 */

/**
  * @brief SDIO Commands  Index
  */
#define SD_CMD_GO_IDLE_STATE                       ((uint8_t)0)
#define SD_CMD_SEND_OP_COND                        ((uint8_t)1)
#define SD_CMD_ALL_SEND_CID                        ((uint8_t)2)
#define SD_CMD_SET_REL_ADDR                        ((uint8_t)3) ///< SDIO_SEND_REL_ADDR for SD Card
#define SD_CMD_SET_DSR                             ((uint8_t)4)
#define SD_CMD_SDIO_SEN_OP_COND                    ((uint8_t)5)
#define SD_CMD_HS_SWITCH                           ((uint8_t)6)
#define SD_CMD_SEL_DESEL_CARD                      ((uint8_t)7)
#define SD_CMD_HS_SEND_EXT_CSD                     ((uint8_t)8)
#define SD_CMD_SEND_CSD                            ((uint8_t)9)
#define SD_CMD_SEND_CID                            ((uint8_t)10)
#define SD_CMD_READ_DAT_UNTIL_STOP                 ((uint8_t)11) ///< SD Card doesn't support it
#define SD_CMD_STOP_TRANSMISSION                   ((uint8_t)12)
#define SD_CMD_SEND_STATUS                         ((uint8_t)13)
#define SD_CMD_HS_BUSTEST_READ                     ((uint8_t)14)
#define SD_CMD_GO_INACTIVE_STATE                   ((uint8_t)15)
#define SD_CMD_SET_BLOCKLEN                        ((uint8_t)16)
#define SD_CMD_READ_SINGLE_BLOCK                   ((uint8_t)17)
#define SD_CMD_READ_MULT_BLOCK                     ((uint8_t)18)
#define SD_CMD_HS_BUSTEST_WRITE                    ((uint8_t)19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP                ((uint8_t)20) ///< SD Card doesn't support it
#define SD_CMD_SET_BLOCK_COUNT                     ((uint8_t)23) ///< SD Card doesn't support it
#define SD_CMD_WRITE_SINGLE_BLOCK                  ((uint8_t)24)
#define SD_CMD_WRITE_MULT_BLOCK                    ((uint8_t)25)
#define SD_CMD_PROG_CID                            ((uint8_t)26) ///< reserved for manufacturers
#define SD_CMD_PROG_CSD                            ((uint8_t)27)
#define SD_CMD_SET_WRITE_PROT                      ((uint8_t)28)
#define SD_CMD_CLR_WRITE_PROT                      ((uint8_t)29)
#define SD_CMD_SEND_WRITE_PROT                     ((uint8_t)30)
#define SD_CMD_SD_ERASE_GRP_START                  ((uint8_t)32) ///< To set the address of the first write block to be erased. (For SD card only)
#define SD_CMD_SD_ERASE_GRP_END                    ((uint8_t)33) ///< To set the address of the last write block of the continuous range to be erased. (For SD card only)
#define SD_CMD_ERASE_GRP_START                     ((uint8_t)35) ///< To set the address of the first write block to be erased. (For MMC card only spec 3.31)
#define SD_CMD_ERASE_GRP_END                       ((uint8_t)36) ///< To set the address of the last write block of the continuous range to be erased. (For MMC card only spec 3.31)
#define SD_CMD_ERASE                               ((uint8_t)38)
#define SD_CMD_FAST_IO                             ((uint8_t)39) ///< SD Card doesn't support it
#define SD_CMD_GO_IRQ_STATE                        ((uint8_t)40) ///< SD Card doesn't support it
#define SD_CMD_LOCK_UNLOCK                         ((uint8_t)42)
#define SD_CMD_APP_CMD                             ((uint8_t)55)
#define SD_CMD_GEN_CMD                             ((uint8_t)56)
#define SD_CMD_NO_CMD                              ((uint8_t)64)

/**
  * @brief Following commands are SD Card Specific commands.
  *        SDIO_APP_CMD should be sent before sending these commands.
  */
#define SD_CMD_APP_SD_SET_BUSWIDTH                 ((uint8_t)6)  ///< For SD Card only
#define SD_CMD_SD_APP_STAUS                        ((uint8_t)13) ///< For SD Card only
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS        ((uint8_t)22) ///< For SD Card only
#define SD_CMD_SD_APP_OP_COND                      ((uint8_t)41) ///< For SD Card only
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT          ((uint8_t)42) ///< For SD Card only
#define SD_CMD_SD_APP_SEND_SCR                     ((uint8_t)51) ///< For SD Card only
#define SD_CMD_SDIO_RW_DIRECT                      ((uint8_t)52) ///< For SD I/O Card only
#define SD_CMD_SDIO_RW_EXTENDED                    ((uint8_t)53) ///< For SD I/O Card only

/**
  * @brief Following commands are SD Card Specific security commands.
  *        SDIO_APP_CMD should be sent before sending these commands.
  */
#define SD_CMD_SD_APP_GET_MKB                      ((uint8_t)43) ///< For SD Card only
#define SD_CMD_SD_APP_GET_MID                      ((uint8_t)44) ///< For SD Card only
#define SD_CMD_SD_APP_SET_CER_RN1                  ((uint8_t)45) ///< For SD Card only
#define SD_CMD_SD_APP_GET_CER_RN2                  ((uint8_t)46) ///< For SD Card only
#define SD_CMD_SD_APP_SET_CER_RES2                 ((uint8_t)47) ///< For SD Card only
#define SD_CMD_SD_APP_GET_CER_RES1                 ((uint8_t)48) ///< For SD Card only
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((uint8_t)18) ///< For SD Card only
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((uint8_t)25) ///< For SD Card only
#define SD_CMD_SD_APP_SECURE_ERASE                 ((uint8_t)38) ///< For SD Card only
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA           ((uint8_t)49) ///< For SD Card only
#define SD_CMD_SD_APP_SECURE_WRITE_MKB             ((uint8_t)48) ///< For SD Card only


#if FAMILY == STM32F1
#define SDIO_FIFO_ADDRESS                ((uint32_t)0x40018080)
/**
  * @brief  SDIO Intialization Frequency (400KHz max)
  */
#define SDIO_INIT_CLK_DIV                ((uint8_t)0xB2)
/**
  * @brief  SDIO Data Transfer Frequency (25MHz max)
  * F_SDIO = 72MHz/(SDIO_TRANSFER_CLK_DIV+2) = 18MHz
  */
#define SDIO_TRANSFER_CLK_DIV            ((uint8_t)0x02)

#define SD_SDIO_DMA_IRQHANDLER          DMA2_Channel4_5_IRQHandler
#define SD_SDIO_DMA_IRQn               DMA2_Channel4_5_IRQn

#elif FAMILY == STM32F4
#define SDIO_FIFO_ADDRESS                ((uint32_t)0x40012C80)
/**
  * @brief  SDIO Intialization Frequency (400KHz max)
  */
#define SDIO_INIT_CLK_DIV                ((uint8_t)0x76)
/**
  * @brief  SDIO Data Transfer Frequency (25MHz max)
  * F_SDIO = 48MHz/(SDIO_TRANSFER_CLK_DIV+2) = 16MHz
  */
#define SDIO_TRANSFER_CLK_DIV            ((uint8_t)0x01)


#define SD_SDIO_DMA_STREAM3           3
//#define SD_SDIO_DMA_STREAM6           6
#ifdef SD_SDIO_DMA_STREAM3
 #define SD_SDIO_DMA_STREAM            DMA2_Stream3
 #define SD_SDIO_DMA_CHANNEL           DMA_Channel_4
 #define SD_SDIO_DMA_FLAG_FEIF         DMA_FLAG_FEIF3
 #define SD_SDIO_DMA_FLAG_DMEIF        DMA_FLAG_DMEIF3
 #define SD_SDIO_DMA_FLAG_TEIF         DMA_FLAG_TEIF3
 #define SD_SDIO_DMA_FLAG_HTIF         DMA_FLAG_HTIF3
 #define SD_SDIO_DMA_FLAG_TCIF         DMA_FLAG_TCIF3
 #define SD_SDIO_DMA_IRQn              DMA2_Stream3_IRQn
 #define SD_SDIO_DMA_IRQHANDLER        DMA2_Stream3_IRQHandler
#elif defined SD_SDIO_DMA_STREAM6
 #define SD_SDIO_DMA_STREAM            DMA2_Stream6
 #define SD_SDIO_DMA_CHANNEL           DMA_Channel_4
 #define SD_SDIO_DMA_FLAG_FEIF         DMA_FLAG_FEIF6
 #define SD_SDIO_DMA_FLAG_DMEIF        DMA_FLAG_DMEIF6
 #define SD_SDIO_DMA_FLAG_TEIF         DMA_FLAG_TEIF6
 #define SD_SDIO_DMA_FLAG_HTIF         DMA_FLAG_HTIF6
 #define SD_SDIO_DMA_FLAG_TCIF         DMA_FLAG_TCIF6
 #define SD_SDIO_DMA_IRQn              DMA2_Stream6_IRQn
 #define SD_SDIO_DMA_IRQHANDLER        DMA2_Stream6_IRQHandler
#endif /* SD_SDIO_DMA_STREAM3 */


#endif


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

/**
  * @brief  Mask for errors Card Status R1 (OCR Register)
  */
#define SD_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE       ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)

/**
  * @brief  Masks for R6 Response
  */
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define SD_STD_CAPACITY                 ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)

#define SD_ALLZERO                      ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)

#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO                     ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES                ((uint32_t)0x00000020)

/**
  * @brief  Command Class Supported
  */
#define SD_CCCC_LOCK_UNLOCK             ((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT              ((uint32_t)0x00000040)
#define SD_CCCC_ERASE                   ((uint32_t)0x00000020)

/**
  * @brief  Following commands are SD Card Specific commands.
  *         SDIO_APP_CMD should be sent before sending these commands.
  */
#define SDIO_SEND_IF_COND               ((uint32_t)0x00000008)

#define SD_CARD_PORT                GPIOC
#define SD_CARD_CK_PIN              GPIO_Pin_12
#define SD_CARD_D0_PIN              GPIO_Pin_8
#define SD_CARD_D1_PIN              GPIO_Pin_9
#define SD_CARD_D2_PIN              GPIO_Pin_10
#define SD_CARD_D3_PIN              GPIO_Pin_11
#define SD_CARD_CK_PINSOURCE        GPIO_PinSource12
#define SD_CARD_D0_PINSOURCE        GPIO_PinSource8
#define SD_CARD_D1_PINSOURCE        GPIO_PinSource9
#define SD_CARD_D2_PINSOURCE        GPIO_PinSource10
#define SD_CARD_D3_PINSOURCE        GPIO_PinSource11
#define SD_CARD_CMD_PORT            GPIOD
#define SD_CARD_CMD_PIN             GPIO_Pin_2
#define SD_CARD_CMD_PINSOURCE       GPIO_PinSource2


typedef struct {
#if USE_THREAD_AWARE_SDIO
    QueueHandle_t wait_on_io;
#else
    bool dma_xfer_end;
#endif
    SD_Error sdio_xfer_error;
    bool sdio_xfer_multi_block;
    uint8_t card_type;
    uint8_t rca;
    uint32_t csd_table[4];
    uint32_t cid_table[4];
}sdio_state_t;

volatile sdio_state_t sdio_state = {
#if USE_THREAD_AWARE_SDIO
    .wait_on_io = NULL,
#else
    .dma_xfer_end = false,
#endif
    .sdio_xfer_error = SD_OK,
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
    SD_NVIC_Configuration();
    SD_IO_Init();
    errorstatus = SD_PowerON();

#if USE_THREAD_AWARE_SDIO
    sdio_state.wait_on_io = xQueueCreate(2, 1);
    assert_true(sdio_state.wait_on_io);
#endif

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
#if USE_THREAD_AWARE_SDIO
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        uint8_t dummy;
        xQueueSendFromISR(sdio_state.wait_on_io, (const void*)&dummy, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else
        sdio_state.dma_xfer_end = true;
#endif
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
#if USE_THREAD_AWARE_SDIO
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        uint8_t dummy;
        xQueueSendFromISR(sdio_state.wait_on_io, (const void*)&dummy, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#else
        sdio_state.dma_xfer_end = true;
#endif
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
  * @brief  Enquires cards about their operating voltage and configures
  *   clock controls.
  *
  * @retval SD_Error: SD Card Error code.
  */
SD_Error SD_PowerON(void)
{
    SD_Error errorstatus = SD_OK;
    uint32_t timeout;

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

    if(errorstatus == SD_OK)
        sdio_state.card_type = SDIO_STD_CAPACITY_SD_CARD_V2_0;
    else
    {
        sdio_state.card_type = SDIO_STD_CAPACITY_SD_CARD_V1_1;
        /*!< CMD55 */
        sdio_send_cmd(0x00, SD_CMD_APP_CMD, SDIO_Response_Short);
        errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
    }
    /*!< CMD55 */
    sdio_send_cmd(0x00, SD_CMD_APP_CMD, SDIO_Response_Short);
    errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

    /*!< If errorstatus is Command TimeOut, it is a MMC card */
    /*!< If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch)
    or SD card 1.x */
    if(errorstatus == SD_OK)
    {
        timeout = xTaskGetTickCount() + 2000;
        /*!< Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
        while ((!(SDIO_GetResponse(SDIO_RESP1) & (1<<31))) && (xTaskGetTickCount() < timeout))
        {
            /*!< SEND CMD55 APP_CMD with RCA as 0 */
            sdio_send_cmd(0x00, SD_CMD_APP_CMD, SDIO_Response_Short);
            errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
            if(errorstatus != SD_OK)
                return errorstatus;

            sdio_send_cmd(SD_VOLTAGE_WINDOW_SD | SD_HIGH_CAPACITY, SD_CMD_SD_APP_OP_COND, SDIO_Response_Short);
            errorstatus = CmdResp3Error();
            if(errorstatus != SD_OK)
                return errorstatus;
        }
        if(xTaskGetTickCount() >= timeout)
        {
            errorstatus = SD_INVALID_VOLTRANGE;
            return errorstatus;
        }

        if(SDIO_GetResponse(SDIO_RESP1) & SD_HIGH_CAPACITY)
            sdio_state.card_type = SDIO_HIGH_CAPACITY_SD_CARD;
    }

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

#if USE_THREAD_AWARE_SDIO
    (void)io_flag;
    uint8_t dummy;
    // wait for DMA end
    xQueueReceive(sdio_state.wait_on_io, &dummy, 5000/portTICK_PERIOD_MS);
    // wait for SDIO end
    xQueueReceive(sdio_state.wait_on_io, &dummy, 5000/portTICK_PERIOD_MS);
    errorstatus = sdio_state.sdio_xfer_error;
#else
    uint32_t timeout;
    timeout = xTaskGetTickCount() + SDIO_WAITTIMEOUT;

    while (!sdio_state.dma_xfer_end &&
            (sdio_state.sdio_xfer_error == SD_ACTIVE) &&
            (xTaskGetTickCount() < timeout));

    sdio_state.dma_xfer_end = false;

    timeout = xTaskGetTickCount() + SDIO_WAITTIMEOUT;

    while((SDIO->STA & io_flag) && (xTaskGetTickCount() < timeout));

    if((xTaskGetTickCount() >= timeout) && (errorstatus == SD_OK))
        errorstatus = SD_DATA_TIMEOUT;

    if(sdio_state.sdio_xfer_error != SD_OK)
        errorstatus = sdio_state.sdio_xfer_error;
#endif

    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
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

#if USE_THREAD_AWARE_SDIO
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t dummy;
    xQueueSendFromISR(sdio_state.wait_on_io, (const void*)&dummy, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
}

/**
  * @brief  Checks for error conditions for CMD0.
  *
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdError(void)
{
    uint32_t timeout = xTaskGetTickCount() + SDIO_CMD0TIMEOUT;

    while((xTaskGetTickCount() < timeout) &&
          (SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) == RESET));

    if(xTaskGetTickCount() >= timeout)
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
    uint32_t timeout = xTaskGetTickCount() + SDIO_CMD0TIMEOUT;

    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)) &&
            (xTaskGetTickCount() < timeout))
    {
        status = SDIO->STA;
    }

    if(status & SDIO_FLAG_CMDREND)
    {
        /*!< Card is SD V2.0 compliant */
        SDIO_ClearFlag(SDIO_FLAG_CMDREND);
        return SD_OK;
    }

    if((xTaskGetTickCount() >= timeout) || (status & SDIO_FLAG_CTIMEOUT))
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

    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
        status = SDIO->STA;

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

    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
        status = SDIO->STA;

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

    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
        status = SDIO->STA;

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

    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
        status = SDIO->STA;

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
    while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
        status = SDIO->STA;

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
