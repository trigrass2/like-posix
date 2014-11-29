/**
 * @file      startup_stm32f10x_hd.c
 * @author    Stefano Oliveri (software@stf12.net)
 * @version   V3.1.0
 * @date      22/06/2009
 * @brief     STM32F10x High Density Devices vector table for GCC toolchain.
 *            This module performs:
 *                - Set the initial SP
 *                - Set the initial PC == Reset_Handler,
 *                - Set the vector table entries with the exceptions ISR address,
 *                - Configure external SRAM mounted on STM3210E-EVAL board
 *                  to be used as data memory (optional, to be enabled by user)
 *                - Branches to main in the C library (which eventually
 *                  calls main()).
 *            After Reset the Cortex-M3 processor is in Thread mode,
 *            priority is Privileged, and the Stack is set to Main.
 * @attention modified the vector table and the Reset_Handler to use
 *            the demo with eclipse and GCC and to provide support for the
 *            newlib C runtime library.
 *            Modified by Stefano Oliveri (software@stf12.net)
 * @attention added unhandled interrupt catcher
 *            Modified by Mike Stuart (spaceorbot@gmail.com)
 *******************************************************************************
 * @copy
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING USERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STEFANO OLIVERI SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2009 Stefano Oliveri</center></h2>
 */

#include "startup.c"

/*******************************************************************************
*
*            Forward declaration of the default fault handlers.
*
*******************************************************************************/

/* External Interrupts */
void WEAK NMI_Handler(void);
void WEAK HardFault_Handler(void);
void WEAK MemManage_Handler(void);
void WEAK BusFault_Handler(void);
void WEAK UsageFault_Handler(void);
void WEAK SVC_Handler(void);
void WEAK DebugMon_Handler(void);
void WEAK PendSV_Handler(void);
void WEAK SysTick_Handler(void);
void WEAK WWDG_IRQHandler(void);
void WEAK PVD_IRQHandler(void);
void WEAK TAMPER_IRQHandler(void);
void WEAK RTC_IRQHandler(void);
void WEAK FLASH_IRQHandler(void);
void WEAK RCC_IRQHandler(void);
void WEAK EXTI0_IRQHandler(void);
void WEAK EXTI1_IRQHandler(void);
void WEAK EXTI2_IRQHandler(void);
void WEAK EXTI3_IRQHandler(void);
void WEAK EXTI4_IRQHandler(void);
void WEAK DMA1_Channel1_IRQHandler(void);
void WEAK DMA1_Channel2_IRQHandler(void);
void WEAK DMA1_Channel3_IRQHandler(void);
void WEAK DMA1_Channel4_IRQHandler(void);
void WEAK DMA1_Channel5_IRQHandler(void);
void WEAK DMA1_Channel6_IRQHandler(void);
void WEAK DMA1_Channel7_IRQHandler(void);
void WEAK ADC1_2_IRQHandler(void);
void WEAK CAN1_TX_IRQHandler(void);
void WEAK CAN1_RX0_IRQHandler(void);
void WEAK CAN1_RX1_IRQHandler(void);
void WEAK CAN1_SCE_IRQHandler(void);
void WEAK EXTI9_5_IRQHandler(void);
void WEAK TIM1_BRK_IRQHandler(void);
void WEAK TIM1_UP_IRQHandler(void);
void WEAK TIM1_TRG_COM_IRQHandler(void);
void WEAK TIM1_CC_IRQHandler(void);
void WEAK TIM2_IRQHandler(void);
void WEAK TIM3_IRQHandler(void);
void WEAK TIM4_IRQHandler(void);
void WEAK I2C1_EV_IRQHandler(void);
void WEAK I2C1_ER_IRQHandler(void);
void WEAK I2C2_EV_IRQHandler(void);
void WEAK I2C2_ER_IRQHandler(void);
void WEAK SPI1_IRQHandler(void);
void WEAK SPI2_IRQHandler(void);
void WEAK USART1_IRQHandler(void);
void WEAK USART2_IRQHandler(void);
void WEAK USART3_IRQHandler(void);
void WEAK EXTI15_10_IRQHandler(void);
void WEAK RTCAlarm_IRQHandler(void);
void WEAK OTG_FS_WKUP_IRQHandler(void);
void WEAK TIM5_IRQHandler(void);
void WEAK SPI3_IRQHandler(void);
void WEAK UART4_IRQHandler(void);
void WEAK UART5_IRQHandler(void);
void WEAK TIM6_IRQHandler(void);
void WEAK TIM7_IRQHandler(void);
void WEAK DMA2_Channel1_IRQHandler(void);
void WEAK DMA2_Channel2_IRQHandler(void);
void WEAK DMA2_Channel3_IRQHandler(void);
void WEAK DMA2_Channel4_IRQHandler(void);
void WEAK DMA2_Channel5_IRQHandler(void);
void WEAK ETH_IRQHandler(void);
void WEAK ETH_WKUP_IRQHandler(void);
void WEAK CAN2_TX_IRQHandler(void);
void WEAK CAN2_RX0_IRQHandler(void);
void WEAK CAN2_RX1_IRQHandler(void);
void WEAK CAN2_SCE_IRQHandler(void);
void WEAK OTG_FS_IRQHandler(void);


