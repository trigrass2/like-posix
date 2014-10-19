/**
 ******************************************************************************
 * @file      startup_stm32f10x_md.c
 * @author    MCD Application Team, modified by Martin Thomas
 * @version   V3.0.0-mthomas2
 * @date      5. Dec. 2009
 * @brief     STM32F10x Medium Density Devices vector table for GNU toolchain.
 *            This module performs:
 *                - Set the initial SP
 *                - Set the initial PC == Reset_Handler,
 *                - Set the vector table entries with the exceptions ISR address,
 *                - Branches to main in the C library (which eventually
 *                  calls main()).
 *            After Reset the Cortex-M3 processor is in Thread mode,
 *            priority is Privileged, and the Stack is set to Main.
 *******************************************************************************
 * @copy
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
 */

/* Modified by Martin Thomas
   - to take VECT_TAB_RAM setting into account, also see the linker-script
   - to avoid warning "ISO C forbids initialization between function pointer and 'void *'".
   - added optional startup-delay to avoid unwanted operations while connecting with
     debugger/programmer
   - tested with the GNU arm-eabi toolchain as in CS G++ lite Q1/2009-161
   - minor modification in .data copy to avoid copy while "run from RAM"
     during debugging
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define WEAK        __attribute__ ((weak))
#define BSS_FILL    0
#define STACK_FILL  0xA5A5A5A5

#ifndef EXTENDED_UNHANDLED_INTERRUPT_HANDLER
#define EXTENDED_UNHANDLED_INTERRUPT_HANDLER 0
#endif

/* Private macro -------------------------------------------------------------*/
extern unsigned long _etext;
/* start address for the initialization values of the .data section.
defined in linker script */
extern unsigned long _sidata;

/* start address for the .data section. defined in linker script */
extern unsigned long _sdata;

/* end address for the .data section. defined in linker script */
extern unsigned long _edata;

/* start address for the .bss section. defined in linker script */
extern unsigned long _sbss;

/* end address for the .bss section. defined in linker script */
extern unsigned long _ebss;

/* init value for the stack pointer. defined in linker script */
extern unsigned long _estack;

/* size of the stack. defined in linker script */
extern unsigned long _sstack;

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void Reset_Handler() __attribute__((__interrupt__));
void __Init_Data();
void Default_Handler();
void SystemInit_ExtMemCtl_Dummy();

/* External function prototypes ----------------------------------------------*/
extern int main();                /* Application's main function */
extern void __libc_init_array();  /* calls CTORS of static objects */

/*******************************************************************************
*
*            Forward declaration of the default fault handlers.
*
*******************************************************************************/
//mthomas void WEAK Reset_Handler(void);
void WEAK NMI_Handler(void);
void WEAK HardFault_Handler(void);
void WEAK MemManage_Handler(void);
void WEAK BusFault_Handler(void);
void WEAK UsageFault_Handler(void);
void WEAK SVC_Handler(void);
void WEAK DebugMon_Handler(void);
void WEAK PendSV_Handler(void);
void WEAK SysTick_Handler(void);

