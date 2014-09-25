/**
 * @addtogroup minstring
 *
 * @file minstring.h
 * @{
 */

#ifndef MINSTRING_H_
#define MINSTRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "strutils.h"

int toupper(int c);
int tolower(int c);

#include <string.h>

#ifdef __cplusplus
}
#endif

#endif /* MINSTRING_H_ */

/**
 * @}
 */
