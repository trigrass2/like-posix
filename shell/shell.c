/*
 * Copyright (c) 2015 Michael Stuart.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the nutensils project, <https://github.com/drmetal/nutensils>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "shell.h"

#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "builtins.h"

typedef struct
{
	shell_cmd_t* cmd;
	const char* args[SHELL_MAX_ARGS];
	uint16_t nargs;
}current_command_t;

typedef struct _shell_instance_t{
	char input_buffer[SHELL_CMD_BUFFER_SIZE];		///< input_buffer is a memory space that stores user input, and is @ref CMD_BUFFER_SIZE in size
	char history[SHELL_HISTORY_LENGTH][SHELL_CMD_BUFFER_SIZE];	///< history is a memory space that stores previous user input, and is @ref CMD_BUFFER_SIZE in size
	uint16_t cursor_index;							///< cursor_index is the index of the cursor.
	uint16_t input_index;							///< input_index is the index of the end of the chacters in the command buffer.
	int8_t history_index;							///< points to the last item in history
	unsigned char history_save_index;						///< points to the last item in history
	shell_cmd_t* head_cmd;
	bool exitflag;
	int fdes;
	current_command_t current_command;
}shell_instance_t;


static void prompt(shell_instance_t* sh);
static void historic_prompt(shell_instance_t* sh);
static void clear_prompt(shell_instance_t* sh);
static void put_prompt(shell_instance_t* sh, const char* promptstr, const char* argstr, bool newline);
static void parse_input(shell_instance_t* sh, current_command_t* cmd);
static void shell_builtins(shell_instance_t* sh, int code, shell_cmd_t* cmd);
static void shell_instance_thread(sock_conn_t* conn);

/**
 * starts a shell server. requires a config file that meets the needs of the threaded server...
 *
 * @param   shellserver is a pointer to a fresh shell server structure, its contents will be fully initialized.
 * @param   config file is a filepath to a configuration file. it must contain port and conns settings.
 * @retval  returns -1 on error, and a non zero value on success.
 */
int start_shell(shellserver_t* shellserver, const char* configfile)
{
    memset(shellserver, 0, sizeof(shellserver_t));

	register_command(shellserver, &sh_help_cmd, NULL, NULL, NULL);
	register_command(shellserver, &sh_exit_cmd, NULL, NULL, NULL);
	register_command(shellserver, &sh_date_cmd, NULL, NULL, NULL);
    register_command(shellserver, &sh_uname_cmd, NULL, NULL, NULL);
    register_command(shellserver, &sh_reboot_cmd, NULL, NULL, NULL);

	return start_threaded_server(&shellserver->server, configfile, shell_instance_thread, shellserver, SHELL_TASK_STACK_SIZE, SHELL_TASK_PRIORITY);
}

/**
 * registers a command with the Shell.
 * once a shell_cmd_t has been registered, it can be run from the shell.
 *
 * The command structure may be pre-populated, or populated via this function.
 *
\code

// function to run for this command
int sh_mycmd(int fdes, const char** args, unsigned char nargs)
{
    // do stuff
    return SHELL_CMD_EXIT;
}

// pre populated case:
shell_cmd_t mycmd = {
    .name = "mycmd",
    .usage = "help string for mycmd",
    .cmdfunc = sh_mycmd
};
register_command(&shellserver, &mycmd, NULL, NULL, NULL);

// un populated case:
shell_cmd_t mycmd;
register_command(&shellserver, &mycmd, sh_mycmd, "mycmd", "help string for mycmd");

\endcode
 *
 * @param   shellserver shellserver is a pointer to a the owning shell server structure.
 * @param   cmd is a pointer to a shell_cmd_t variable.
 * @param   cmdfunc is a pointer to a shell_cmd_func_t function pointer to use for the command.
 * @param   name is a pointer to a string that names the command.
 * @param   usage is a pointer to a string that describes the usage of the command.
 */
void register_command(shellserver_t* shellserver, shell_cmd_t* cmd, shell_cmd_func_t cmdfunc, const char* name, const char* usage)
{
    if(cmd)
    {
        shell_cmd_init(cmd, cmdfunc, name, usage);
        if(shellserver->head_cmd)
            cmd->next = shellserver->head_cmd;
        shellserver->head_cmd = cmd;
    }
}

