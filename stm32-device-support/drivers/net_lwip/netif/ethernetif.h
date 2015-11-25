#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include <stdbool.h>

#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);
err_t ethernetif_input(struct netif *netif);

bool eth_link_status();
short eth_link_speed();
bool eth_link_full_duplex();

#if USE_DRIVER_MII_RMII_PHY && USE_DRIVER_LWIP_NET

#define ethernetif_incoming()  (err_t)(ERR_OK);

#elif USE_DRIVER_ENC28J60_PHY && USE_DRIVER_LWIP_NET

#define ethernetif_incoming()  (err_t)(enc28j60_check_incoming() ? ERR_OK : ERR_MEM);

#endif

#endif // __ETHERNETIF_H__
