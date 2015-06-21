
<h1>Appleseed</h1>

<h2>introduction</h2>

Appleseed is an application framework for devices based on STM32Fx microcontrollers.

It provides a set of tools and services and for growing powerful applications on small targets.

Application development is made easy, in posix style, on platforms that are too low on memory to run Linux.

Includes support for multi tasking (freertos), networking (lwip), filesystem (chans fat filesystem), posix system calls (like-posix), graphical/touch lcd, audio streaming and many other things.

Each folder in this repository holds a module or a category of modules. These are described below....

<h3>Including appleseed in a project</h3>

This repository may sit anywhere releative to a particular project, but a nice way to do it is with git submodule.

Assume that the project is called "myproject", the following step is required once only, to add the submodule.
```bash
## things to do once
# get a copy of myproject
git clone url-of-myproject-repo
# add appleseed as a submodule
git submodule add https://github.com/drmetal/applseed.git
# initialise and update
git submodule init
git submodule update
# commit and push the new submodule
git commit .gitmodules applseed -m "adding appleseed"
git push

# from now on pulls on the myproject repo may be done to include the submodukes as well
git pull --recurse-submodules

# pushes are done separately, since myproject and appleed are still two different repos
git commit ...
git push ...
cd appleseed
git commit ...
git push ...
```

Future clones of "myproject" can be made including  appleseed.

```bash
#this will recursively bring all submodules in
git clone --recursive url-of-myproject-repo
# pull myproject and all submodules
git pull --recurse-submodules
# pull just myproject
git pull
# pull just appleseed
cd appleseed
git pull
# commit and push as above...
git commit ...
git push ...
cd appleseed
git commit ...
git push ...
```

Once you have the code on disk, include appleseed in your project makefile, this is outlined in the next section which describes the build-env folder.


<h2>build-env</h2>

<h3>Overview</h3>

this module collects up a bunch of makefiles and scripts that can be used to build STM32 projects.

Both STM32F1 and STM32F4 are supported, as well as multiple boards. Chip and board support is implemented in 
stm32-device-support/board and stm32-device-support/device.
 
 
<h3>Options in project Makefiles</h3>

There are many options that may be specified in the project makefile. these are set to defaults in the file **setup.mk**.

All of the settings in **setup.mk** may be overridden in your project makefile, or as envirponment variables, or from the command line.

Project configuration makes up the first section of the project makefile. **setup.mk** must be included at the end of you Makefile,
to enable building with appleseed.

Here is an example:

``` make

#******************************************************************************
# Sample project makefile
#******************************************************************************

## this file lives where your project source is.

# to set options, define them here, set to an appriopriate value 

# set board if not specified on the command line
BOARD ?= HY-STM32_100P

# set project if not specified on the command line
PROJECT_NAME ?= demo-project

# specify project source
SOURCE = main.c

# the following are modules to enable - see setup.mk for a complete list of options.
USE_MINSTDLIBS = 1
USE_LOGGER = 1
USE_POSIX_STYLE_IO = 1
USE_DRIVER_LEDS = 1
USE_DRIVER_USART = 1
USE_DRIVER_SDCARD = 1
USE_DRIVER_FAT_FILESYSTEM = 1

## be sure to include the other makefiles
include appleseed/build-env/setup.mk
```

<h3>build options</h3>

 - make clean (remove all build artifacts)
 - make all (default build, no debug, optimization set to 2)
 - make small (default build but with optimization for size)
 - make release (no debug, optimization set to 2)
 - make debug (debug enabled, no optimization)

to rebuild completely:

 - make clean release
 - make clean debug

```bash
#example with build customization via command line parameters 
make clean release PROJECT_NAME="demo-project" BOARD="HY-STM32_100P"
```

<h2>stm32-device-support</h2>

A collection of code that supports building for different stm32fxxx devices.

Includes chip and board configurations, and hardware drivers.

chips supported: STM32F103VE, STM32F107RC, STM32F407VE, STM32F407VG