/**
 * this function runs inside a new thread, spawned by the threaded_server
 */
void shell_instance_thread(sock_conn_t* conn)
{
	shellserver_t* shellserver = (shellserver_t*)conn->ctx;
	shell_instance_t* sh = calloc(sizeof(shell_instance_t), 1);

	if(sh)
	{
		sh->exitflag = false;
		sh->input_index = 0;
		sh->cursor_index = 0;
		sh->head_cmd = shellserver->head_cmd;
		sh->history_index = -1;
		sh->history_save_index = 0;
		sh->fdes = conn->connfd;

		// blocks here running the shell
		prompt(sh);

		free(sh);
	}
}

/**
 * this function loops while the shell instance exitflag is set to false.
 * it processes serial IO, decodes commands and executes them.
 * the exit flag is set when the user runs the built in command "exit",
 * or when the client connection closes.
 */
void prompt(shell_instance_t* sh)
{
	int code;
	unsigned char data = 0;
	unsigned char i = 0;

	while(!sh->exitflag)
	{
		if(recv(sh->fdes, &data, 1, 0) <= 0)
			sh->exitflag = true;
		else
		{
			if(data == 0x1B) // ESC
			{
				data = 0;
				recv(sh->fdes, &data, 1, 0);
				if(data == 0x5B)	// ANSI escaped sequences, ascii '['
				{
					data = 0;
					recv(sh->fdes, &data, 1, 0);

					if(data == 0x33) // ascii '3'
					{
						data = 0;
						recv(sh->fdes, &data, 1, 0);
						if(data == 0x7E) // DELETE, ascii '~'
						{
							if(sh->cursor_index < sh->input_index)
							{
								// wipe last character
								sh->input_index--;
								for(i = sh->cursor_index; i < sh->input_index; i++)
									sh->input_buffer[i] = sh->input_buffer[i+1];

								sh->input_buffer[sh->input_index] = ' ';
								sh->input_buffer[sh->input_index+1] = '\0';
								put_prompt(sh, SHELL_PROMPT, sh->input_buffer, false);
								// re print prompt
								sh->input_buffer[sh->input_index] = '\0';
								put_prompt(sh, SHELL_PROMPT, sh->input_buffer, false);

								// put cursor back where it should be
								for(i = sh->input_index; i > sh->cursor_index; i--)
								{
									send(sh->fdes, SHELL_LEFTARROW, sizeof(SHELL_LEFTARROW)-1, 0);
								}
							}
						}
					}
					else if(data == 0x41) // UP
					{
						sh->history_index--;
						if(sh->history_index < 0)
						{
							sh->history_index = SHELL_HISTORY_LENGTH-1;
						}
						historic_prompt(sh);
					}
					else if(data == 0x42) // DOWN
					{
						sh->history_index = -1;
						historic_prompt(sh);
					}
					else if(data == 0x44) // LEFT
					{
						if(sh->cursor_index > 0)
						{
							sh->cursor_index--;
							send(sh->fdes, SHELL_LEFTARROW, sizeof(SHELL_LEFTARROW)-1, 0);
						}
					}
					else if(data == 0x43) // RIGHT
					{
						if(sh->cursor_index < sh->input_index)
						{
							sh->cursor_index++;
							send(sh->fdes, SHELL_RIGHTARROW, sizeof(SHELL_RIGHTARROW)-1, 0);
						}
					}
				}
				else if(data == 0x4F)	// HOME, END
				{
					data = 0;
					recv(sh->fdes, &data, 1, 0);
					if(data == 0x48) // HOME
					{
						while(sh->cursor_index > 0)
						{
							send(sh->fdes, SHELL_LEFTARROW, sizeof(SHELL_LEFTARROW)-1, 0);
							sh->cursor_index--;
						}
					}
					else if(data == 0x46) // END
					{
						while(sh->cursor_index < sh->input_index)
						{
							send(sh->fdes, SHELL_RIGHTARROW, sizeof(SHELL_RIGHTARROW)-1, 0);
							sh->cursor_index++;
						}
					}
				}
			}
			else if(data == '\n') // NEWLINE
			{
				sh->input_buffer[sh->input_index] = '\0';

				parse_input(sh, &sh->current_command);

				if(sh->current_command.cmd)
				{
					// use args[1] as args[0] points to the command
				    code = shell_cmd_exec(sh->current_command.cmd, sh->fdes, sh->current_command.args + 1, sh->current_command.nargs);
					shell_builtins(sh, code, sh->current_command.cmd);
				}

				sh->input_index = 0;
				sh->cursor_index = 0;
				put_prompt(sh, SHELL_PROMPT, NULL, true);
			}
			else if(data == 0x7F) // BACKSPACE
			{
				if(sh->cursor_index > 0)
				{
					// wipe last character
					sh->input_index--;
					sh->cursor_index--;
					for(i = sh->cursor_index; i < sh->input_index; i++)
						sh->input_buffer[i] = sh->input_buffer[i+1];

					sh->input_buffer[sh->input_index] = ' ';
					sh->input_buffer[sh->input_index+1] = '\0';
					put_prompt(sh, SHELL_PROMPT, sh->input_buffer, false);
					// re print prompt
					sh->input_buffer[sh->input_index] = '\0';
					put_prompt(sh, SHELL_PROMPT, sh->input_buffer, false);

					// put cursor back where it should be
					for(i = sh->input_index; i > sh->cursor_index; i--)
					{
						send(sh->fdes, SHELL_LEFTARROW, sizeof(SHELL_LEFTARROW)-1, 0);
					}
				}
			}
			else // all others
			{
				// ignore unprintable characters below Space
				// not sure about characters above 126, Tilde
				if(data >= ' ')
				{
					if(sh->input_index < SHELL_CMD_BUFFER_SIZE-1)
					{
						for(i = sh->input_index; i > sh->cursor_index; i--)
						{
							sh->input_buffer[i] = sh->input_buffer[i-1];
						}
						sh->input_buffer[sh->cursor_index] = data;
						sh->cursor_index++;
						sh->input_index++;
					}
					else
					{
						// ignore data overflow
						sh->input_index = 1;
						sh->cursor_index = 1;
						sh->input_buffer[0] = data;
					}

					sh->input_buffer[sh->input_index] = '\0';

					if(sh->input_index == 0)
					{
						put_prompt(sh, SHELL_PROMPT, NULL, true);
					}

					send(sh->fdes, &sh->input_buffer[sh->cursor_index-1], strlen((const char*)&sh->input_buffer[sh->cursor_index-1]), 0);

					// put cursor back where it should be
					for(i = sh->input_index; i > sh->cursor_index; i--)
					{
						send(sh->fdes, SHELL_LEFTARROW, sizeof(SHELL_LEFTARROW)-1, 0);
					}
				}
			}
		}
	}
}

