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
#include "board_config.h"

#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "spi.h"
#include "logger.h"

#define ENC28J60_SPI_BUFFERSIZE 256


volatile uint8_t enc28j60_current_bank = 0;
volatile uint16_t enc28j60_rxrdpt = 0;
static logger_t enclog;
static SPI_HANDLE_t enc_spi;


#define enc28j60_read_control_register_byte(address) enc28j60_read_register_byte(ENC28J60_SPI_RCR, address)
#define enc28j60_read_control_register_word(address) enc28j60_read_register_word(ENC28J60_SPI_RCR, address)
#define enc28j60_write_control_register_byte(address, data) enc28j60_write_register_byte(ENC28J60_SPI_WCR, address, data)
#define enc28j60_write_control_register_word(address, data) enc28j60_write_register_word(ENC28J60_SPI_WCR, address, data)
#define enc28j60_clear_control_register_bit(address, mask) enc28j60_write_register_byte(ENC28J60_SPI_BFC, address, mask)
#define enc28j60_set_control_register_bit(address, mask) enc28j60_write_register_byte(ENC28J60_SPI_BFS, address, mask)


static void enc28j60_set_bank(uint8_t adr);
static uint8_t enc28j60_read_register_byte(uint8_t cmd, uint8_t adr);
static uint16_t enc28j60_read_register_word(uint8_t cmd, uint8_t adr);
static void enc28j60_write_register_byte(uint8_t cmd, uint8_t adr, uint8_t data);
static void enc28j60_write_register_word(uint8_t cmd, uint8_t adr, uint16_t data);
static void enc28j60_gpio_init();
static void enc28j60_soft_reset();
static void enc28j60_reset();

static void enc28j60_write_phy(uint8_t adr, uint16_t data);

/*
 * Init & packet Rx/Tx
 */

