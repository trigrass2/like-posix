stm32-build-env
===============

Overview
--------

this project compiles a bunch of makefiles and scripts that can be used to build STM32 projects.

Both STM32F1 and STM32F4 are supported, as well as multiple boards. Chip and board support is implemented in 
stm32-device-support/board and stm32-device-support/device.

Requires
--------

to use, clone the following projects into a new directory:

 - https://github.com/drmetal/stm32-build-env
 - https://github.com/drmetal/stm32-device-support
 - https://github.com/drmetal/like-posix
 - https://github.com/drmetal/freertos
 - https://github.com/drmetal/minstdlibs
 - https://github.com/drmetal/cutensils
 - https://github.com/drmetal/nutensils
 - https://github.com/drmetal/LwIP
 - https://github.com/drmetal/FatFs
 
 
Demos
-----

**Led Blinker**

https://github.com/drmetal/led-blink-demo.git

**Posix File IO**

https://github.com/drmetal/file-io-demo.git

**Posix Sockets**

https://github.com/drmetal/socket-demo.git


Options in project Makefiles
----------------------------

There are a few options that may be specified in the project makefile. 

build options are:

 - make clean (remove all build artifacts)
 - make all (default build)
 - make release (no debug)
 - make debug (debug enabled)

to rebuild completely:

 - make clean release
 - make clean debug
 
to change settings from the command line, Eg:

 - make clean release PROJECT_NAME="demo-project" BOARD="HY-STM32_100P"

Here is an example of a makefile

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

# the following is a random set of enabled settings - see setup.mk for a complete list of options.
USE_MINSTDLIBS = 1
USE_LOGGER = 1
USE_POSIX_STYLE_IO = 1
USE_DRIVER_LEDS = 1
USE_DRIVER_USART = 1
USE_DRIVER_SDCARD_SDIO = 1
USE_DRIVER_FAT_FILESYSTEM = 1

## be sure to include the other makefiles
include ../stm32-build-env/setup.mk
```


About memory on the STM32 devices, with FreeRTOS
------------------------------------------------

This is my ramble on memory usage on small processors.

For an overview WRT freertos, see:
http://www.freertos.org/Stacks-and-stack-overflow-checking.html

**The main stack, on the STM32F4, with FreeRTOS**

I chose to place the CPU/main Stack in CCRAM... this presents a conundrum.

 - the scheduler itself will utilise the main stack
 - ISR’s will use the main stack
 - nothing else utilizes the main stack
 - CCRAM may therefore be terribly underutilised

Ways to harness CCRAM then...

 - Specify a new RAM region at link time (havent actually done this) that uses a chunk of CCRAM - we would have to use the memory “manually”. 
 - Specify large static buffers as local variables in main, passing them to tasks as they are created by the task management thread. These will remain in scope since main never returns.

**Memory used in tasks**

Task stacks are allocated on the freertos heap, which lives in the bss section, in main SRAM

Many tasks will require buffer space for string or signal processing or whatever. these can sometimes be optimal when made quite large.
Where the memory comes from must be carefully designed in some cases….

**static** - *text/bss*

- lives in main SRAM
- functions using the memory are not reentrant unless mutex is used
- if memory is used only in a task function, no need to consider reentrance if the task may run only as a singleton...
- using statically defined memory cannot introduce task instability.
- speed is good as no allocation code runs at all.
	
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
	
**dynamic memory** - *freertos heap*
	
- there is no general, unmanaged heap
- freertos claims a large memory array for its heap
- this is where tasks are created, and where malloc gets its memory from.
- All dynamic memory used by tasks (stacked and heaped) exists here
- its slow to get at… not managed by hardware, 
- can fragment, causing memory “loss”
- not easy to plan dynamic allocation for an entire project, especially when 3rd party libraries are included
- obviously leads to leaks when not manged properly
- very convenient…. sort of
	 
**main stack** - *SRAM/CCRAM*

 - on STM32F4 I chose to use the core coupled ram *CCRAM* as the stack.
 - on the STM32F1 the stack resides like normal, at the top of *SRAM*
 - this is the cpu stack - only the main function and ISR’s are able to use this stack memory under FreeRTOS.
 - this is a big chunk of RAM, but it will never be utilised fully in the current situation.
 - many applications use data structures, and static buffering. these can be stacked in main, and fed to the tasks as they start up. This is one way to use this RAM without too much hard thinking.


Memory Use Strategies
---------------------

 - large buffers cant be taken from the task stack - it just bloats the task stacks way too much.
 - if they are used all the time, and really dont need to change in size, just define in bss and optionally protect access using a mutex.
 - if they are used not so often, or frequently change in size so that a static buffer would waste space, dynamic memory could be a good option.



