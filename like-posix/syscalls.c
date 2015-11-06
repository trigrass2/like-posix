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
 * @defgroup syscalls System Calls
 *
 * A set of system calls and support functions, aimed to link up:
 *
 * 	- FreeRTOS
 * 	- ChaNs FatFs
 * 	- LwIP
 *
 * The API given on the newlib site was used as a basis: https://sourceware.org/newlib/
 *
 * The result is Device, File and Socket IO, all avaliable under an almost standard C
 * API, including open, close, read, write, fsync, flseek, etc.
 *
 * relies upon:
 * - FreeRTOS
 * - FatFs
 * - cutensils
 *
 * @file syscalls.c
 * @{
 */

#include <sys/socket.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
//#include <errno.h>
#include <time.h>
#include <string.h>
#include "syscalls.h"
#include "cutensils.h"
#include "strutils.h"
#include "systime.h"


/**
 * filetable entry definition
 */
typedef struct {
	dev_ioctl_t* device;	///< pointer to the device interface
    int fdes;               ///< a secondary file descriptor, used only for sockets at this time.
    int mode;               ///< the the mode under which the device was opened
	int flags;				///< the the mode under which the device was opened
	FIL file;				///< regular file, or device interface file
	unsigned int size;		///< size, used only for queues
	SemaphoreHandle_t read_lock; 	///< file read lock, mutex
	SemaphoreHandle_t write_lock; 	///< file write lock, mutex
	unsigned char dupcount;	///< increments for every dup / dup2
}filtab_entry_t;

/**
 * file table definition.
 */
typedef struct {
	int count;									///< the number of open files, 0 means nothing open yet
	filtab_entry_t* tab[FILE_TABLE_LENGTH];		///< the file table
	dev_ioctl_t* devtab[DEVICE_TABLE_LENGTH];	///< the device table
	SemaphoreHandle_t lock;                     ///< file table lock.
	int hwm;                                    ///< file table high water mark
}_filtab_t;

/**
 * the max number of bytes to read from a device interface file
 */
#define DEVICED_INTERFACE_FILE_SIZE		32

#define DEFAULT_DEVICE_TIMEOUT          1000
#define DEFAULT_FILETABLE_TIMEOUT		40000
#define DEFAULT_FILE_LOCK_TIMEOUT		10000

#define lock_filtab()                   (xSemaphoreTake(filtab.lock, DEFAULT_FILETABLE_TIMEOUT/portTICK_RATE_MS) == pdTRUE)
#define unlock_filtab()                 xSemaphoreGive(filtab.lock)


#undef errno
extern int errno;
char *__env[1] = {0};
char **__environ = __env;
static _filtab_t filtab;

/**
 * to make STDIO work with serial IO,
 * please define "void phy_putc(char c)" somewhere
 */
extern void phy_putc(char c) __attribute__((weak));
extern char phy_getc() __attribute__((weak));

/**
 * initialses likeposix state.
 */
void init_likeposix()
{
    if(filtab.lock == NULL)
    {
        filtab.lock = xSemaphoreCreateMutex();
        assert_true(filtab.lock);
    }
}


/**
 * get the file table entry for the specified file descriptor.
 *
 * @param	file is a file pointer to a device file.
 * @retval 	the file table entry for a given file descriptor,
 * 			or NULL if the file descriptor was not valid, or the file table entry was NULL (file is not open).
 */
inline filtab_entry_t* __get_entry(int file)
{
	file -= FILE_TABLE_OFFSET;

	if(!filtab.count || file < 0 || (file > FILE_TABLE_LENGTH))
		return NULL;

	return filtab.tab[file];
}

/**
 * deletes the structures of a file table entry.
 * does not remove the entry from the file table.
 *
 * @param fte is a pointer to the file table entry to delete.
 */
inline void __delete_filtab_item(filtab_entry_t* fte)
{
	if(fte->dupcount > 0)
		fte->dupcount--;
	else
	{
		if((fte->mode == S_IFREG) || (fte->mode == S_IFIFO))
		{
			// #1 close the file
			f_close(&fte->file);
			// # 2 remove pipe
			if(fte->device)
			{
				// remove read & write queues
				if(fte->device->pipe.read)
					vQueueDelete(fte->device->pipe.read);
				if(fte->device->pipe.write)
					vQueueDelete(fte->device->pipe.write);
			}
		}
	#if ENABLE_LIKEPOSIX_SOCKETS
		else if(fte->mode == S_IFSOCK)
		{
			if(fte->fdes != -1)
				lwip_close(fte->fdes);
		}
	#endif

		if(fte->read_lock != NULL)
			 vSemaphoreDelete(fte->read_lock);
		if(fte->write_lock != NULL)
			 vSemaphoreDelete(fte->write_lock);

		// #3 delete file table node
		vPortFree(fte);
	}
}

/**
 * create a new file stat structure.
 *
 * does not enter the structure in to the file table...
 *
 * if mode contains S_IFREG, the file number returned operates on a regular file, as per the conditions
 * given in flags.
 *
 * if mode contains S_IFIFO, the file number returned operates on a pair of queues, rather than a file.
 *  - if flags contains FREAD, then a read queue of length bytes becomes available to the read() function.
 *  - if flags contains FWRITE, then a write queue of length bytes becomes available to the write() function.
 *  - the opposing ends of the queues may be interfaced to a device in a device driver module...
 *
 * @param 	fdes is a pointer to a raw file table entry, which doesnt have to be pre initialized.
 * @param	name is the name of the file, or device file to open.
 * @param	flags may be a combination of one of O_RDONLY, O_WRONLY, or O_RDWR,
 * 			and any of O_APPEND | O_CREAT | O_TRUNC | O_NONBLOCK.
 * 			When used by S_IFSOCK, specify O_CREAT to open a new socket, or anything else to specify accept.
 * @param 	mode is one of S_IFDIR | S_IFCHR | S_IFBLK | S_IFREG | S_IFLNK | S_IFSOCK | S_IFIFO.
 * 			only S_IFREG, S_IFSOCK and S_IFIFO are supported.
 * @param   length specifies the queue length to assign to S_IFIFO type devices only. may be set to 0 for S_IFSOCK and S_IFREG.
 * @param   when flags is set to O_CREAT, sockparam1 specifies the socket namespace to assign to S_IFSOCK type devices only. may be set to 0 for S_IFIFO and S_IFREG.
 * 			when flags is not set to O_CREAT, sockparam1 specifies the socket file descriptor to accept with, for S_IFSOCK type devices only. may be set to 0 for S_IFIFO and S_IFREG.
 * @param   sockparam2 specifies the socket style to assign to S_IFSOCK type devices only. may be set to 0 for S_IFIFO and S_IFREG.
 * 			when flags is not set to O_CREAT, sockparam2 specifies the socket "struct sockaddr *" to accept with, for S_IFSOCK type devices only. may be set to 0 for S_IFIFO and S_IFREG.
 * @param   sockparam3 specifies the socket protocol to assign to S_IFSOCK type devices only. may be set to 0 for S_IFIFO and S_IFREG.
 * 			when flags is not set to O_CREAT, sockparam3 specifies the socket "socklen_t *" to accept with, for S_IFSOCK type devices only. may be set to 0 for S_IFIFO and S_IFREG.
 *
 * @retval 0 on success, -1 on failure.
 */
inline int __create_filtab_item(filtab_entry_t** fdes, const char* name, int flags, int mode, int length, int sockparam1, int sockparam2, int sockparam3)
{
	int success = EOF;
	BYTE ff_flags = 0;

	// create new file table node
	filtab_entry_t* fte = (filtab_entry_t*)pvPortMalloc(sizeof(filtab_entry_t));

	if(fte)
	{
		fte->fdes = -1;
		fte->mode = mode;
		fte->device = NULL;
		fte->flags = flags+1;
		fte->size = length;
		fte->read_lock = NULL;
		fte->write_lock = NULL;
		fte->dupcount = 0;

		/**********************************
		 * create file
		 **********************************/

		if(fte->mode == S_IFREG)
		{
			if(fte->flags&FREAD)
				ff_flags |= FA_READ;
			if(fte->flags&FWRITE)
				ff_flags |= FA_WRITE;

			if(fte->flags&O_CREAT)
			{
				if(fte->flags&O_TRUNC)
					ff_flags |= FA_CREATE_ALWAYS;
				else
					ff_flags |= FA_OPEN_ALWAYS;
			}
			else
				ff_flags |= FA_OPEN_EXISTING;

			// TODO can we used this flag? FA_CREATE_NEW
		}
		else if(fte->mode == S_IFIFO)
		{
		    // for character devices always read device descriptor file
		    ff_flags = FA_READ;
		}

		// we only open the device file if ff_flags has a non zero value
		if(ff_flags == 0 || (name && f_open(&fte->file, (const TCHAR*)name, (BYTE)ff_flags) == FR_OK))
		{
			if(fte->mode == S_IFREG)
			{
				success = 0;
				if(fte->flags&O_APPEND)
					f_lseek(&fte->file, f_size(&fte->file));
			}
			else if(fte->mode == S_IFIFO)
			{
				/**********************************
				 * create data pipe
				 **********************************/

				// fetch device interface index
				unsigned char buf[DEVICED_INTERFACE_FILE_SIZE];
				int devindex;
				unsigned int n = 0;

				// read device index (buf[0])
				f_read(&fte->file, buf, (UINT)DEVICED_INTERFACE_FILE_SIZE, (UINT*)&n);
				devindex = (int)buf[0];

				if(n > 0 && devindex < DEVICE_TABLE_LENGTH)
					fte->device = filtab.devtab[devindex];

				// populate "pipe", timeout values
				if(fte->device)
				{
				    if(fte->flags & O_NONBLOCK)
				        fte->device->timeout = 0;
				    else
				        fte->device->timeout = DEFAULT_DEVICE_TIMEOUT/portTICK_RATE_MS;

					// create write device queue
					char write_q = 1;
					fte->device->pipe.write = NULL;
					if(fte->flags&FWRITE)
					{
						fte->device->pipe.write = xQueueCreate(fte->size, 1);
						write_q = fte->device->pipe.write ? 1 : 0;
					}

					// create read device queue
					char read_q = 1;
					fte->device->pipe.read = NULL;
					if(fte->flags&FREAD)
					{
						fte->device->pipe.read = xQueueCreate(fte->size, 1);
						read_q = fte->device->pipe.read ? 1 : 0;
					}

					if(read_q && write_q)
						success = 0;
				}
			}
#if ENABLE_LIKEPOSIX_SOCKETS
			else if(fte->mode == S_IFSOCK)
			{
				if(fte->flags & FCREAT)
				{
					fte->flags = FWRITE | FREAD;
					fte->size = 0;
					fte->fdes = lwip_socket(sockparam1, sockparam2, sockparam3);
					if(fte->fdes != -1)
						success = 0;
				}
				else
				{
					fte->flags = FWRITE | FREAD;
					fte->size = 0;
					fte->fdes = lwip_accept(sockparam1, (struct sockaddr *)sockparam2, (socklen_t *)sockparam3);
					if(fte->fdes != -1)
						success = 0;
				}
			}
#else
			(void)sockparam1;
			(void)sockparam2;
			(void)sockparam3;
#endif
		}

		if(success == 0)
		{
			fte->read_lock = xSemaphoreCreateMutex();
			fte->write_lock = xSemaphoreCreateMutex();

			if(fte->read_lock && fte->write_lock)
				*fdes = fte;
			else
				success = EOF;
		}

		if(success != 0)
			__delete_filtab_item(fte);
	}

	return success;
}

/**
 * put a file table entry into file table.
 * NOTE: locks the file table. may not be called within a function that locks the file table.
 *
 * @param 	fte is a pointer to a file table entry, which NEEDS to have been pre initialized.
 * @retval 	the file descriptor if successful, or -1 on error.
 */
inline int __insert_entry(filtab_entry_t* fte)
{
	int ret = EOF;
	if(lock_filtab())
	{
		int file;
		for(file = 0; file < FILE_TABLE_LENGTH; file++)
		{
			if(filtab.tab[file] == NULL)
			{
				filtab.tab[file] = fte;
				filtab.count++;
				if(filtab.hwm < filtab.count)
					filtab.hwm = filtab.count;
				ret = file + FILE_TABLE_OFFSET;
				break;
			}
		}
		unlock_filtab();
	}

	return ret;
}

/**
 * put a file table entry into file table at the specified file index.
 * NOTE: locks the file table. may not be called within a function that locks the file table.
 *
 * @param 	fte is a pointer to a file table entry, which NEEDS to have been pre initialized.
 * @param 	file is a file descriptor.
 * @retval 	the file descriptor if successful, or -1 on error.
 */
inline int __insert_entry_at(filtab_entry_t* fte, int file)
{
	int ret = EOF;
	if(lock_filtab())
	{
		file -= FILE_TABLE_OFFSET;

		if(filtab.tab[file] == NULL)
		{
			filtab.tab[file] = fte;
			filtab.count++;
			if(filtab.hwm < filtab.count)
				filtab.hwm = filtab.count;
			ret = file + FILE_TABLE_OFFSET;
		}
		unlock_filtab();
	}

	return ret;
}

/**
 * determine the mode to open the file with - this is a customization of the mode passed into _open()...
 *
 * @param name - /dev/anything - yields S_IFIFO
 * 				 /anything/anything - yields S_IFREG
 * 				 NULL - yields S_IFSOCK
 *
 * returns a combination of the following...
 *  @arg     S_IFDIR - not implemented
 *  @arg     S_IFCHR - not implemented
 *  @arg     S_IFBLK - not implemented
 *  @arg     S_IFREG - signifies regular a file
 *  @arg     S_IFLNK - not implemented
 *  @arg     S_IFSOCK - not implemented
 *  @arg     S_IFIFO - signifies open a FIFO (interfacing a device)
 */
inline int __determine_mode(const char *name)
{
#if ENABLE_LIKEPOSIX_SOCKETS
	if(!name)
		return S_IFSOCK;
#endif
	return strncmp(name, DEVICE_INTERFACE_DIRECTORY, sizeof(DEVICE_INTERFACE_DIRECTORY)-1) == 0 ? S_IFIFO : S_IFREG;
}

/**
 * installs a device for use by the application.
 *
 * an index into filtab.devtab is written into the device file,
 * this index is used by open() to interface a one of filtab.tab to one of filtab.devtab.
 *
 * @param	name is the full path to the file to associate with the device.
 * @param	dev_ctx is a pointer to some data that will be passed to the device driver
 *			driver ioctl functions.
 * @param	read_enable is an ioctl function that can enable a device to read data.
 * @param	write_enable is an ioctl function that can enable a device to write data.
 * @param   open is an ioctl function that can enable a device.
 * @param	close is an ioctl function that can disable a device.
 * @param	ioctl is an ioctl function that can set the hardware settings of a device.
 * @retval	returns a pointer to the created dev_ioctl_t structure, or NULL on error.
 */
dev_ioctl_t* install_device(char* name,
					void* dev_ctx,
					dev_ioctl_fn_t read_enable,
					dev_ioctl_fn_t write_enable,
                    dev_ioctl_fn_t open_dev,
					dev_ioctl_fn_t close_dev,
					dev_ioctl_fn_t ioctl)
{
	FIL f;
	unsigned char buf[DEVICED_INTERFACE_FILE_SIZE];
	int device;
	dev_ioctl_t* ret = NULL;
	unsigned int n = 0;
	bool file_installed = false;

	log_syslog(NULL, "installing %s...", name);

	// try to create the directory first, if it fails it is already there
	// or, the disk isnt in. the next step will also fail in that case.
	f_mkdir(DEVICE_INTERFACE_DIRECTORY);

    // is it possible to open the device file?
    if(f_open(&f, (const TCHAR*)name, FA_WRITE|FA_READ|FA_OPEN_ALWAYS) == FR_OK)
    {
        for(device = 0; device < DEVICE_TABLE_LENGTH; device++)
        {
            // found an empty slot
            if(filtab.devtab[device] == NULL)
            {
            	// check is file installed already?
            	n = 0;
            	buf[0] = MAX_DEVICE_TABLE_ENTRIES;
            	file_installed = f_read(&f, buf, (UINT)1, (UINT*)&n) == FR_OK && n == 1 && buf[0] == (unsigned char)device;

            	// if not attempt to install
            	if(!file_installed)
            	{
					n = 0;
					buf[0] = (unsigned char)device;
					file_installed = f_write(&f, buf, (UINT)1, (UINT*)&n) == FR_OK && n == 1;
            	}

            	// setup the device if appropriate
                if(file_installed)
                {
                	// create device io structure and populate api
					filtab.devtab[device] = pvPortMalloc(sizeof(dev_ioctl_t));
					if(filtab.devtab[device])
					{
						// note that filtab.devtab[device]->pipe is populated by _open()
						filtab.devtab[device]->timeout = 0;
						filtab.devtab[device]->read_enable = read_enable;
						filtab.devtab[device]->write_enable = write_enable;
						filtab.devtab[device]->ioctl = ioctl;
						filtab.devtab[device]->open = open_dev;
						filtab.devtab[device]->close = close_dev;
						filtab.devtab[device]->ctx = dev_ctx;
						filtab.devtab[device]->termios = NULL;
					}
					ret = filtab.devtab[device];
					log_syslog(NULL, "%s installed", name);
                }
				else
					log_error(NULL, "failed to install device %s", name);

                break;
            }
        }
        f_close(&f);
    }
    else
        log_error(NULL, "failed to open device %s", name);

	return ret;
}

/**
 * @retval  the number of files open right now.
 */
int file_table_open_files()
{
   return filtab.count;
}

/**
 * @retval  the highest number of files opened since boot.
 */
int file_table_hwm()
{
    return filtab.hwm;
}

/**
 * system call, 'open'
 *
 * opens a file for disk IO, or a FreeRTOS Queue, for
 * device IO or data transfer/sharing.
 *
 * **this is a non standard implementation**
 *
 * @param	the name of the file to open
 * @param	flags -
 * 			if mode is S_IFREG, may be a combination of:
 *  		one of O_RDONLY, O_WRONLY, or O_RDWR,
 * 			and any of O_APPEND | O_CREAT | O_TRUNC
 * @param	mode - repurposed - in the case of device files, specified the queue length.
 * 			otherwise ignored.
 * @retval	returns a file descriptor, that may be used with
 * 			read(), write(), close(), or -1 if there was an error.
 */
int _open(const char *name, int flags, int mode)
{
	if(filtab.count > FILE_TABLE_LENGTH || !name)
		return EOF;

	filtab_entry_t* fte = NULL;
	int length = mode;
	int file = EOF;

	// if we got 0 here it means a file table entry was made successfully
	if(__create_filtab_item(&fte, name, flags, __determine_mode(name), length, 0, 0, 0) == 0)
	{
		if((fte->mode == S_IFIFO) && fte->device)
		{
			// call device open
			if(fte->device->open)
				fte->device->open(fte->device);
			// enable reading
			if((fte->flags & FREAD) && fte->device->read_enable)
				fte->device->read_enable(fte->device);
			// writing is enabled in _write()...
		}

		// add file to table
		file = __insert_entry(fte);

		// add failed, close and delete
		if(file == EOF)
		{
			if((fte->mode == S_IFIFO) && fte->device && fte->device->close)
				fte->device->close(fte->device);
			__delete_filtab_item(fte);
		}
	}

	return file;
}

/**
 * close the specified file descriptor.
 *
 * Note: this will wait for DEFAULT_FILE_LOCK_TIMEOUT before closing a file that is currently reading or writing.
 * if wait times out, an attempt will be made to close the file. The safety of this behaviour is dependent on the
 * behavior of the underlying file technology (socket, device, disk, etc)
 *
 * @param	file is the file descriptor to close.
 * @retval 	0 on success, -1 on error.
 */
int _close(int file)
{
	int res = EOF;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{

	}
	else if(lock_filtab())
	{
		filtab_entry_t* fte = __get_entry(file);

		if(fte)
		{
			if(fte->flags & FWRITE)
				xSemaphoreTake(fte->write_lock, DEFAULT_FILE_LOCK_TIMEOUT/portTICK_RATE_MS);

			if(fte->flags & FREAD)
				xSemaphoreTake(fte->read_lock, DEFAULT_FILE_LOCK_TIMEOUT/portTICK_RATE_MS);

			// disable device IO first
			if((fte->mode == S_IFIFO) && fte->device && fte->device->close)
			{
				// call device close
				fte->device->close(fte->device);
			}
			// then remove the file table entry
			filtab.tab[file-FILE_TABLE_OFFSET] = NULL;
			filtab.count--;

			xSemaphoreGive(fte->write_lock);
			xSemaphoreGive(fte->read_lock);

			// then delete all the file structures
			__delete_filtab_item(fte);
			res = 0;
		}
		unlock_filtab();
	}

	return res;
}

/**
 * TODO: doesnt work for STDIN_FILENO, STDOUTFILENO, STDERR_FILENO
 * TODO: not thread safe
 */
int _dup(int file)
{
	int res = EOF;
	filtab_entry_t* fte = __get_entry(file);
	if(fte)
	{
		res = __insert_entry(fte);
		if(res != EOF)
			fte->dupcount++;
	}
	return res;
}

/**
 * TODO: doesnt work for STDIN_FILENO, STDOUTFILENO, STDERR_FILENO
 * TODO: not thread safe
 */
int _dup2(int old, int new)
{
	int res = EOF;

	filtab_entry_t* fteold = __get_entry(old);

	if(fteold)
	{
		if(old == new)
			res = new;
		else
		{
			if(__get_entry(new))
				_close(new);

			res = __insert_entry_at(fteold, new);
			if(res != EOF)
				fteold->dupcount++;
		}
	}

	return res;
}

static inline filtab_entry_t* __lock(int file, bool read, bool write)
{
	int lock_successful = pdTRUE;
	if(lock_filtab())
	{
		filtab_entry_t* fte = __get_entry(file);
		if(fte)
		{
			if(write && (fte->flags & FWRITE))
				lock_successful = xSemaphoreTake(fte->write_lock, DEFAULT_FILE_LOCK_TIMEOUT/portTICK_RATE_MS);

			if(read && (fte->flags & FREAD) && (lock_successful == pdTRUE))
				lock_successful = xSemaphoreTake(fte->read_lock, DEFAULT_FILE_LOCK_TIMEOUT/portTICK_RATE_MS);
		}
		unlock_filtab();
		if(lock_successful == pdTRUE)
			return fte;
		else
			return NULL;
	}
	return NULL;
}

static inline void __unlock(filtab_entry_t* fte, bool read, bool write)
{
	if(write && (fte->flags & FWRITE))
		xSemaphoreGive(fte->write_lock);
	if(read && (fte->flags & FREAD))
		xSemaphoreGive(fte->read_lock);
}

/**
 * writes a buffer to the file specified.
 *
 * @param	file is a file descriptor, may be the value returned by
 * 			a call to the open() syscall, or STDOUT_FILENO or STDERR_FILENO.
 * @param	a buffer of characters to write.
 * @param	count, the number of characters to write.file
 * @retval	the number of characters written or -1 on error.
 */
int _write(int file, char *buffer, unsigned int count)
{
	int n = EOF;

	if(count == 0)
		return 0;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{
		for(n = 0; n < (int)count; n++)
			phy_putc(*buffer++);
	}
	else
	{
		filtab_entry_t* fte = __lock(file, false, true);

		if(fte)
		{
			if(fte->flags & FWRITE)
			{
				if(fte->mode == S_IFREG)
				{
					if(f_write(&fte->file, (void*)buffer, (UINT)count, (UINT*)&n) != FR_OK)
						n = EOF;
				}
				else if((fte->mode == S_IFIFO) && fte->device)
				{
				    unsigned int timeout = 0;
					bool tx_idle = uxQueueMessagesWaiting(fte->device->pipe.write) == 0;

					// write the remaining data
					n = 0;
					while(n < (int)count)
					{
						if(xQueueSend(fte->device->pipe.write, buffer, timeout) != pdTRUE)
						{
							if(timeout)
								break;
							else
							{
								timeout = fte->device->timeout;
								// enable the physical device to write
								if(fte->device->write_enable)
									fte->device->write_enable(fte->device);
								tx_idle = false;
							}
						}
						else
						{
							n++;
							buffer++;
						}
					}

					if(tx_idle)
					{
						if(fte->device->write_enable)
							fte->device->write_enable(fte->device);
					}
				}
#if ENABLE_LIKEPOSIX_SOCKETS
				else if(fte->mode == S_IFSOCK)
				{
					n = lwip_write(fte->fdes, buffer, count);
				}
#endif
			}
			__unlock(fte, false, true);
		}
	}

	return n;
}

/**
 * reads a number of characters into a buffer from the file specified.
 *
 * @param	file is a file descriptor, may be the value returned by
 * 			a call to the open() syscall, or STDIN_FILENO
 * @param	a buffer for the characters to read.
 * @param	count, the number of characters to read.
 * @retval	the number of characters read or -1 on error.
 */
int _read(int file, char *buffer, int count)
{
    unsigned int timeout;
	int n = EOF;

	if(count == 0)
		return 0;

	if(file == STDIN_FILENO)
	{
		for(n = 0; n < count; n++)
			*buffer++ = phy_getc();
	}
	else
	{
		filtab_entry_t* fte = __lock(file, true, false);

		if(fte)
		{
			if(fte->flags & FREAD)
			{
				if(fte->mode == S_IFREG)
				{
					f_read(&fte->file, (void*)buffer, (UINT)count, (UINT*)&n);
				}
				else if((fte->mode == S_IFIFO) && fte->device)
				{
					timeout = fte->device->timeout;

					for(n = 0; n < count; n++)
					{
						if(xQueueReceive(fte->device->pipe.read, buffer++, timeout) != pdTRUE)
							break;
						timeout = 0;
					}
				}
	#if ENABLE_LIKEPOSIX_SOCKETS
				else if(fte->mode == S_IFSOCK)
				{
					n = lwip_read(fte->fdes, buffer, count);
				}
	#endif
			}
			__unlock(fte, true, false);
		}
	}

	return n;
}

int _fsync(int file)
{
	int res = EOF;
	if(file == STDIN_FILENO ||
			file == STDOUT_FILENO ||
			file == STDERR_FILENO)
	{
		res = 0;
	}
	else
	{
		filtab_entry_t* fte = __lock(file, false, true);

		if(fte)
		{
			if(fte->mode == S_IFREG)
			{
				f_sync(&fte->file);
				res = 0;
			}
			__unlock(fte, false, true);
		}
	}

	return res;
}

/**
 * populates a struct stat type with:
 *
 *  - st_size	- the size of the file
 *  - st_mode 	- the mode of the file (S_IFCHR, S_IFREG, S_IFIFO, etc)
 *
 *  ... from an open file.
 */
int _fstat(int file, struct stat *st)
{
	int res = EOF;
	if(file == STDIN_FILENO ||
			file == STDOUT_FILENO ||
			file == STDERR_FILENO)
	{
		if(st)
		{
			st->st_size = 1;
			st->st_mode = S_IFCHR;
		}
		res = 0;
	}
	else
	{
		filtab_entry_t* fte = __lock(file, false, true);

		if(fte)
		{
			if(st)
			{
				if(fte->mode == S_IFREG)
				{
					st->st_size = f_size(&fte->file);
					st->st_blksize = _MAX_SS;
				}
				if(fte->mode == S_IFIFO)
				{
					st->st_size = fte->size;
				}

				st->st_mode = fte->mode;
			}

			res = 0;
			__unlock(fte, false, true);
		}
	}

	return res;
}

long int _ftell(int file)
{
	int res = EOF;

	filtab_entry_t* fte = __lock(file, false, true);

	if(fte)
	{
		if(fte->mode == S_IFREG)
			res = f_tell(&fte->file);
		__unlock(fte, false, true);
	}

	return res;
}

/**
 * populates a struct stat type with:
 *
 *  - st_size	- the size of the file
 *  - st_mode 	- the mode of the file (S_IFCHR, S_IFREG, S_IFIFO, etc)
 *
 *  ... from a file that is not already open.
 */
int _stat(char *file, struct stat *st)
{
	int res = EOF;
	int fd = _open(file, O_RDONLY, 0);
	if(fd == EOF)
		return EOF;
	res = _fstat(fd, st);
	_close(fd);
	return res;
}

/**
 * returns 1 if the file is a device, or stdio endpoint, 0 otherwise.
 *
 */
int _isatty(int file)
{
	int res = 0;

	if(file == STDIN_FILENO ||
			file == STDOUT_FILENO ||
			file == STDERR_FILENO)
		res = 1;
	else
	{
		filtab_entry_t* fte = __lock(file, false, true);

		if(fte)
		{
			if(fte->mode == S_IFIFO)
				res = 1;
			__unlock(fte, false, true);
		}
	}
	return res;
}

/**
 * only works for files with mode = S_IFREG (not devices, or stdio's)
 *
 * SEEK_SET 	Offset is to be measured in absolute terms.
 * SEEK_CUR 	Offset is to be measured relative to the current location of the pointer.
 * SEEK_END 	Offset is to be measured back, relative to the end of the file.
 */
int _lseek(int file, int offset, int whence)
{
	int res = EOF;

	filtab_entry_t* fte = __lock(file, true, true);

	if(fte)
	{
		if(fte->mode == S_IFREG)
		{
			if(whence == SEEK_CUR)
				offset = f_tell(&fte->file) + offset;
			else if(whence == SEEK_END)
				offset = f_size(&fte->file) - offset;

			if(offset < 0)
			    offset = 0;

			if(f_lseek(&fte->file, offset) == FR_OK)
				res = 0;
		}
		__unlock(fte, true, true);
	}

	return res;
}

int _chdir(const char *path)
{
    return f_chdir((TCHAR*)path) == FR_OK ? 0 : -1;
}

/**
 * gets the current working directory - follows the GNU version
 * in that id buffer is set to NULL, a buffer of size bytes is allocated
 * to hold the cwd string. it must be freed afterward by the user...
 */
char* _getcwd(char* buffer, size_t size)
{
    bool alloc = false;
    if(buffer == NULL)
    {
        alloc = true;
        buffer = malloc(size);
    }

    if(buffer)
    {
        if(f_getcwd((TCHAR*)buffer, (UINT)size) != FR_OK)
        {
            if(alloc)
                free(buffer);
            buffer = NULL;
        }
    }

    return buffer;
}

int _unlink(char *name)
{
	FRESULT res = f_unlink((const TCHAR*)name);
	return res == FR_OK ? 0 : EOF;
}

int _rename(const char *oldname, const char *newname)
{
	FRESULT res = f_rename((const TCHAR*)oldname, (const TCHAR*)newname);
	return res == FR_OK ? 0 : EOF;
}

void _exit(int i)
{
	printf("Program exit with code %d", i);
	while (1);
}

caddr_t _sbrk(int incr)
{
	(void)incr;
    return NULL;
}

int _link(char *old, char *new)
{
	(void)old;
	(void)new;
	errno = EMLINK;
	return -1;
}

int _execve(char *name, char **argv, char **env)
{
	(void)name;
	(void)argv;
	(void)env;
	errno = ENOMEM;
	return -1;
}

int _fork()
{
	errno = EAGAIN;
	return -1;
}

int _getpid()
{
	return 1;
}

int _kill(int pid, int sig)
{
	(void)pid;
	(void)sig;
	errno = EINVAL;
	return (-1);
}

int times(struct tm *buf)
{
	(void)buf;
	return -1;
}

//#ifdef __cplusplus
//#ifndef _init()
void _init()
{

}
//#endif
//#ifndef _fini
void _fini()
{

}
//#endif
//#endif

int _wait(int *status)
{
	(void)status;
	errno = ECHILD;
	return -1;
}

int __tcflush(filtab_entry_t* fte, int flags)
{
    int res = EOF;
	if(fte->mode == S_IFIFO)
	{
		if(flags == TCIFLUSH)
		{
			xQueueReset(fte->device->pipe.read);
			res = 0;
		}

		else if(flags == TCOFLUSH)
		{
			xQueueReset(fte->device->pipe.write);
			res = 0;
		}

		else if(flags == TCIOFLUSH)
		{
			xQueueReset(fte->device->pipe.write);
			xQueueReset(fte->device->pipe.read);
			res = 0;
		}
	}
    return res;
}

int __tcdrain(filtab_entry_t* fte)
{
    unsigned long timeout;
    int res = EOF;

	if(fte->mode == S_IFIFO)
	{
		timeout = get_hw_time_ms() + fte->device->timeout;
		while(uxQueueMessagesWaiting(fte->device->pipe.write) > 0 && get_hw_time_ms() < timeout)
			portYIELD();

		if(get_hw_time_ms() < timeout)
			res = 0;
	}

    return res;
}

int _tcgetattr(int fildes, struct termios *termios_p)
{
    int ret = -1;
    if(termios_p == NULL || isatty(fildes) == 0)
        return ret;

    memset(termios_p, 0, sizeof(struct termios));

    if(fildes == STDOUT_FILENO || fildes == STDERR_FILENO)
    {
        termios_p->c_cflag = B115200|CS8;
        ret = 0;
    }
    else if(fildes == STDIN_FILENO)
    {
        termios_p->c_cflag = B115200|CS8;
        ret = 0;
    }
    else
	{
		filtab_entry_t* fte = __lock(fildes, false, true);

        if(fte)
        {
			if(fte->device && fte->device->ioctl)
			{
				fte->device->termios = termios_p;
				ret = fte->device->ioctl(fte->device);
				fte->device->termios = NULL;
			}
			__unlock(fte, false, true);
        }
    }

    return ret;
}

int _tcsetattr(int fildes, int when, const struct termios *termios_p)
{
    int ret = -1;
    if(termios_p == NULL || isatty(fildes) == 0)
        return ret;

    if(fildes == STDOUT_FILENO || fildes == STDERR_FILENO)
    {

    }
    else if(fildes == STDIN_FILENO)
    {

    }
    else
	{
		filtab_entry_t* fte = __lock(fildes, false, true);

        if(fte)
        {
			if(fte->device && fte->device->ioctl)
			{
				fte->device->termios = (struct termios *)termios_p;
				if(when == TCSADRAIN)
		        	__tcdrain(fte);
				else if(when == TCSAFLUSH)
					__tcflush(fte, TCIOFLUSH);
				ret = fte->device->ioctl(fte->device);
				fte->device->termios = NULL;
			}
			__unlock(fte, false, true);
        }
    }

    return ret;
}

int _tcdrain(int file)
{
    int res = EOF;

    if(file == STDOUT_FILENO || file == STDERR_FILENO)
    {
        res = 0;
    }
    else if(file == STDIN_FILENO)
    {
        res = 0;
    }
    else
	{
		filtab_entry_t* fte = __lock(file, false, true);

        if(fte)
        {
        	res = __tcdrain(fte);
            __unlock(fte, false, true);
        }
    }

    return res;
}


int _tcflush(int file, int flags)
{
    int res = EOF;

    if(file == STDOUT_FILENO || file == STDERR_FILENO)
    {
        res = 0;
    }
    else if(file == STDIN_FILENO)
    {
        res = 0;
    }
    else
	{
		filtab_entry_t* fte = __lock(file, false, true);

        if(fte)
        {
        	res = __tcflush(fte, flags);
        	__unlock(fte, false, true);
        }
    }
    return res;
}

/**
 * not implemented....
 */
int _tcflow(int file, int flags)
{
    int res = EOF;

    if(file == STDOUT_FILENO || file == STDERR_FILENO)
    {
        res = 0;
    }
    else if(file == STDIN_FILENO)
    {
        res = 0;
    }
    else
	{
		filtab_entry_t* fte = __lock(file, false, true);

        if(fte)
        {
            if(fte->mode == S_IFIFO)
            {
                (void)flags;
                res = EOF;
            }
            __unlock(fte, false, true);
        }
    }
    return res;
}

#if ENABLE_LIKEPOSIX_SOCKETS

/**
 * wrapper for interfacing lwip functions with like-posix
 */
#define SOCKET_WRAPPER(lwip_function, read, write, sockfd, ...)     \
    int res = EOF; 					                               	\
    filtab_entry_t* fte = __lock(sockfd, read, write);              \
    if(fte) {														\
    	if(fte->mode == S_IFSOCK) {                                 \
    		res = lwip_function(fte->fdes, __VA_ARGS__);     		\
    	}															\
		__unlock(fte, true, true);									\
    }																\
	return res;


/**
 * creates a new socket and adds it to the file table.
 *
 * @retval  returns a file descriptor, that may be used with
 *          read(), write(), close(), closesocket(), or -1 if there was an error.
 */
int socket(int namespace, int style, int protocol)
{
	if(filtab.count > FILE_TABLE_LENGTH)
		return EOF;

	filtab_entry_t* fte = NULL;
	int file = EOF;

	// if we got 0 here it means a file table entry was made successfully
	if(__create_filtab_item(&fte, NULL, O_CREAT, __determine_mode(NULL), 0, namespace, style, protocol) == 0)
	{
		// add file to table
		file = __insert_entry(fte);

		// add failed, close and delete
		if(file == EOF)
			__delete_filtab_item(fte);
	}

	return file;
}

/**
 * accepts incoming socket connections and adds them to the file table.
 *
 * ** note, this function is not thread safe, like the other functions in this module **
 *
 * @retval  returns a file descriptor, that may be used with
 *          read(), write(), close(), closesocket(), or -1 if there was an error.
 */
int accept(int sockfd, struct sockaddr *addr, socklen_t *length_ptr)
{
	if(filtab.count > FILE_TABLE_LENGTH)
		return EOF;

	filtab_entry_t* fte = NULL;
	int file = EOF;
	filtab_entry_t* parent = __get_entry(sockfd);

	if(parent && parent->fdes != -1)
	{
		// if we got 0 here it means a file table entry was made successfully
		if(__create_filtab_item(&fte, NULL, 0, __determine_mode(NULL), 0, parent->fdes, (int)addr, (int)length_ptr) == 0)
		{
			// add file to table
			file = __insert_entry(fte);

			// add failed, close and delete
			if(file == EOF)
				__delete_filtab_item(fte);
		}
	}

	return file;
}

int connect(int sockfd, struct sockaddr *addr, socklen_t length)
{
    SOCKET_WRAPPER(lwip_connect, true, true, sockfd, addr, length);
}

int bind(int sockfd, struct sockaddr *addr, socklen_t length)
{
    SOCKET_WRAPPER(lwip_bind, true, true, sockfd, addr, length);
}

int shutdown(int sockfd, int how)
{
    SOCKET_WRAPPER(lwip_shutdown, true, true, sockfd, how);
}

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *length)
{
    SOCKET_WRAPPER(lwip_getsockname, true, true, sockfd, addr, length);
}

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *length)
{
    SOCKET_WRAPPER(lwip_getpeername, true, true, sockfd, addr, length);
}

