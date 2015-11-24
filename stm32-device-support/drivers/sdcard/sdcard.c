///*
// * Copyright (c) 2015 Michael Stuart.
// * All rights reserved.
// *
// * Redistribution and use in source and binary forms, with or without modification,
// * are permitted provided that the following conditions are met:
// *
// * 1. Redistributions of source code must retain the above copyright notice,
// *    this list of conditions and the following disclaimer.
// * 2. Redistributions in binary form must reproduce the above copyright notice,
// *    this list of conditions and the following disclaimer in the documentation
// *    and/or other materials provided with the distribution.
// * 3. The name of the author may not be used to endorse or promote products
// *    derived from this software without specific prior written permission.
// *
// * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
// * OF SUCH DAMAGE.
// *
// * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
// *
// * Author: Michael Stuart <spaceorbot@gmail.com>
// *
// */
//
///**
// * @addtogroup sdfs
// *
// * SD Card SDIO driver.
// *
// * This is a heavily modified version of the STM32 SD Card drivers for STM32F1/F4,
// * merged and modified for brevity and slight performance improvement.
// *
// * - supports DMA mode only
// * - does use interrupts, but only to set flags used for polled checking at this time
// * - supports 512 byte blocksize only
// * - SDC MMC card support untested
// * - SDC V1, V2.x, SDHC tested
// * - loosely emulates the original API by ST, with some modifications
// *
// * @file
// * @{
// */
//

#include "board_config.h"
#include "sdcard.h"

#if USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// TODO - add queue logic to makew driver block the calling thread while performing IO
#endif


#if SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_4BIT || SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_1BIT

static SD_HandleTypeDef uSdHandle;

static void sd_gpio_init(void);
static void sd_gpio_deinit(void);
static void sd_hardware_init(SD_HandleTypeDef *hsd);
static void sd_hardware_deinit(SD_HandleTypeDef *hsd);


HAL_SD_ErrorTypedef sd_init(HAL_SD_CardInfoTypedef* uSdCardInfo)
{
	HAL_SD_ErrorTypedef sd_state;

	uSdHandle.Instance = SDIO;
	uSdHandle.Init.ClockEdge           = SDIO_CLOCK_EDGE_RISING;
	uSdHandle.Init.ClockBypass         = SDIO_CLOCK_BYPASS_DISABLE;
	uSdHandle.Init.ClockPowerSave      = SDIO_CLOCK_POWER_SAVE_DISABLE;
	uSdHandle.Init.BusWide             = SDIO_BUS_WIDE_1B;
	uSdHandle.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE;
	uSdHandle.Init.ClockDiv            = SDIO_TRANSFER_CLK_DIV;

	sd_gpio_init();

	if(sd_detect() != SD_PRESENT)
		return SD_ERROR;

	sd_hardware_init(&uSdHandle);

	sd_state = HAL_SD_Init(&uSdHandle, uSdCardInfo);

	if(sd_state == SD_OK)
	{
#if SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_4BIT
		sd_state = HAL_SD_WideBusOperation_Config(&uSdHandle, SDIO_BUS_WIDE_4B);
#elif SDCARD_DRIVER_MODE == SDCARD_DRIVER_MODE_SDIO_1BIT
		sd_state = HAL_SD_WideBusOperation_Config(&uSdHandle, SDIO_BUS_WIDE_1B);
#endif
	}

	return sd_state;
}

HAL_SD_ErrorTypedef sd_deinit(void)
{
	HAL_SD_ErrorTypedef sd_state;

	uSdHandle.Instance = SDIO;

	sd_state = HAL_SD_DeInit(&uSdHandle);

	uSdHandle.Instance = SDIO;
	sd_hardware_deinit(&uSdHandle);
	sd_gpio_deinit();

	return  sd_state;
}

