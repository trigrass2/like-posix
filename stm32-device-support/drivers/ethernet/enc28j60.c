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
 *
 * @file enc28j60.c
 * @{
 */

#include "enc28j60.h"

#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "spi.h"
#include "logger.h"

volatile uint8_t enc28j60_current_bank = 0;
volatile uint16_t enc28j60_rxrdpt = 0;
static logger_t enclog;
static SPI_HANDLE_t enc_spi;

#define ENC28J60_SPI_BAUDRATE 10000000
#define ENC28J60_RESET_TIME_MS 50000

#define enc28j60_select() spi_clear_ss(enc_spi)
#define enc28j60_release() spi_set_ss(enc_spi)
#define enc28j60_rxtx(data) spi_transfer_polled(enc_spi, data)
#define enc28j60_rx() enc28j60_rxtx(0xff)
#define enc28j60_tx(data) enc28j60_rxtx(data)

#define enc28j60_assert_reset() HAL_GPIO_WritePin(ENC28J60_SPI_NRST_PORT, ENC28J60_SPI_NRST_PIN, GPIO_PIN_RESET);
#define enc28j60_deassert_reset() HAL_GPIO_WritePin(ENC28J60_SPI_NRST_PORT, ENC28J60_SPI_NRST_PIN, GPIO_PIN_SET);

static void enc28j60_gpio_init();
static void enc28j60_soft_reset();
static void enc28j60_reset();

/*
 * Init & packet Rx/Tx
 */

void enc28j60_init(uint8_t *macadr)
{
    log_init(&enclog, "enc28j60");

	enc28j60_gpio_init();

	enc_spi = spi_init_polled(ENC28J60_SPI_PERIPH, true, ENC28J60_SPI_BAUDRATE, SPI_FIRSTBIT_MSB, SPI_PHASE_1EDGE, SPI_POLARITY_LOW, SPI_DATASIZE_8BIT);

	enc28j60_release();
	enc28j60_reset();
	enc28j60_soft_reset();

	// Setup Rx/Tx buffer
	enc28j60_rxrdpt = ENC28J60_RXSTART;
	enc28j60_wcr16(ERXST, ENC28J60_RXSTART);
	enc28j60_wcr16(ERXRDPT, ENC28J60_RXSTART);
	enc28j60_wcr16(ERXND, ENC28J60_RXEND);
	enc28j60_wcr16(ETXST, ENC28J60_TXSTART);
	enc28j60_wcr16(ETXND, ENC28J60_BUFEND);

	// enc28j60_wcr(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_BCEN);
	enc28j60_wcr(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN|ERXFCON_BCEN);
	enc28j60_wcr16(EPMM0, 0x303f);
	enc28j60_wcr16(EPMCSL, 0xf7f9);

	// Setup MAC
	enc28j60_wcr(MACON1, MACON1_TXPAUS|MACON1_RXPAUS|MACON1_MARXEN);
	enc28j60_wcr(MACON2, 0);
	// enc28j60_bfs(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);//|MACON3_FULDPX);
	enc28j60_wcr(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);
	enc28j60_wcr16(MAMXFL, ENC28J60_MAXFRAME);
	enc28j60_wcr(MABBIPG, 0x12);
	// enc28j60_wcr(MABBIPG, 0x15); // Set inter-frame gap
	enc28j60_wcr16(MAIPGL, 0x0C12);
	enc28j60_wcr(MAADR5, macadr[0]); // Set MAC address
	enc28j60_wcr(MAADR4, macadr[1]);
	enc28j60_wcr(MAADR3, macadr[2]);
	enc28j60_wcr(MAADR2, macadr[3]);
	enc28j60_wcr(MAADR1, macadr[4]);
	enc28j60_wcr(MAADR0, macadr[5]);

	// Setup PHY
	enc28j60_write_phy(PHCON1, PHCON1_PDPXMD); // Force full-duplex mode
	enc28j60_write_phy(PHCON2, PHCON2_HDLDIS); // Disable loopback

	// Enable interrupts
	enc28j60_bfs(EIE, EIE_INTIE|EIE_PKTIE);
	// Enable Rx packets
	enc28j60_bfs(ECON1, ECON1_RXEN);

	enc28j60_write_phy(PHLCON, PHLCON_LACFG2|
		PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|
		PHLCON_LFRQ0|PHLCON_STRCH);

	log_debug(&enclog, "device rev.%X", enc28j60_revision());
}

void enc28j60_gpio_init()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.Pin = ENC28J60_SPI_NRST_PIN;
    HAL_GPIO_Init(ENC28J60_SPI_NRST_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin = ENC28J60_SPI_NINT_PIN;
    HAL_GPIO_Init(ENC28J60_SPI_NINT_PORT, &GPIO_InitStruct);
}


// Generic SPI read command
uint8_t enc28j60_read_op(uint8_t cmd, uint8_t adr)
{
	uint8_t data;

	enc28j60_select();
	enc28j60_tx(cmd | (adr & ENC28J60_ADDR_MASK));
	if(adr & 0x80) // throw out dummy byte
		enc28j60_rx(); // when reading MII/MAC register
	data = enc28j60_rx();
	enc28j60_release();
	return data;
}

// Generic SPI write command
void enc28j60_write_op(uint8_t cmd, uint8_t adr, uint8_t data)
{
	enc28j60_select();
	enc28j60_tx(cmd | (adr & ENC28J60_ADDR_MASK));
	enc28j60_tx(data);
	enc28j60_release();
}

// Initiate software reset
void enc28j60_soft_reset()
{
    log_debug(&enclog, "soft reset");
	enc28j60_select();
	enc28j60_tx(ENC28J60_SPI_SC);
	enc28j60_release();

	enc28j60_current_bank = 0;
	for(volatile int i = 50000; i > 0; i--);
}

// Initiate hardware reset
void enc28j60_reset()
{
    log_debug(&enclog, "hard reset");
    enc28j60_assert_reset();
    usleep(ENC28J60_RESET_TIME_MS);
    enc28j60_deassert_reset();
}

// read the revision of the chip:
uint8_t enc28j60_revision(void)
{
    uint8_t rev;
    rev=enc28j60_rcr(EREVID);
    // microchip forgot to step the number on the silcon when they
    // released the revision B7. 6 is now rev B7. We still have
    // to see what they do when they release B8. At the moment
    // there is no B8 out yet
    if (rev>5) rev++;
	return rev;
}

// // interrupt pin
// void enc28j6_nint()
// {
// 	return GPIO_ReadInputDataBit(ENC28J60_SPI_NRST_PORT, ENC28J60_SPI_NRST_PIN) == Bit_RESET;
// }

/*
 * Memory access
 */

// Set register bank
void enc28j60_set_bank(uint8_t adr)
{
	uint8_t bank;

	if( (adr & ENC28J60_ADDR_MASK) < ENC28J60_COMMON_CR )
	{
		bank = (adr >> 5) & 0x03; //BSEL1|BSEL0=0x03
		if(bank != enc28j60_current_bank)
		{
			enc28j60_write_op(ENC28J60_SPI_BFC, ECON1, 0x03);
			enc28j60_write_op(ENC28J60_SPI_BFS, ECON1, bank);
			enc28j60_current_bank = bank;
		}
	}
}

// Read register
uint8_t enc28j60_rcr(uint8_t adr)
{
	enc28j60_set_bank(adr);
	return enc28j60_read_op(ENC28J60_SPI_RCR, adr);
}

// Read register pair
uint16_t enc28j60_rcr16(uint8_t adr)
{
	enc28j60_set_bank(adr);
	return enc28j60_read_op(ENC28J60_SPI_RCR, adr) |
		(enc28j60_read_op(ENC28J60_SPI_RCR, adr+1) << 8);
}

// Write register
void enc28j60_wcr(uint8_t adr, uint8_t arg)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
}

