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
 * This file is part of the Appleseed project, <https://github.com/drmetal/app-l-seed>
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

/**
 * Text Shell
 *
 * threaded server operation:
 * after start_shell() is called, a socket is opened. for every connection made on that socket
 * a new shell thread is spawned.
 * the shell is configured from a config file in this mode. the config file should contain:
 *  - port 22
 *  - conns 5
 *  - name shelld
 * the socket connection and shell instance will exit when read() returns a value <= 0,
 * or if the exit command is issued.
 *
 * operation without threaded server:
 * to run a shell outside a threaded server, call shell_instance().
 *
 *  - the shellserver_t structure should have commands registered on it before use.
 *  - the rdfd and wrfd file descriptors point to an input file and an output file,
 *  	these may be regular files or sockets, or devices, and may be different or the same.
 *  - the shell_instance() function allocates a shell_instance_t which will be around 768bytes
 *  	when SHELL_HISTORY_LENGTH is set to 4, around 256bytes when SHELL_HISTORY_LENGTH is set to 0.
 *
 * shell_instance() blocks while running. it will exit when read() returns a value <= 0,
 * or if the exit command is issued.
 *
 *
 * Note:
 *
 * 	- the system calls read, write, open, fdopen, fclose, getcwd, stat are used. if run under appleseed,
 * 	   requires USE_POSIX_STYLE_IO set to 1 is a must among other things, and ENABLE_LIKEPOSIX_SOCKETS
 * 	   must be set to 1 in likeposix_config.h
 * 	- shells share the global current working directory
 * 	- shells support user defined and built in commands, registered via the register_command() function.
 * 	- shells support running command(s) from within files, as shell scripts. when a filename is specified
 * 		that is a regular file, it is opened and its contents passed to the shell line by line.
 * 		control is then returned to the original file stream.
 * 	- shell scripts must end with a newline, to execute the last line in the script.
 * 	- supports whitespace in arguments, eg:
 *	 	 	 	 	 	 	 	 	 	 	 # echo hello there
 *	 	 	 	 	 	 	 	 	 	 	 # hello
 *	 	 	 	 	 	 	 	 	 	 	 # echo "hello there"
 *	 	 	 	 	 	 	 	 	 	 	 # hello there
 *	 	 	 	 	 	 	 	 	 	 	 # echo 'hello there'
 *	 	 	 	 	 	 	 	 	 	 	 # hello there
 *	 	 	 	 	 	 	 	 	 	 	 # echo `hello there`
 *	 	 	 	 	 	 	 	 	 	 	 # hello there
 *	 	 	 	 	 	 	 	 	 	 	 # echo "'hello there'"
 *	 	 	 	 	 	 	 	 	 	 	 # 'hello there'
 *	 	 	 	 	 	 	 	 	 	 	 # echo '"hello there"'
 *	 	 	 	 	 	 	 	 	 	 	 # "hello there"
 * 	- supports redirect to file eg:
 *									# ls > file.txt
 * 									# echo "hello there" >> file.txt
 * 									# shell scripts do not support file output
 * 									# ./myscript.sh > file.txt
 * 	- nested shell scripts are supported but **ONLY** when commands in the scripts do not perform read operations.
 */

#include "shell.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "builtins.h"

static char shell_cwd[SHELL_CWD_LENGTH_MAX];

typedef struct
{
	shell_cmd_t* cmd;
	const char* args[SHELL_MAX_ARGS];
	uint16_t nargs;
}shell_input_t;

typedef struct _shell_instance_t{
	char input_buffer[SHELL_CMD_BUFFER_SIZE];		///< input_buffer is a memory space that stores user input, and is @ref CMD_BUFFER_SIZE in size
	char history[SHELL_HISTORY_LENGTH][SHELL_CMD_BUFFER_SIZE];	///< history is a memory space that stores previous user input, and is @ref CMD_BUFFER_SIZE in size
	uint16_t cursor_index;							///< cursor_index is the index of the cursor.
	uint16_t input_index;							///< input_index is the index of the end of the chacters in the command buffer.
	int8_t history_index;							///< points to the last item in history
	unsigned char history_save_index;						///< points to the last item in history
	shell_cmd_t* head_cmd;
	bool exitflag;
	FILE* rdfs;
	int rdfd;
	int rdfd_hold;
	int wrfd;
	int wrfd_hold;
	struct stat sstat;
	shell_input_t input_cmd;
}shell_instance_t;