/* External Interrupts */
void WEAK WWDG_IRQHandler(void);                /* Window WatchDog              */
void WEAK PVD_IRQHandler(void);                 /* PVD through EXTI Line detection */
void WEAK TAMP_STAMP_IRQHandler(void);          /* Tamper and TimeStamps through the EXTI line */
void WEAK RTC_WKUP_IRQHandler(void);            /* RTC Wakeup through the EXTI line */
void WEAK FLASH_IRQHandler(void);               /* FLASH                        */
void WEAK RCC_IRQHandler(void);                 /* RCC                          */
void WEAK EXTI0_IRQHandler(void);               /* EXTI Line0                   */
void WEAK EXTI1_IRQHandler(void);               /* EXTI Line1                   */
void WEAK EXTI2_IRQHandler(void);               /* EXTI Line2                   */
void WEAK EXTI3_IRQHandler(void);               /* EXTI Line3                   */
void WEAK EXTI4_IRQHandler(void);               /* EXTI Line4                   */
void WEAK DMA1_Stream0_IRQHandler(void);        /* DMA1 Stream 0                */
void WEAK DMA1_Stream1_IRQHandler(void);        /* DMA1 Stream 1                */
void WEAK DMA1_Stream2_IRQHandler(void);        /* DMA1 Stream 2                */
void WEAK DMA1_Stream3_IRQHandler(void);        /* DMA1 Stream 3                */
void WEAK DMA1_Stream4_IRQHandler(void);        /* DMA1 Stream 4                */
void WEAK DMA1_Stream5_IRQHandler(void);        /* DMA1 Stream 5                */
void WEAK DMA1_Stream6_IRQHandler(void);        /* DMA1 Stream 6                */
void WEAK ADC_IRQHandler(void);                 /* ADC1, ADC2 and ADC3s         */
void WEAK CAN1_TX_IRQHandler(void);             /* CAN1 TX                      */
void WEAK CAN1_RX0_IRQHandler(void);            /* CAN1 RX0                     */
void WEAK CAN1_RX1_IRQHandler(void);            /* CAN1 RX1                     */
void WEAK CAN1_SCE_IRQHandler(void);            /* CAN1 SCE                     */
void WEAK EXTI9_5_IRQHandler(void);             /* External Line[9:5]s          */
void WEAK TIM1_BRK_TIM9_IRQHandler(void);       /* TIM1 Break and TIM9          */
void WEAK TIM1_UP_TIM10_IRQHandler(void);       /* TIM1 Update and TIM10        */
void WEAK TIM1_TRG_COM_TIM11_IRQHandler(void);   /* TIM1 Trigger and Commutation and TIM11 */
void WEAK TIM1_CC_IRQHandler(void);             /* TIM1 Capture Compare         */
void WEAK TIM2_IRQHandler(void);                /* TIM2                         */
void WEAK TIM3_IRQHandler(void);                /* TIM3                         */
void WEAK TIM4_IRQHandler(void);                /* TIM4                         */
void WEAK I2C1_EV_IRQHandler(void);             /* I2C1 Event                   */
void WEAK I2C1_ER_IRQHandler(void);             /* I2C1 Error                   */
void WEAK I2C2_EV_IRQHandler(void);             /* I2C2 Event                   */
void WEAK I2C2_ER_IRQHandler(void);             /* I2C2 Error                   */
void WEAK SPI1_IRQHandler(void);                /* SPI1                         */
void WEAK SPI2_IRQHandler(void);                /* SPI2                         */
void WEAK USART1_IRQHandler(void);              /* USART1                       */
void WEAK USART2_IRQHandler(void);              /* USART2                       */
void WEAK USART3_IRQHandler(void);              /* USART3                       */
void WEAK EXTI15_10_IRQHandler(void);           /* External Line[15:10]s        */
void WEAK RTC_Alarm_IRQHandler(void);           /* RTC Alarm (A and B) through EXTI Line */
void WEAK OTG_FS_WKUP_IRQHandler(void);         /* USB OTG FS Wakeup through EXTI line */
void WEAK TIM8_BRK_TIM12_IRQHandler(void);      /* TIM8 Break and TIM12         */
void WEAK TIM8_UP_TIM13_IRQHandler(void);       /* TIM8 Update and TIM13        */
void WEAK TIM8_TRG_COM_TIM14_IRQHandler(void);   /* TIM8 Trigger and Commutation and TIM14 */
void WEAK TIM8_CC_IRQHandler(void);             /* TIM8 Capture Compare         */
void WEAK DMA1_Stream7_IRQHandler(void);        /* DMA1 Stream7                 */
void WEAK FSMC_IRQHandler(void);                /* FSMC                         */
void WEAK SDIO_IRQHandler(void);                /* SDIO                         */
void WEAK TIM5_IRQHandler(void);                /* TIM5                         */
void WEAK SPI3_IRQHandler(void);                /* SPI3                         */
void WEAK UART4_IRQHandler(void);               /* UART4                        */
void WEAK UART5_IRQHandler(void);               /* UART5                        */
void WEAK TIM6_DAC_IRQHandler(void);            /* TIM6 and DAC1&2 underrun errors */
void WEAK TIM7_IRQHandler(void);                /* TIM7                         */
void WEAK DMA2_Stream0_IRQHandler(void);        /* DMA2 Stream 0                */
void WEAK DMA2_Stream1_IRQHandler(void);        /* DMA2 Stream 1                */
void WEAK DMA2_Stream2_IRQHandler(void);        /* DMA2 Stream 2                */
void WEAK DMA2_Stream3_IRQHandler(void);        /* DMA2 Stream 3                */
void WEAK DMA2_Stream4_IRQHandler(void);        /* DMA2 Stream 4                */
void WEAK ETH_IRQHandler(void);                 /* Ethernet                     */
void WEAK ETH_WKUP_IRQHandler(void);            /* Ethernet Wakeup through EXTI line */
void WEAK CAN2_TX_IRQHandler(void);             /* CAN2 TX                      */
void WEAK CAN2_RX0_IRQHandler(void);            /* CAN2 RX0                     */
void WEAK CAN2_RX1_IRQHandler(void);            /* CAN2 RX1                     */
void WEAK CAN2_SCE_IRQHandler(void);            /* CAN2 SCE                     */
void WEAK OTG_FS_IRQHandler(void);              /* USB OTG FS                   */
void WEAK DMA2_Stream5_IRQHandler(void);        /* DMA2 Stream 5                */
void WEAK DMA2_Stream6_IRQHandler(void);        /* DMA2 Stream 6                */
void WEAK DMA2_Stream7_IRQHandler(void);        /* DMA2 Stream 7                */
void WEAK USART6_IRQHandler(void);              /* USART6                       */
void WEAK I2C3_EV_IRQHandler(void);             /* I2C3 event                   */
void WEAK I2C3_ER_IRQHandler(void);             /* I2C3 error                   */
void WEAK OTG_HS_EP1_OUT_IRQHandler(void);      /* USB OTG HS End Point 1 Out   */
void WEAK OTG_HS_EP1_IN_IRQHandler(void);       /* USB OTG HS End Point 1 In    */
void WEAK OTG_HS_WKUP_IRQHandler(void);         /* USB OTG HS Wakeup through EXTI */
void WEAK OTG_HS_IRQHandler(void);              /* USB OTG HS                   */
void WEAK DCMI_IRQHandler(void);                /* DCMI                         */
void WEAK CRYP_IRQHandler(void);                /* CRYP crypto                  */
void WEAK HASH_RNG_IRQHandler(void);            /* Hash and Rng                 */
void WEAK FPU_IRQHandler(void);                    /* FPU                          */