boards supported: HY-STM32_100P, stm32f4_discovery, uemb1, uemb4


<h3>build options</h3>

 - DEVICE_SUPPORT_DIR
	 - the path to the stm32-device-support root directory, relative to the project makefile
 - BOARD
	 - the name of the board to build for - this must be one of those listed in board.mk->BOARDS
 - APP_ADDRESS_OFFSET		
 	- can be set to some other value to make the built application run from a location other than the flash origin. for example setting to 0x4000 will offset the application in flash by 16KBytes. The first 16KBytes could then be used for a bootloader or non volatile data space... defaults to **0x0000**.


<h3>Timers</h3>

Timers are used to support device drivers. It can be hard to work out if its safe to use a timer in the application. The drivers that use timers are:

|module	|file      |set in              |defaults to   |shared with|
|-------|----------|--------------------|--------------|-----------|
|systimer|systimer.c|board/<board>/systime_config.h|TIM2|lcd|
|adc_stream|adc_stream.c|<project>/adc_stream_config.h|TIM3||
|dac_stream|dac_stream.c|<project>/dac_stream_config.h|TIM6||
|i2s_stream|i2s_stream.c|<project>/i2s_stream_config.h|TIM5||
|ds1820|ds1820.c|<project>/ds1820_config.h|TIM4||
|lcd|lcd.c|<project>/board/<board>/lcd_config.h|TIM2|systimer|
|pwm|pwm.c|pwm_config.h|any left over||

<h3>About memory on the STM32 devices, with FreeRTOS</h3>

This is my ramble on memory usage on small processors.

For an overview WRT freertos, see:
http://www.freertos.org/Stacks-and-stack-overflow-checking.html

**The main stack, on the STM32F4, with FreeRTOS**

CPU/main Stack is in CCRAM.

 - the scheduler itself will utilise the main stack
 - ISR’s will use the main stack
 - nothing else utilizes the main stack

Ways to harness CCRAM in the user program:

 - Use the special memory allocation api, accessible via heap_ccram.h. this provides the functions malloc_ccram(), calloc_ccram and free_ccram()

**Memory used in tasks**

Task stacks are allocated on the freertos heap, which lives in the bss section, in main SRAM

Many tasks will require buffer space for string or signal processing or whatever. these can sometimes be optimal when made quite large.
Where the memory comes from must be carefully designed in some cases….

**static** - *text/bss*

- lives in main SRAM
- functions using the memory are not reentrant unless mutex is used
- if memory is used only in a task function, no need to consider reentrance if the task may run only as a singleton...
- using statically defined memory cannot introduce task instability.
- speed is good as no allocation code is required.
	
**task stack** - *local variables*
	
- lives in the freertos heap, allocated with the task
- normally stack memory is one way to get dynamic memory fast - low overhead, managed by hardware, etc..
- but in freertos the task stack is of fixed size, prealllocated at task startup
- so… it can overflow if local variables are used extensively, large buffers etc
- local variables are not “temporary” as they usually would be. the stack memory is "in use" all the time until the task dies. 
- The task stack has to be large enough to deal with all the local variables of all functions called, and adds up as function calls nest.
- not being dynamic, this means the task uses a lot of memory when it may not be using it.
- the task designer must scale the task stack with any changes to the local variables …
- the task does not have to be a singleton though - reentrance is achieved since its stack is duplicated for every instance of it.
	
**dynamic memory** 

- *freertos heap*	
- there is no general, unmanaged heap
- freertos claims a large memory array for its heap
- this is where tasks are created, and where malloc gets its memory from.
- All dynamic memory used by tasks (stacked and heaped) exists here
- its slow to get at… not managed by hardware, 
- can fragment, causing memory “loss”
- not easy to plan dynamic allocation for an entire project, especially when 3rd party libraries are included
- leads to leaks when not manged properly

