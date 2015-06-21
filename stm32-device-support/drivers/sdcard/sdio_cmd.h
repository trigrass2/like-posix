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
 * @{
 */

#ifndef SDCARD_CMD_H_
#define SDCARD_CMD_H_

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

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)

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


#endif // SDCARD_CMD_H_

/**
  * @}
  */