/**
 * copies a prompt from a the history buffer to the input buffer, then displays it.
 */
void historic_prompt(shell_instance_t* sh)
{
	if(sh->history_index >= 0 && sh->history_index < SHELL_HISTORY_LENGTH)
	{
		if(sh->history[sh->history_index][0] != '\0')
		{
			clear_prompt(sh);
			strncpy((char*)sh->input_buffer, (const char*)sh->history[sh->history_index], sizeof(sh->input_buffer)-1);
			sh->input_index = sh->cursor_index = strlen((const char*)sh->input_buffer);
			put_prompt(sh, SHELL_PROMPT, sh->input_buffer, false);
		}
	}
	else
	{
		clear_prompt(sh);
		sh->input_index = sh->cursor_index = 0;
		sh->input_buffer[sh->input_index] = '\0';
		put_prompt(sh, SHELL_PROMPT, NULL, false);
	}
}

/**
 * clears the input buffer to a fresh prompt.
 */
void clear_prompt(shell_instance_t* sh)
{
	sh->input_index = sh->cursor_index = 0;
	while(sh->input_buffer[sh->input_index])
	{
		sh->input_buffer[sh->input_index] = ' ';
		sh->input_index++;
	}
	sh->input_index = 0;
	put_prompt(sh, SHELL_PROMPT, sh->input_buffer, false);
}

/**
 * prints the prompt string.
 */
void put_prompt(shell_instance_t* sh, const char* promptstr, const char* argstr, bool newline)
{
    char* path;
	send(sh->fdes, "\r", 1, 0);
	if(newline)
		send(sh->fdes, "\n", 1, 0);

	path = getcwd(NULL, SHELL_CWD_LENGTH_MAX);

    if(path != NULL)
    {
       send(sh->fdes, path, strlen(path), 0);
       free(path);
    }

	if(promptstr)
		send(sh->fdes, promptstr, strlen((const char*)promptstr), 0);
	if(argstr)
		send(sh->fdes, argstr, strlen((const char*)argstr), 0);
}

