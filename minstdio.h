/**
 * @addtogroup minstdio
 *
 * @file minstdio.h
 * @{
 */

#ifndef MINSTDIO_H_
#define MINSTDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/**
 * physical IO interfac - the implementation must be defined in code
 * that is tailored for the target device or system.
 *
 * Eg for stm32:
\code
void phy_putc(char c)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint16_t)c);
}
\endcode
 *
 * @param	c is a character to send via some physical interface.
 */
void phy_putc(char c);

#ifdef __cplusplus
}
#endif

#endif /* MINSTDIO_H_ */

/**
 * @}
 */