- *CCRAM heap*
- same as a normal heap but managed via the calls malloc_ccram() and free_ccram().
- care must be taken in choosing to use it, CCRAM may not be used by the DMA controllers.
	 
**main stack** - *SRAM/CCRAM*

 - on STM32F4 the core coupled ram *CCRAM* as the stack.
 - on the STM32F1 the stack resides like normal, at the top of *SRAM*
 - this is the cpu stack - only the main function and ISR’s are able to use this stack memory under FreeRTOS.


<h3>Memory Region Choices</h3>

 - large buffers shouldnt be taken from the task stack - it bloats them too much.
 - if they are used all the time, and dont need to change in size, define them in bss. Access is global, so thread safe access should be considered.
 - if they are used not so often, or frequently change in size so that a static buffer would waste space, dynamic memory could be a good option.

<h2>autensils</h2>

audio utensils. some of the modules are hardware dependant.

<h3>Wav</h3>

wave file parsing module.

<h3>Wav Stream</h3>

streams wave file data out of the specified stream interface.

<h3>Siggen</h3>

streams a synthesised sine wave out of the specified stream interface.


<h2>cutensils</h2>

software utensils, written in c. all the software in cutensils is in pure hardware independent c.

<h3>Logger</h3>

This is a general logging module.
supporting logging:
 - to file
 - to serial port
 - to socket
 - or any other file like device
 - optionally with ansi colour
 - optionally with timestamp

<h3>Confparse</h3>

configuration file parser / writer. supports:
 - key value storage in files
 - inline / whole line comments with the '#' character
 - insertion / deletion / reading of key value pairs
 - iteration over all key value pairs.

```
# Eg, example config file

# whitespace is ok

mykey myvalue
an_integer 1234
more data # inline comment
# spaces inside keys or values not allowed
mykey my value # illegal
```

<h2>graphics</h2>

thread safe lcd graphics library that works on top of the lcd driver in stm32-device-support. the library provides...

 - smooth fonts
 - text
 - shapes - lines, squares, circles. etc
 - images - just bitmaps stored in rom
 - widgets - touch keys, statusbar
 
<h2>like-posix</h2>

Builds on top of FreeRTOS, and FatFs by ChaN, providing posix style system calls.


relies upon:
 - FatFs by ChaN
 - FreeRTOS
 - cutensils
 - minstdlibs
 - the file likeposix_config.h / modified for specific project


**system calls**

 * int open(const char *name, int flags, int mode)
 * int close(int file)
 * int write(int file, char *buffer, unsigned int count)
 * int read(int file, char *buffer, int count)
 * int fsync(int file)
 * int fstat(int file, struct stat *st)
 * int stat(char *file, struct stat *st)
 * int isatty(int file)
 * int lseek(int file, int offset, int whence)
 * int unlink(char *name)
 * int rename(const char *oldname, const char *newname)
 * char* getcwd(char* buffer, size_t size)
 * DIR* opendir(const char *name)
 * int closedir(DIR *dirp)
 * struct dirent* readdir(DIR *dirp)
 * int chdir(const char *path)
 * int mkdir(const char *pathname, mode_t mode)
 * int gettimeofday(struct timeval *tp, struct timezone *tzp)
 * time_t time(time_t* time)
 * unsigned int sleep(unsigned int secs)
 * int usleep(useconds_t usecs)
 
 ** termios calls **
 
 * int tcgetattr(int fildes, struct termios *termios_p)
 * int tcsetattr(int fildes, int when, struct termios *termios_p)
 * speed_t cfgetispeed(const struct termios* termios)
 * speed_t cfgetospeed(const struct termios* termios)
 * int cfsetispeed(struct termios* termios, speed_t ispeed)
 * int cfsetospeed(struct termios* termios, speed_t ospeed)
 * int tcdrain(int file)
 * int tcflow(int file, int flags)
 * int tcflush(int file, int flags)

 **lwip based socket api, when enabled***

 * int socket(int namespace, int style, int protocol);
 * int closesocket(int socket);
 * int accept(int socket, struct sockaddr *addr, socklen_t *length_ptr);
 * int connect(int socket, struct sockaddr *addr, socklen_t length);
 * int bind(int socket, struct sockaddr *addr, socklen_t length);
 * int shutdown(int socket, int how);
 * int getsockname(int socket, struct sockaddr *addr, socklen_t *length);
 * int getpeername(int socket, struct sockaddr *addr, socklen_t *length);
 * int setsockopt(int socket, int level, int optname, void *optval, socklen_t optlen);
 * int getsockopt(int socket, int level, int optname, void *optval, socklen_t *optlen);
 * int listen(int socket, int n);
 * int recv(int socket, void *buffer, size_t size, int flags);
 * int recvfrom(int socket, void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t *length);
 * int send(int socket, const void *buffer, size_t size, int flags);
 * int sendto(int socket, const void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t length);
 * int ioctlsocket(int socket, int cmd, void* argp);
 * gethostbyname (directly mapped to lwip_gethostbyname)
 * gethostbyname_r (directly mapped to lwip_gethostbyname_r)
 * freeaddrinfo  (directly mapped to lwip_freeaddrinfo)
 * getaddrinfo (directly mapped to lwip_getaddrinfo)
 
