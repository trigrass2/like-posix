/**
 * @addtogroup ethernet
 * @{
 *
 * @file MII_RMII.c
 */

#include "board_config.h"
#include "net_config.h"

#ifndef MII_RMII_H_
#define MII_RMII_H_

#ifdef __cplusplus
 extern "C" {
#endif


 /* Uncomment the line below when using time stamping and/or IPv4 checksum offload */
 #define USE_ENHANCED_DMA_DESCRIPTORS

//#define USE_Delay
#define _eth_delay_    ETH_Delay 	/* Default _eth_delay_ function with less precise timing */
//#define _eth_delay_    delay 		/* Default _eth_delay_ function with less precise timing */
 /* PHY Reset delay */
 #define PHY_RESET_DELAY    ((uint32_t)0x000FFFFF)
 /* PHY Configuration delay */
 #define PHY_CONFIG_DELAY   ((uint32_t)0x00FFFFFF)

/**
 * @defgroup py_regs PHY Register Definitions
 * @{
 */

#if defined(LAN8700_PHY)

/** PHY register set */
#define PHY_BCR                          0          /*!< Tranceiver Basic Control Register */
#define PHY_BSR                          1          /*!< Tranceiver Basic Status Register */
#define PHY_SR                           31         /*!< Tranceiver Status Register */

/** PHY_BCR bit definitions */
#define PHY_Reset                       ((u16)0x8000)      /*!< PHY Reset */
#define PHY_Loopback                    ((u16)0x4000)      /*!< Select loop-back mode */
#define PHY_FULLDUPLEX_100M             ((u16)0x2100)      /*!< Set the full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((u16)0x2000)      /*!< Set the half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((u16)0x0100)      /*!< Set the full-duplex mode at 10 Mb/s */
#define PHY_HALFDUPLEX_10M              ((u16)0x0000)      /*!< Set the half-duplex mode at 10 Mb/s */
#define PHY_AutoNegotiation             ((u16)0x1000)      /*!< Enable auto-negotiation function */
#define PHY_Restart_AutoNegotiation     ((u16)0x0200)      /*!< Restart auto-negotiation function */
#define PHY_Powerdown                   ((u16)0x0800)      /*!< Select the power down mode */
#define PHY_Isolate                     ((u16)0x0400)      /*!< Isolate PHY from MII */
/** PHY_BSR bit definitions */
#define PHY_AutoNego_Complete           ((u16)0x0020)      /*!< Auto-Negotioation process completed */
#define PHY_Linked_Status               ((u16)0x0004)      /*!< Valid link established */
#define PHY_Jabber_detection            ((u16)0x0002)      /*!< Jabber condition detected */
/** PHY_SR bit definitions */
#define PHY_DUPLEX_STATUS           	((u16)0x0010)    /*!< Configured information of Duplex: Full-duplex */
#define PHY_SPEED_STATUS            	((u16)0x0004)    /*!< Configured information of Speed: 10Mbps */
#elif defined(DP83848_PHY)

/** PHY register set */
#define PHY_BCR                          0          /*!< Tranceiver Basic Control Register */
#define PHY_BSR                          1          /*!< Tranceiver Basic Status Register */
#define PHY_SR                           16     	 /*!< Tranceiver Status Register */

/** PHY_BCR bit definitions */
#define PHY_Reset                       ((u16)0x8000)      /*!< PHY Reset */
#define PHY_Loopback                    ((u16)0x4000)      /*!< Select loop-back mode */
#define PHY_FULLDUPLEX_100M             ((u16)0x2100)      /*!< Set the full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((u16)0x2000)      /*!< Set the half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((u16)0x0100)      /*!< Set the full-duplex mode at 10 Mb/s */
#define PHY_HALFDUPLEX_10M              ((u16)0x0000)      /*!< Set the half-duplex mode at 10 Mb/s */
#define PHY_AutoNegotiation             ((u16)0x1000)      /*!< Enable auto-negotiation function */
#define PHY_Restart_AutoNegotiation     ((u16)0x0200)      /*!< Restart auto-negotiation function */
#define PHY_Powerdown                   ((u16)0x0800)      /*!< Select the power down mode */
#define PHY_Isolate                     ((u16)0x0400)      /*!< Isolate PHY from MII */
/** PHY_BSR bit definitions */
#define PHY_AutoNego_Complete           ((u16)0x0020)      /*!< Auto-Negotioation process completed */
#define PHY_Linked_Status               ((u16)0x0004)      /*!< Valid link established */
#define PHY_Jabber_detection            ((u16)0x0002)      /*!< Jabber condition detected */
/** PHY_SR bit definitions */
#define PHY_DUPLEX_STATUS           	((u16)0x0004)    /*!< Configured information of Duplex: Full-duplex */
#define PHY_SPEED_STATUS            	((u16)0x0002)    /*!< Configured information of Speed: 10Mbps */
#elif defined(BCM5241_PHY)

/** PHY register set */
#define PHY_BCR                          0x00          /*!< Tranceiver Basic Control Register */
#define PHY_BSR                          0x01          /*!< Tranceiver Basic Status Register */
#define PHY_SR 							 0x18	     	/*!< Auxiliary Control/Status register */

/** PHY_BCR bit definitions */
#define PHY_Reset                       ((u16)0x8000)      /*!< PHY Reset */
#define PHY_Loopback                    ((u16)0x4000)      /*!< Select loop-back mode */
#define PHY_SpeedSelec              	 ((u16)0x2000)      /*!< Force Speed selection */
#define PHY_AutoNegotiation             ((u16)0x1000)      /*!< Enable Auto Negotiation */
#define PHY_Powerdown                   ((u16)0x0800)      /*!< Power Down */
#define PHY_Isolate                     ((u16)0x0400)      /*!< Isolate */
#define PHY_Restart_AutoNegotiation     ((u16)0x0200)      /*!< Restart Auto Negotiation */
#define PHY_DuplexMode                  ((u16)0x0100)      /*!< Select duplex mode */
#define PHY_FULLDUPLEX_100M             ((u16)0x2100)      /*!< Mask full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((u16)0x2000)      /*!< Mask half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((u16)0x0100)      /*!< Mask full-duplex mode at 10 Mb/s */
#define PHY_HALFDUPLEX_10M              ((u16)0x0000)      /*!< Mask half-duplex mode at 10 Mb/s */
/** PHY_BSR bit definitions */
#define PHY_AutoNego_Complete           ((u16)0x0020)      /*!< Auto-Negotioation process completed */
#define PHY_Linked_Status               ((u16)0x0004)      /*!< Valid link established */
#define PHY_Jabber_detection            ((u16)0x0002)      /*!< Jabber condition detected */
/** PHY_SR bit definitions */
#define PHY_DUPLEX_STATUS           	((u16)0x0001)    /*!< Configured information of Duplex: Full-duplex */
#define PHY_SPEED_STATUS            	((u16)0x0002)    /*!< Configured information of Speed: 10Mbps */
#endif

#define IS_ETH_PHY_ADDRESS(ADDRESS) ((ADDRESS) <= 0x20)
#define IS_ETH_PHY_REG(REG) (((REG) == PHY_BCR) || \
                             ((REG) == PHY_BSR) || \
                             ((REG) == PHY_SR))

void ETH_Configuration(const uint8_t* macaddr);
bool link_status(void);
bool auto_negotiation_status(void);
uint16_t link_speed(void);
bool link_full_duplex(void);


#ifdef __cplusplus
 }
#endif
#endif /* MII_RMII_H_ */
/**
 * @}
 */