/**
  * @brief  Initializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
void sd_hardware_init(SD_HandleTypeDef *hsd)
{
	static DMA_HandleTypeDef dma_rx_handle;
	static DMA_HandleTypeDef dma_tx_handle;

	/* Enable SDIO clock */
	__HAL_RCC_SDIO_CLK_ENABLE();

	/* Enable DMA2 clocks */
	__DMAx_TxRx_CLK_ENABLE();

	/* NVIC configuration for SDIO interrupts */
	HAL_NVIC_SetPriority(SDIO_IRQn, SDCARD_IT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(SDIO_IRQn);

	/* Configure DMA Rx parameters */
	dma_rx_handle.Init.Channel             = SD_DMAx_Rx_CHANNEL;
	dma_rx_handle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	dma_rx_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
	dma_rx_handle.Init.MemInc              = DMA_MINC_ENABLE;
	dma_rx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	dma_rx_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
	dma_rx_handle.Init.Mode                = DMA_PFCTRL;
	dma_rx_handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
	dma_rx_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
	dma_rx_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	dma_rx_handle.Init.MemBurst            = DMA_MBURST_INC4;
	dma_rx_handle.Init.PeriphBurst         = DMA_PBURST_INC4;

	dma_rx_handle.Instance = SD_DMAx_Rx_STREAM;

	/* Associate the DMA handle */
	__HAL_LINKDMA(hsd, hdmarx, dma_rx_handle);

	/* Deinitialize the stream for new transfer */
	HAL_DMA_DeInit(&dma_rx_handle);

	/* Configure the DMA stream */
	HAL_DMA_Init(&dma_rx_handle);

	/* Configure DMA Tx parameters */
	dma_tx_handle.Init.Channel             = SD_DMAx_Tx_CHANNEL;
	dma_tx_handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	dma_tx_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
	dma_tx_handle.Init.MemInc              = DMA_MINC_ENABLE;
	dma_tx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	dma_tx_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
	dma_tx_handle.Init.Mode                = DMA_PFCTRL;
	dma_tx_handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
	dma_tx_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
	dma_tx_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	dma_tx_handle.Init.MemBurst            = DMA_MBURST_INC4;
	dma_tx_handle.Init.PeriphBurst         = DMA_PBURST_INC4;

	dma_tx_handle.Instance = SD_DMAx_Tx_STREAM;

	/* Associate the DMA handle */
	__HAL_LINKDMA(hsd, hdmatx, dma_tx_handle);

	/* Deinitialize the stream for new transfer */
	HAL_DMA_DeInit(&dma_tx_handle);

	/* Configure the DMA stream */
	HAL_DMA_Init(&dma_tx_handle);

	/* NVIC configuration for DMA transfer complete interrupt */
	HAL_NVIC_SetPriority(SD_DMAx_Rx_IRQn, SDCARD_IT_PRIORITY+1, 0);
	HAL_NVIC_EnableIRQ(SD_DMAx_Rx_IRQn);

	/* NVIC configuration for DMA transfer complete interrupt */
	HAL_NVIC_SetPriority(SD_DMAx_Tx_IRQn, SDCARD_IT_PRIORITY+1, 0);
	HAL_NVIC_EnableIRQ(SD_DMAx_Tx_IRQn);
}

void sd_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;

#if defined(SD_CARD_PRES_PIN) && defined(SD_CARD_PRES_PORT)
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin = SD_CARD_PRES_PIN;
	HAL_GPIO_Init(SD_CARD_PRES_PORT, &GPIO_InitStructure);
#elif defined(SD_CARD_NPRES_PIN) && defined(SD_CARD_NPRES_PORT)
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Pin = SD_CARD_NPRES_PIN;
	HAL_GPIO_Init(SD_CARD_NPRES_PORT, &GPIO_InitStructure);
#endif

#if defined(SD_CARD_WP_PORT)
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin = SD_CARD_WP_PIN;
	HAL_GPIO_Init(SD_CARD_WP_PORT, &GPIO_InitStructure);
#elif defined(SD_CARD_NWP_PORT)
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Pin = SD_CARD_NWP_PIN;
	HAL_GPIO_Init(SD_CARD_NWP_PORT, &GPIO_InitStructure);
#endif

	GPIO_InitStructure.Pin =  SD_CARD_CK_PIN | SD_CARD_D0_PIN | SD_CARD_D1_PIN | SD_CARD_D2_PIN | SD_CARD_D3_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