/* Private functions ---------------------------------------------------------*/
/******************************************************************************
*
* mthomas: If been built with VECT_TAB_RAM this creates two tables:
* (1) a minimal table (stack-pointer, reset-vector) used during startup
*     before relocation of the vector table using SCB_VTOR
* (2) a full table which is copied to RAM and used after vector relocation
*     (NVIC_SetVectorTable)
* If been built without VECT_TAB_RAM the following comment from STM is valid:
* The minimal vector table for a Cortex M3.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/


__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
  (void (*)(void))((unsigned long)&_estack), // The initial stack pointer
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

    /* External Interrupts */
    WWDG_IRQHandler,                  /* Window WatchDog              */
    PVD_IRQHandler,                   /* PVD through EXTI Line detection */
    TAMP_STAMP_IRQHandler,            /* Tamper and TimeStamps through the EXTI line */
    RTC_WKUP_IRQHandler,              /* RTC Wakeup through the EXTI line */
    FLASH_IRQHandler,                 /* FLASH                        */
    RCC_IRQHandler,                   /* RCC                          */
    EXTI0_IRQHandler,                 /* EXTI Line0                   */
    EXTI1_IRQHandler,                 /* EXTI Line1                   */
    EXTI2_IRQHandler,                 /* EXTI Line2                   */
    EXTI3_IRQHandler,                 /* EXTI Line3                   */
    EXTI4_IRQHandler,                 /* EXTI Line4                   */
    DMA1_Stream0_IRQHandler,          /* DMA1 Stream 0                */
    DMA1_Stream1_IRQHandler,          /* DMA1 Stream 1                */
    DMA1_Stream2_IRQHandler,          /* DMA1 Stream 2                */
    DMA1_Stream3_IRQHandler,          /* DMA1 Stream 3                */
    DMA1_Stream4_IRQHandler,          /* DMA1 Stream 4                */
    DMA1_Stream5_IRQHandler,          /* DMA1 Stream 5                */
    DMA1_Stream6_IRQHandler,          /* DMA1 Stream 6                */
    ADC_IRQHandler,                   /* ADC1, ADC2 and ADC3s         */
    CAN1_TX_IRQHandler,               /* CAN1 TX                      */
    CAN1_RX0_IRQHandler,              /* CAN1 RX0                     */
    CAN1_RX1_IRQHandler,              /* CAN1 RX1                     */
    CAN1_SCE_IRQHandler,              /* CAN1 SCE                     */
    EXTI9_5_IRQHandler,               /* External Line[9:5]s          */
    TIM1_BRK_TIM9_IRQHandler,         /* TIM1 Break and TIM9          */
    TIM1_UP_TIM10_IRQHandler,         /* TIM1 Update and TIM10        */
    TIM1_TRG_COM_TIM11_IRQHandler,     /* TIM1 Trigger and Commutation and TIM11 */
    TIM1_CC_IRQHandler,               /* TIM1 Capture Compare         */
    TIM2_IRQHandler,                  /* TIM2                         */
    TIM3_IRQHandler,                  /* TIM3                         */
    TIM4_IRQHandler,                  /* TIM4                         */
    I2C1_EV_IRQHandler,               /* I2C1 Event                   */
    I2C1_ER_IRQHandler,               /* I2C1 Error                   */
    I2C2_EV_IRQHandler,               /* I2C2 Event                   */
    I2C2_ER_IRQHandler,               /* I2C2 Error                   */
    SPI1_IRQHandler,                  /* SPI1                         */
    SPI2_IRQHandler,                  /* SPI2                         */
    USART1_IRQHandler,                /* USART1                       */
    USART2_IRQHandler,                /* USART2                       */
    USART3_IRQHandler,                /* USART3                       */
    EXTI15_10_IRQHandler,             /* External Line[15:10]s        */
    RTC_Alarm_IRQHandler,             /* RTC Alarm (A and B) through EXTI Line */
    OTG_FS_WKUP_IRQHandler,           /* USB OTG FS Wakeup through EXTI line */
    TIM8_BRK_TIM12_IRQHandler,        /* TIM8 Break and TIM12         */
    TIM8_UP_TIM13_IRQHandler,         /* TIM8 Update and TIM13        */
    TIM8_TRG_COM_TIM14_IRQHandler,     /* TIM8 Trigger and Commutation and TIM14 */
    TIM8_CC_IRQHandler,               /* TIM8 Capture Compare         */
    DMA1_Stream7_IRQHandler,          /* DMA1 Stream7                 */
    FSMC_IRQHandler,                  /* FSMC                         */
    SDIO_IRQHandler,                  /* SDIO                         */
    TIM5_IRQHandler,                  /* TIM5                         */
    SPI3_IRQHandler,                  /* SPI3                         */
    UART4_IRQHandler,                 /* UART4                        */
    UART5_IRQHandler,                 /* UART5                        */
    TIM6_DAC_IRQHandler,              /* TIM6 and DAC1&2 underrun errors */
    TIM7_IRQHandler,                  /* TIM7                         */
    DMA2_Stream0_IRQHandler,          /* DMA2 Stream 0                */
    DMA2_Stream1_IRQHandler,          /* DMA2 Stream 1                */
    DMA2_Stream2_IRQHandler,          /* DMA2 Stream 2                */
    DMA2_Stream3_IRQHandler,          /* DMA2 Stream 3                */
    DMA2_Stream4_IRQHandler,          /* DMA2 Stream 4                */
    ETH_IRQHandler,                   /* Ethernet                     */
    ETH_WKUP_IRQHandler,              /* Ethernet Wakeup through EXTI line */
    CAN2_TX_IRQHandler,               /* CAN2 TX                      */
    CAN2_RX0_IRQHandler,              /* CAN2 RX0                     */
    CAN2_RX1_IRQHandler,              /* CAN2 RX1                     */
    CAN2_SCE_IRQHandler,              /* CAN2 SCE                     */
    OTG_FS_IRQHandler,                /* USB OTG FS                   */
    DMA2_Stream5_IRQHandler,          /* DMA2 Stream 5                */
    DMA2_Stream6_IRQHandler,          /* DMA2 Stream 6                */
    DMA2_Stream7_IRQHandler,          /* DMA2 Stream 7                */
    USART6_IRQHandler,                /* USART6                       */
    I2C3_EV_IRQHandler,               /* I2C3 event                   */
    I2C3_ER_IRQHandler,               /* I2C3 error                   */
    OTG_HS_EP1_OUT_IRQHandler,        /* USB OTG HS End Point 1 Out   */
    OTG_HS_EP1_IN_IRQHandler,         /* USB OTG HS End Point 1 In    */
    OTG_HS_WKUP_IRQHandler,           /* USB OTG HS Wakeup through EXTI */
    OTG_HS_IRQHandler,                /* USB OTG HS                   */
    DCMI_IRQHandler,                  /* DCMI                         */
    CRYP_IRQHandler,                  /* CRYP crypto                  */
    HASH_RNG_IRQHandler,              /* Hash and Rng                 */
    FPU_IRQHandler                    /* FPU                          */
};

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called.
*/

void Reset_Handler()
{
	/* Initialize data and bss */
	__Init_Data();

	/* Call CTORS of static objects */
	__libc_init_array();

	/* Call Clock/RCC init */
	SystemInit();

	// call init_target (in board_config.c)
	init_target();

	/* Call the application's entry point.*/
	main();

	while(1);
}

/**
 * @brief  initializes data and bss sections
 */