/******************************************************************************
*
* The minimal vector table for a Cortex M3.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/


__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
    (intfunc)((unsigned long)&_estack), /* The stack pointer after relocation */
    Reset_Handler,              /* Reset Handler */
    NMI_Handler,                /* NMI Handler */
    HardFault_Handler,          /* Hard Fault Handler */
    MemManage_Handler,          /* MPU Fault Handler */
    BusFault_Handler,           /* Bus Fault Handler */
    UsageFault_Handler,         /* Usage Fault Handler */
    0,                          /* Reserved */
    0,                          /* Reserved */
    0,                          /* Reserved */
    0,                          /* Reserved */
    SVC_Handler,                /* SVCall Handler */
    DebugMon_Handler,           /* Debug Monitor Handler */
    0,                          /* Reserved */
    PendSV_Handler,             /* PendSV Handler */
    SysTick_Handler,            /* SysTick Handler */
    WWDG_IRQHandler,            /* Window Watchdog */
    PVD_IRQHandler,             /* PVD through EXTI Line detect */
    TAMPER_IRQHandler,          /* Tamper */
    RTC_IRQHandler,             /* RTC */
    FLASH_IRQHandler,           /* Flash */
    RCC_IRQHandler,             /* RCC */
    EXTI0_IRQHandler,           /* EXTI Line 0 */
    EXTI1_IRQHandler,           /* EXTI Line 1 */
    EXTI2_IRQHandler,           /* EXTI Line 2 */
    EXTI3_IRQHandler,           /* EXTI Line 3 */
    EXTI4_IRQHandler,           /* EXTI Line 4 */
    DMA1_Channel1_IRQHandler,   /* DMA1 Channel 1 */
    DMA1_Channel2_IRQHandler,   /* DMA1 Channel 2 */
    DMA1_Channel3_IRQHandler,   /* DMA1 Channel 3 */
    DMA1_Channel4_IRQHandler,   /* DMA1 Channel 4 */
    DMA1_Channel5_IRQHandler,   /* DMA1 Channel 5 */
    DMA1_Channel6_IRQHandler,   /* DMA1 Channel 6 */
    DMA1_Channel7_IRQHandler,   /* DMA1 Channel 7 */
    ADC1_2_IRQHandler,          /* ADC1 & ADC2 */
    CAN1_TX_IRQHandler,  /* USB High Priority or CAN1 TX */
    CAN1_RX0_IRQHandler, /* USB Low  Priority or CAN1 RX0 */
    CAN1_RX1_IRQHandler,        /* CAN1 RX1 */
    CAN1_SCE_IRQHandler,        /* CAN1 SCE */
    EXTI9_5_IRQHandler,         /* EXTI Line 9..5 */
    TIM1_BRK_IRQHandler,        /* TIM1 Break */
    TIM1_UP_IRQHandler,         /* TIM1 Update */
    TIM1_TRG_COM_IRQHandler,    /* TIM1 Trigger and Commutation */
    TIM1_CC_IRQHandler,         /* TIM1 Capture Compare */
    TIM2_IRQHandler,            /* TIM2 */
    TIM3_IRQHandler,            /* TIM3 */
    TIM4_IRQHandler,            /* TIM4 */
    I2C1_EV_IRQHandler,         /* I2C1 Event */
    I2C1_ER_IRQHandler,         /* I2C1 Error */
    I2C2_EV_IRQHandler,         /* I2C2 Event */
    I2C2_ER_IRQHandler,         /* I2C2 Error */
    SPI1_IRQHandler,            /* SPI1 */
    SPI2_IRQHandler,            /* SPI2 */
    USART1_IRQHandler,          /* USART1 */
    USART2_IRQHandler,          /* USART2 */
    USART3_IRQHandler,          /* USART3 */
    EXTI15_10_IRQHandler,       /* EXTI Line 15..10 */
    RTCAlarm_IRQHandler,        /* RTC Alarm through EXTI Line */
    OTG_FS_WKUP_IRQHandler,       /* USB Wakeup from suspend */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    TIM5_IRQHandler,
    SPI3_IRQHandler,
    UART4_IRQHandler,
    UART5_IRQHandler,
    TIM6_IRQHandler,
    TIM7_IRQHandler,
    DMA2_Channel1_IRQHandler,
    DMA2_Channel2_IRQHandler,
    DMA2_Channel3_IRQHandler,
    DMA2_Channel4_IRQHandler,
    DMA2_Channel5_IRQHandler,
    ETH_IRQHandler,
    ETH_WKUP_IRQHandler,
    CAN2_TX_IRQHandler,
    CAN2_RX0_IRQHandler,
    CAN2_RX1_IRQHandler,
    CAN2_SCE_IRQHandler,
    OTG_FS_IRQHandler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    (intfunc)0xF1E0F85F       /* @0x1E0. This is for boot in RAM mode for
                           STM32F10x High Density devices. */
};

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