**minimal system calls**

 * void _exit(int i)
 * caddr_t _sbrk(int incr)
 * int _link(char *old, char *new)
 * int _execve(char *name, char **argv, char **env)
 * int _fork()
 * int _getpid()
 * int _kill(int pid, int sig)
 * int times(struct tm *buf)
 * int _wait(int *status)

**stdio**
 
Note, these are implemented in the minstdlibs module.

 * FILE* fopen(const char * filename, const char * mode);
 * int fclose(FILE* stream);
 * int fprintf(FILE* stream, const char * fmt, ...);
 * int fputc(int character, FILE* stream);
 * int fputs(const char* str, FILE* stream);
 * int fgetc(FILE* stream);
 * char* fgets(char* str, int num, FILE* stream);
 * long int ftell(FILE* stream);
 * int fseek(FILE * stream, long int offset, int origin);

**sdlib**

When linked with like-posix these utilize tyhe FreeRTOS memory API.

 * void* malloc(size_t num);
 * void* calloc(size_t num, size_t size);
 * void* realloc(void* old, size_t newsize);
 * void free(void* ptr);
 

<h3>Configuration</h3>

the file likeposix_config.h is required at project level, to configure like-posix.

``` c

/**
 * sample configuration for the like-posix module.
 */

#ifndef LIKEPOSIX_CONFIG_H_
#define LIKEPOSIX_CONFIG_H_

/**
 * fudge factor for the file table, presently can be any value higher than STDIN_FILENO...
 */
#define FILE_TABLE_OFFSET		10
/**
 * the maximum number of open files/devices
 */
#define FILE_TABLE_LENGTH 		32
/**
 * the maximum number of installed devices, maximum of 255
 */
#define DEVICE_TABLE_LENGTH 	10
/**
 * location where devices get installed to. 
 * this directory is special, dont write regular files here :)
 * it is reserved for devices.
 */
#define DEVICE_INTERFACE_DIRECTORY 	"/dev/"
/**
 * this is a hack that adds an ofset in seconds onto the time returned by time/gettimeofday.
 * corrects time set by NTP for your timezone. 12 hours for NZT
 */
#define TIMEZONE_OFFSET (12 * 60 * 60)
/**
 * enable full integration of lwip sockets in likeposix
 */
#define ENABLE_LIKEPOSIX_SOCKETS    1

#endif /* LIKEPOSIX_CONFIG_H_ */

```

<h3>Base Filesystem</h3>