void __Init_Data()
{
    unsigned long *pulSrc, *pulDest;

    /* Copy the data segment initializers from flash to SRAM */
    pulSrc  = &_sidata;
    pulDest = &_sdata;

    if(pulSrc != pulDest)
    {
        while(pulDest < &_edata)
            *(pulDest++) = *(pulSrc++);
    }

    /* Zero fill the bss segment. */
    for(pulDest = &_sbss; pulDest < &_ebss; pulDest++)
        *pulDest = BSS_FILL;

    // Fill the stack with a known value.
    for(pulDest = &_sstack; pulDest < &_estack; pulDest++)
        *pulDest = STACK_FILL;
}

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

#if EXTENDED_UNHANDLED_INTERRUPT_HANDLER

/**
 * @brief  unexpected interrupt handler
*/
void Default_Handler(const char* file, const char* function, const char* line)
{
    printf("unhandled interrupt: %s, %s, line %s\n", file, function, line);
    while(1);
}

#define MESSAGE() Default_Handler(__FILE__, __FUNCTION__, __LINE__);

/* External Interrupts */
static inline void __WWDG_IRQHandler(void){MESSAGE()}                /* Window WatchDog              */
static inline void __PVD_IRQHandler(void){MESSAGE()}                 /* PVD through EXTI Line detection */
static inline void __TAMP_STAMP_IRQHandler(void){MESSAGE()}          /* Tamper and TimeStamps through the EXTI line */
static inline void __RTC_WKUP_IRQHandler(void){MESSAGE()}            /* RTC Wakeup through the EXTI line */
static inline void __FLASH_IRQHandler(void){MESSAGE()}               /* FLASH                        */
static inline void __RCC_IRQHandler(void){MESSAGE()}                 /* RCC                          */
static inline void __EXTI0_IRQHandler(void){MESSAGE()}               /* EXTI Line0                   */
static inline void __EXTI1_IRQHandler(void){MESSAGE()}               /* EXTI Line1                   */
static inline void __EXTI2_IRQHandler(void){MESSAGE()}               /* EXTI Line2                   */
static inline void __EXTI3_IRQHandler(void){MESSAGE()}               /* EXTI Line3                   */
static inline void __EXTI4_IRQHandler(void){MESSAGE()}               /* EXTI Line4                   */
static inline void __DMA1_Stream0_IRQHandler(void){MESSAGE()}        /* DMA1 Stream 0                */
static inline void __DMA1_Stream1_IRQHandler(void){MESSAGE()}        /* DMA1 Stream 1                */
static inline void __DMA1_Stream2_IRQHandler(void){MESSAGE()}        /* DMA1 Stream 2                */
static inline void __DMA1_Stream3_IRQHandler(void){MESSAGE()}        /* DMA1 Stream 3                */
static inline void __DMA1_Stream4_IRQHandler(void){MESSAGE()}        /* DMA1 Stream 4                */
static inline void __DMA1_Stream5_IRQHandler(void){MESSAGE()}        /* DMA1 Stream 5                */
static inline void __DMA1_Stream6_IRQHandler(void){MESSAGE()}        /* DMA1 Stream 6                */
static inline void __ADC_IRQHandler(void){MESSAGE()}                 /* ADC1, ADC2 and ADC3s         */
static inline void __CAN1_TX_IRQHandler(void){MESSAGE()}             /* CAN1 TX                      */
static inline void __CAN1_RX0_IRQHandler(void){MESSAGE()}            /* CAN1 RX0                     */
static inline void __CAN1_RX1_IRQHandler(void){MESSAGE()}            /* CAN1 RX1                     */
static inline void __CAN1_SCE_IRQHandler(void){MESSAGE()}            /* CAN1 SCE                     */
static inline void __EXTI9_5_IRQHandler(void){MESSAGE()}             /* External Line[9:5]s          */
static inline void __TIM1_BRK_TIM9_IRQHandler(void){MESSAGE()}       /* TIM1 Break and TIM9          */
static inline void __TIM1_UP_TIM10_IRQHandler(void){MESSAGE()}       /* TIM1 Update and TIM10        */
static inline void __TIM1_TRG_COM_TIM11_IRQHandler(void){MESSAGE()}   /* TIM1 Trigger and Commutation and TIM11 */
static inline void __TIM1_CC_IRQHandler(void){MESSAGE()}             /* TIM1 Capture Compare         */
static inline void __TIM2_IRQHandler(void){MESSAGE()}                /* TIM2                         */
static inline void __TIM3_IRQHandler(void){MESSAGE()}                /* TIM3                         */
static inline void __TIM4_IRQHandler(void){MESSAGE()}                /* TIM4                         */
static inline void __I2C1_EV_IRQHandler(void){MESSAGE()}             /* I2C1 Event                   */
static inline void __I2C1_ER_IRQHandler(void){MESSAGE()}             /* I2C1 Error                   */
static inline void __I2C2_EV_IRQHandler(void){MESSAGE()}             /* I2C2 Event                   */
static inline void __I2C2_ER_IRQHandler(void){MESSAGE()}             /* I2C2 Error                   */
static inline void __SPI1_IRQHandler(void){MESSAGE()}                /* SPI1                         */
static inline void __SPI2_IRQHandler(void){MESSAGE()}                /* SPI2                         */
static inline void __USART1_IRQHandler(void){MESSAGE()}              /* USART1                       */
static inline void __USART2_IRQHandler(void){MESSAGE()}              /* USART2                       */
static inline void __USART3_IRQHandler(void){MESSAGE()}              /* USART3                       */
static inline void __EXTI15_10_IRQHandler(void){MESSAGE()}           /* External Line[15:10]s        */
static inline void __RTC_Alarm_IRQHandler(void){MESSAGE()}           /* RTC Alarm (A and B) through EXTI Line */
static inline void __OTG_FS_WKUP_IRQHandler(void){MESSAGE()}         /* USB OTG FS Wakeup through EXTI line */
static inline void __TIM8_BRK_TIM12_IRQHandler(void){MESSAGE()}      /* TIM8 Break and TIM12         */
static inline void __TIM8_UP_TIM13_IRQHandler(void){MESSAGE()}       /* TIM8 Update and TIM13        */
static inline void __TIM8_TRG_COM_TIM14_IRQHandler(void){MESSAGE()}   /* TIM8 Trigger and Commutation and TIM14 */
static inline void __TIM8_CC_IRQHandler(void){MESSAGE()}             /* TIM8 Capture Compare         */
static inline void __DMA1_Stream7_IRQHandler(void){MESSAGE()}        /* DMA1 Stream7                 */
static inline void __FSMC_IRQHandler(void){MESSAGE()}                /* FSMC                         */
static inline void __SDIO_IRQHandler(void){MESSAGE()}                /* SDIO                         */
static inline void __TIM5_IRQHandler(void){MESSAGE()}                /* TIM5                         */
static inline void __SPI3_IRQHandler(void){MESSAGE()}                /* SPI3                         */
static inline void __UART4_IRQHandler(void){MESSAGE()}               /* UART4                        */
static inline void __UART5_IRQHandler(void){MESSAGE()}               /* UART5                        */
static inline void __TIM6_DAC_IRQHandler(void){MESSAGE()}            /* TIM6 and DAC1&2 underrun errors */
static inline void __TIM7_IRQHandler(void){MESSAGE()}                /* TIM7                         */
static inline void __DMA2_Stream0_IRQHandler(void){MESSAGE()}        /* DMA2 Stream 0                */
static inline void __DMA2_Stream1_IRQHandler(void){MESSAGE()}        /* DMA2 Stream 1                */
static inline void __DMA2_Stream2_IRQHandler(void){MESSAGE()}        /* DMA2 Stream 2                */
static inline void __DMA2_Stream3_IRQHandler(void){MESSAGE()}        /* DMA2 Stream 3                */
static inline void __DMA2_Stream4_IRQHandler(void){MESSAGE()}        /* DMA2 Stream 4                */
static inline void __ETH_IRQHandler(void){MESSAGE()}                 /* Ethernet                     */
static inline void __ETH_WKUP_IRQHandler(void){MESSAGE()}            /* Ethernet Wakeup through EXTI line */
static inline void __CAN2_TX_IRQHandler(void){MESSAGE()}             /* CAN2 TX                      */
static inline void __CAN2_RX0_IRQHandler(void){MESSAGE()}            /* CAN2 RX0                     */
static inline void __CAN2_RX1_IRQHandler(void){MESSAGE()}            /* CAN2 RX1                     */
static inline void __CAN2_SCE_IRQHandler(void){MESSAGE()}            /* CAN2 SCE                     */
static inline void __OTG_FS_IRQHandler(void){MESSAGE()}              /* USB OTG FS                   */
static inline void __DMA2_Stream5_IRQHandler(void){MESSAGE()}        /* DMA2 Stream 5                */
static inline void __DMA2_Stream6_IRQHandler(void){MESSAGE()}        /* DMA2 Stream 6                */
static inline void __DMA2_Stream7_IRQHandler(void){MESSAGE()}        /* DMA2 Stream 7                */
static inline void __USART6_IRQHandler(void){MESSAGE()}              /* USART6                       */
static inline void __I2C3_EV_IRQHandler(void){MESSAGE()}             /* I2C3 event                   */
static inline void __I2C3_ER_IRQHandler(void){MESSAGE()}             /* I2C3 error                   */
static inline void __OTG_HS_EP1_OUT_IRQHandler(void){MESSAGE()}      /* USB OTG HS End Point 1 Out   */
static inline void __OTG_HS_EP1_IN_IRQHandler(void){MESSAGE()}       /* USB OTG HS End Point 1 In    */
static inline void __OTG_HS_WKUP_IRQHandler(void){MESSAGE()}         /* USB OTG HS Wakeup through EXTI */
static inline void __OTG_HS_IRQHandler(void){MESSAGE()}              /* USB OTG HS                   */
static inline void __DCMI_IRQHandler(void){MESSAGE()}                /* DCMI                         */
static inline void __CRYP_IRQHandler(void){MESSAGE()}                /* CRYP crypto                  */
static inline void __HASH_RNG_IRQHandler(void){MESSAGE()}            /* Hash and Rng                 */
static inline void __FPU_IRQHandler(void){MESSAGE()}                    /* FPU                          */