#if FAMILY == STM32F4
	GPIO_InitStructure.Alternate = GPIO_AF12_SDIO;
#endif
	HAL_GPIO_Init(SD_CARD_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.Pin =  SD_CARD_CMD_PIN;
	HAL_GPIO_Init(SD_CARD_CMD_PORT, &GPIO_InitStructure);

//	/* Enable and set SD detect EXTI Interrupt to the lowest priority */
//	HAL_NVIC_SetPriority((IRQn_Type)(SD_DETECT_EXTI_IRQn), 0x0F, 0x00);
//	HAL_NVIC_EnableIRQ((IRQn_Type)(SD_DETECT_EXTI_IRQn));
}

/**
  * @brief  DeInitializes the SDIO interface.
  */
void sd_gpio_deinit(void)
{
    HAL_GPIO_DeInit(SD_CARD_PORT, SD_CARD_CK_PIN | SD_CARD_D0_PIN | SD_CARD_D1_PIN | SD_CARD_D2_PIN | SD_CARD_D3_PIN);
    HAL_GPIO_DeInit(SD_CARD_CMD_PORT, SD_CARD_CMD_PIN);
}

void sd_hardware_deinit(SD_HandleTypeDef *hsd)
{
    static DMA_HandleTypeDef dma_rx_handle;
    static DMA_HandleTypeDef dma_tx_handle;

    /* Disable NVIC for DMA transfer complete interrupts */
    HAL_NVIC_DisableIRQ(SD_DMAx_Rx_IRQn);
    HAL_NVIC_DisableIRQ(SD_DMAx_Tx_IRQn);

    /* Deinitialize the stream for new transfer */
    dma_rx_handle.Instance = SD_DMAx_Rx_STREAM;
    HAL_DMA_DeInit(&dma_rx_handle);

    /* Deinitialize the stream for new transfer */
    dma_tx_handle.Instance = SD_DMAx_Tx_STREAM;
    HAL_DMA_DeInit(&dma_tx_handle);

    /* Disable NVIC for SDIO interrupts */
    HAL_NVIC_DisableIRQ(SDIO_IRQn);

    /* Disable SDIO clock */
    __HAL_RCC_SDIO_CLK_DISABLE();
}

uint8_t sd_detect(void)
{
#if defined(SD_CARD_PRES_PORT)
    if (HAL_GPIO_ReadPin(SD_CARD_PRES_PORT, SD_CARD_PRES_PIN))
        return SD_PRESENT;
    return SD_NOT_PRESENT;
#elif defined(SD_CARD_NPRES_PORT)
    if (HAL_GPIO_ReadPin(SD_CARD_NPRES_PORT, SD_CARD_NPRES_PIN))
        return SD_NOT_PRESENT;
    return SD_PRESENT;
#else
    return SD_PRESENT;
#endif
}

uint8_t sd_write_protected(void)
{
#if defined(SD_CARD_WP_PORT)
   if(HAL_GPIO_ReadPin(SD_CARD_WP_PORT, SD_CARD_WP_PIN))
       return SD_WRITE_PROTECTED;
   return SD_NOT_WRITE_PROTECTED;
#elif defined(SD_CARD_NWP_PORT)
   if(HAL_GPIO_ReadPin(SD_CARD_NWP_PORT, SD_CARD_NWP_PIN))
       return SD_NOT_WRITE_PROTECTED;
   return SD_WRITE_PROTECTED;
#else
   return SD_NOT_WRITE_PROTECTED;
#endif
}

void SD_IRQHandler(void)
{
  HAL_SD_IRQHandler(&uSdHandle);
}

void SD_DMA_Tx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(uSdHandle.hdmatx);
}

void SD_DMA_Rx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(uSdHandle.hdmarx);
}


void HAL_SD_XferCpltCallback(SD_HandleTypeDef *hsd)
{
//	printf("HAL_SD_XferCpltCallback\n");
}

void HAL_SD_XferErrorCallback(SD_HandleTypeDef *hsd)
{
//	printf("HAL_SD_XferErrorCallback\n");
}

void HAL_SD_DMA_RxCpltCallback(DMA_HandleTypeDef *hdma)
{
//	printf("HAL_SD_DMA_RxCpltCallback\n");
}

