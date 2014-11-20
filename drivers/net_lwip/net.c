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

#include "eth_mac.h"

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
#include "lwip/stats.h"

#include "strutils.h"
#include "cutensils.h"

#include "net.h"

#ifdef NET_LINK_LED
#include "leds.h"
#endif

#if LWIP_DHCP
static void dhcp_begin(netconf_t* netconf);
static void dhcp_process(netconf_t* netconf);
#endif
static void net_task(void *pvParameters);
static void link_callback(struct netif *netif);
static void status_callback(struct netif *netif);
static void tcpip_init_done(void *arg);

extern struct netif *netif_list;

void net_init(netconf_t* netconf)
{
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

    netconf->address_ok = xSemaphoreCreateBinary();
    assert_true(netconf->address_ok != NULL);
    xSemaphoreTake(netconf->address_ok, 100/portTICK_RATE_MS);

#if LWIP_DHCP
    if(netconf->resolv == NET_RESOLV_DHCP)
        dhcp_begin(netconf);
    else
    {
        netif_set_up(&netconf->netif);
        xSemaphoreGive(netconf->address_ok);
    }
#endif

	xTaskCreate(net_task,
				"lwIP",
				configMINIMAL_STACK_SIZE+NET_TASK_STACK,
				netconf,
				tskIDLE_PRIORITY+NET_TASK_PRIORITY,
				NULL);
}

bool wait_for_address(netconf_t* netconf)
{
    return xSemaphoreTake(netconf->address_ok, 10000/portTICK_RATE_MS) == pdTRUE;
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

		if(ethernetif_incoming() == ERR_OK)
		{
#if !NO_SYS
		    LOCK_TCPIP_CORE();
			ethernetif_input(&netconf->netif);
			UNLOCK_TCPIP_CORE();
#else
			ethernetif_input(&netconf->netif);
#endif
		}
		else
		    vTaskDelay(1/portTICK_RATE_MS); // TODO - sort this out!! required in some cases to get CPU time for other tasks :|

#if LWIP_DHCP
	    if(netconf->resolv == NET_RESOLV_DHCP)
	        dhcp_process(netconf);
#endif

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

#if LWIP_DHCP

static void dhcp_begin(netconf_t* netconf)
{
    dhcp_start(&netconf->netif);
    netconf->dhcp_state = DHCP_STATE_INIT;
    log_info(&netconf->log, "DHCP started");
}

void dhcp_process(netconf_t* netconf)
{
    uint32_t localtime;
    bool run_sm = false;

    localtime = xTaskGetTickCount()/portTICK_PERIOD_MS;

    if(localtime - netconf->dhcp_coarse_timer >= DHCP_COARSE_TIMER_MSECS)
    {
        netconf->dhcp_coarse_timer =  localtime;
        dhcp_coarse_tmr();
        run_sm = true;
    }

    // Fine DHCP periodic process every 500ms
    if(localtime - netconf->dhcp_fine_timer >= DHCP_FINE_TIMER_MSECS)
    {
        netconf->dhcp_fine_timer =  localtime;
        dhcp_fine_tmr();
        run_sm = true;
    }

    if(run_sm)
    {
        switch(netconf->dhcp_state)
        {
            case DHCP_STATE_INIT:
                netif_set_down(&netconf->netif);
                netconf->dhcp_state = DHCP_STATE_DISCOVER;
            break;

            case DHCP_STATE_DISCOVER:
#ifdef NET_LINK_LED
                toggle_led(NET_LINK_LED);
#endif
                if(netconf->netif.dhcp->state == DHCP_BOUND)
                {
                    netif_set_up(&netconf->netif);
                    netconf->dhcp_state = DHCP_STATE_DONE;
                    xSemaphoreGive(netconf->address_ok);
                    log_info(&netconf->log, "DHCP finished");
                }
            break;

            case DHCP_STATE_DONE:
                if(netconf->netif.dhcp->state != DHCP_BOUND)
                    netconf->dhcp_state = DHCP_STATE_INIT;
            break;
        }
    }
}

#endif

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

	if(netif_is_up(netif))
	{
#ifdef NET_LINK_LED
	    set_led(NET_LINK_LED);
#endif
	}
}

void tcpip_init_done(void *arg)
{
	logger_t tcp_cb_log;
	log_init(&tcp_cb_log, "tcpip_init_done");
	struct netif* netif = (struct netif*)arg;
	log_info(&tcp_cb_log, "%s, %s", __FUNCTION__, netif->hostname);
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

unsigned short net_mtu()
{
    return netif_list->mtu;
}

unsigned char* net_hwaddr()
{
    return netif_list->hwaddr;
}

ip_addr_t net_ipaddr()
{
    return netif_list->ip_addr;
}

ip_addr_t net_gwaddr()
{
    return netif_list->gw;
}

ip_addr_t net_netmask()
{
    return netif_list->netmask;
}