void shell_thread(sock_conn_t* conn);
static void prompt(shell_instance_t* shell_inst);
static void historic_prompt(shell_instance_t* shell_inst);
static void clear_prompt(shell_instance_t* shell_inst);
static void put_prompt(shell_instance_t* shell_inst, const char* argstr, bool newline);
static void parse_input_line(shell_instance_t* shell_inst);
static void return_code_catcher(shell_instance_t* shell_inst, int code);
static void open_output_file(shell_instance_t* shell_inst);
static void close_output_file(shell_instance_t* shell_inst);
static bool open_input_file(shell_instance_t* shell_inst);
static char close_input_file(shell_instance_t* shell_inst, char input_char);

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
    register_command(shellserver, &sh_echo_cmd, NULL, NULL, NULL);

	return start_threaded_server(&shellserver->server, configfile, shell_thread, shellserver, SHELL_TASK_STACK_SIZE, SHELL_TASK_PRIORITY);
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
void register_command(shellserver_t* shell, shell_cmd_t* cmd, shell_cmd_func_t cmdfunc, const char* name, const char* usage)
{
    if(cmd)
    {
        shell_cmd_init(cmd, cmdfunc, name, usage);
        if(shell->head_cmd)
            cmd->next = shell->head_cmd;
        shell->head_cmd = cmd;
    }
}

/**
 * this function runs inside a new thread, spawned by the threaded_server
 */
void shell_thread(sock_conn_t* conn)
{
	shellserver_t* shell = (shellserver_t*)conn->ctx;
	shell_instance(shell, conn->connfd, conn->connfd);
}

/**
 * this function runs a shell instance, blocking till the file descriptors become invalid or until the exit command is isssued.
 */
void shell_instance(shellserver_t* shell, int rdfd, int wrfd)
{
	shell_instance_t* shell_inst = calloc(sizeof(shell_instance_t), 1);

	if(shell_inst)
	{
		getcwd(shell_cwd, SHELL_CWD_LENGTH_MAX);
		shell_inst->exitflag = false;
		shell_inst->input_index = 0;
		shell_inst->cursor_index = 0;
		shell_inst->head_cmd = shell->head_cmd;
		shell_inst->history_index = -1;
		shell_inst->history_save_index = 0;
		shell_inst->rdfd = rdfd;
		shell_inst->rdfs = NULL;
		shell_inst->wrfd = wrfd;
		shell_inst->rdfd_hold = -1;
		shell_inst->wrfd_hold = -1;
		shell_inst->sstat.st_size = 0;
		shell_inst->sstat.st_mode = 0;

		prompt(shell_inst);
		free(shell_inst);
	}
}

/**
 * this function loops while the shell instance exitflag is set to false.
 * it processes serial IO, decodes commands and executes them.
 * the exit flag is set when the user runs the built in command "exit",
 * or when the client connection closes.
 */
