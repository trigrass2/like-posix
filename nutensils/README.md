nutensils
=========

network oriented utilities.

relies upon:

 - the cutensils project, https://github.com/drmetal/cutensils.

Socket
------

this is a collection of socket utilities that can simplify socket programming at application level.

HTTP
----

this is a collection of HTTP protocol utilities. These build upon the nutensils socket module.


Threaded Server
---------------

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

Shell
-----

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