void HAL_SD_DMA_RxErrorCallback(DMA_HandleTypeDef *hdma)
{
//	printf("HAL_SD_DMA_RxErrorCallback\n");
}

void HAL_SD_DMA_TxCpltCallback(DMA_HandleTypeDef *hdma)
{
//	printf("HAL_SD_DMA_TxCpltCallback\n");
}


void HAL_SD_DMA_TxErrorCallback(DMA_HandleTypeDef *hdma)
{
//	printf("HAL_SD_DMA_TxErrorCallback\n");
}


HAL_SD_TransferStateTypedef sd_get_transfer_state(void)
{
	return HAL_SD_GetStatus(&uSdHandle);
}

void sd_get_card_info(HAL_SD_CardInfoTypedef *CardInfo)
{
	/* Get SD card Information */
	HAL_SD_Get_CardInfo(&uSdHandle, CardInfo);
}

HAL_SD_ErrorTypedef sd_read(uint8_t *pData, uint32_t sector, uint32_t sectors)
{
	HAL_SD_ErrorTypedef sd_state = HAL_SD_ReadBlocks_DMA(&uSdHandle, (uint32_t*)pData, (uint64_t)(sector * SD_SECTOR_SIZE), SD_SECTOR_SIZE, sectors);

	if(sd_state == SD_OK)
		sd_state = HAL_SD_CheckReadOperation(&uSdHandle, (uint32_t)SD_DATATIMEOUT);

	return sd_state;
}

HAL_SD_ErrorTypedef sd_write(uint8_t *pData, uint32_t sector, uint32_t sectors)
{
	HAL_SD_ErrorTypedef sd_state = HAL_SD_WriteBlocks_DMA(&uSdHandle, (uint32_t*)pData, (uint64_t)(sector * SD_SECTOR_SIZE), SD_SECTOR_SIZE, sectors);

	if(sd_state == SD_OK)
		sd_state = HAL_SD_CheckWriteOperation(&uSdHandle, (uint32_t)SD_DATATIMEOUT);

	return sd_state;
}

HAL_SD_ErrorTypedef sd_erase(uint32_t startsector, uint32_t endsector)
{
	return HAL_SD_Erase(&uSdHandle, startsector, endsector);
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
static HAL_SD_ErrorTypedef SD_WaitForReady();
static HAL_SD_ErrorTypedef SD_WaitForToken(uint8_t token);
static HAL_SD_ErrorTypedef SD_WaitForDataResponse();
static HAL_SD_ErrorTypedef SD_WaitForR1Response();
static HAL_SD_ErrorTypedef SD_SendCmd(uint8_t cmd, uint32_t arg);
static HAL_SD_ErrorTypedef SD_ReadDataBlock(uint8_t* data, uint32_t length);
static HAL_SD_ErrorTypedef SD_WriteDataBlock(const uint8_t* data, uint32_t length, uint8_t token);

static HAL_SD_ErrorTypedef SD_GetCardInfo(SD_CardInfo *cardinfo);


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
HAL_SD_ErrorTypedef SD_Init(SD_CardInfo* cardinfo)
{
    uint32_t timer;
	uint16_t i;
	uint8_t frame[6];
	HAL_SD_ErrorTypedef code = SD_UNSUPPORTED_HW;

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
			spi_set_baudrate(SDCARD_SPI_PERIPHERAL, SDCARD_SPI_FULL_BAUDRATE);
			sdcard_state.card_type = cardinfo->CardType;
		}
	}

	return code;
}

void SD_LowLevel_Init()
{
	spi_deassert_nss(SDCARD_SPI_PERIPHERAL);
	spi_init(SDCARD_SPI_PERIPHERAL, NULL, true);
	spi_set_baudrate(SDCARD_SPI_PERIPHERAL, SDCARD_SPI_INIT_BAUDRATE);
}

void SD_LowLevel_DeInit()
{
	spi_deassert_nss(SDCARD_SPI_PERIPHERAL);
}

HAL_SD_ErrorTypedef SD_PowerON(void)
{
	return SD_OK;
}