void prompt(shell_instance_t* shell_inst)
{
	int code;
	unsigned char input_char = 0;
	unsigned char inject = '\0';
	unsigned char i = 0;

	while(!shell_inst->exitflag)
	{
		if(inject == '\0' && read(shell_inst->rdfd, &input_char, 1) <= 0)
			shell_inst->exitflag = true;
		else
		{
			// used by the shell to append a character to the stream
			if(inject != '\0')
			{
				input_char = inject;
				inject = '\0';
			}

			inject = close_input_file(shell_inst, input_char);

			// process special input characters UP, DOWN, LEFT_RIGHT, HOME,
			// END, DELETE, NEWLINE, BACKSPACE and finally any normal characters
			if(input_char == 0x1B) // ESC
			{
				input_char = 0;
				read(shell_inst->rdfd, &input_char, 1);
				if(input_char == 0x5B)	// ANSI escaped sequences, ascii '['
				{
					input_char = 0;
					read(shell_inst->rdfd, &input_char, 1);

					if(input_char == 0x33) // ascii '3'
					{
						input_char = 0;
						read(shell_inst->rdfd, &input_char, 1);
						if(input_char == 0x7E) // DELETE, ascii '~'
						{
							if(shell_inst->cursor_index < shell_inst->input_index)
							{
								// wipe last character
								shell_inst->input_index--;
								for(i = shell_inst->cursor_index; i < shell_inst->input_index; i++)
									shell_inst->input_buffer[i] = shell_inst->input_buffer[i+1];

								shell_inst->input_buffer[shell_inst->input_index] = ' ';
								shell_inst->input_buffer[shell_inst->input_index+1] = '\0';
								put_prompt(shell_inst, shell_inst->input_buffer, false);
								// re print prompt
								shell_inst->input_buffer[shell_inst->input_index] = '\0';
								put_prompt(shell_inst, shell_inst->input_buffer, false);

								// put cursor back where it should be
								for(i = shell_inst->input_index; i > shell_inst->cursor_index; i--)
								{
									write(shell_inst->wrfd, SHELL_LEFTARROW, sizeof(SHELL_LEFTARROW)-1);
								}
							}
						}
					}
					else if(input_char == 0x41) // UP
					{
						shell_inst->history_index--;
						if(shell_inst->history_index < 0)
						{
							shell_inst->history_index = SHELL_HISTORY_LENGTH-1;
						}
						historic_prompt(shell_inst);
					}
					else if(input_char == 0x42) // DOWN
					{
						shell_inst->history_index = -1;
						historic_prompt(shell_inst);
					}
					else if(input_char == 0x44) // LEFT
					{
						if(shell_inst->cursor_index > 0)
						{
							shell_inst->cursor_index--;
							write(shell_inst->wrfd, SHELL_LEFTARROW, sizeof(SHELL_LEFTARROW)-1);
						}
					}
					else if(input_char == 0x43) // RIGHT
					{
						if(shell_inst->cursor_index < shell_inst->input_index)
						{
							shell_inst->cursor_index++;
							write(shell_inst->wrfd, SHELL_RIGHTARROW, sizeof(SHELL_RIGHTARROW)-1);
						}
					}
				}
				else if(input_char == 0x4F)	// HOME, END
				{
					input_char = 0;
					read(shell_inst->rdfd, &input_char, 1);
					if(input_char == 0x48) // HOME
					{
						while(shell_inst->cursor_index > 0)
						{
							write(shell_inst->wrfd, SHELL_LEFTARROW, sizeof(SHELL_LEFTARROW)-1);
							shell_inst->cursor_index--;
						}
					}
					else if(input_char == 0x46) // END
					{
						while(shell_inst->cursor_index < shell_inst->input_index)
						{
							write(shell_inst->wrfd, SHELL_RIGHTARROW, sizeof(SHELL_RIGHTARROW)-1);
							shell_inst->cursor_index++;
						}
					}
				}
			}
			else if(input_char == '\n') // NEWLINE
			{
				shell_inst->input_buffer[shell_inst->input_index] = '\0';

				// parse_input_line opens input and output files - note they are closed by this function
				parse_input_line(shell_inst);

				// if the command decodes as a shell command
				if(shell_inst->input_cmd.cmd)
				{
					// run the command, passing the arguments to it
					// use args[1] as args[0] points to the command
					code = shell_cmd_exec(shell_inst->input_cmd.cmd, shell_inst->wrfd, shell_inst->input_cmd.args + 1, shell_inst->input_cmd.nargs);
					return_code_catcher(shell_inst, code);

					close_output_file(shell_inst);
				}

				// reset the shell input buffer
				shell_inst->input_index = 0;
				shell_inst->cursor_index = 0;
				put_prompt(shell_inst, NULL, true);
			}
			else if(input_char == 0x7F) // BACKSPACE
			{
				if(shell_inst->cursor_index > 0)
				{
					// wipe last character
					shell_inst->input_index--;
					shell_inst->cursor_index--;
					for(i = shell_inst->cursor_index; i < shell_inst->input_index; i++)
						shell_inst->input_buffer[i] = shell_inst->input_buffer[i+1];

					shell_inst->input_buffer[shell_inst->input_index] = ' ';
					shell_inst->input_buffer[shell_inst->input_index+1] = '\0';
					put_prompt(shell_inst, shell_inst->input_buffer, false);
					// re print prompt
					shell_inst->input_buffer[shell_inst->input_index] = '\0';
					put_prompt(shell_inst, shell_inst->input_buffer, false);

					// put cursor back where it should be
					for(i = shell_inst->input_index; i > shell_inst->cursor_index; i--)
					{
						write(shell_inst->wrfd, SHELL_LEFTARROW, sizeof(SHELL_LEFTARROW)-1);
					}
				}
			}
			else // all others
			{
				// ignore unprintable characters below Space
				// not sure about characters above 126, Tilde
				if(input_char >= ' ')
				{
					if(shell_inst->input_index < SHELL_CMD_BUFFER_SIZE-1)
					{
						for(i = shell_inst->input_index; i > shell_inst->cursor_index; i--)
						{
							shell_inst->input_buffer[i] = shell_inst->input_buffer[i-1];
						}
						shell_inst->input_buffer[shell_inst->cursor_index] = input_char;
						shell_inst->cursor_index++;
						shell_inst->input_index++;
					}
					else
					{
						// ignore input_char overflow
						shell_inst->input_index = 1;
						shell_inst->cursor_index = 1;
						shell_inst->input_buffer[0] = input_char;
					}

					shell_inst->input_buffer[shell_inst->input_index] = '\0';

					if(shell_inst->input_index == 0)
					{
						put_prompt(shell_inst, NULL, true);
					}

					write(shell_inst->wrfd, &shell_inst->input_buffer[shell_inst->cursor_index-1], strlen((const char*)&shell_inst->input_buffer[shell_inst->cursor_index-1]));

					// put cursor back where it should be
					for(i = shell_inst->input_index; i > shell_inst->cursor_index; i--)
					{
						write(shell_inst->wrfd, SHELL_LEFTARROW, sizeof(SHELL_LEFTARROW)-1);
					}
				}
			}
		}
	}
}