#if EXTENDED_UNHANDLED_INTERRUPT_HANDLER

static inline void __NMI_Handler(void){MESSAGE()}
static inline void __HardFault_Handler(void){MESSAGE()}
static inline void __MemManage_Handler(void){MESSAGE()}
static inline void __BusFault_Handler(void){MESSAGE()}
static inline void __UsageFault_Handler(void){MESSAGE()}
static inline void __SVC_Handler(void){MESSAGE()}
static inline void __DebugMon_Handler(void){MESSAGE()}
static inline void __PendSV_Handler(void){MESSAGE()}
static inline void __SysTick_Handler(void){MESSAGE()}
static inline void __WWDG_IRQHandler(void){MESSAGE()}
static inline void __PVD_IRQHandler(void){MESSAGE()}
static inline void __TAMPER_IRQHandler(void){MESSAGE()}
static inline void __RTC_IRQHandler(void){MESSAGE()}
static inline void __FLASH_IRQHandler(void){MESSAGE()}
static inline void __RCC_IRQHandler(void){MESSAGE()}
static inline void __EXTI0_IRQHandler(void){MESSAGE()}
static inline void __EXTI1_IRQHandler(void){MESSAGE()}
static inline void __EXTI2_IRQHandler(void){MESSAGE()}
static inline void __EXTI3_IRQHandler(void){MESSAGE()}
static inline void __EXTI4_IRQHandler(void){MESSAGE()}
static inline void __DMA1_Channel1_IRQHandler(void){MESSAGE()}
static inline void __DMA1_Channel2_IRQHandler(void){MESSAGE()}
static inline void __DMA1_Channel3_IRQHandler(void){MESSAGE()}
static inline void __DMA1_Channel4_IRQHandler(void){MESSAGE()}
static inline void __DMA1_Channel5_IRQHandler(void){MESSAGE()}
static inline void __DMA1_Channel6_IRQHandler(void){MESSAGE()}
static inline void __DMA1_Channel7_IRQHandler(void){MESSAGE()}
static inline void __ADC1_2_IRQHandler(void){MESSAGE()}
static inline void __CAN1_TX_IRQHandler(void){MESSAGE()}
static inline void __CAN1_RX0_IRQHandler(void){MESSAGE()}
static inline void __CAN1_RX1_IRQHandler(void){MESSAGE()}
static inline void __CAN1_SCE_IRQHandler(void){MESSAGE()}
static inline void __EXTI9_5_IRQHandler(void){MESSAGE()}
static inline void __TIM1_BRK_IRQHandler(void){MESSAGE()}
static inline void __TIM1_UP_IRQHandler(void){MESSAGE()}
static inline void __TIM1_TRG_COM_IRQHandler(void){MESSAGE()}
static inline void __TIM1_CC_IRQHandler(void){MESSAGE()}
static inline void __TIM2_IRQHandler(void){MESSAGE()}
static inline void __TIM3_IRQHandler(void){MESSAGE()}
static inline void __TIM4_IRQHandler(void){MESSAGE()}
static inline void __I2C1_EV_IRQHandler(void){MESSAGE()}
static inline void __I2C1_ER_IRQHandler(void){MESSAGE()}
static inline void __I2C2_EV_IRQHandler(void){MESSAGE()}
static inline void __I2C2_ER_IRQHandler(void){MESSAGE()}
static inline void __SPI1_IRQHandler(void){MESSAGE()}
static inline void __SPI2_IRQHandler(void){MESSAGE()}
static inline void __USART1_IRQHandler(void){MESSAGE()}
static inline void __USART2_IRQHandler(void){MESSAGE()}
static inline void __USART3_IRQHandler(void){MESSAGE()}
static inline void __EXTI15_10_IRQHandler(void){MESSAGE()}
static inline void __RTCAlarm_IRQHandler(void){MESSAGE()}
static inline void __OTG_FS_WKUP_IRQHandler(void){MESSAGE()}
static inline void __TIM5_IRQHandler(void){MESSAGE()}
static inline void __SPI3_IRQHandler(void){MESSAGE()}
static inline void __UART4_IRQHandler(void){MESSAGE()}
static inline void __UART5_IRQHandler(void){MESSAGE()}
static inline void __TIM6_IRQHandler(void){MESSAGE()}
static inline void __TIM7_IRQHandler(void){MESSAGE()}
static inline void __DMA2_Channel1_IRQHandler(void){MESSAGE()}
static inline void __DMA2_Channel2_IRQHandler(void){MESSAGE()}
static inline void __DMA2_Channel3_IRQHandler(void){MESSAGE()}
static inline void __DMA2_Channel4_IRQHandler(void){MESSAGE()}
static inline void __DMA2_Channel5_IRQHandler(void){MESSAGE()}
static inline void __ETH_IRQHandler(void){MESSAGE()}
static inline void __ETH_WKUP_IRQHandler(void){MESSAGE()}
static inline void __CAN2_TX_IRQHandler(void){MESSAGE()}
static inline void __CAN2_RX0_IRQHandler(void){MESSAGE()}
static inline void __CAN2_RX1_IRQHandler(void){MESSAGE()}
static inline void __CAN2_SCE_IRQHandler(void){MESSAGE()}
static inline void __OTG_FS_IRQHandler(void){MESSAGE()}