int setsockopt(int sockfd, int level, int optname, void *optval, socklen_t optlen)
{
    SOCKET_WRAPPER(lwip_setsockopt, true, true, sockfd, level, optname, optval, optlen);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    SOCKET_WRAPPER(lwip_getsockopt, true, true, sockfd, level, optname, optval, optlen);
}

int listen(int sockfd, int n)
{
    SOCKET_WRAPPER(lwip_listen, false, false, sockfd, n);
}

int recv(int sockfd, void *buffer, size_t size, int flags)
{
    SOCKET_WRAPPER(lwip_recv, true, false, sockfd, buffer, size, flags);
}

int recvfrom(int sockfd, void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t *length)
{
    SOCKET_WRAPPER(lwip_recvfrom, true, false, sockfd, buffer, size, flags, addr, length);
}

int send(int sockfd, const void *buffer, size_t size, int flags)
{
    SOCKET_WRAPPER(lwip_send, false, true, sockfd, buffer, size, flags);
}

int sendto(int sockfd, const void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t length)
{
    SOCKET_WRAPPER(lwip_sendto, false, true, sockfd, buffer, size, flags, addr, length);
}

/**
 * TODO - finish me
 */
//int select(int nfds, fd_set *read-fds, fd_set *write-fds, fd_set *except-fds, struct timeval *timeout)
//{
//    SOCKET_WRAPPER(lwip_select, sockfd, buffer, size, flags, addr, length);
//}

int ioctlsocket(int sockfd, int cmd, void* argp)
{
    SOCKET_WRAPPER(lwip_ioctl, true, true, sockfd, cmd, argp);
}

/**
 * clashes with signature in standard header sys/_default_fcntl.h
 */
//int fcntl(int filedes, int cmd, int val)
//{
//    SOCKET_WRAPPER(lwip_fcntl, filedes, cmd, val);
//}
int closesocket(int socket)
{
    return _close(socket);
}
#endif
/**
 * @}
 */