/**
 * copies a prompt from a the history buffer to the input buffer, then displays it.
 */
void historic_prompt(shell_instance_t* shell_inst)
{
	if(shell_inst->history_index >= 0 && shell_inst->history_index < SHELL_HISTORY_LENGTH)
	{
		if(shell_inst->history[shell_inst->history_index][0] != '\0')
		{
			clear_prompt(shell_inst);
			strncpy((char*)shell_inst->input_buffer, (const char*)shell_inst->history[shell_inst->history_index], sizeof(shell_inst->input_buffer)-1);
			shell_inst->input_index = shell_inst->cursor_index = strlen((const char*)shell_inst->input_buffer);
			put_prompt(shell_inst, shell_inst->input_buffer, false);
		}
	}
	else
	{
		clear_prompt(shell_inst);
		shell_inst->input_index = shell_inst->cursor_index = 0;
		shell_inst->input_buffer[shell_inst->input_index] = '\0';
		put_prompt(shell_inst, NULL, false);
	}
}

/**
 * clears the input buffer to a fresh prompt.
 */
void clear_prompt(shell_instance_t* shell_inst)
{
	shell_inst->input_index = shell_inst->cursor_index = 0;
	while(shell_inst->input_buffer[shell_inst->input_index])
	{
		shell_inst->input_buffer[shell_inst->input_index] = ' ';
		shell_inst->input_index++;
	}
	shell_inst->input_index = 0;
	put_prompt(shell_inst, shell_inst->input_buffer, false);
}

/**
 * prints the prompt string.
 */