Many projects are better when based on a structured filessystem.
like-posix provides a filesytem template that can be installed on an SD card.
It is loosely structured, as follows:

 - /dev
    - this directory is generated by the system.
 	- devices such as serial and USB ports installed here, IO may be performed on them just like normal files.
 	- for serial devices the file naming convention will be "ttySx", starting at 0
 - /var/log
 	- log files are stored here, syslog, bootlog and errorlog for example
 - /var/lib/httpd
 	- default location for files served by an http server
 - /etc/network
 	- network configuration files live here: interface, ntp, resolv...
 - /etc/logging
 	- logging configuration files live here: logging.conf
 - /etc/shell
 	- shell server configuration files live here: shelld_config
 - /etc/echo
 	- echo server configuration files live here: echod_config
 - /etc/http
 	- http server configuration files live here: httpd_config
 - /rom
    - firmware images for use by the sdcard bootloader live here


<h2>minstdlibs</h2>

tiny implementations of c standard library functions.

The code is targeted for use on small embedded targets where memory is limited.

<h3>Min stdio</h3>

the minimal standard io code has some dependancies - the functions _read, _write, _lseek, _ftell 
must be defined somewhere in your project.

they are typically defined in syscalls.c... just #include <stdio.h> like normal wherever you want to use stdio.

``` c
// examples of complete implementations targeting FreeRTOS and FatFs by ChaN 
// may be found in syscalls.c in the freertos module

// example of _write, function to write a characters to a device or file
int _write(int file, char *buffer, unsigned int count)
{
	int n = EOF;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{
		for(n = 0; n < (int)count; n++) {
			phy_putc(*buffer++); // some code here to write a serial device, or memory, or whatever
		}
	}
	return n;
}
// example of _read, function to read a characters from a device or file
int _read(int file, char *buffer, unsigned int count)
{
	int n = EOF;

	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{
		for(n = 0; n < (int)count; n++) {
			phy_getc(*buffer++); // some code here to read a serial device, or memory, or whatever
		}
	}

	return n;
}
// example of _lseek, function to change the position of the file pointer
extern int _lseek(int file, int offset, int whence)
{
	return -1;
}
// example of _ftell, function to read the position of the file pointer
extern long int _ftell(int fd)
{
	return -1;
}

```

supported string formatting:



| % format	| operation |
| ----------|-----------|
| %		| double % prints % symbol |
| c		| print character |
| s		| print null terminated string |
| i		| print signed int |
| d		| print signed int |
| u		| print unsigned int |
| x		| print unsigned int, in lower case hexadecimal format |
| X		| print unsigned int, in upper case hexadecimal format |
| p		| print pointer address, in lower case hexadecimal format, preceded by '0x' |
| f		| print floating point value |

|modifier|operation|
|------------|---------|
| 0, space	| left padding with space and 0 characters are supported by i, u, d, x, X, p |
| #     	| a '0x' precedes hexadecimal formatted numbers when # is inserted. supported by x, X |
| +			| a + preceeds numeric formatted numbers when + is inserted, and the number is > 0. supported by i, u, d, f: |
| l,h		| ignored. normally specify width of the data type as long or short respectively|
| -         | ignored.

**Examples**
``` c
// l, h modifiers ignored....
printf("ignore lh: %llu %lld %lu %ld %hu %hd", 1234, 1234, 1234, 1234, 1234, 1234);
prints: 
    `ignore lh: 1234 1234 1234 1234 1234 1234`
```
``` c
// zero padding
printf("padded integer: %06d", 123);
prints: 
    `padded integer: 000123`
```
``` c
// space padding
printf("padded integer: % 6d\n", 123); 
printf("padded integer: % 6d", 4567); 
printf("padded integer: %6d\n", 123); 
printf("padded integer: %6d", 4567); 
prints: 
    padded integer:    123 
    padded integer:   4567 
    padded integer:    123 
    padded integer:   4567 
```
``` c
// hex with 0x
printf("modified hex: %#x", 1234); 
printf("modified hex: %#X", 1234); 
prints: 
    `modified hex: 0x4d2`
    `modified hex: 0x4D2`
```
``` c
// padded hex
printf("modified hex: %#06x", 1234); 
printf("modified hex: %#06", 1); 
prints: 
    `modified hex: 0x0004d2`
    `modified hex: 0x000001`
```