/**
 * parses a string captured by prompt() for valid commands. when a valid command is detected,
 * the function associated with that command is called.
 * populates cmd with the command if one is found.
 * if no command is found, cmd->cmd is set to NULL.
 */
void parse_input(shell_instance_t* sh, current_command_t* cmd)
{
	shell_cmd_t* head = sh->head_cmd;
	char* iter;
	cmd->cmd = NULL;
    unsigned char delimiter;

	if(head)
	{
		// be sure that the sh->input_buffer is 0 terminated
		sh->input_buffer[sizeof(sh->input_buffer)-1] = '\0';

		// if there was some input, (overwrite oldest save)
		if(sh->input_buffer[0] != '\0')
		{
			strncpy((char*)sh->history[sh->history_save_index], (const char*)sh->input_buffer, sizeof(sh->history[sh->history_save_index]) - 1);
			sh->history_save_index++;
			if(sh->history_save_index >= SHELL_HISTORY_LENGTH)
				sh->history_save_index = 0;
		}

		memset(cmd->args, 0, sizeof(cmd->args));

		// split input into string chunks delimited by spaces, each is an argument
		// args[0] is special, it holds the command
		iter = sh->input_buffer;
		cmd->nargs = 0;
		while(*iter)
		{
			// split string into text blocks, delimited by whitespace, trim all whitespace
			// if whitespace followed by a quote character is found,
			// then everything after that is added into a text block including whitespace.
			// a matching quote chartacter ends the text block

			delimiter = 0;

			// iterate until a non space is found
			while(*iter && (*iter == ' '))
				iter++;

			if(!*iter)
				break;

			// if are we entering a quote text block, keep the delimiters value
			if((*iter == '`') || (*iter == '\'') || (*iter == '"'))
			{
				delimiter = *iter;
				iter++;
			}

			// add text block to list
			cmd->args[cmd->nargs] = iter;
			cmd->nargs++;
			if(cmd->nargs >= SHELL_MAX_ARGS)
				break;

			// if there was no sh->starting delimiter, close the block on the next space
			if(!delimiter)
				delimiter = ' ';

			// iterate until the closing delimiter is found
			while(*iter && (*iter != delimiter))
				iter++;

			// zero terminate if we arent at the end already
			if(*iter)
			{
				*iter = '\0';
				iter++;
			}
		}

		// match input args[0] (the command) to one of the commands
		while(head && head->name)
		{
			if(!strncmp((const char*)cmd->args[0], (const char*)head->name, sizeof(sh->input_buffer)-1))
				break;
			head = head->next;
		}

		// return command if one was found
		if(head && head->name)
		{
			send(sh->fdes, SHELL_NEWLINE, sizeof(SHELL_NEWLINE)-1, 0);
			// reduce number of arguments by one as first arg is just the command
			cmd->nargs--;
			cmd->cmd = head;
		}
		else if(cmd->args[0] != NULL)
		{
			// print error message if the buffer had some content but no valid command
			send(sh->fdes, SHELL_NO_SUCH_COMMAND, sizeof(SHELL_NO_SUCH_COMMAND)-1, 0);
			send(sh->fdes, cmd->args[0], strlen((const char*)cmd->args[0]), 0);
		}
	}
}

/**
 * processes special shell_cmd_t return codes
 */
void shell_builtins(shell_instance_t* sh, int code, shell_cmd_t* cmd)
{
	shell_cmd_t* head;

	switch(code)
	{
		case SHELL_CMD_KILL:
			sh->exitflag = true;
		break;
		case SHELL_CMD_PRINT_CMDS:
			head = sh->head_cmd;
			send(sh->fdes, SHELL_HELP_STR, sizeof(SHELL_HELP_STR)-1, 0);
			while(head)
			{
				send(sh->fdes, SHELL_NEWLINE, sizeof(SHELL_NEWLINE)-1, 0);
				send(sh->fdes, head->name, strlen((const char*)head->name), 0);
				head = head->next;
			}
		break;
		case SHELL_CMD_PRINT_USAGE:
			cmd_usage(cmd, sh->fdes);
		break;
	}
}

