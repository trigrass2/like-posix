/*
 * builtins.h
 *
 *  Created on: 29/10/2014
 *      Author: stuartm
 */

#ifndef SHELL_BUILTINS_H_
#define SHELL_BUILTINS_H_

#include <stdint.h>
#include "shell_command.h"

extern shell_cmd_t help_cmd;
extern shell_cmd_t exit_cmd;

int help_sh(int fdes, const char** args, unsigned char nargs);
int exit_sh(int fdes, const char** args, unsigned char nargs);

#endif /* SHELL_BUILTINS_H_ */
