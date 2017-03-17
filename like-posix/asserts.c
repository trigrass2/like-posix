
/**
 * @addtogroup system
 *
 * this code was copied from one of the official stm32 firmware demos.
 *
 * @file asserts.c
 * @{
 */

#include <stdio.h>
#if USE_DRIVER_LEDS
#include "leds.h"
#endif
#include "board_config.h"

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
#if USE_DRIVER_LEDS && defined(ERROR_LED)
	set_led(ERROR_LED);
#endif
    printf("Assert Failed in %s, line %d\n", file, line);
    for( ;; );
}
#endif

/**
 * @}
 */