<h3>Testing</h3>

Setting up gtest

```
# acquire, build and install gtest
wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
unzip gtest-1.7.0.zip
cd gtest-1.7.0
./configure
make
sudo cp -a include/gtest /usr/include
sudo cp -a lib/.libs/* /usr/lib/
sudo ldconfig -v | grep gtest
# check that the gtest so's are in place
libgtest.so.0 -> libgtest.so.0.0.0
libgtest_main.so.0 -> libgtest_main.so.0.0.0
# optionally remove gtest sources
cd ..
rm -rf gtest-1.7.0

```

Building and runnning the tests

```
cd minstdlibs/test
make clean all run
```
<h2>nutensils</h2>

network oriented utilities.

relies upon:

 - the cutensils module

<h3>Socket</h3>

this is a collection of socket utilities that can simplify socket programming at application level.

<h3>HTTP</h3>

this is a collection of HTTP protocol utilities. These build upon the nutensils socket module.


<h3>Threaded Server</h3>

this is small multithreaded addition to the socket server given in the socket module.

Example of use, an echo server:

These at least would be needed in the makefile:

``` make 

USE_MINSTDLIBS = 1
USE_CONFPARSE = 1
USE_LOGGER = 1
USE_POSIX_STYLE_IO = 1
USE_SOCK_UTILS = 1
USE_HTTP_UTILS = 1
USE_THREADED_SERVER = 1
USE_DRIVER_LEDS = 1
USE_DRIVER_USART = 1
USE_DRIVER_LWIP_NET = 1
USE_DRIVER_SDCARD = 1
USE_DRIVER_FAT_FILESYSTEM = 1

```

A config file on target, located in /etc/echo/echod_config, should be set up with the following entries:

```

# the TCP port to listen on
port 7
# the maximum umber of concurrent connections
conns 1

```

Some source code to do an echo server:

``` c

#include <sys/socket.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sdfs.h"
#include "net.h"
#include "threaded_server.h"

#define ECHO_BUFFER_SIZE                16
#define ECHO_TASK_STACK_SIZE            64	// increase if you get a stack overflow at runtime...
#define ECHO_TASK_PRIORITY              1

void start_echo(sock_server_t* echo, char* configfile);

/**
 * this function is called in a thread spawned by the server.
 * most socket operations may be performed, except close() and closesocket()
 * most rtos calls may be made, except vTaskDelete()
 * to exit the task and close the socket, allow the loop to terminate.
 */
void echo_thread(sock_conn_t* conn)
{
	int n;
	char buffer[ECHO_BUFFER_SIZE];
	
	// loop while the connection is to remain open.
	for(;;)
	{
		// conn->connfd is the file descriptor of the socket to use...
		// if recv returns 0 or less exit - the client has closed the connection.
		n = recv(conn->connfd, buffer, sizeof(buffer), 0);
		if(n <= 0)
			break;
	
		// if send returns -1 or less exit - the client has closed the connection.
		if(send(conn->connfd, buffer, n, 0) < 0)
			break;
	}
	
	// if we get here, the server will close the socket for you... 
	// do NOT call close(conn->connfd) or closesocket(conn->connfd)
}

void start_echo(sock_server_t* echo, char* configfile)
{

	// init filesystem
	sdfs_init();
	while(!sdfs_ready());
	    
	// init networking
	net_config(&netconf, DEFAULT_RESOLV_CONF_PATH, DEFAULT_NETIF_CONF_PATH);
	net_init(&netconf);
	
	while(!wait_for_address(&netconf));

	// starts the server, a new instance of echo_thread will run for every client connected.
	start_threaded_server(echo, 
	                      configfile, 
	                      echo_thread, 
	                      "echo", 
	                      NULL, 
	                      ECHO_TASK_STACK_SIZE, 
	                      ECHO_TASK_PRIORITY);
}

```