// Write register pair
void enc28j60_wcr16(uint8_t adr, uint16_t arg)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr+1, arg>>8);
}

// Clear bits in register (reg &= ~mask)
void enc28j60_bfc(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_BFC, adr, mask);
}

// Set bits in register (reg |= mask)
void enc28j60_bfs(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_BFS, adr, mask);
}

// Read Rx/Tx buffer (at ERDPT)
void enc28j60_read_buffer(uint8_t *buf, uint16_t len)
{
	enc28j60_select();
	enc28j60_tx(ENC28J60_SPI_RBM);
	while(len--)
		*(buf++) = enc28j60_rx();
	enc28j60_release();
}

// Write Rx/Tx buffer (at EWRPT)
void enc28j60_write_buffer(uint8_t *buf, uint16_t len)
{
	enc28j60_select();
	enc28j60_tx(ENC28J60_SPI_WBM);
	while(len--)
	{
		enc28j60_tx(*buf);
		buf++;
	}
	enc28j60_release();
}

// Read PHY register
uint16_t enc28j60_read_phy(uint8_t adr)
{
	enc28j60_wcr(MIREGADR, adr);
	enc28j60_bfs(MICMD, MICMD_MIIRD);
	while(enc28j60_rcr(MISTAT) & MISTAT_BUSY);
	enc28j60_bfc(MICMD, MICMD_MIIRD);
	return enc28j60_rcr16(MIRD);
}

// Write PHY register
void enc28j60_write_phy(uint8_t adr, uint16_t data)
{
	enc28j60_wcr(MIREGADR, adr);
	enc28j60_wcr16(MIWR, data);
	while(enc28j60_rcr(MISTAT) & MISTAT_BUSY);
}