#pragma weak MMI_Handler = __MMI_Handler
#pragma weak MemManage_Handler = __MemManage_Handler
#pragma weak BusFault_Handler = __BusFault_Handler
#pragma weak UsageFault_Handler = __UsageFault_Handler
#pragma weak SVC_Handler = __SVC_Handler
#pragma weak DebugMon_Handler = __DebugMon_Handler
#pragma weak PendSV_Handler = __PendSV_Handler
#pragma weak SysTick_Handler = __SysTick_Handler

/* External Interrupts */
#pragma weak WWDG_IRQHandler = __WWDG_IRQHandler                /* Window WatchDog              */
#pragma weak PVD_IRQHandler = __PVD_IRQHandler                 /* PVD through EXTI Line detection */
#pragma weak TAMP_STAMP_IRQHandler = __TAMP_STAMP_IRQHandler          /* Tamper and TimeStamps through the EXTI line */
#pragma weak RTC_WKUP_IRQHandler = __RTC_WKUP_IRQHandler            /* RTC Wakeup through the EXTI line */
#pragma weak FLASH_IRQHandler = __FLASH_IRQHandler               /* FLASH                        */
#pragma weak RCC_IRQHandler = __RCC_IRQHandler                 /* RCC                          */
#pragma weak EXTI0_IRQHandler = __EXTI0_IRQHandler               /* EXTI Line0                   */
#pragma weak EXTI1_IRQHandler = __EXTI1_IRQHandler               /* EXTI Line1                   */
#pragma weak EXTI2_IRQHandler = __EXTI2_IRQHandler               /* EXTI Line2                   */
#pragma weak EXTI3_IRQHandler = __EXTI3_IRQHandler               /* EXTI Line3                   */
#pragma weak EXTI4_IRQHandler = __EXTI4_IRQHandler               /* EXTI Line4                   */
#pragma weak DMA1_Stream0_IRQHandler = __DMA1_Stream0_IRQHandler        /* DMA1 Stream 0                */
#pragma weak DMA1_Stream1_IRQHandler = __DMA1_Stream1_IRQHandler        /* DMA1 Stream 1                */
#pragma weak DMA1_Stream2_IRQHandler = __DMA1_Stream2_IRQHandler        /* DMA1 Stream 2                */
#pragma weak DMA1_Stream3_IRQHandler = __DMA1_Stream3_IRQHandler        /* DMA1 Stream 3                */
#pragma weak DMA1_Stream4_IRQHandler = __DMA1_Stream4_IRQHandler        /* DMA1 Stream 4                */
#pragma weak DMA1_Stream5_IRQHandler = __DMA1_Stream5_IRQHandler        /* DMA1 Stream 5                */
#pragma weak DMA1_Stream6_IRQHandler = __DMA1_Stream6_IRQHandler        /* DMA1 Stream 6                */
#pragma weak ADC_IRQHandler = __ADC_IRQHandler                 /* ADC1, ADC2 and ADC3s         */
#pragma weak CAN1_TX_IRQHandler = __CAN1_TX_IRQHandler             /* CAN1 TX                      */
#pragma weak CAN1_RX0_IRQHandler = __CAN1_RX0_IRQHandler            /* CAN1 RX0                     */
#pragma weak CAN1_RX1_IRQHandler = __CAN1_RX1_IRQHandler            /* CAN1 RX1                     */
#pragma weak CAN1_SCE_IRQHandler = __CAN1_SCE_IRQHandler            /* CAN1 SCE                     */
#pragma weak EXTI9_5_IRQHandler = __EXTI9_5_IRQHandler             /* External Line[9:5]s          */
#pragma weak TIM1_BRK_TIM9_IRQHandler = __TIM1_BRK_TIM9_IRQHandler       /* TIM1 Break and TIM9          */
#pragma weak TIM1_UP_TIM10_IRQHandler = __TIM1_UP_TIM10_IRQHandler       /* TIM1 Update and TIM10        */
#pragma weak TIM1_TRG_COM_TIM11_IRQHandler = __TIM1_TRG_COM_TIM11_IRQHandler   /* TIM1 Trigger and Commutation and TIM11 */
#pragma weak TIM1_CC_IRQHandler = __TIM1_CC_IRQHandler             /* TIM1 Capture Compare         */
#pragma weak TIM2_IRQHandler = __TIM2_IRQHandler                /* TIM2                         */
#pragma weak TIM3_IRQHandler = __TIM3_IRQHandler                /* TIM3                         */
#pragma weak TIM4_IRQHandler = __TIM4_IRQHandler                /* TIM4                         */
#pragma weak I2C1_EV_IRQHandler = __I2C1_EV_IRQHandler             /* I2C1 Event                   */
#pragma weak I2C1_ER_IRQHandler = __I2C1_ER_IRQHandler             /* I2C1 Error                   */
#pragma weak I2C2_EV_IRQHandler = __I2C2_EV_IRQHandler             /* I2C2 Event                   */
#pragma weak I2C2_ER_IRQHandler = __I2C2_ER_IRQHandler             /* I2C2 Error                   */
#pragma weak SPI1_IRQHandler = __SPI1_IRQHandler                /* SPI1                         */
#pragma weak SPI2_IRQHandler = __SPI2_IRQHandler                /* SPI2                         */
#pragma weak USART1_IRQHandler = __USART1_IRQHandler              /* USART1                       */
#pragma weak USART2_IRQHandler = __USART2_IRQHandler              /* USART2                       */
#pragma weak USART3_IRQHandler = __USART3_IRQHandler              /* USART3                       */
#pragma weak EXTI15_10_IRQHandler = __EXTI15_10_IRQHandler           /* External Line[15:10]s        */
#pragma weak RTC_Alarm_IRQHandler = __RTC_Alarm_IRQHandler           /* RTC Alarm (A and B) through EXTI Line */
#pragma weak OTG_FS_WKUP_IRQHandler = __OTG_FS_WKUP_IRQHandler         /* USB OTG FS Wakeup through EXTI line */
#pragma weak TIM8_BRK_TIM12_IRQHandler = __TIM8_BRK_TIM12_IRQHandler      /* TIM8 Break and TIM12         */
#pragma weak TIM8_UP_TIM13_IRQHandler = __TIM8_UP_TIM13_IRQHandler       /* TIM8 Update and TIM13        */
#pragma weak TIM8_TRG_COM_TIM14_IRQHandler = __TIM8_TRG_COM_TIM14_IRQHandler   /* TIM8 Trigger and Commutation and TIM14 */
#pragma weak TIM8_CC_IRQHandler = __TIM8_CC_IRQHandler             /* TIM8 Capture Compare         */
#pragma weak DMA1_Stream7_IRQHandler = __DMA1_Stream7_IRQHandler        /* DMA1 Stream7                 */
#pragma weak FSMC_IRQHandler = __FSMC_IRQHandler                /* FSMC                         */
#pragma weak SDIO_IRQHandler = __SDIO_IRQHandler                /* SDIO                         */
#pragma weak TIM5_IRQHandler = __TIM5_IRQHandler                /* TIM5                         */
#pragma weak SPI3_IRQHandler = __SPI3_IRQHandler                /* SPI3                         */
#pragma weak UART4_IRQHandler = __UART4_IRQHandler               /* UART4                        */
#pragma weak UART5_IRQHandler = __UART5_IRQHandler               /* UART5                        */
#pragma weak TIM6_DAC_IRQHandler = __TIM6_DAC_IRQHandler            /* TIM6 and DAC1&2 underrun errors */
#pragma weak TIM7_IRQHandler = __TIM7_IRQHandler                /* TIM7                         */
#pragma weak DMA2_Stream0_IRQHandler = __DMA2_Stream0_IRQHandler        /* DMA2 Stream 0                */
#pragma weak DMA2_Stream1_IRQHandler = __DMA2_Stream1_IRQHandler        /* DMA2 Stream 1                */
#pragma weak DMA2_Stream2_IRQHandler = __DMA2_Stream2_IRQHandler        /* DMA2 Stream 2                */
#pragma weak DMA2_Stream3_IRQHandler = __DMA2_Stream3_IRQHandler        /* DMA2 Stream 3                */
#pragma weak DMA2_Stream4_IRQHandler = __DMA2_Stream4_IRQHandler        /* DMA2 Stream 4                */
#pragma weak ETH_IRQHandler = __ETH_IRQHandler                 /* Ethernet                     */
#pragma weak ETH_WKUP_IRQHandler = __ETH_WKUP_IRQHandler            /* Ethernet Wakeup through EXTI line */
#pragma weak CAN2_TX_IRQHandler = __CAN2_TX_IRQHandler             /* CAN2 TX                      */
#pragma weak CAN2_RX0_IRQHandler = __CAN2_RX0_IRQHandler            /* CAN2 RX0                     */
#pragma weak CAN2_RX1_IRQHandler = __CAN2_RX1_IRQHandler            /* CAN2 RX1                     */
#pragma weak CAN2_SCE_IRQHandler = __CAN2_SCE_IRQHandler            /* CAN2 SCE                     */
#pragma weak OTG_FS_IRQHandler = __OTG_FS_IRQHandler              /* USB OTG FS                   */
#pragma weak DMA2_Stream5_IRQHandler = __DMA2_Stream5_IRQHandler        /* DMA2 Stream 5                */
#pragma weak DMA2_Stream6_IRQHandler = __DMA2_Stream6_IRQHandler        /* DMA2 Stream 6                */
#pragma weak DMA2_Stream7_IRQHandler = __DMA2_Stream7_IRQHandler        /* DMA2 Stream 7                */
#pragma weak USART6_IRQHandler = __USART6_IRQHandler              /* USART6                       */
#pragma weak I2C3_EV_IRQHandler = __I2C3_EV_IRQHandler             /* I2C3 event                   */
#pragma weak I2C3_ER_IRQHandler = __I2C3_ER_IRQHandler             /* I2C3 error                   */
#pragma weak OTG_HS_EP1_OUT_IRQHandler = __OTG_HS_EP1_OUT_IRQHandler      /* USB OTG HS End Point 1 Out   */
#pragma weak OTG_HS_EP1_IN_IRQHandler = __OTG_HS_EP1_IN_IRQHandler       /* USB OTG HS End Point 1 In    */
#pragma weak OTG_HS_WKUP_IRQHandler = __OTG_HS_WKUP_IRQHandler         /* USB OTG HS Wakeup through EXTI */
#pragma weak OTG_HS_IRQHandler = __OTG_HS_IRQHandler              /* USB OTG HS                   */
#pragma weak DCMI_IRQHandler = __DCMI_IRQHandler                /* DCMI                         */
#pragma weak CRYP_IRQHandler = __CRYP_IRQHandler                /* CRYP crypto                  */
#pragma weak HASH_RNG_IRQHandler = __HASH_RNG_IRQHandler            /* Hash and Rng                 */
#pragma weak FPU_IRQHandler = __FPU_IRQHandler                    /* FPU           */