#pragma weak NMI_Handler = __NMI_Handler
#pragma weak HardFault_Handler = __HardFault_Handler
#pragma weak MemManage_Handler = __MemManage_Handler
#pragma weak BusFault_Handler = __BusFault_Handler
#pragma weak UsageFault_Handler = __UsageFault_Handler
#pragma weak SVC_Handler = __SVC_Handler
#pragma weak DebugMon_Handler = __DebugMon_Handler
#pragma weak PendSV_Handler = __PendSV_Handler
#pragma weak SysTick_Handler = __SysTick_Handler
#pragma weak WWDG_IRQHandler = __WWDG_IRQHandler
#pragma weak PVD_IRQHandler = __PVD_IRQHandler
#pragma weak TAMPER_IRQHandler = __TAMPER_IRQHandler
#pragma weak RTC_IRQHandler = __RTC_IRQHandler
#pragma weak FLASH_IRQHandler = __FLASH_IRQHandler
#pragma weak RCC_IRQHandler = __RCC_IRQHandler
#pragma weak EXTI0_IRQHandler = __EXTI0_IRQHandler
#pragma weak EXTI1_IRQHandler = __EXTI1_IRQHandler
#pragma weak EXTI2_IRQHandler = __EXTI2_IRQHandler
#pragma weak EXTI3_IRQHandler = __EXTI3_IRQHandler
#pragma weak EXTI4_IRQHandler = __EXTI4_IRQHandler
#pragma weak DMA1_Channel1_IRQHandler = __DMA1_Channel1_IRQHandler
#pragma weak DMA1_Channel2_IRQHandler = __DMA1_Channel2_IRQHandler
#pragma weak DMA1_Channel3_IRQHandler = __DMA1_Channel3_IRQHandler
#pragma weak DMA1_Channel4_IRQHandler = __DMA1_Channel4_IRQHandler
#pragma weak DMA1_Channel5_IRQHandler = __DMA1_Channel5_IRQHandler
#pragma weak DMA1_Channel6_IRQHandler = __DMA1_Channel6_IRQHandler
#pragma weak DMA1_Channel7_IRQHandler = __DMA1_Channel7_IRQHandler
#pragma weak ADC1_2_IRQHandler = __ADC1_2_IRQHandler
#pragma weak CAN1_TX_IRQHandler = __CAN1_TX_IRQHandler
#pragma weak CAN1_RX0_IRQHandler = __CAN1_RX0_IRQHandler
#pragma weak CAN1_RX1_IRQHandler = __CAN1_RX1_IRQHandler
#pragma weak CAN1_SCE_IRQHandler = __CAN1_SCE_IRQHandler
#pragma weak EXTI9_5_IRQHandler = __EXTI9_5_IRQHandler
#pragma weak TIM1_BRK_IRQHandler = __TIM1_BRK_IRQHandler
#pragma weak TIM1_UP_IRQHandler = __TIM1_UP_IRQHandler
#pragma weak TIM1_TRG_COM_IRQHandler = __TIM1_TRG_COM_IRQHandler
#pragma weak TIM1_CC_IRQHandler = __TIM1_CC_IRQHandler
#pragma weak TIM2_IRQHandler = __TIM2_IRQHandler
#pragma weak TIM3_IRQHandler = __TIM3_IRQHandler
#pragma weak TIM4_IRQHandler = __TIM4_IRQHandler
#pragma weak I2C1_EV_IRQHandler = __I2C1_EV_IRQHandler
#pragma weak I2C1_ER_IRQHandler = __I2C1_ER_IRQHandler
#pragma weak I2C2_EV_IRQHandler = __I2C2_EV_IRQHandler
#pragma weak I2C2_ER_IRQHandler = __I2C2_ER_IRQHandler
#pragma weak SPI1_IRQHandler = __SPI1_IRQHandler
#pragma weak SPI2_IRQHandler = __SPI2_IRQHandler
#pragma weak USART1_IRQHandler = __USART1_IRQHandler
#pragma weak USART2_IRQHandler = __USART2_IRQHandler
#pragma weak USART3_IRQHandler = __USART3_IRQHandler
#pragma weak EXTI15_10_IRQHandler = __EXTI15_10_IRQHandler
#pragma weak RTCAlarm_IRQHandler = __RTCAlarm_IRQHandler
#pragma weak OTG_FS_WKUP_IRQHandler = __OTG_FS_WKUP_IRQHandler
#pragma weak TIM5_IRQHandler = __TIM5_IRQHandler
#pragma weak SPI3_IRQHandler = __SPI3_IRQHandler
#pragma weak UART4_IRQHandler = __UART4_IRQHandler
#pragma weak UART5_IRQHandler = __UART5_IRQHandler
#pragma weak TIM6_IRQHandler = __TIM6_IRQHandler
#pragma weak TIM7_IRQHandler = __TIM7_IRQHandler
#pragma weak DMA2_Channel1_IRQHandler = __DMA2_Channel1_IRQHandler
#pragma weak DMA2_Channel2_IRQHandler = __DMA2_Channel2_IRQHandler
#pragma weak DMA2_Channel3_IRQHandler = __DMA2_Channel3_IRQHandler
#pragma weak DMA2_Channel4_IRQHandler = __DMA2_Channel4_IRQHandler
#pragma weak DMA2_Channel5_IRQHandler = __DMA2_Channel5_IRQHandler
#pragma weak ETH_IRQHandler = __ETH_IRQHandler
#pragma weak ETH_WKUP_IRQHandler = __ETH_WKUP_IRQHandler
#pragma weak CAN2_TX_IRQHandler = __CAN2_TX_IRQHandler
#pragma weak CAN2_RX0_IRQHandler = __CAN2_RX0_IRQHandler
#pragma weak CAN2_RX1_IRQHandler = __CAN2_RX1_IRQHandler
#pragma weak CAN2_SCE_IRQHandler = __CAN2_SCE_IRQHandler
#pragma weak OTG_FS_IRQHandler = __OTG_FS_IRQHandler

