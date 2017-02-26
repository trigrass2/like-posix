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
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ethernetif.h"
#include "lwip/tcp_impl.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "etharp.h"
#include "lwip/tcpip.h"
#include "lwip/stats.h"

#include "strutils.h"
#include "cutensils.h"

#include "net.h"

#ifdef NET_LINK_LED
#include "leds.h"
#endif


#define give_rx_ready_from_isr() \
	static BaseType_t xHigherPriorityTaskWoken; \
	xHigherPriorityTaskWoken = pdFALSE; \
	xSemaphoreGiveFromISR(netconf_default->rxpkt, &xHigherPriorityTaskWoken); \
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

#define give_rx_ready() xSemaphoreGive(netconf_default->rxpkt)

#if USE_DRIVER_MII_RMII_PHY
#define wait_for_rx_ready() (xSemaphoreTake(netconf->rxpkt, 100/portTICK_RATE_MS) == pdTRUE)
#elif USE_DRIVER_ENC28J60_PHY
#include "enc28j60.h"
#define wait_for_rx_ready() enc28j60_check_incoming()
#endif


#if LWIP_DHCP
static void dhcp_begin(netconf_t* netconf);
static void dhcp_process(netconf_t* netconf);
#endif
static void net_task(netconf_t* netconf);
static void link_callback(struct netif *netif);
static void status_callback(struct netif *netif);
static void tcpip_init_done(void *arg);

static char mac[18];
static char lip[16];
netconf_t* netconf_default;

void net_init(netconf_t* netconf)
{
	log_init(&netconf->log, "net_init");
	netconf_default = netconf;
	netconf->net_task_enabled = true;
	netconf_default->address_ok = xSemaphoreCreateBinary();
	assert_true(netconf_default->address_ok != NULL);
	netconf_default->rxpkt = xSemaphoreCreateBinary();
	assert_true(netconf_default->rxpkt != NULL);

#if NO_SYS
	lwip_init();
	tcpip_init_done();
#else
	tcpip_init(tcpip_init_done, (void*)&netconf->netif);
#endif
}

void net_deinit(netconf_t* netconf)
{
	netif_set_down(&netconf->netif);
	netconf->net_task_enabled = false;
}

bool wait_for_address(netconf_t* netconf)
{
    return xSemaphoreTake(netconf->address_ok, 2000/portTICK_RATE_MS) == pdTRUE;
}

#if USE_DRIVER_MII_RMII_PHY

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
	give_rx_ready_from_isr();
}

#endif

void net_task(netconf_t* netconf)
{
    while(netconf->net_task_enabled)
    {
    	if(wait_for_rx_ready()) {
    		ethernetif_input(&netconf->netif);
    	}
#if NO_SYS
	    sys_check_timeouts();
#endif
    }

    vTaskDelete(NULL);
}

/**
 * link notification
 */
void link_callback(struct netif *netif)
{
	log_info(&netconf_default->log, "link state: %s", netif_is_up(netif) ? "up" : "down");
//	ethernetif_update_config(netif);
//
//	if (!netif_is_up(netif) && netif_is_link_up(netif)) {
//		dhcp_start(netif);
//	}
//
//	if (netif_is_up(netif) && !netif_is_link_up(netif)) {
//		dhcp_release(netif);
//	}
}

/**
 * status notification
 */
void status_callback(struct netif *netif)
{
#if USE_LOGGER
	uint8_t* pt;
	pt = (uint8_t*)&(netif->ip_addr.addr);
	log_info(&netconf_default->log, "ip: %d.%d.%d.%d", pt[0], pt[1], pt[2], pt[3]);
	pt = (uint8_t*)&(netif->netmask.addr);
	log_info(&netconf_default->log, "nm: %d.%d.%d.%d", pt[0], pt[1], pt[2], pt[3]);
	pt = (uint8_t*)&(netif->gw.addr);
	log_info(&netconf_default->log, "gw: %d.%d.%d.%d", pt[0], pt[1], pt[2], pt[3]);
	pt = netif->hwaddr;
	log_info(&netconf_default->log, "mac: %02x:%02x:%02x:%02x:%02x:%02x", pt[0], pt[1], pt[2], pt[3], pt[4], pt[5]);
#endif
	if(netif_is_up(netif))
	{
		xSemaphoreGive(netconf_default->address_ok);
#ifdef NET_LINK_LED
	    set_led(NET_LINK_LED);
#endif
	}
}

void tcpip_init_done(void *arg)
{
	(void)arg;

	log_info(&netconf_default->log, "hostname: %s", netconf_default->hostname);

	netif_add(&netconf_default->netif, &netconf_default->addr_cache[0], &netconf_default->addr_cache[1], &netconf_default->addr_cache[2], NULL, &ethernetif_init, &ethernet_input);
	netif_set_link_callback(&netconf_default->netif, link_callback);
	netif_set_status_callback(&netconf_default->netif, status_callback);
	netif_set_default(&netconf_default->netif);

#if LWIP_DHCP
	if(netconf_default->resolv == NET_RESOLV_DHCP)
	{
		netif_set_down(&netconf_default->netif);
		dhcp_start(&netconf_default->netif);
		log_info(&netconf_default->log, "DHCP started");
	}
	else
	{
		log_info(&netconf_default->log, "Static IP");
		netif_set_up(&netconf_default->netif);
		dns_setserver(0, &netconf_default->addr_cache[3]);
		dns_setserver(1, &netconf_default->addr_cache[4]);
	}
#endif

	xTaskCreate((TaskFunction_t)net_task,"lwip_receive",
					configMINIMAL_STACK_SIZE+NET_TASK_STACK, netconf_default,
					tskIDLE_PRIORITY+NET_TASK_PRIORITY, NULL);
}

bool net_is_up()
{
    return netif_is_up(&netconf_default->netif);
}

unsigned long net_ip_packets_sent()
{
    return lwip_stats.ip.xmit;
}

unsigned long net_ip_packets_received()
{
    return lwip_stats.ip.recv;
}

unsigned long net_ip_packets_dropped()
{
    return lwip_stats.ip.drop;
}

unsigned long net_ip_errors()
{
    return lwip_stats.ip.err;
}

struct netif* get_interfaces()
{
    return &netconf_default->netif;
}

unsigned short net_mtu()
{
    return netconf_default->netif.mtu;
}

unsigned char* net_hwaddr()
{
    return netconf_default->netif.hwaddr;
}

ip_addr_t net_ipaddr()
{
    return netconf_default->netif.ip_addr;
}

ip_addr_t net_gwaddr()
{
    return netconf_default->netif.gw;
}

ip_addr_t net_netmask()
{
    return netconf_default->netif.netmask;
}

const char* net_hostname()
{
    return netconf_default->netif.hostname;
}


const char* net_mac()
{
    // get the local IP address
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
             ((char*)&netconf_default->netif.hwaddr)[0],
             ((char*)&netconf_default->netif.hwaddr)[1],
             ((char*)&netconf_default->netif.hwaddr)[2],
             ((char*)&netconf_default->netif.hwaddr)[3],
             ((char*)&netconf_default->netif.hwaddr)[4],
             ((char*)&netconf_default->netif.hwaddr)[5]);
    return (const char*)mac;
}

const char* net_lip()
{
    // get the local IP address
    sprintf(lip, "%d.%d.%d.%d",
             ((char*)&netconf_default->netif.ip_addr)[0],
             ((char*)&netconf_default->netif.ip_addr)[1],
             ((char*)&netconf_default->netif.ip_addr)[2],
             ((char*)&netconf_default->netif.ip_addr)[3]);
    return (const char*)lip;
}