void put_prompt(shell_instance_t* shell_inst, const char* argstr, bool newline)
{
	int len = strlen(shell_cwd);

	write(shell_inst->wrfd, "\r", 1);
	if(newline)
		write(shell_inst->wrfd, "\n", 1);

	if(len > 0)
	{
		write(shell_inst->wrfd, SHELL_DRIVE, sizeof(SHELL_DRIVE)-1);
		write(shell_inst->wrfd, shell_cwd, len);
		write(shell_inst->wrfd, SHELL_PROMPT, sizeof(SHELL_PROMPT)-1);
	}
	else
		write(shell_inst->wrfd, SHELL_ROOT_PROMPT, sizeof(SHELL_ROOT_PROMPT)-1);

	if(argstr)
		write(shell_inst->wrfd, argstr, strlen((const char*)argstr));
}

/**
 * parses a string captured by prompt() for valid commands. when a valid command is detected,
 * the function associated with that command is called.
 * populates cmd with the command if one is found.
 * if no command is found, cmd->cmd is set to NULL.
 */
void parse_input_line(shell_instance_t* shell_inst)
{
	shell_cmd_t* head = shell_inst->head_cmd;
	char* iter;
	shell_inst->input_cmd.cmd = NULL;
    unsigned char delimiter;

	if(head)
	{
		// be sure that the shell_inst->input_buffer is 0 terminated
		shell_inst->input_buffer[sizeof(shell_inst->input_buffer)-1] = '\0';

		// if there was some input, (overwrite oldest save)
		if(shell_inst->input_buffer[0] != '\0')
		{
			strncpy((char*)shell_inst->history[shell_inst->history_save_index],
					(const char*)shell_inst->input_buffer,
					sizeof(shell_inst->history[shell_inst->history_save_index]) - 1);
			shell_inst->history_save_index++;
			if(shell_inst->history_save_index >= SHELL_HISTORY_LENGTH)
				shell_inst->history_save_index = 0;
		}

		memset(shell_inst->input_cmd.args, 0, sizeof(shell_inst->input_cmd.args));

		// split input into string chunks delimited by spaces, each is an argument
		// args[0] is special, it holds the command
		iter = shell_inst->input_buffer;
		shell_inst->input_cmd.nargs = 0;
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
			shell_inst->input_cmd.args[shell_inst->input_cmd.nargs] = iter;
			shell_inst->input_cmd.nargs++;
			if(shell_inst->input_cmd.nargs >= SHELL_MAX_ARGS)
				break;

			// if there was no shell_inst->starting delimiter, close the block on the next space
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

		// null terminate args
		shell_inst->input_cmd.args[shell_inst->input_cmd.nargs] = NULL;


		// match input args[0] (the command) to one of the commands
		while(head && head->name)
		{
			if(!strncmp((const char*)shell_inst->input_cmd.args[0], (const char*)head->name, sizeof(shell_inst->input_buffer)-1))
				break;
			head = head->next;
		}

		// return command if one was matched
		if(head && head->name)
		{
			write(shell_inst->wrfd, SHELL_NEWLINE, sizeof(SHELL_NEWLINE)-1);

			// find special characters
			open_output_file(shell_inst);

			// reduce number of arguments by one as first arg is just the command
			shell_inst->input_cmd.nargs--;
			shell_inst->input_cmd.cmd = head;
		}
		// attempt to process an input file
		else if(open_input_file(shell_inst))
		{

		}
		else if(*shell_inst->input_cmd.args)
		{
			// print error message if the buffer had some content but no valid command or input file
			write(shell_inst->wrfd, SHELL_NO_SUCH_COMMAND, sizeof(SHELL_NO_SUCH_COMMAND)-1);
			write(shell_inst->wrfd, *shell_inst->input_cmd.args, strlen((const char*)*shell_inst->input_cmd.args));
		}
	}
}

/**
 * finds the special characters > and >> for output file redirect.
 *
 * opens the new output file if possible. the output file must be closed by
 * calling close_output_file().
 */