void enc28j60_send_packet(uint8_t *data, uint16_t len)
{
	while(enc28j60_rcr(ECON1) & ECON1_TXRTS)
	{
		// TXRTS may not clear - ENC28J60 bug. We must reset
		// transmit logic in cause of Tx error
		if(enc28j60_rcr(EIR) & EIR_TXERIF)
		{
			enc28j60_bfs(ECON1, ECON1_TXRST);
			enc28j60_bfc(ECON1, ECON1_TXRST);
		}
	}

	enc28j60_wcr16(EWRPT, ENC28J60_TXSTART);
	enc28j60_write_buffer((uint8_t*)"\0", 1);
	enc28j60_write_buffer(data, len);

	enc28j60_wcr16(ETXST, ENC28J60_TXSTART);
	enc28j60_wcr16(ETXND, ENC28J60_TXSTART + len);
 	// Request packet send
	enc28j60_bfs(ECON1, ECON1_TXRTS);
}

void enc28j60_send_packet_start()
{
    while(enc28j60_rcr(ECON1) & ECON1_TXRTS)
    {
        // TXRTS may not clear - ENC28J60 bug. We must reset
        // transmit logic in cause of Tx error
        if(enc28j60_rcr(EIR) & EIR_TXERIF)
        {
            enc28j60_bfs(ECON1, ECON1_TXRST);
            enc28j60_bfc(ECON1, ECON1_TXRST);
        }
    }

    enc28j60_wcr16(EWRPT, ENC28J60_TXSTART);
    enc28j60_write_buffer((uint8_t*)"\0", 1);
}

void enc28j60_send_packet_end(uint16_t len)
{
    enc28j60_wcr16(ETXST, ENC28J60_TXSTART);
    enc28j60_wcr16(ETXND, ENC28J60_TXSTART + len);
    // Request packet send
    enc28j60_bfs(ECON1, ECON1_TXRTS);
}

uint16_t enc28j60_recv_packet(uint8_t *buf, uint16_t buflen)
{
	uint16_t len = 0, rxlen, status, temp;

	if(enc28j60_rcr(EPKTCNT))
	{
		enc28j60_wcr16(ERDPT, enc28j60_rxrdpt);

		enc28j60_read_buffer((void*)&enc28j60_rxrdpt, sizeof(enc28j60_rxrdpt));
		enc28j60_read_buffer((void*)&rxlen, sizeof(rxlen));
		enc28j60_read_buffer((void*)&status, sizeof(status));

		if(status & ENC28J60_RX_STATUS_VECTOR_RX_OK)
		{
			len = rxlen - 4; //throw out crc
			if(len > buflen)
				len = buflen;
			enc28j60_read_buffer(buf, len);
		}

		// Set Rx read pointer to next packet
		// take care, never set ERXRDPT to an even number.
		// sinceenc28j60_rxrdpt is always even, subtract 1.
		// also deal with wrap around.
		if(enc28j60_rxrdpt == ENC28J60_RXSTART)
			temp = ENC28J60_RXEND;
		else
			temp = enc28j60_rxrdpt - 1;
		enc28j60_wcr16(ERXRDPT, temp);
		// Decrement packet counter
		enc28j60_bfs(ECON2, ECON2_PKTDEC);
	}

	return len;
}

uint8_t enc28j60_check_incoming()
{
    return enc28j60_rcr(EPKTCNT);
}

uint16_t enc28j60_recv_packet_start(uint16_t maxlen)
{
    uint16_t len = 0, rxlen, status;

    enc28j60_wcr16(ERDPT, enc28j60_rxrdpt);

    enc28j60_read_buffer((void*)&enc28j60_rxrdpt, sizeof(enc28j60_rxrdpt));
    enc28j60_read_buffer((void*)&rxlen, sizeof(rxlen));
    enc28j60_read_buffer((void*)&status, sizeof(status));

    if(status & ENC28J60_RX_STATUS_VECTOR_RX_OK)
    {
        len = rxlen - 4; //throw out crc
        if(len > maxlen)
            len = maxlen;
    }

    return len;
}

void enc28j60_recv_packet_end()
{
    uint16_t temp;
    // Set Rx read pointer to next packet
    // take care, never set ERXRDPT to an even number.
    // sinceenc28j60_rxrdpt is always even, subtract 1.
    // also deal with wrap around.
    if(enc28j60_rxrdpt == ENC28J60_RXSTART)
        temp = ENC28J60_RXEND;
    else
        temp = enc28j60_rxrdpt - 1;
    enc28j60_wcr16(ERXRDPT, temp);
    // Decrement packet counter
    enc28j60_bfs(ECON2, ECON2_PKTDEC);
}

bool eth_link_status()
{
    return (bool)(enc28j60_read_phy(PHSTAT2) & PHSTAT2_LSTAT);
}

short eth_link_speed()
{
    return 10;
}

bool eth_link_full_duplex()
{
    return (bool)(enc28j60_read_phy(PHSTAT2) & PHSTAT2_DPXSTAT);
}

/**
 * @}
 */
