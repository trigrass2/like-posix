/**
 * @addtogroup system
 *
 * some of this code was copied from one of the official stm32 firmware demos.
 *
 * prvGetRegistersFromStack() and the contents of HardFault_Handler()
 * were copied from a freertos demo - https://github.com/Zizzle/stm32_freertos_example
 *
 * @file hardware_exception.c
 * @{
 */

#include <stdio.h>
#include "system.h"
#include "board_config.h"

const char* stack_regs[] = {
        "R0",
        "R1",
        "R2",
        "R3",
        "R12",
        "LR",
        "PC",
        "PSR"
};

void prvGetRegistersFromStack(unsigned int *pulFaultStackAddress )
{
#if DEBUG_PRINTF_EXCEPTIONS

    int frame;
    int reg;

    for(frame = 0; frame < STACKTRACE_DEPTH; frame++)
    {
        printf("frame %d\n", frame);
        for(reg = 0; reg < STACKFRAME_DEPTH; reg++)
        {
            printf("\t%s=%#08x\n", stack_regs[reg], pulFaultStackAddress[(frame * STACKFRAME_DEPTH) + reg]);
        }
    }
#else
    (void)pulFaultStackAddress;
#endif

    for( ;; );
}

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval : None
  */
void NMI_Handler()
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval : None
  */
void HardFault_Handler()
{
    /* Go to infinite loop when Hard Fault exception occurs */
    __asm volatile
    (
      " tst lr, #4                                                \n"
      " ite eq                                                    \n"
      " mrseq r0, msp                                             \n"
      " mrsne r0, psp                                             \n"
      " ldr r1, [r0, #24]                                         \n"
      " ldr r2, handler2_address_const                            \n"
      " bx r2                                                     \n"
      " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval : None
  */
void MemManage_Handler()
{
  /* Go to infinite loop when Memory Manage exception occurs */
#if DEBUG_PRINTF_EXCEPTIONS
    printf("memanage fault\n");
#endif
  while(1);
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval : None
  */
void BusFault_Handler()
{
  /* Go to infinite loop when Bus Fault exception occurs */
#if DEBUG_PRINTF_EXCEPTIONS
    printf("bus fault\n");
#endif
  while (1);
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval : None
  */
void UsageFault_Handler()
{
  /* Go to infinite loop when Usage Fault exception occurs */
#if DEBUG_PRINTF_EXCEPTIONS
    printf("usage fault\n");
#endif
  while (1);
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval : None
  */
void DebugMon_Handler()
{

}

/**
 * @}
 */
