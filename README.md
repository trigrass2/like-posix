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

Example of use:

``` make 

# these at least would be needed in the makefile
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
USE_DRIVER_SDCARD_SDIO = 1
USE_DRIVER_FAT_FILESYSTEM = 1

```

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
#define ECHO_CONNECTIONS                5		// can serve 5 echo clients at a time

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
	net_config(&netconf, "/etc/network/resolv", "/etc/network/interface");
	net_init(&netconf);
	
	while(!wait_for_address(&netconf));

	// starts the server, a new instance of echo_thread will run for every client connected.
	start_threaded_server(echo, 
	                      configfile, 
	                      echo_thread, 
	                      "echo", 
	                      NULL, 
	                      ECHO_TASK_STACK_SIZE, 
	                      ECHO_TASK_PRIORITY, 
	                      ECHO_CONNECTIONS);
}

```
