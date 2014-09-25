/**
 * @addtogroup minstdlib
 * @file minstdlib.h
 * @{
 */

#ifndef MINSTDLIB_H_
#define MINSTDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

void strreverse(char* begin, char* end);
char* itoa(int value, char* str, int base);
char* ditoa(int64_t value, char* str, int base);
char* ftoa(char *s, double n);

#include <stdlib.h>

#ifdef __cplusplus
}
#endif

#endif /* MINSTDLIB_H_ */

/**
 * @}
 */