void open_output_file(shell_instance_t* shell_inst)
{
	int outflags = -1;
	int fdes;
	const char** args;

	// redirect to output file
	for(args = shell_inst->input_cmd.args, shell_inst->input_cmd.nargs = 0; *args; args++, shell_inst->input_cmd.nargs++)
	{
		// select file mode from redirect character
		if(strcmp(*args, ">") == 0)
			outflags = O_WRONLY|O_CREAT|O_TRUNC;
		else if(strcmp(*args, ">>") == 0)
			outflags = O_WRONLY|O_CREAT|O_APPEND;

		if(outflags != -1)
		{
			// remove the special character and filename (if any)
			*args = NULL;
			args++;
			if(*args)
			{
				// open the file and add a newline if required
				fdes = open(*args, outflags);
				if(fdes != -1 && (outflags & O_APPEND))
					write(fdes, "\n", sizeof("\n")-1);

				if(fdes != -1)
				{
					shell_inst->wrfd_hold = shell_inst->wrfd;
					shell_inst->wrfd = fdes;
				}
			}
			break;
		}
	}
}

/**
 * closes output file and directs back to the original write file descriptor.
 */
void close_output_file(shell_instance_t* shell_inst)
{
	// close output file if any
	if(shell_inst->wrfd_hold != -1 && shell_inst->wrfd != -1)
	{
		close(shell_inst->wrfd);
		shell_inst->wrfd = shell_inst->wrfd_hold;
		shell_inst->wrfd_hold = -1;
	}
}

/**
 * try to open an input file (shell script)
 */
bool open_input_file(shell_instance_t* shell_inst)
{
	// if we are not already processing an input file
	// check if the file is regular and can be opened and has a length
	if(!shell_inst->sstat.st_size && !stat(shell_inst->input_buffer, &shell_inst->sstat))
	{
		if(shell_inst->sstat.st_mode == S_IFREG && shell_inst->sstat.st_size)
		{
			// attempt to open file as input source
			shell_inst->rdfd_hold = shell_inst->rdfd;
			shell_inst->rdfd = open(shell_inst->input_buffer, O_RDONLY);
			if(shell_inst->rdfd == -1)
			{
				shell_inst->rdfd = shell_inst->rdfd_hold;
				shell_inst->sstat.st_size = 0;
				shell_inst->sstat.st_mode = 0;
			}
			else
			{
				shell_inst->rdfs = fdopen(shell_inst->rdfd, "r");
				return true;
			}
		}
	}
	return false;
}

/**
 * closes the input file when it reaches EOF,and redirects input to the originial file descriptor.
 * returns '\0' when not closing. when closing, and input_char is not '\n', returns '\n'.
 */
char close_input_file(shell_instance_t* shell_inst, char input_char)
{
	// process EOF condition in the input file, if any
	if(shell_inst->sstat.st_size)
	{
		// end of input file reached
		if(ftell(shell_inst->rdfs) == shell_inst->sstat.st_size)
		{
			fclose(shell_inst->rdfs);
			shell_inst->rdfs = NULL;
			shell_inst->rdfd = shell_inst->rdfd_hold;
			shell_inst->sstat.st_size = 0;
			shell_inst->sstat.st_mode = 0;
			// check if a trailing newline is needed
			if(input_char != '\n')
				return '\n';
		}
	}

	return '\0';
}

/**
 * processes special shell_cmd_t return codes
 */
void return_code_catcher(shell_instance_t* shell_inst, int code)
{
	shell_cmd_t* head;

	switch(code)
	{
		case SHELL_CMD_KILL:
			shell_inst->exitflag = true;
		break;
        case SHELL_CMD_CHDIR:
            getcwd(shell_cwd, SHELL_CWD_LENGTH_MAX);
        break;
		case SHELL_CMD_PRINT_CMDS:
			head = shell_inst->head_cmd;
			write(shell_inst->wrfd, SHELL_HELP_STR, sizeof(SHELL_HELP_STR)-1);
			while(head)
			{
				write(shell_inst->wrfd, SHELL_NEWLINE, sizeof(SHELL_NEWLINE)-1);
				write(shell_inst->wrfd, head->name, strlen((const char*)head->name));
				head = head->next;
			}
		break;
		case SHELL_CMD_PRINT_USAGE:
			cmd_usage(shell_inst->input_cmd.cmd, shell_inst->wrfd);
		break;
	}
}

