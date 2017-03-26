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
 * This code is based heavily on demo code from microchip...
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * @addtogroup ethernet
 * @{
 *
 * @file enc28j60.h
 */

#ifndef ENC28J60_H_
#define ENC28J60_H_

#include <stdbool.h>
#include <stdint.h>
#include "net_config.h"
#include "board_config.h"

// Init ENC28J60
void enc28j60_init(uint8_t *macadr);
uint8_t enc28j60_revision(void);
bool enc28j6_nint();

// Snd/Rcv packets
void enc28j60_send_packet(uint8_t *data, uint16_t len);
void enc28j60_send_packet_start();
void enc28j60_send_packet_end(uint16_t len);

uint16_t enc28j60_recv_packet(uint8_t *buf, uint16_t buflen);
uint8_t enc28j60_check_incoming();
uint16_t enc28j60_recv_packet_start(uint16_t maxlen);
void enc28j60_recv_packet_end();
// R/W Rx/Tx buffer
void enc28j60_read_buffer(uint8_t *buf, uint16_t len);
void enc28j60_write_buffer(uint8_t *buf, uint16_t len);


#ifndef ETH_MAX_ETH_PAYLOAD
#pragma message ( "Note: using default ETH_MAX_ETH_PAYLOAD = 1500" )
#define ETH_MAX_ETH_PAYLOAD             1500        // Maximum Ethernet payload size
#endif
#define ETH_HEADER                  14          // 6 byte Dest addr, 6 byte Src addr, 2 byte length/type
#define ETH_CRC                     4           // Ethernet CRC
#define ETH_EXTRA                   2           // Extra bytes in some cases
#define ETH_VLAN_TAG                    4           // optional 802.1q VLAN Tag
#define ETH_MIN_ETH_PAYLOAD             46          // Minimum Ethernet payload size
#define ENC28J60_MAXFRAME           (ETH_MAX_ETH_PAYLOAD + ETH_HEADER + ETH_EXTRA + ETH_CRC)
#define ETH_MAX_PACKET_SIZE         ENC28J60_MAXFRAME

#define ENC28J60_TX_STATUS_VECTOR_LENGTH    7

#define ENC28J60_BUFSIZE    0x2000
#define ENC28J60_RXSIZE		(ENC28J60_BUFSIZE - ENC28J60_MAXFRAME - ENC28J60_TX_STATUS_VECTOR_LENGTH)
#define ENC28J60_RXSTART    0
#define ENC28J60_RXEND      (ENC28J60_RXSIZE - 2)
#define ENC28J60_TXSTART	(ENC28J60_RXSIZE - 1)
#define ENC28J60_BUFEND     (ENC28J60_BUFSIZE - 1)

#define ENC28J60_SPI_RCR	0x00
#define ENC28J60_SPI_RBM	0x3A
#define ENC28J60_SPI_WCR	0x40
#define ENC28J60_SPI_WBM	0x7A
#define ENC28J60_SPI_BFS	0x80
#define ENC28J60_SPI_BFC	0xA0
#define ENC28J60_SPI_SC		0xFF

#define ENC28J60_ADDR_MASK	0x1F
#define ENC28J60_COMMON_CR	0x1B




/**
 * registers addresses are arranged in 4 banks, with a range of 0x00 to 0x1F (5 bits wide)
 *
 * we add extra info in the upper 3 bits to identify the bank: ADDR|0x00 = bank1, addr|0x20 = bank2, addr|0x40 = bank3, addr|0x60 = bank4
 * decode bank with: bank = (addr >> 5) & 0x03
 *
 * we add extra info in the upper 3 bits to identify MII and MAC registers: ADDR|0x00 = non MII/MAC, ADDR|0x80 = MII/MAC register
 * MAC and MII registers are read with a 3 byte sequence: {opcode | address, dummy, data}
 * all other registers are read with a 2 byte sequence: {opcode | address, data}
 *
 * addresses must be masked with ENC28J60_ADDR_MASK to get rid of the upper 3 bits before sending to the device.
 */

/*
 * Main registers
 */

#define EIE 				0x1B
#define EIR 				0x1C
#define ESTAT 				0x1D
#define ECON2 				0x1E
#define ECON1 				0x1F


// Buffer read pointer
#define ERDPTL 				0x00
#define ERDPTH 				0x01
#define ERDPT				ERDPTL

// Buffer write pointer
#define EWRPTL 				0x02
#define EWRPTH 				0x03
#define EWRPT				EWRPTL

// Tx packet start pointer
#define ETXSTL 				0x04
#define ETXSTH 				0x05
#define ETXST				ETXSTL

// Tx packet end pointer
#define ETXNDL 				0x06
#define ETXNDH 				0x07
#define ETXND				ETXNDL

// Rx FIFO start pointer
#define ERXSTL 				0x08
#define ERXSTH 				0x09
#define ERXST				ERXSTL

// Rx FIFO end pointer
#define ERXNDL 				0x0A
#define ERXNDH 				0x0B
#define ERXND				ERXNDL

// Rx FIFO read pointer
#define ERXRDPTL 			0x0C
#define ERXRDPTH 			0x0D
#define ERXRDPT				ERXRDPTL

// Rx FIFO write pointer
#define ERXWRPTL 			0x0E
#define ERXWRPTH 			0x0F
#define ERXWRPT				ERXWRPTL

// DMA source block start pointer
#define EDMASTL 			0x10
#define EDMASTH 			0x11
#define EDMAST				EDMASTL

// DMA source block end pointer
#define EDMANDL 			0x12
#define EDMANDH 			0x13
#define EDMAND				EDMANDL

// DMA destination pointer
#define EDMADSTL 			0x14
#define EDMADSTH 			0x15
#define	EDMADST				EDMADSTL

// DMA checksum
#define EDMACSL 			0x16
#define EDMACSH 			0x17
#define EDMACS				EDMACSL

// Hash table registers
#define EHT0 				(0x00 | 0x20)
#define EHT1 				(0x01 | 0x20)
#define EHT2 				(0x02 | 0x20)
#define EHT3 				(0x03 | 0x20)
#define EHT4 				(0x04 | 0x20)
#define EHT5 				(0x05 | 0x20)
#define EHT6 				(0x06 | 0x20)
#define EHT7 				(0x07 | 0x20)

// Pattern match registers
#define EPMM0 				(0x08 | 0x20)
#define EPMM1 				(0x09 | 0x20)
#define EPMM2 				(0x0A | 0x20)
#define EPMM3 				(0x0B | 0x20)
#define EPMM4 				(0x0C | 0x20)
#define EPMM5 				(0x0D | 0x20)
#define EPMM6 				(0x0E | 0x20)
#define EPMM7 				(0x0F | 0x20)
#define EPMCSL 				(0x10 | 0x20)
#define EPMCSH 				(0x11 | 0x20)
#define EPMOL 				(0x14 | 0x20)
#define EPMOH 				(0x15 | 0x20)

// Wake-on-LAN interrupt registers
#define EWOLIE 				(0x16 | 0x20)
#define EWOLIR 				(0x17 | 0x20)

// Receive filters mask
#define ERXFCON 			(0x18 | 0x20)

// Packet counter
#define EPKTCNT 			(0x19 | 0x20)


// MAC control registers
#define MACON1 				(0x00 | 0x40 | 0x80)
#define MACON2 				(0x01 | 0x40 | 0x80)
#define MACON3 				(0x02 | 0x40 | 0x80)
#define MACON4 				(0x03 | 0x40 | 0x80)

// MAC Back-to-back gap
#define MABBIPG 			(0x04 | 0x40 | 0x80)

// MAC Non back-to-back gap
#define MAIPGL 				(0x06 | 0x40 | 0x80)
#define MAIPGH 				(0x07 | 0x40 | 0x80)

// Collision window & rexmit timer
#define MACLCON1 			(0x08 | 0x40 | 0x80)
#define MACLCON2 			(0x09 | 0x40 | 0x80)

// Max frame length
#define MAMXFLL 			(0x0A | 0x40 | 0x80)
#define MAMXFLH 			(0x0B | 0x40 | 0x80)
#define MAMXFL				MAMXFLL

// MAC-PHY support register
#define MAPHSUP 			(0x0D | 0x40 | 0x80)
#define MICON 				(0x11 | 0x40 | 0x80)

// MII registers
#define MICMD 				(0x12 | 0x40 | 0x80)
#define MIREGADR 			(0x14 | 0x40 | 0x80)

#define MIWRL 				(0x16 | 0x40 | 0x80)
#define MIWRH 				(0x17 | 0x40 | 0x80)
#define MIWR				MIWRL

#define MIRDL 				(0x18 | 0x40 | 0x80)
#define MIRDH 				(0x19 | 0x40 | 0x80)
#define MIRD				MIRDL

// MAC Address
#define MAADR1 				(0x00 | 0x60 | 0x80)
#define MAADR0 				(0x01 | 0x60 | 0x80)
#define MAADR3 				(0x02 | 0x60 | 0x80)
#define MAADR2 				(0x03 | 0x60 | 0x80)
#define MAADR5 				(0x04 | 0x60 | 0x80)
#define MAADR4 				(0x05 | 0x60 | 0x80)

// Built-in self-test
#define EBSTSD 				(0x06 | 0x60)
#define EBSTCON 			(0x07 | 0x60)
#define EBSTCSL 			(0x08 | 0x60)
#define EBSTCSH 			(0x09 | 0x60)
#define MISTAT 				(0x0A | 0x60 | 0x80)

// Revision ID
#define EREVID 				(0x12 | 0x60)

// Clock output control register
#define ECOCON 				(0x15 | 0x60)

// Flow control registers
#define EFLOCON 			(0x17 | 0x60)
#define EPAUSL 				(0x18 | 0x60)
#define EPAUSH 				(0x19 | 0x60)

// PHY registers
#define PHCON1 				0x00
#define PHSTAT1 			0x01
#define PHID1 				0x02
#define PHID2 				0x03
#define PHCON2 				0x10
#define PHSTAT2 			0x11
#define PHIE 				0x12
#define PHIR 				0x13
#define PHLCON 				0x14

// EIE
#define EIE_INTIE			0x80
#define EIE_PKTIE			0x40
#define EIE_DMAIE			0x20
#define EIE_LINKIE			0x10
#define EIE_TXIE			0x08
#define EIE_WOLIE			0x04
#define EIE_TXERIE			0x02
#define EIE_RXERIE			0x01

// EIR
#define EIR_PKTIF			0x40
#define EIR_DMAIF			0x20
#define EIR_LINKIF			0x10
#define EIR_TXIF			0x08
#define EIR_WOLIF			0x04
#define EIR_TXERIF			0x02
#define EIR_RXERIF			0x01

// ESTAT
#define ESTAT_INT			0x80
#define ESTAT_LATECOL		0x10
#define ESTAT_RXBUSY		0x04
#define ESTAT_TXABRT		0x02
#define ESTAT_CLKRDY		0x01

// ECON2
#define ECON2_AUTOINC		0x80
#define ECON2_PKTDEC		0x40
#define ECON2_PWRSV			0x20
#define ECON2_VRPS			0x08

// ECON1
#define ECON1_TXRST			0x80
#define ECON1_RXRST			0x40
#define ECON1_DMAST			0x20
#define ECON1_CSUMEN		0x10
#define ECON1_TXRTS			0x08
#define ECON1_RXEN			0x04
#define ECON1_BSEL1			0x02
#define ECON1_BSEL0			0x01

// EWOLIE
#define EWOLIE_UCWOLIE		0x80
#define EWOLIE_AWOLIE		0x40
#define EWOLIE_PMWOLIE		0x10
#define EWOLIE_MPWOLIE		0x08
#define EWOLIE_HTWOLIE		0x04
#define EWOLIE_MCWOLIE		0x02
#define EWOLIE_BCWOLIE		0x01

// EWOLIR
#define EWOLIR_UCWOLIF		0x80
#define EWOLIR_AWOLIF		0x40
#define EWOLIR_PMWOLIF		0x10
#define EWOLIR_MPWOLIF		0x08
#define EWOLIR_HTWOLIF		0x04
#define EWOLIR_MCWOLIF		0x02
#define EWOLIR_BCWOLIF		0x01

// ERXFCON
#define ERXFCON_UCEN		0x80
#define ERXFCON_ANDOR		0x40
#define ERXFCON_CRCEN		0x20
#define ERXFCON_PMEN		0x10
#define ERXFCON_MPEN		0x08
#define ERXFCON_HTEN		0x04
#define ERXFCON_MCEN		0x02
#define ERXFCON_BCEN		0x01

// MACON1
#define MACON1_LOOPBK		0x10
#define MACON1_TXPAUS		0x08
#define MACON1_RXPAUS		0x04
#define MACON1_PASSALL		0x02
#define MACON1_MARXEN		0x01

// MACON2
#define MACON2_MARST		0x80
#define MACON2_RNDRST		0x40
#define MACON2_MARXRST		0x08
#define MACON2_RFUNRST		0x04
#define MACON2_MATXRST		0x02
#define MACON2_TFUNRST		0x01

// MACON3
#define MACON3_PADCFG2		0x80
#define MACON3_PADCFG1		0x40
#define MACON3_PADCFG0		0x20
#define MACON3_TXCRCEN		0x10
#define MACON3_PHDRLEN		0x08
#define MACON3_HFRMEN		0x04
#define MACON3_FRMLNEN		0x02
#define MACON3_FULDPX		0x01

// MACON4
#define MACON4_DEFER		0x40
#define MACON4_BPEN			0x20
#define MACON4_NOBKOFF		0x10
#define MACON4_LONGPRE		0x02
#define MACON4_PUREPRE		0x01

// MAPHSUP
#define MAPHSUP_RSTINTFC	0x80
#define MAPHSUP_RSTRMII		0x08

// MICON
#define MICON_RSTMII		0x80

// MICMD
#define MICMD_MIISCAN		0x02
#define MICMD_MIIRD			0x01

// EBSTCON
#define EBSTCON_PSV2		0x80
#define EBSTCON_PSV1		0x40
#define EBSTCON_PSV0		0x20
#define EBSTCON_PSEL		0x10
#define EBSTCON_TMSEL1		0x08
#define EBSTCON_TMSEL0		0x04
#define EBSTCON_TME			0x02
#define EBSTCON_BISTST		0x01

// MISTAT
#define MISTAT_NVALID		0x04
#define MISTAT_SCAN			0x02
#define MISTAT_BUSY			0x01

// ECOCON
#define ECOCON_COCON2		0x04
#define ECOCON_COCON1		0x02
#define ECOCON_COCON0		0x01

// EFLOCON
#define EFLOCON_FULDPXS		0x04
#define EFLOCON_FCEN1		0x02
#define EFLOCON_FCEN0		0x01

// PHCON1
#define PHCON1_PRST			0x8000
#define PHCON1_PLOOPBK		0x4000
#define PHCON1_PPWRSV		0x0800
#define PHCON1_PDPXMD		0x0100

// PHSTAT1
#define PHSTAT1_PFDPX		0x1000
#define PHSTAT1_PHDPX		0x0800
#define PHSTAT1_LLSTAT		0x0004
#define PHSTAT1_JBSTAT		0x0002

// PHCON2
#define PHCON2_FRCLNK		0x4000
#define PHCON2_TXDIS		0x2000
#define PHCON2_JABBER		0x0400
#define PHCON2_HDLDIS		0x0100

// PHSTAT2
#define PHSTAT2_TXSTAT		0x2000
#define PHSTAT2_RXSTAT		0x1000
#define PHSTAT2_COLSTAT		0x0800
#define PHSTAT2_LSTAT		0x0400
#define PHSTAT2_DPXSTAT		0x0200
#define PHSTAT2_PLRITY		0x0010

// PHIE
#define PHIE_PLNKIE			0x0010
#define PHIE_PGEIE			0x0002

// PHIR
#define PHIR_PLNKIF			0x0010
#define PHIR_PGIF			0x0004

// PHLCON
#define PHLCON_LACFG3		0x0800
#define PHLCON_LACFG2		0x0400
#define PHLCON_LACFG1		0x0200
#define PHLCON_LACFG0		0x0100
#define PHLCON_LBCFG3		0x0080
#define PHLCON_LBCFG2		0x0040
#define PHLCON_LBCFG1		0x0020
#define PHLCON_LBCFG0		0x0010
#define PHLCON_LFRQ1		0x0008
#define PHLCON_LFRQ0		0x0004
#define PHLCON_STRCH		0x0002

#define ENC28J60_RX_STATUS_VECTOR_RX_OK     0x80

#endif // ENC28J60_H_

/**
 * @}
 */
