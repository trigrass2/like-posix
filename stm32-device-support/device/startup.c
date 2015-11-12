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

#include "board_config.h"
#include "services.h"

#if USE_FREERTOS
#if USE_PTHREADS
#include <pthread.h>
typedef void*(*threadfunc)(void*);
#else
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
typedef TaskHandle_t threadfunc;
#endif
#endif



#define WEAK        __attribute__ ((weak))
#define BSS_FILL    0
#define STACK_FILL  0xA5A5A5A5

#ifndef CLEVER_DEFAULT_INTERRUPT_HANDLER
#define CLEVER_DEFAULT_INTERRUPT_HANDLER 0
#endif

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

void Reset_Handler() __attribute__((__interrupt__));
void __Init_Data();

#if CLEVER_DEFAULT_INTERRUPT_HANDLER
#include<stdio.h>
void Default_Handler(const char* file, const char* function, const int line);
#define MESSAGE() Default_Handler(__FILE__, __FUNCTION__, __LINE__);
#else
void Default_Handler();
#endif

extern int main();                /* Application's main function */
#ifdef __cplusplus
extern void __libc_init_array();  /* calls CTORS of static objects */
#endif

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
#ifdef __cplusplus
	__libc_init_array();
#endif

	/* Call Clock/RCC init */
	SystemInit();

	// call init_target (in board_config.c)
	init_target();

    // call init_services (in services.c)
    init_services();

	/* Call the application's entry point.*/
#if USE_FREERTOS
#if USE_PTHREADS
	pthread_t main_thread;
	pthread_attr_t main_attr;
	pthread_attr_init(&main_attr);
	pthread_attr_setstacksize(&main_attr, configMAIN_STACK_SIZE);
	pthread_attr_setdetachstate(&main_attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&main_thread, &main_attr, (threadfunc)main, NULL);
	pthread_attr_destroy(&main_attr);
#else
    xTaskCreate((threadfunc)main, "main", configMAIN_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif
	vTaskStartScheduler();
#else
	main();
#endif

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


/**
 * @brief  unexpected interrupt handler
*/
#if CLEVER_DEFAULT_INTERRUPT_HANDLER
void Default_Handler(const char* file, const char* function, const int line)
{
    while(1)
    {
        printf("unhandled interrupt: %s, %s, line %d\n", file, function, line);
        delay(2000);
    }
}
#else // CLEVER_DEFAULT_INTERRUPT_HANDLER
void Default_Handler()
{
    while(1);
}
#endif // CLEVER_DEFAULT_INTERRUPT_HANDLER

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
