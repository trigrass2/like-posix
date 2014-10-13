
/**
 * @addtogroup system
 *
 * this code was copied from one of the official stm32 firmware demos.
 *
 * @file asserts.c
 * @{
 */

#include <stdio.h>
#include "board_config.h"

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
    printf("Assert Failed: \nfile: %s\nline: %u\n", file, (unsigned int)line);
    for( ;; );
}
#endif

/**
 * @}
 */