#else // EXTENDED_UNHANDLED_INTERRUPT_HANDLER

#pragma weak NMI_Handler = Default_Handler
#pragma weak HardFault_Handler = Default_Handler
#pragma weak MemManage_Handler = Default_Handler
#pragma weak BusFault_Handler = Default_Handler
#pragma weak UsageFault_Handler = Default_Handler
#pragma weak SVC_Handler = Default_Handler
#pragma weak DebugMon_Handler = Default_Handler
#pragma weak PendSV_Handler = Default_Handler
#pragma weak SysTick_Handler = Default_Handler
#pragma weak WWDG_IRQHandler = Default_Handler
#pragma weak PVD_IRQHandler = Default_Handler
#pragma weak TAMPER_IRQHandler = Default_Handler
#pragma weak RTC_IRQHandler = Default_Handler
#pragma weak FLASH_IRQHandler = Default_Handler
#pragma weak RCC_IRQHandler = Default_Handler
#pragma weak EXTI0_IRQHandler = Default_Handler
#pragma weak EXTI1_IRQHandler = Default_Handler
#pragma weak EXTI2_IRQHandler = Default_Handler
#pragma weak EXTI3_IRQHandler = Default_Handler
#pragma weak EXTI4_IRQHandler = Default_Handler
#pragma weak DMA1_Channel1_IRQHandler = Default_Handler
#pragma weak DMA1_Channel2_IRQHandler = Default_Handler
#pragma weak DMA1_Channel3_IRQHandler = Default_Handler
#pragma weak DMA1_Channel4_IRQHandler = Default_Handler
#pragma weak DMA1_Channel5_IRQHandler = Default_Handler
#pragma weak DMA1_Channel6_IRQHandler = Default_Handler
#pragma weak DMA1_Channel7_IRQHandler = Default_Handler
#pragma weak ADC1_2_IRQHandler = Default_Handler
#pragma weak CAN1_TX_IRQHandler = Default_Handler
#pragma weak CAN1_RX0_IRQHandler = Default_Handler
#pragma weak CAN1_RX1_IRQHandler = Default_Handler
#pragma weak CAN1_SCE_IRQHandler = Default_Handler
#pragma weak EXTI9_5_IRQHandler = Default_Handler
#pragma weak TIM1_BRK_IRQHandler = Default_Handler
#pragma weak TIM1_UP_IRQHandler = Default_Handler
#pragma weak TIM1_TRG_COM_IRQHandler = Default_Handler
#pragma weak TIM1_CC_IRQHandler = Default_Handler
#pragma weak TIM2_IRQHandler = Default_Handler
#pragma weak TIM3_IRQHandler = Default_Handler
#pragma weak TIM4_IRQHandler = Default_Handler
#pragma weak I2C1_EV_IRQHandler = Default_Handler
#pragma weak I2C1_ER_IRQHandler = Default_Handler
#pragma weak I2C2_EV_IRQHandler = Default_Handler
#pragma weak I2C2_ER_IRQHandler = Default_Handler
#pragma weak SPI1_IRQHandler = Default_Handler
#pragma weak SPI2_IRQHandler = Default_Handler
#pragma weak USART1_IRQHandler = Default_Handler
#pragma weak USART2_IRQHandler = Default_Handler
#pragma weak USART3_IRQHandler = Default_Handler
#pragma weak EXTI15_10_IRQHandler = Default_Handler
#pragma weak RTCAlarm_IRQHandler = Default_Handler
#pragma weak OTG_FS_WKUP_IRQHandler = Default_Handler
#pragma weak TIM5_IRQHandler = Default_Handler
#pragma weak SPI3_IRQHandler = Default_Handler
#pragma weak UART4_IRQHandler = Default_Handler
#pragma weak UART5_IRQHandler = Default_Handler
#pragma weak TIM6_IRQHandler = Default_Handler
#pragma weak TIM7_IRQHandler = Default_Handler
#pragma weak DMA2_Channel1_IRQHandler = Default_Handler
#pragma weak DMA2_Channel2_IRQHandler = Default_Handler
#pragma weak DMA2_Channel3_IRQHandler = Default_Handler
#pragma weak DMA2_Channel4_IRQHandler = Default_Handler
#pragma weak DMA2_Channel5_IRQHandler = Default_Handler
#pragma weak ETH_IRQHandler = Default_Handler
#pragma weak ETH_WKUP_IRQHandler = Default_Handler
#pragma weak CAN2_TX_IRQHandler = Default_Handler
#pragma weak CAN2_RX0_IRQHandler = Default_Handler
#pragma weak CAN2_RX1_IRQHandler = Default_Handler
#pragma weak CAN2_SCE_IRQHandler = Default_Handler
#pragma weak OTG_FS_IRQHandler = Default_Handler

#endif // EXTENDED_UNHANDLED_INTERRUPT_HANDLER



/********************* (C) COPYRIGHT 2007 STMicroelectronics  *****END OF FILE****/
