
USE_DRIVER_LWIP_NET_VALUES = 0 1

ifeq ($(filter $(USE_DRIVER_LWIP_NET),$(USE_DRIVER_LWIP_NET_VALUES)), )
$(error USE_DRIVER_LWIP_NET is not set. set to one of: $(USE_DRIVER_LWIP_NET_VALUES))
endif

CFLAGS += -DUSE_DRIVER_LWIP_NET=$(USE_DRIVER_LWIP_NET)

ifeq ($(USE_DRIVER_LWIP_NET), 1)
SOURCE += $(LWIP_DIR)/src/netif/etharp.c
SOURCE += $(LWIP_DIR)/src/netif/loopif.c
SOURCE += $(LWIP_DIR)/src/netif/slipif.c
SOURCE += $(LWIP_DIR)/src/core/dhcp.c
SOURCE += $(LWIP_DIR)/src/core/dns.c
SOURCE += $(LWIP_DIR)/src/core/init.c
SOURCE += $(LWIP_DIR)/src/core/mem.c
SOURCE += $(LWIP_DIR)/src/core/memp.c
SOURCE += $(LWIP_DIR)/src/core/netif.c
SOURCE += $(LWIP_DIR)/src/core/pbuf.c
SOURCE += $(LWIP_DIR)/src/core/raw.c
SOURCE += $(LWIP_DIR)/src/core/stats.c
SOURCE += $(LWIP_DIR)/src/core/sys.c
SOURCE += $(LWIP_DIR)/src/core/tcp_in.c
SOURCE += $(LWIP_DIR)/src/core/tcp_out.c
SOURCE += $(LWIP_DIR)/src/core/tcp.c
SOURCE += $(LWIP_DIR)/src/core/udp.c
SOURCE += $(LWIP_DIR)/src/core/ipv4/autoip.c
SOURCE += $(LWIP_DIR)/src/core/ipv4/icmp.c
SOURCE += $(LWIP_DIR)/src/core/ipv4/igmp.c
SOURCE += $(LWIP_DIR)/src/core/ipv4/inet_chksum.c
SOURCE += $(LWIP_DIR)/src/core/ipv4/inet.c
SOURCE += $(LWIP_DIR)/src/core/ipv4/ip_addr.c
SOURCE += $(LWIP_DIR)/src/core/ipv4/ip_frag.c
SOURCE += $(LWIP_DIR)/src/core/ipv4/ip.c
SOURCE += $(LWIP_DIR)/src/api/api_lib.c
SOURCE += $(LWIP_DIR)/src/api/api_msg.c
SOURCE += $(LWIP_DIR)/src/api/err.c
SOURCE += $(LWIP_DIR)/src/api/netbuf.c
SOURCE += $(LWIP_DIR)/src/api/netdb.c
SOURCE += $(LWIP_DIR)/src/api/netifapi.c
SOURCE += $(LWIP_DIR)/src/api/sockets.c
SOURCE += $(LWIP_DIR)/src/api/tcpip.c

SOURCE += $(LWIP_DIR)/src/ethernetif.c

CFLAGS += -I$(LWIP_DIR)/inc/lwip
CFLAGS += -I$(LWIP_DIR)/inc/lwip/arch
CFLAGS += -I$(LWIP_DIR)/inc/netif
CFLAGS += -I$(LWIP_DIR)/inc
endif