#else // EXTENDED_UNHANDLED_INTERRUPT_HANDLER

/**
 * @brief  unexpected interrupt handler
*/
void Default_Handler()
{
    while(1);
}

#pragma weak MMI_Handler = Default_Handler
#pragma weak MemManage_Handler = Default_Handler
#pragma weak BusFault_Handler = Default_Handler
#pragma weak UsageFault_Handler = Default_Handler
#pragma weak SVC_Handler = Default_Handler
#pragma weak DebugMon_Handler = Default_Handler
#pragma weak PendSV_Handler = Default_Handler
#pragma weak SysTick_Handler = Default_Handler

/* External Interrupts */
#pragma weak WWDG_IRQHandler = Default_Handler                /* Window WatchDog              */
#pragma weak PVD_IRQHandler = Default_Handler                 /* PVD through EXTI Line detection */
#pragma weak TAMP_STAMP_IRQHandler = Default_Handler          /* Tamper and TimeStamps through the EXTI line */
#pragma weak RTC_WKUP_IRQHandler = Default_Handler            /* RTC Wakeup through the EXTI line */
#pragma weak FLASH_IRQHandler = Default_Handler               /* FLASH                        */
#pragma weak RCC_IRQHandler = Default_Handler                 /* RCC                          */
#pragma weak EXTI0_IRQHandler = Default_Handler               /* EXTI Line0                   */
#pragma weak EXTI1_IRQHandler = Default_Handler               /* EXTI Line1                   */
#pragma weak EXTI2_IRQHandler = Default_Handler               /* EXTI Line2                   */
#pragma weak EXTI3_IRQHandler = Default_Handler               /* EXTI Line3                   */
#pragma weak EXTI4_IRQHandler = Default_Handler               /* EXTI Line4                   */
#pragma weak DMA1_Stream0_IRQHandler = Default_Handler        /* DMA1 Stream 0                */
#pragma weak DMA1_Stream1_IRQHandler = Default_Handler        /* DMA1 Stream 1                */
#pragma weak DMA1_Stream2_IRQHandler = Default_Handler        /* DMA1 Stream 2                */
#pragma weak DMA1_Stream3_IRQHandler = Default_Handler        /* DMA1 Stream 3                */
#pragma weak DMA1_Stream4_IRQHandler = Default_Handler        /* DMA1 Stream 4                */
#pragma weak DMA1_Stream5_IRQHandler = Default_Handler        /* DMA1 Stream 5                */
#pragma weak DMA1_Stream6_IRQHandler = Default_Handler        /* DMA1 Stream 6                */
#pragma weak ADC_IRQHandler = Default_Handler                 /* ADC1, ADC2 and ADC3s         */
#pragma weak CAN1_TX_IRQHandler = Default_Handler             /* CAN1 TX                      */
#pragma weak CAN1_RX0_IRQHandler = Default_Handler            /* CAN1 RX0                     */
#pragma weak CAN1_RX1_IRQHandler = Default_Handler            /* CAN1 RX1                     */
#pragma weak CAN1_SCE_IRQHandler = Default_Handler            /* CAN1 SCE                     */
#pragma weak EXTI9_5_IRQHandler = Default_Handler             /* External Line[9:5]s          */
#pragma weak TIM1_BRK_TIM9_IRQHandler = Default_Handler       /* TIM1 Break and TIM9          */
#pragma weak TIM1_UP_TIM10_IRQHandler = Default_Handler       /* TIM1 Update and TIM10        */
#pragma weak TIM1_TRG_COM_TIM11_IRQHandler = Default_Handler   /* TIM1 Trigger and Commutation and TIM11 */
#pragma weak TIM1_CC_IRQHandler = Default_Handler             /* TIM1 Capture Compare         */
#pragma weak TIM2_IRQHandler = Default_Handler                /* TIM2                         */
#pragma weak TIM3_IRQHandler = Default_Handler                /* TIM3                         */
#pragma weak TIM4_IRQHandler = Default_Handler                /* TIM4                         */
#pragma weak I2C1_EV_IRQHandler = Default_Handler             /* I2C1 Event                   */
#pragma weak I2C1_ER_IRQHandler = Default_Handler             /* I2C1 Error                   */
#pragma weak I2C2_EV_IRQHandler = Default_Handler             /* I2C2 Event                   */
#pragma weak I2C2_ER_IRQHandler = Default_Handler             /* I2C2 Error                   */
#pragma weak SPI1_IRQHandler = Default_Handler                /* SPI1                         */
#pragma weak SPI2_IRQHandler = Default_Handler                /* SPI2                         */
#pragma weak USART1_IRQHandler = Default_Handler              /* USART1                       */
#pragma weak USART2_IRQHandler = Default_Handler              /* USART2                       */
#pragma weak USART3_IRQHandler = Default_Handler              /* USART3                       */
#pragma weak EXTI15_10_IRQHandler = Default_Handler           /* External Line[15:10]s        */
#pragma weak RTC_Alarm_IRQHandler = Default_Handler           /* RTC Alarm (A and B) through EXTI Line */
#pragma weak OTG_FS_WKUP_IRQHandler = Default_Handler         /* USB OTG FS Wakeup through EXTI line */
#pragma weak TIM8_BRK_TIM12_IRQHandler = Default_Handler      /* TIM8 Break and TIM12         */
#pragma weak TIM8_UP_TIM13_IRQHandler = Default_Handler       /* TIM8 Update and TIM13        */
#pragma weak TIM8_TRG_COM_TIM14_IRQHandler = Default_Handler   /* TIM8 Trigger and Commutation and TIM14 */
#pragma weak TIM8_CC_IRQHandler = Default_Handler             /* TIM8 Capture Compare         */
#pragma weak DMA1_Stream7_IRQHandler = Default_Handler        /* DMA1 Stream7                 */
#pragma weak FSMC_IRQHandler = Default_Handler                /* FSMC                         */
#pragma weak SDIO_IRQHandler = Default_Handler                /* SDIO                         */
#pragma weak TIM5_IRQHandler = Default_Handler                /* TIM5                         */
#pragma weak SPI3_IRQHandler = Default_Handler                /* SPI3                         */
#pragma weak UART4_IRQHandler = Default_Handler               /* UART4                        */
#pragma weak UART5_IRQHandler = Default_Handler               /* UART5                        */
#pragma weak TIM6_DAC_IRQHandler = Default_Handler            /* TIM6 and DAC1&2 underrun errors */
#pragma weak TIM7_IRQHandler = Default_Handler                /* TIM7                         */
#pragma weak DMA2_Stream0_IRQHandler = Default_Handler        /* DMA2 Stream 0                */
#pragma weak DMA2_Stream1_IRQHandler = Default_Handler        /* DMA2 Stream 1                */
#pragma weak DMA2_Stream2_IRQHandler = Default_Handler        /* DMA2 Stream 2                */
#pragma weak DMA2_Stream3_IRQHandler = Default_Handler        /* DMA2 Stream 3                */
#pragma weak DMA2_Stream4_IRQHandler = Default_Handler        /* DMA2 Stream 4                */
#pragma weak ETH_IRQHandler = Default_Handler                 /* Ethernet                     */
#pragma weak ETH_WKUP_IRQHandler = Default_Handler            /* Ethernet Wakeup through EXTI line */
#pragma weak CAN2_TX_IRQHandler = Default_Handler             /* CAN2 TX                      */
#pragma weak CAN2_RX0_IRQHandler = Default_Handler            /* CAN2 RX0                     */
#pragma weak CAN2_RX1_IRQHandler = Default_Handler            /* CAN2 RX1                     */
#pragma weak CAN2_SCE_IRQHandler = Default_Handler            /* CAN2 SCE                     */
#pragma weak OTG_FS_IRQHandler = Default_Handler              /* USB OTG FS                   */
#pragma weak DMA2_Stream5_IRQHandler = Default_Handler        /* DMA2 Stream 5                */
#pragma weak DMA2_Stream6_IRQHandler = Default_Handler        /* DMA2 Stream 6                */
#pragma weak DMA2_Stream7_IRQHandler = Default_Handler        /* DMA2 Stream 7                */
#pragma weak USART6_IRQHandler = Default_Handler              /* USART6                       */
#pragma weak I2C3_EV_IRQHandler = Default_Handler             /* I2C3 event                   */
#pragma weak I2C3_ER_IRQHandler = Default_Handler             /* I2C3 error                   */
#pragma weak OTG_HS_EP1_OUT_IRQHandler = Default_Handler      /* USB OTG HS End Point 1 Out   */
#pragma weak OTG_HS_EP1_IN_IRQHandler = Default_Handler       /* USB OTG HS End Point 1 In    */
#pragma weak OTG_HS_WKUP_IRQHandler = Default_Handler         /* USB OTG HS Wakeup through EXTI */
#pragma weak OTG_HS_IRQHandler = Default_Handler              /* USB OTG HS                   */
#pragma weak DCMI_IRQHandler = Default_Handler                /* DCMI                         */
#pragma weak CRYP_IRQHandler = Default_Handler                /* CRYP crypto                  */
#pragma weak HASH_RNG_IRQHandler = Default_Handler            /* Hash and Rng                 */
#pragma weak FPU_IRQHandler = Default_Handler                    /* FPU                          */

#endif // EXTENDED_UNHANDLED_INTERRUPT_HANDLER

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