HAL_SD_ErrorTypedef SD_PowerOFF(void)
{
	return SD_OK;
}

HAL_SD_TransferStateTypedef SD_GetTransferState(void)
{
	return SD_TRANSFER_OK;
}

HAL_SD_ErrorTypedef SD_Erase(uint32_t startaddr, uint32_t endaddr)
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
  * @retval HAL_SD_ErrorTypedef: SD Card Error code.
  */
HAL_SD_ErrorTypedef SD_QueryStatus(HAL_SD_CardStateTypedef* cardstatus)
{
    HAL_SD_ErrorTypedef sderr = SD_OK;

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
//    HAL_SD_ErrorTypedef SD_SendCmd(SD_CMD13, 0);

    return sderr;
}

HAL_SD_ErrorTypedef SD_WaitForReady()
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

HAL_SD_ErrorTypedef SD_WaitForToken(uint8_t token)
{
    uint8_t response = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);
    uint32_t timer = (uint32_t)gettime_ms() + TOKEN_RESP_TIMEOUT;

    while((response != token) && ((uint32_t)gettime_ms() < timer))
        response = spi_transfer(SDCARD_SPI_PERIPHERAL, SD_DUMMY_BYTE);

    if(response != token)
        return SD_BUSY_TIMEOUT;

	return SD_OK;
}

HAL_SD_ErrorTypedef SD_WaitForDataResponse()
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

HAL_SD_ErrorTypedef SD_WaitForR1Response()
{
    uint32_t timer = (uint32_t)gettime_ms() + R1_RESP_TIMEOUT;
    uint8_t response = SD_R1_START;
    HAL_SD_ErrorTypedef err;

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
 * @retval  one of the HAL_SD_ErrorTypedef codes.
  */
HAL_SD_ErrorTypedef SD_SendCmd(uint8_t cmd, uint32_t arg)
{
	HAL_SD_ErrorTypedef err;
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
  * @retval returns one of the HAL_SD_ErrorTypedef codes.
  */
HAL_SD_ErrorTypedef SD_GetCardInfo(SD_CardInfo *cardinfo)
{
  	HAL_SD_ErrorTypedef err;
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
  * @retval  returns SD_OK if successful, or one of the other HAL_SD_ErrorTypedef codes if not.
  */
HAL_SD_ErrorTypedef SD_ReadBlock(uint8_t *readbuff, uint32_t sector)//uint8_t* pBuffer, uint32_t sector, uint16_t BlockSize)
{
	HAL_SD_ErrorTypedef err;
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
  * @retval  returns SD_OK if successful, or one of the other HAL_SD_ErrorTypedef codes if not.
  */
HAL_SD_ErrorTypedef SD_ReadMultiBlocks(uint8_t *readbuff, uint32_t sector, uint32_t NumberOfBlocks)//uint8_t* pBuffer, uint32_t sector, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	HAL_SD_ErrorTypedef err;
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
HAL_SD_ErrorTypedef SD_ReadDataBlock(uint8_t* data, uint32_t length)
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
  * @retval  returns SD_OK if successful, or one of the other HAL_SD_ErrorTypedef codes if not.
  */
HAL_SD_ErrorTypedef SD_WriteBlock(const uint8_t* writebuf, uint32_t sector)//, uint16_t BlockSize)
{
	HAL_SD_ErrorTypedef err;
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
  * @retval  returns SD_OK if successful, or one of the other HAL_SD_ErrorTypedef codes if not.
  */
HAL_SD_ErrorTypedef SD_WriteMultiBlocks(const uint8_t* writebuf, uint32_t sector, uint32_t NumberOfBlocks)
{
	HAL_SD_ErrorTypedef err = SD_ACTIVE;

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

HAL_SD_ErrorTypedef SD_WaitIOOperation(sdio_wait_on_io_t io_flag)
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
 * @retval  returns SD_OK if successful, or one of the other HAL_SD_ErrorTypedef codes if not.
 */
HAL_SD_ErrorTypedef SD_WriteDataBlock(const uint8_t* data, uint32_t length, uint8_t token)
{
    HAL_SD_ErrorTypedef err = SD_OK;

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