void enc28j60_init(uint8_t *macadr)
{
    log_init(&enclog, "enc28j60");

	enc28j60_gpio_init();

	enc_spi = spi_create_async(ENC28J60_SPI_PERIPH, true, SPI_FIRSTBIT_MSB, SPI_PHASE_1EDGE, SPI_POLARITY_LOW, SPI_DATASIZE_8BIT, ENC28J60_SPI_BAUDRATE, ENC28J60_SPI_BUFFERSIZE);

	spi_set_ss(enc_spi);

	enc28j60_reset();
	enc28j60_soft_reset();

	// Setup Rx/Tx buffer
	enc28j60_rxrdpt = ENC28J60_RXSTART;
	enc28j60_write_control_register_word(ERXST, ENC28J60_RXSTART);
	enc28j60_write_control_register_word(ERXRDPT, ENC28J60_RXSTART);
	enc28j60_write_control_register_word(ERXND, ENC28J60_RXEND);
	enc28j60_write_control_register_word(ETXST, ENC28J60_TXSTART);
	enc28j60_write_control_register_word(ETXND, ENC28J60_BUFEND);

	// enc28j60_write_control_register_byte(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_BCEN);
	enc28j60_write_control_register_byte(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN|ERXFCON_BCEN);
	enc28j60_write_control_register_word(EPMM0, 0x303f);
	enc28j60_write_control_register_word(EPMCSL, 0xf7f9);

	// Setup MAC
	enc28j60_write_control_register_byte(MACON1, MACON1_TXPAUS|MACON1_RXPAUS|MACON1_MARXEN);
	enc28j60_write_control_register_byte(MACON2, 0);
	// enc28j60_set_control_register_bit(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);//|MACON3_FULDPX);
	enc28j60_write_control_register_byte(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);
	enc28j60_write_control_register_word(MAMXFL, ENC28J60_MAXFRAME);
	enc28j60_write_control_register_byte(MABBIPG, 0x12);
	// enc28j60_write_control_register_byte(MABBIPG, 0x15); // Set inter-frame gap
	enc28j60_write_control_register_word(MAIPGL, 0x0C12);
	enc28j60_write_control_register_byte(MAADR5, macadr[0]); // Set MAC address
	enc28j60_write_control_register_byte(MAADR4, macadr[1]);
	enc28j60_write_control_register_byte(MAADR3, macadr[2]);
	enc28j60_write_control_register_byte(MAADR2, macadr[3]);
	enc28j60_write_control_register_byte(MAADR1, macadr[4]);
	enc28j60_write_control_register_byte(MAADR0, macadr[5]);

	// Setup PHY
	enc28j60_write_phy(PHCON1, PHCON1_PDPXMD); // Force full-duplex mode
	enc28j60_write_phy(PHCON2, PHCON2_HDLDIS); // Disable loopback

	// Enable interrupts
	enc28j60_set_control_register_bit(EIE, EIE_INTIE|EIE_PKTIE);
	// Enable Rx packets
	enc28j60_set_control_register_bit(ECON1, ECON1_RXEN);

	enc28j60_write_phy(PHLCON, PHLCON_LACFG2|PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|PHLCON_LFRQ0|PHLCON_STRCH);

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

// Initiate hardware reset
void enc28j60_reset()
{
    log_debug(&enclog, "hard reset");
    HAL_GPIO_WritePin(ENC28J60_SPI_NRST_PORT, ENC28J60_SPI_NRST_PIN, GPIO_PIN_RESET);
    usleep(ENC28J60_RESET_TIME_US);
    HAL_GPIO_WritePin(ENC28J60_SPI_NRST_PORT, ENC28J60_SPI_NRST_PIN, GPIO_PIN_SET);
}

// read interrupt pin
bool enc28j6_nint()
{
	return HAL_GPIO_ReadPin(ENC28J60_SPI_NRST_PORT, ENC28J60_SPI_NRST_PIN) == GPIO_PIN_RESET;
}








/*
 * Memory access
 */

// Set register bank
void enc28j60_set_bank(uint8_t adr)
{
	//printf("enc28j60_set_bank()\n");
	uint8_t bank;

	if((adr & ENC28J60_ADDR_MASK) < ENC28J60_COMMON_CR)
	{
		bank = (adr >> 5) & 0x03; // BSEL1|BSEL0=0x03
		if(bank != enc28j60_current_bank)
		{
			// bit clear BSEL1|BSEL0 then bit set bank in BSEL1, BSEL0
			uint8_t data_[4] = {ENC28J60_SPI_BFC | (ECON1 & ENC28J60_ADDR_MASK), 0x03, ENC28J60_SPI_BFS | (ECON1 & ENC28J60_ADDR_MASK), bank};
			spi_put_async(enc_spi, data_, 4);
			spi_get_async(enc_spi, data_, 4, 1000); // dont care
			enc28j60_current_bank = bank;
		}
	}
}

uint8_t enc28j60_read_register_byte(uint8_t cmd, uint8_t adr)
{
	//printf("enc28j60_read_register_byte()\n");
	spi_clear_ss(enc_spi);

	enc28j60_set_bank(adr);

	adr &= ENC28J60_ADDR_MASK;

	uint8_t data[3] = {cmd | adr, 0xff, 0xff};
	uint8_t len = adr & 0x80 ? 3 : 2;

	spi_put_async(enc_spi, data, len);
	spi_get_async(enc_spi, data, len, 1000);
	spi_set_ss(enc_spi);

	len -= 1;
	return data[len];
}

uint16_t enc28j60_read_register_word(uint8_t cmd, uint8_t adr)
{
	//printf("enc28j60_read_register_word()\n");
	spi_clear_ss(enc_spi);

	enc28j60_set_bank(adr);

	adr &= ENC28J60_ADDR_MASK;

	uint8_t data[6] = {cmd | adr, 0xff, 0xff, 0xff, 0xff, 0xff};
	uint8_t len = adr & 0x80 ? 6 : 4;
	data[len/2] = cmd | (adr + 1);

	spi_put_async(enc_spi, data, len);
	spi_get_async(enc_spi, data, len, 1000);
	spi_set_ss(enc_spi);

	return data[(len/2)-1] | (data[len-1] << 8);
}

void enc28j60_write_register_byte(uint8_t cmd, uint8_t adr, uint8_t data)
{
	//printf("enc28j60_write_register_byte()\n");
	spi_clear_ss(enc_spi);
	enc28j60_set_bank(adr);

	adr &= ENC28J60_ADDR_MASK;

	uint8_t data_[2] = {cmd | adr, data};

	spi_put_async(enc_spi, data_, sizeof(data_));
	spi_get_async(enc_spi, data_, sizeof(data_), 1000); // dont care
	spi_set_ss(enc_spi);
}

void enc28j60_write_register_word(uint8_t cmd, uint8_t adr, uint16_t data)
{
	//printf("enc28j60_write_register_word()\n");
	spi_clear_ss(enc_spi);
	enc28j60_set_bank(adr);

	adr &= ENC28J60_ADDR_MASK;

	uint8_t data_[4] = {cmd | adr, (uint8_t)data, cmd | (adr + 1), data>>8};

	spi_put_async(enc_spi, data_, sizeof(data_));
	spi_get_async(enc_spi, data_, sizeof(data_), 1000); // dont care
	spi_set_ss(enc_spi);
}

// Initiate software reset
void enc28j60_soft_reset()
{
	//printf("enc28j60_soft_reset()\n");
	uint8_t data = ENC28J60_SPI_SC;
    log_debug(&enclog, "soft reset");
    spi_clear_ss(enc_spi);
    spi_put_async(enc_spi, &data, 1);
    spi_get_async(enc_spi, &data, 1, 1000); // dont care
	spi_set_ss(enc_spi);
	enc28j60_current_bank = 0;
	usleep(ENC28J60_RESET_TIME_US);
}

/**
 * read the revision of the chip
 *
 * microchip forgot to step the number on the silcon when they
 * released the revision B7. 6 is now rev B7. We still have
 * to see what they do when they release B8. At the moment
 * there is no B8 out yet
 *
 */
uint8_t enc28j60_revision(void)
{
	//printf("enc28j60_revision()\n");
    uint8_t rev = enc28j60_read_control_register_byte(EREVID);
    if(rev>5) {
    	rev++;
    }
	return rev;
}

// Read Rx/Tx buffer (at ERDPT)
void enc28j60_read_buffer(uint8_t *buf, uint16_t len)
{
	//printf("enc28j60_read_buffer()\n");
	uint16_t sent = 0;
	uint16_t received = 0;
	uint8_t cmd = ENC28J60_SPI_RBM;

	spi_clear_ss(enc_spi);
    spi_put_async(enc_spi, &cmd, 1);
    spi_get_async(enc_spi, &cmd, 1, 1000); // dont care
    while(received < len) {
		sent = spi_put_async(enc_spi, buf, len - received); // dont care
		//printf("r s %d\n", sent);
		received += spi_get_async(enc_spi, &buf[received], sent, 1000);
		//printf("r r %d\n", received);
    }
	spi_set_ss(enc_spi);
}

// Write Rx/Tx buffer (at EWRPT)
void enc28j60_write_buffer(uint8_t *buf, uint16_t len)
{
	//printf("enc28j60_write_buffer()\n");
	int16_t sent = 0;
	uint16_t received = 0;
	uint8_t cmd = ENC28J60_SPI_WBM;

	spi_clear_ss(enc_spi);
	spi_put_async(enc_spi, &cmd, 1);
	spi_get_async(enc_spi, &cmd, 1, 1000); // dont care
	while(received < len) {
		sent = spi_put_async(enc_spi, &buf[received], len - received);
		//printf("w s %d\n", sent);
		received += spi_get_async(enc_spi, &buf[received], sent, 1000); // dont care
		//printf("w r %d\n", received);
	}
	spi_set_ss(enc_spi);
}

// Read PHY register
uint16_t enc28j60_read_phy(uint8_t adr)
{
	//printf("enc28j60_read_phy()\n");
	enc28j60_write_control_register_byte(MIREGADR, adr);
	enc28j60_set_control_register_bit(MICMD, MICMD_MIIRD);
	while(enc28j60_read_control_register_byte(MISTAT) & MISTAT_BUSY);
	enc28j60_clear_control_register_bit(MICMD, MICMD_MIIRD);
	return enc28j60_read_control_register_word(MIRD);
}

// Write PHY register
void enc28j60_write_phy(uint8_t adr, uint16_t data)
{
	//printf("enc28j60_write_phy()\n");
	enc28j60_write_control_register_byte(MIREGADR, adr);
	enc28j60_write_control_register_word(MIWR, data);
	while(enc28j60_read_control_register_byte(MISTAT) & MISTAT_BUSY);
}

void enc28j60_send_packet(uint8_t *data, uint16_t len)
{
	//printf("enc28j60_send_packet()\n");
	while(enc28j60_read_control_register_byte(ECON1) & ECON1_TXRTS)
	{
		// TXRTS may not clear - ENC28J60 bug. We must reset
		// transmit logic in cause of Tx error
		if(enc28j60_read_control_register_byte(EIR) & EIR_TXERIF)
		{
			enc28j60_set_control_register_bit(ECON1, ECON1_TXRST);
			enc28j60_clear_control_register_bit(ECON1, ECON1_TXRST);
		}
	}

	enc28j60_write_control_register_word(EWRPT, ENC28J60_TXSTART);
	enc28j60_write_buffer((uint8_t*)"\0", 1);
	enc28j60_write_buffer(data, len);

	enc28j60_write_control_register_word(ETXST, ENC28J60_TXSTART);
	enc28j60_write_control_register_word(ETXND, ENC28J60_TXSTART + len);
 	// Request packet send
	enc28j60_set_control_register_bit(ECON1, ECON1_TXRTS);
}

void enc28j60_send_packet_start()
{
	//printf("enc28j60_send_packet_start()\n");
    while(enc28j60_read_control_register_byte(ECON1) & ECON1_TXRTS)
    {
        // TXRTS may not clear - ENC28J60 bug. We must reset
        // transmit logic in cause of Tx error
        if(enc28j60_read_control_register_byte(EIR) & EIR_TXERIF)
        {
            enc28j60_set_control_register_bit(ECON1, ECON1_TXRST);
            enc28j60_clear_control_register_bit(ECON1, ECON1_TXRST);
        }
    }

    enc28j60_write_control_register_word(EWRPT, ENC28J60_TXSTART);
    enc28j60_write_buffer((uint8_t*)"\0", 1);
}

void enc28j60_send_packet_end(uint16_t len)
{
	//printf("enc28j60_send_packet_end()\n");
    enc28j60_write_control_register_word(ETXST, ENC28J60_TXSTART);
    enc28j60_write_control_register_word(ETXND, ENC28J60_TXSTART + len);
    // Request packet send
    enc28j60_set_control_register_bit(ECON1, ECON1_TXRTS);
}

uint16_t enc28j60_recv_packet(uint8_t *buf, uint16_t buflen)
{
	//printf("enc28j60_recv_packet()\n");
	uint16_t len = 0, rxlen, status, temp;

	if(enc28j60_read_control_register_byte(EPKTCNT))
	{
		enc28j60_write_control_register_word(ERDPT, enc28j60_rxrdpt);

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
		enc28j60_write_control_register_word(ERXRDPT, temp);
		// Decrement packet counter
		enc28j60_set_control_register_bit(ECON2, ECON2_PKTDEC);
	}

	return len;
}

uint8_t enc28j60_check_incoming()
{
	//printf("enc28j60_check_incoming()\n");
    return enc28j60_read_control_register_byte(EPKTCNT);
}

uint16_t enc28j60_recv_packet_start(uint16_t maxlen)
{
	//printf("enc28j60_recv_packet_start()\n");
    uint16_t len = 0, rxlen, status;

    enc28j60_write_control_register_word(ERDPT, enc28j60_rxrdpt);

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
	//printf("enc28j60_recv_packet_end()\n");
    uint16_t temp;
    // Set Rx read pointer to next packet
    // take care, never set ERXRDPT to an even number.
    // sinceenc28j60_rxrdpt is always even, subtract 1.
    // also deal with wrap around.
    if(enc28j60_rxrdpt == ENC28J60_RXSTART)
        temp = ENC28J60_RXEND;
    else
        temp = enc28j60_rxrdpt - 1;
    enc28j60_write_control_register_word(ERXRDPT, temp);
    // Decrement packet counter
    enc28j60_set_control_register_bit(ECON2, ECON2_PKTDEC);
}

bool eth_link_status()
{
	//printf("eth_link_status()\n");
    return (bool)(enc28j60_read_phy(PHSTAT2) & PHSTAT2_LSTAT);
}

short eth_link_speed()
{
    return 10;
}

bool eth_link_full_duplex()
{
	//printf("eth_link_full_duplex()\n");
    return (bool)(enc28j60_read_phy(PHSTAT2) & PHSTAT2_DPXSTAT);
}

/**
 * @}
 */