Use netcat to test....

``` bash

nc <ipaddress> 7

```

<h3>Shell</h3>

The shell is an extensible multi threaded server that can run commands in the form of function calls.

Example of use:


A config file on target, located in /etc/shell/shelld_config, should be set up with the following entries:

```

# the TCP port to listen on
port 22
# the maximum umber of concurrent connections
conns 5

```

Source code to make the shell work:

``` C

shell_cmd_t reboot_cmd = {
    .name = "reboot",
    .usage = "example help for custom reboot command!"
};

/**
 * @brief   reboot command - resets the MCU.
 */
int reboot_sh(int fdes, const char** args, unsigned char nargs)
{
    (void)args;
    (void)nargs;

    send(fdes, "rebooting", sizeof("rebooting")-1, 0);
    soft_reset();

    return SHELL_CMD_EXIT;
}

void init()
{
	// init filesytem and network....
	
	// register custom commands... 
	// if the command structure is empty to start with, the name and usage can be set here.
	register_command(&sh, &reboot_cmd, reboot_sh, NULL, NULL);
	
	// start shell
	start_shell(&sh, DEFAULT_SHELL_CONFIG_PATH);
}
    
```

Use netcat to test....

``` bash

nc <ipaddress> 22

```

<h3>jsmn extensions</h3>

provides a JSON object/array accessor API supporting keying/indexing and iteration.

Some examples.....

