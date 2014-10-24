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

#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32_eth.h"

#include "ethernetif.h"
#include "tcp_impl.h"
#include "dhcp.h"
#include "mem.h"
#include "memp.h"
#include "tcp.h"
#include "udp.h"
#include "etharp.h"
#include "tcpip.h"
#include "init.h"

#include "strutils.h"
#include "cutensils.h"

#include "net.h"

#ifdef NET_LINK_LED
#include "leds.h"
#endif

static void net_task(void *pvParameters);
static void link_callback(struct netif *netif);
static void status_callback(struct netif *netif);
static void tcpip_init_done(void *arg);


void net_init(netconf_t* netconf)
{
#if LWIP_DHCP
	uint32_t localtime;
#endif

	log_init(&netconf->log, "net_init");

#if NO_SYS
	lwip_init();
#else
	tcpip_init(tcpip_init_done, (void*)&netconf->netif);
#endif

	netif_add(&netconf->netif, &netconf->addr_cache[0], &netconf->addr_cache[1], &netconf->addr_cache[2], NULL, &ethernetif_init, &ethernet_input);
	netif_set_link_callback(&netconf->netif, link_callback);
	netif_set_status_callback(&netconf->netif, status_callback);
	netif_set_default(&netconf->netif);

	xTaskCreate(net_task,
				"lwIP",
				configMINIMAL_STACK_SIZE+NET_TASK_STACK,
				netconf,
				tskIDLE_PRIORITY+NET_TASK_PRIORITY,
				NULL);

#if LWIP_DHCP
	if(netconf->resolv == NET_RESOLV_DHCP)
	{
		dhcp_start(&netconf->netif); // call dhcp_fine_tmr() and dhcp_coarse_tmr()
		log_info(&netconf->log, "DHCP started");

		// loop here while DHCP is ongoing... we arnt going anywhere fast without an IP address
		// TODO - I saw that DHCP has timeouts enabled in timers.c... why do I need this loop? DHCP doesnt seem to work otherwise.
		while(netconf->netif.dhcp->state != DHCP_OFF)
		{
			localtime = xTaskGetTickCount()/portTICK_PERIOD_MS;

			// DHCP Coarse periodic process every 60s
			if (localtime - netconf->dhcp_coarse_timer >= DHCP_COARSE_TIMER_MSECS)
			{
				netconf->dhcp_coarse_timer =  localtime;
				dhcp_coarse_tmr();
			}

			// Fine DHCP periodic process every 500ms
			if (localtime - netconf->dhcp_fine_timer >= DHCP_FINE_TIMER_MSECS)
			{

				netconf->dhcp_fine_timer =  localtime;
				dhcp_fine_tmr();

				if(netconf->netif.dhcp->state == DHCP_BOUND)
				{
					dhcp_stop(&netconf->netif);
					log_info(&netconf->log, "DHCP finished");
				}
				else if(netconf->netif.dhcp->state != DHCP_OFF)
				{
#ifdef NET_LINK_LED
					toggle_led(NET_LINK_LED);
#endif
				}
			}
			taskYIELD();
		}
	}
#endif

	netif_set_up(&netconf->netif);
}

void net_task(void *pvParameters)
{
#if NO_SYS
	uint32_t localtime;
#endif

	netconf_t* netconf = (netconf_t*)pvParameters;

    for(;;)
    {
    	// TODO - use a semaphore to trigger ethernetif_input from an packet received interrupt.
    	// run the other TCP stuff in a separate thread in that case...
		if (ETH_CheckFrameReceived())
		{
			ethernetif_input(&netconf->netif);
		}

#if NO_SYS
	// only need the following if we specify NO_SYS
#if LWIP_TCP
	/* TCP periodic process every 250 ms */
	if (localtime - netconf->tcp_timer >= TCP_TMR_INTERVAL)
	{
		netconf->tcp_timer = localtime;
		tcp_tmr();
	}
#endif
	/* ARP periodic process every 5s */
	if ((localtime - netconf->arp_timer) >= ARP_TMR_INTERVAL)
	{
		netconf->arp_timer = localtime;
		etharp_tmr();
	}
#endif
		taskYIELD();
    }
}

/**
 * link notification
 * TODO make this do something useful
 */
void link_callback(struct netif *netif)
{
	logger_t link_cb_log;
	log_init(&link_cb_log, "net_link_callback");
	(void)netif;
	log_info(&link_cb_log, "link info....");
}

/**
 * status notification
 * TODO make this do something useful
 */
void status_callback(struct netif *netif)
{
	logger_t status_cb_log;
	log_init(&status_cb_log, "net_status_callback");
	uint8_t* pt;
	pt = (uint8_t*)&(netif->ip_addr.addr);
	log_info(&status_cb_log, "ip: %d.%d.%d.%d", pt[0], pt[1], pt[2], pt[3]);
	pt = (uint8_t*)&(netif->netmask.addr);
	log_info(&status_cb_log, "nm: %d.%d.%d.%d", pt[0], pt[1], pt[2], pt[3]);
	pt = (uint8_t*)&(netif->gw.addr);
	log_info(&status_cb_log, "gw: %d.%d.%d.%d", pt[0], pt[1], pt[2], pt[3]);
	pt = netif->hwaddr;
	log_info(&status_cb_log, "mac: %02x:%02x:%02x:%02x:%02x:%02x", pt[0], pt[1], pt[2], pt[3], pt[4], pt[5]);
#ifdef NET_LINK_LED
	set_led(NET_LINK_LED);
#endif
}

void tcpip_init_done(void *arg)
{
	logger_t tcp_cb_log;
	log_init(&tcp_cb_log, "tcpip_init_done");
	struct netif* netif = (struct netif*)arg;
	log_info(&tcp_cb_log, "%s, %s", __FUNCTION__, netif->hostname);
}

