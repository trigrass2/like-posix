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
 * This file is part of the FatFs project, <https://github.com/drmetal/FatFs>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * @addtogroup sdfs
 *
 * Disk IO interface, links up the SD card drivers with FatFs by ChaN.
 *
 * @file
 * @{
 */

#include <stdio.h>
#include "diskio.h"
#include "sdcard.h"
#include "cutensils.h"

SD_CardInfo SDCardInfo;             // card information
DSTATUS Status = STA_NOINIT;        // Disk status

DSTATUS disk_initialize(BYTE drv)      /* Physical drive number (0) */
{
	SD_Error err = SD_NOT_CONFIGURED;

	// reset Status to not initialised
	Status = STA_NOINIT;

	log_syslog(NULL, "disk init");

	// update status, based on hardware state
	// SD card must be present and drive number set to 0
	if((SD_Detect() == SD_PRESENT) && (drv == 0))
		Status &= ~STA_NODISK;
	else
		Status |= STA_NODISK;

	if(SD_WPDetect() == SD_WRITE_PROTECTED)
		Status |= STA_PROTECT;
	else
		Status &= ~STA_PROTECT;

	// this will only be true if disk is present and not write protected
	if(Status == STA_NOINIT)
	{
		// init SD card
		err = SD_Init(&SDCardInfo);
		if(err == SD_OK)
		{
			log_syslog(NULL, "capacity: %uMB", (unsigned int)((SDCardInfo.CardBlockSize/512)*(SDCardInfo.CardCapacity/(2*1000))));
			log_syslog(NULL, "sector size: %uB", (unsigned int)SDCardInfo.CardBlockSize);
			log_syslog(NULL, "card type: %u", (unsigned int)SDCardInfo.CardType);
			Status &= ~STA_NOINIT;     		 // indicate success
		}
	}

	if(Status & STA_NOINIT)
		log_error(NULL, "disk init error: sderror=%d, status=%d", (int)err, (int)Status);

	return Status;
}

DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, BYTE count)
{
	DRESULT res = RES_ERROR;
	SD_Error err = SD_OK;
	SDTransferState xferstate = SD_TRANSFER_BUSY;

	// if drive invalid then parameter error
	if (drv)
		return RES_PARERR;

	// if card is not initialised then not ready error
	// if card is not inserted then not ready error
	if(Status & (STA_NODISK | STA_NOINIT))
		return RES_NOTRDY;

	if(count == 1)
	{
		err = SD_ReadBlock((uint8_t*)buff, sector);
		if(err == SD_OK)
		{
			err = SD_WaitReadOperation();
			if(err == SD_OK)
			{
				while(xferstate != SD_TRANSFER_OK)
					xferstate = SD_GetStatus();
				if(xferstate == SD_TRANSFER_OK)
					res = RES_OK;
				else
					log_error(NULL, "read block xfer failed: %d", xferstate);
			}
			else
				log_error(NULL, "read block wait failed: %d", err);
		}
		else
			log_error(NULL, "read block failed: %d", err);
			
	}
	else if(count > 1)
	{
		err = SD_ReadMultiBlocks((uint8_t*)buff, sector, count);
		if(err == SD_OK)
		{
			err = SD_WaitReadOperation();
			if(err == SD_OK)
			{
				while(xferstate != SD_TRANSFER_OK)
					xferstate = SD_GetStatus();
				if(xferstate == SD_TRANSFER_OK)
					res = RES_OK;
				else
					log_error(NULL, "read blocks xfer failed: %d", xferstate);
			}
			else
				log_error(NULL, "read blocks wait failed: %d", err);
		}
		else
			log_error(NULL, "read blocks failed: %d", err);
	}

	return res;
}

#if _FS_READONLY == 0
DRESULT disk_write (BYTE drv, const BYTE *buff, DWORD sector, BYTE count)
{
	DRESULT res = RES_ERROR;
	SD_Error err = SD_OK;
	SDTransferState xferstate = SD_TRANSFER_BUSY;

	// if drive invalid then parameter error
	if (drv)
		return RES_PARERR;

	// if card is not initialised then not ready error
	// if card is not inserted then not ready error
	if(Status & (STA_NODISK | STA_NOINIT))
		return RES_NOTRDY;

	// if card is write protected then write protected error
	if(Status & STA_PROTECT)
		return RES_WRPRT;

	if(count == 1)
	{
		err = SD_WriteBlock((const uint8_t*)buff, sector);
		if(err == SD_OK)
		{
			err = SD_WaitWriteOperation();
			if(err == SD_OK)
			{
				while(xferstate != SD_TRANSFER_OK)
					xferstate = SD_GetStatus();
				if(xferstate == SD_TRANSFER_OK)
					res = RES_OK;
				else
					log_error(NULL, "write block xfer failed: %d", xferstate);
			}
			else
				log_error(NULL, "write block wait failed: %d", err);
		}
		else
			log_error(NULL, "write block failed: %d", err);
	}
	else if(count > 1)
	{
		err = SD_WriteMultiBlocks((const uint8_t*)buff, sector, count);
		if(err == SD_OK)
		{
			err = SD_WaitWriteOperation();
			if(err == SD_OK)
			{
				while(xferstate != SD_TRANSFER_OK)
					xferstate = SD_GetStatus();
				if(xferstate == SD_TRANSFER_OK)
					res = RES_OK;
				else
					log_error(NULL, "write blocks xfer failed: %d", xferstate);
			}
			else
				log_error(NULL, "write blocks wait failed: %d", err);
		}
		else
			log_error(NULL, "write blocks failed: %d", err);

	}

	return res;
}
#endif // _FS_READONLY

