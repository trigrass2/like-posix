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
 * @file sdcard.h
 * @{
 */
#ifndef SDCARD_TYPE_H_
#define SDCARD_TYPE_H_

#include <stdint.h>
#include <stdbool.h>
#include "board_config.h"

// typedef enum
// {
// /**
//   * @brief  SDIO specific error defines
//   */
//   SD_CMD_CRC_FAIL                    = (1), ///< Command response received (but CRC check failed)
//   SD_DATA_CRC_FAIL                   = (2), ///< Data bock sent/received (CRC check Failed)
//   SD_CMD_RSP_TIMEOUT                 = (3), ///< Command response timeout
//   SD_DATA_TIMEOUT                    = (4), ///< Data time out
//   SD_TX_UNDERRUN                     = (5), ///< Transmit FIFO under-run
//   SD_RX_OVERRUN                      = (6), ///< Receive FIFO over-run
//   SD_START_BIT_ERR                   = (7), ///< Start bit not detected on all data signals in widE bus mode
//   SD_CMD_OUT_OF_RANGE                = (8), ///< CMD's argument was out of range.
//   SD_ADDR_MISALIGNED                 = (9), ///< Misaligned address
//   SD_BLOCK_LEN_ERR                   = (10), ///< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length
//   SD_ERASE_SEQ_ERR                   = (11), ///< An error in the sequence of erase command occurs.
//   SD_BAD_ERASE_PARAM                 = (12), ///< An Invalid selection for erase groups
//   SD_WRITE_PROT_VIOLATION            = (13), ///< Attempt to program a write protect block
//   SD_LOCK_UNLOCK_FAILED              = (14), ///< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card
//   SD_COM_CRC_FAILED                  = (15), ///< CRC check of the previous command failed
//   SD_ILLEGAL_CMD                     = (16), ///< Command is not legal for the card state
//   SD_CARD_ECC_FAILED                 = (17), ///< Card internal ECC was applied but failed to correct the data
//   SD_CC_ERROR                        = (18), ///< Internal card controller error
//   SD_GENERAL_UNKNOWN_ERROR           = (19), ///< General or Unknown error
//   SD_STREAM_READ_UNDERRUN            = (20), ///< The card could not sustain data transfer in stream read operation.
//   SD_STREAM_WRITE_OVERRUN            = (21), ///< The card could not sustain data programming in stream mode
//   SD_CID_CSD_OVERWRITE               = (22), ///< CID/CSD overwrite error
//   SD_WP_ERASE_SKIP                   = (23), ///< only partial address space was erased
//   SD_CARD_ECC_DISABLED               = (24), ///< Command has been executed without using internal ECC
//   SD_ERASE_RESET                     = (25), ///< Erase sequence was cleared before executing because an out of erase sequence command was received
//   SD_AKE_SEQ_ERROR                   = (26), ///< Error in sequence of authentication.
//   SD_INVALID_VOLTRANGE               = (27),
//   SD_ADDR_OUT_OF_RANGE               = (28),
//   SD_SWITCH_ERROR                    = (29),
//   SD_SDIO_DISABLED                   = (30),
//   SD_SDIO_FUNCTION_BUSY              = (31),
//   SD_SDIO_FUNCTION_FAILED            = (32),
//   SD_SDIO_UNKNOWN_FUNCTION           = (33),
//
// /**
//   * @brief  Standard error defines
//   */
//   SD_INTERNAL_ERROR                  = (34),
//   SD_NOT_CONFIGURED                  = (35),
//   SD_REQUEST_PENDING                 = (36),
//   SD_REQUEST_NOT_APPLICABLE          = (37),
//   SD_INVALID_PARAMETER               = (38),
//   SD_UNSUPPORTED_FEATURE             = (39),
//   SD_UNSUPPORTED_HW                  = (40),
//   SD_ERROR                           = (41),
//   SD_OK                              = (42),
//   SD_ACTIVE                          = (43),
//   SD_IDLE                            = (44),
//   SD_DATA_WRITE_ERROR                = (45),
//   SD_DATA_READ_ERROR                 = (46),
//   SD_BUSY_TIMEOUT                    = (47),
//   SD_DMA_FIFO_ERROR                  = (48),
//   SD_DMA_TRANSMISSION_ERROR          = (49),
// } SD_Error;

///**
//  * @brief  SDIO Transfer state
//  */
//typedef enum
//{
//  SD_TRANSFER_OK  = 0,
//  SD_TRANSFER_BUSY = 1,
//  SD_TRANSFER_ERROR
//} SDTransferState;

///**
//  * @brief  SD Card States
//  */
//typedef enum
//{
//  SD_CARD_READY                  = ((uint32_t)0x00000001),
//  SD_CARD_IDENTIFICATION         = ((uint32_t)0x00000002),
//  SD_CARD_STANDBY                = ((uint32_t)0x00000003),
//  SD_CARD_TRANSFER               = ((uint32_t)0x00000004),
//  SD_CARD_SENDING                = ((uint32_t)0x00000005),
//  SD_CARD_RECEIVING              = ((uint32_t)0x00000006),
//  SD_CARD_PROGRAMMING            = ((uint32_t)0x00000007),
//  SD_CARD_DISCONNECTED           = ((uint32_t)0x00000008),
//  SD_CARD_ERROR                  = ((uint32_t)0x000000FF)
//}SDCardState;

/**
  * @brief  Card Specific Data: CSD Register
  */
typedef struct
{
  uint8_t  CSDStruct;            ///< CSD structure
  uint8_t  SysSpecVersion;       ///< System specification version
  uint8_t  Reserved1;            ///< Reserved
  uint8_t  TAAC;                 ///< Data read access-time 1
  uint8_t  NSAC;                 ///< Data read access-time 2 in CLK cycles
  uint8_t  MaxBusClkFrec;        ///< Max. bus clock frequency
  uint16_t CardComdClasses;      ///< Card command classes
  uint8_t  RdBlockLen;           ///< Max. read data block length
  uint8_t  PartBlockRead;        ///< Partial blocks for read allowed
  uint8_t  WrBlockMisalign;      ///< Write block misalignment
  uint8_t  RdBlockMisalign;      ///< Read block misalignment
  uint8_t  DSRImpl;              ///< DSR implemented
  uint8_t  Reserved2;            ///< Reserved
  uint32_t DeviceSize;           ///< Device Size
  uint8_t  MaxRdCurrentVDDMin;   ///< Max. read current @ VDD min
  uint8_t  MaxRdCurrentVDDMax;   ///< Max. read current @ VDD max
  uint8_t  MaxWrCurrentVDDMin;   ///< Max. write current @ VDD min
  uint8_t  MaxWrCurrentVDDMax;   ///< Max. write current @ VDD max
  uint8_t  DeviceSizeMul;        ///< Device size multiplier
  uint8_t  EraseGrSize;          ///< Erase group size
  uint8_t  EraseGrMul;           ///< Erase group size multiplier
  uint8_t  WrProtectGrSize;      ///< Write protect group size
  uint8_t  WrProtectGrEnable;    ///< Write protect group enable
  uint8_t  ManDeflECC;           ///< Manufacturer default ECC
  uint8_t  WrSpeedFact;          ///< Write speed factor
  uint8_t  MaxWrBlockLen;        ///< Max. write data block length
  uint8_t  WriteBlockPaPartial;  ///< Partial blocks for write allowed
  uint8_t  Reserved3;            ///< Reserded
  uint8_t  ContentProtectAppli;  ///< Content protection application
  uint8_t  FileFormatGrouop;     ///< File format group
  uint8_t  CopyFlag;             ///< Copy flag (OTP)
  uint8_t  PermWrProtect;        ///< Permanent write protection
  uint8_t  TempWrProtect;        ///< Temporary write protection
  uint8_t  FileFormat;           ///< File Format
  uint8_t  ECC;                  ///< ECC code
  uint8_t  CSD_CRC;              ///< CSD CRC
  uint8_t  Reserved4;            ///< always 1
} SD_CSD;

/**
  * @brief  Card Identification Data: CID Register
  */
typedef struct
{
  uint8_t  ManufacturerID;       ///< ManufacturerID
  uint16_t OEM_AppliID;          ///< OEM/Application ID
  uint32_t ProdName1;            ///< Product Name part1
  uint8_t  ProdName2;            ///< Product Name part2
  uint8_t  ProdRev;              ///< Product Revision
  uint32_t ProdSN;               ///< Product Serial Number
  uint8_t  Reserved1;            ///< Reserved1
  uint16_t ManufactDate;         ///< Manufacturing Date
  uint8_t  CID_CRC;              ///< CID CRC
  uint8_t  Reserved2;            ///< always 1
} SD_CID;

/**
  * @brief SD Card information
  */
typedef struct
{
  SD_CSD SD_csd;
  SD_CID SD_cid;
  uint32_t CardCapacity;  ///< Card Capacity in blocks
  uint32_t CardBlockSize; ///< Card Block Size
  uint16_t RCA;
  uint8_t CardType;
} SD_CardInfo;

/**
  * @brief SD Card Status
  */
typedef struct
{
  uint8_t DAT_BUS_WIDTH;
  uint8_t SECURED_MODE;
  uint16_t SD_CARD_TYPE;
  uint32_t SIZE_OF_PROTECTED_AREA;
  uint8_t SPEED_CLASS;
  uint8_t PERFORMANCE_MOVE;
  uint8_t AU_SIZE;
  uint16_t ERASE_SIZE;
  uint8_t ERASE_TIMEOUT;
  uint8_t ERASE_OFFSET;
} SD_CardStatus;

typedef enum {
    WAIT_WHILE_RX_ACTIVE = SDIO_FLAG_RXACT,
    WAIT_WHILE_TX_ACTIVE = SDIO_FLAG_TXACT
}sdio_wait_on_io_t;

#define SDCARD_DRIVER_MODE_SPI          1
#define SDCARD_DRIVER_MODE_SDIO_1BIT    2
#define SDCARD_DRIVER_MODE_SDIO_4BIT    3

/**
 * Common SD card stuff
 */

/**
  * @brief Supported SD Memory Cards
  */
#define SDIO_STD_CAPACITY_SD_CARD_V1_1             ((uint32_t)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0             ((uint32_t)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD                 ((uint32_t)0x00000002)
#define SDIO_MULTIMEDIA_CARD                       ((uint32_t)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD                ((uint32_t)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD            ((uint32_t)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD          ((uint32_t)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD                ((uint32_t)0x00000007)
#define SDIO_UNKNOWN_CARD_TYPE                     ((uint32_t)0x00000008)

/**
  * @brief  SD detection on its memory slot
  */
#define SD_PRESENT                                 ((uint8_t)0x01)
#define SD_NOT_PRESENT                             ((uint8_t)0x00)

/**
  * @brief  SD write protect detection
  */
#define SD_WRITE_PROTECTED                         ((uint8_t)0x01)
#define SD_NOT_WRITE_PROTECTED                     ((uint8_t)0x00)
/**
  * @brief  SDIO block read/write size
  */
#define SD_SECTOR_SIZE 512

#endif // SDCARD_TYPE_H_

/**
 * @}
 */
