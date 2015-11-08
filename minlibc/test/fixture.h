/*
 * fixture.h
 *
 *  Created on: 19/10/2015
 *      Author: mike
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef MINLIBC_TEST_FIXTURE_H_
#define MINLIBC_TEST_FIXTURE_H_

#define BUFFER_SIZE             1024

extern "C" int _write(int file, char *buf, unsigned int count);
extern "C" int _read(int file, char *buf, unsigned int count);
extern "C" long int _ftell(int file);
extern "C" int _lseek(int file, int offset, int whence);
extern "C" int _open(const char *name, int flags, int mode);
extern "C" int _close(int file);
extern "C" int _unlink(char *name);
extern "C" int _rename(const char *oldname, const char *newname);
//extern "C" int mkdir(const char *pathname, mode_t mode);
extern "C" int _fsync(int file);


extern "C" void reset_fixture();
extern "C" char* get_buffer();

#endif /* MINLIBC_TEST_FIXTURE_H_ */