#ifdef _USE_IOCTL // _USE_IOCTL != 0

DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
   DRESULT res = RES_OK;

	// if drive invalid then parameter error
	if (drv)
		return RES_PARERR;

	// if card is not initialised then not ready error
	// if card is not inserted then not ready error
	if(Status & (STA_NODISK | STA_NOINIT))
		return RES_NOTRDY;

   switch (ctrl)
   {
      case CTRL_SYNC :        // error if transfer not complete
    	  if(SD_GetTransferState() == SD_TRANSFER_BUSY)
    		  res = RES_ERROR;
      break;
      case GET_SECTOR_COUNT : // Get number of sectors on the disk
    	  *(DWORD*)buff = SDCardInfo.CardCapacity;
      break;
      case GET_SECTOR_SIZE :  // Get R/W sector size
    	  *(DWORD*)buff = SDCardInfo.CardBlockSize;
      break;
      case GET_BLOCK_SIZE :   // Get erase block size in unit of sector
    	  *(DWORD*)buff = SDCardInfo.SD_csd.EraseGrSize;
      break;
      case MMC_GET_TYPE :     // Get card type
    	  *(BYTE*)buff = SDCardInfo.CardType;
      break;
      case MMC_GET_CSD:

      break;
      case MMC_GET_CID:

      break;
      case MMC_GET_OCR:

      break;
      case MMC_GET_SDSTAT:

      break;
      case CTRL_POWER :
		if(*(BYTE*)buff == 0)
			SD_PowerOFF();
		if(*(BYTE*)buff == 1)
			SD_PowerON();
      break;
      case CTRL_ERASE_SECTOR:
    	  SD_Erase(*(((DWORD*)buff)), *(((DWORD*)buff)+1));
      break;
      default:
         res = RES_PARERR;
      break;
   }

   return res;
}
#endif // _USE_IOCTL != 0

/**
 * @brief	returns fat fs compliant time stamp from scheduler time base.
 *
 * @retval	returns an integer of the following format:
 * 	bits  0:4 		5 bits 	Second/2 		(0..29)
 * 	bits  5:10 		6 bits 	Minute 			(0..59)
 * 	bits  11:15 	5 bits 	Hour 			(0..23)
 * 	bits  16:20 	5 bits 	Day in month	(1..31)
 * 	bits  21:24 	4 bits 	Month 			(1..12)
 * 	bits  25:31 	7 bits 	Year from 1980 	(0..127)
 */
DWORD get_fattime(void)
{
	// uint64_t time = sched_get_time();
	DWORD ftime = 0;

	// time /= 2000;					// time -> '2 seconds'
	// ftime = time%30;				// 2 second count
	// time /= 30;						// time -> minutes
	// ftime |= (time%59)<<5;			// minute count
	// time /= 60;						// time -> hours
	// ftime |= (time%23)<<11;			// hour count
	// time /= 24;						// time -> days
	// ftime |= ((time%30)+1)<<16;		// day count
	// time /= 31;						// time -> months
	// ftime |= ((time%11)+1)<<21;		// month count
	// time /= 12;						// time -> years
	// ftime |= (time%127)<<25;		// year count

	return ftime;
}

/**
 * if disk is present, clears STA_NODISK flag.
 * else sets STA_NODISK flag.
 * if disk is not write protected, clears SD_WRITE_PROTECTED flag.
 * else sets SD_WRITE_PROTECTED flag.
 */
DSTATUS disk_status(BYTE drv)
{
	// update status, based on card inserted state

	// SD card must be present and drive number set to 0
    if((get_diskstatus() == SD_PRESENT) && (drv == 0))
	   Status &= ~STA_NODISK;         // indicate disk present
   else
       Status |= STA_NODISK;         // indicate no-disk

    if(SD_WPDetect() == SD_WRITE_PROTECTED)
       Status |= STA_PROTECT;     // indicate write protected
   else
	   Status &= ~STA_PROTECT;     // indicate not write protected

   return Status;
}

/**
 * @}
 */
