/**
 * @addtogroup strutils
 * @file strutils.h
 * @{
 */

#ifndef STRUTILS_H_
#define STRUTILS_H_

#include <string.h>

/**
 *	returns 0 if string "str" starts exactly with string "startswith"
 */
#define startswith(str, starts_with) strncmp(str, starts_with, strlen(starts_with))

char* strtoupper(char* s);
char* strtolower(char* s);
int ahtoi(char* s);
char adtoi(char d);

#endif // STRUTILS_H_

/**
 * @}
 */