```c

void test_json_iterate_key_value_pairs()
{
    char test1[] = "{\"key1\":\"value1\",\"key2\":\"123\",\"nested\":\"hello\"}";
    jsmntok_t tokens[100];
    json_t json;
    jsmntok_t* object;


    jsmnerr_t e = json_init(&json, tokens, 100, test1, strlen(test1));

    if(e <= 0)
    {
        printf("parse error, e=%d\n", e);
        return;
    }

    // gets the outer object
    object = json_get_current_item(&json);

    // iterate keys
    for(json_reset_iterator(&json, object); json_iterator(&json);)
    {
        printf("%s:%s\n", json_string_value(&json), json_token_string_value(&json, json_iterator_get_object_value(&json)));
        
        if(json_token_value_match(&json, json_iterator_get_object_value(&json), "value1"))
            printf("matched value: %s\n", "value1");
        if(json_token_value_match(&json, json_iterator_get_object_value(&json), "123"))
            printf("matched value: %s\n", "123");
        if(json_token_value_match(&json, json_iterator_get_object_value(&json), "hello"))
            printf("matched value: %s\n", "hello");
    }
}

void test_json_object()
{
    char test1[] = "{\"key1\":\"value1\",\"key2\":\"123\",\"nested\":{\"key3\":\"value2\",\"key4\":\"999\"}}";
    jsmntok_t tokens[100];
    json_t json;
    jsmntok_t* object;


    jsmnerr_t e = json_init(&json, tokens, 100, test1, strlen(test1));

    if(e <= 0)
    {
        printf("parse error, e=%d\n", e);
        return;
    }

    // gets the outer object
    object = json_get_current_item(&json);

    // get values by key
    if(json_get_value_by_key(&json, object, "key1"))
        printf("key 'key1' value: %s\n", json_string_value(&json));
    if(json_get_value_by_key(&json, object, "key2"))
        printf("key 'key2' value: %s\n", json_string_value(&json));
    if(json_get_value_by_key(&json, object, "key3"))
        printf("key 'key3' value (shouldnt be seeing this!): %s\n", json_string_value(&json));
    if(json_get_value_by_key(&json, object, "key4"))
        printf("key 'key4' value (shouldnt be seeing this!): %s\n", json_string_value(&json));

    // iterate keys
    json_reset_iterator(&json, object);
    while(json_iterator(&json))
        printf("key string: %s\n", json_string_value(&json));

    // iterate values
    json_reset_iterator(&json, object);
    while(json_object_value_iterator(&json))
       printf("value string: %s\n", json_string_value(&json));

    // gets the nested object
    object = json_get_value_by_key(&json, tokens, "nested");

    // get values by key
    if(json_get_value_by_key(&json, object, "key1"))
        printf("key 'key1' value (shouldnt be seeing this!): %s\n", json_string_value(&json));
    if(json_get_value_by_key(&json, object, "key2"))
        printf("key 'key2' value (shouldnt be seeing this!): %s\n", json_string_value(&json));
    if(json_get_value_by_key(&json, object, "key3"))
        printf("key 'key3' value: %s\n", json_string_value(&json));
    if(json_get_value_by_key(&json, object, "key4"))
        printf("key 'key4' value: %s\n", json_string_value(&json));

    // iterate keys
    json_reset_iterator(&json, object);
    while(json_iterator(&json))
       printf("key string: %s\n", json_string_value(&json));

    // iterate values
    json_reset_iterator(&json, object);
    while(json_object_value_iterator(&json))
       printf("value string: %s\n", json_string_value(&json));
}

void test_json_object_of_arrays()
{
    jsmntok_t tokens[100];
    json_t json;
    char test1[] = "{\"key1\":[1,2,3.4], \"key2\":[99.9,458643875,123], \"key3\":[\"whats\", \"up\", \"doc\"], \"key4\":[\"XyZ\", 156.987, [1], {\"1\", 1}, 21]}}";
    jsmntok_t* object;
    jsmntok_t* array;
    int i;

    jsmnerr_t e = json_init(&json, tokens, 100, test1, strlen(test1));

    if(e <= 0)
    {
        printf("parse error, e=%d\n", e);
        return;
    }

    // gets the outer object
    object = json_get_current_item(&json);

    if(json_get_value_by_key(&json, object, "key1"))
    {
        array = json_get_current_item(&json);

        json_reset_iterator(&json, array);
        while(json_iterator(&json))
            printf("integer item value: %d\n", json_integer_value(&json));
    }
    else
        printf("object key %s invalid\n", "key1");


    if(json_get_value_by_key(&json, object, "key2"))
    {
        array = json_get_current_item(&json);

        json_reset_iterator(&json, array);
        while(json_iterator(&json))
            printf("float item value: %f\n", json_float_value(&json));
    }
    else
        printf("object key %s invalid\n", "key2");

    if(json_get_value_by_key(&json, object, "key3"))
    {
        array = json_get_current_item(&json);

        json_reset_iterator(&json, array);
        while(json_iterator(&json))
            printf("string item value: %s\n", json_string_value(&json));
    }
    else
        printf("object key %s invalid\n", "key3");

    // test array access by index
    array = json_get_value_by_key(&json, object, "key4");
    // intentionally overrun array bounds
    for(i = -2; i < 6; i++)
    {
        if(json_get_item_by_index(&json, array, i))
        {
            printf("index %d value: %d, %f, %s\n",
                    i,
                    json_integer_value(&json),
                    json_float_value(&json),
                    json_string_value(&json));
        }
        else
            printf("array index %d invalid\n", i);
    }
}

```

<h2>Third Party</h2>

There are several third party libraries used by appleseed.

<h3>LwIP</h3>
obtained from:

http://savannah.nongnu.org/projects/lwip/

currently at V1.4.1


<h3>FatFs</h3>

obtained from: http://elm-chan.org/fsw/ff/00index_e.html

 - appleseed adds a diskio interface for stm32 microcontrollers.
 - appleseed additions rely upon the cutensils module 
 - appleseed additions rely upon the stm32-device-support module, for sdcard.h and sdcard driver implementation(s)
 
currently at V0.11

<h3>FreeRTOS</h3>

obtained from: 

http://www.freertos.org

currently at V8.0.0:rc2

<h3>JSMN</h3>

obtained from:

bitbucket.org/zserge/jsmn

Note: can be used in conjunction with the appleseed JSON API  **jsmn_extensions**.



