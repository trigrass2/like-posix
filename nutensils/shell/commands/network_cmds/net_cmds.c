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


#include "net_cmds.h"

#include "minstdlib.h"
#include <libgen.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include "shell.h"

#include "lwip/inet.h"
#include "net.h"
#include "http_client.h"


#define HTTP_STATUS_ERROR               "http status: %d"SHELL_NEWLINE
#define GET_ERROR                       "http get failed"SHELL_NEWLINE
#define URL_ERROR                       "url not specified"SHELL_NEWLINE
#define MEMORY_ERROR                    "error allocating memory for command"SHELL_NEWLINE
#define NETSTAT_HEADER                  "Proto\tLocal Address\t\tForeign Address\t\tState"SHELL_NEWLINE


void install_net_cmds(shellserver_t* sh)
{
    register_command(sh, &sh_netstat_cmd, NULL, NULL, NULL);
    register_command(sh, &sh_ifconfig_cmd, NULL, NULL, NULL);
    register_command(sh, &sh_wget_cmd, NULL, NULL, NULL);
}

int sh_wget(int fdes, const char** args, unsigned char nargs)
{
    logger_t log;
    char* buffer = malloc(128);
    char* url = (char*)arg_by_index(0, args, nargs);
    char* file;
    http_response_t resp;
    int status;

    log_init(&log, "sh-wget");

    if(buffer)
    {
        if(url)
        {
            file = basename(url);

            if(file && file)
            {
                unlink(file);

                if(!http_get_file(url, &resp, file, buffer, 128))
                    write(fdes, GET_ERROR, sizeof(GET_ERROR)-1);
                status = resp.status;

                // fail if we didnt get the 200 OK. using the buffer here may overwrite the http header info.
                if(status != 200)
                {
                    sprintf(buffer, HTTP_STATUS_ERROR, status);
                    write(fdes, buffer, strlen(buffer));
                }
            }
        }
        else
            write(fdes, URL_ERROR, sizeof(URL_ERROR)-1);

        free(buffer);
    }
    else
        write(fdes, MEMORY_ERROR, sizeof(MEMORY_ERROR)-1);

    return SHELL_CMD_EXIT;
}

int sh_netstat(int fdes, const char** args, unsigned char nargs)
{
    (void)args;
    (void)nargs;
    char buffer[32];
    int i;
    int length;
    const char* ip;
    lwip_sock_stat_t stats[MEMP_NUM_NETCONN];
    lwip_sock_stat(stats, MEMP_NUM_NETCONN);

    write(fdes, NETSTAT_HEADER, sizeof(NETSTAT_HEADER)-1);

    for(i = 0; i < MEMP_NUM_NETCONN-1; i++)
    {

        ip = (const char*)&(stats[i].lip.addr);
        if(stats[i].lip.addr)
            length = sprintf(buffer, "%s\t%d.%d.%d.%d:%d\t", stats[i].type, ip[0],ip[1],ip[2],ip[3], stats[i].lport);
        else
            length = sprintf(buffer, "%s\t%d.%d.%d.%d:%d\t\t", stats[i].type, ip[0],ip[1],ip[2],ip[3], stats[i].lport);
        write(fdes, buffer, length);

        ip = (const char*)&(stats[i].rip.addr);
        if(stats[i].rip.addr)
            length = sprintf(buffer, "%d.%d.%d.%d:%d\t%s"SHELL_NEWLINE, ip[0],ip[1],ip[2],ip[3], stats[i].rport, stats[i].state);
        else
            length = sprintf(buffer, "%d.%d.%d.%d:%d\t\t%s"SHELL_NEWLINE, ip[0],ip[1],ip[2],ip[3], stats[i].rport, stats[i].state);

        write(fdes, buffer, length);
    }

    return SHELL_CMD_EXIT;
}

int sh_ifconfig(int fdes, const char** args, unsigned char nargs)
{
    (void)args;
    (void)nargs;
    int length;
    char* buffer = malloc(1024);

    const char* mac = (const char*)net_hwaddr();
    ip_addr_t ip_addr = net_ipaddr();
    ip_addr_t gw_addr = net_gwaddr();
    ip_addr_t nm_addr = net_netmask();

    const char* ip = (const char*)&ip_addr.addr;
    const char* gw = (const char*)&gw_addr.addr;
    const char* nm = (const char*)&nm_addr.addr;

    if(buffer)
    {
        length = snprintf(buffer, 1023,
"eth0\tLink encap:Ethernet speed:%u HWaddr %02x:%02x:%02x:%02x:%02x:%02x" SHELL_NEWLINE
"\tinet addr: %d.%d.%d.%d Bcast: %d.%d.%d.%d Mask: %d.%d.%d.%d" SHELL_NEWLINE
"\tGWaddr: %d.%d.%d.%d UP:%d DUPLEX:%d MTU:%d" SHELL_NEWLINE
"\tRX packets:%u errors:%u dropped:%u" SHELL_NEWLINE
"\tTX packets:%u",
                eth_link_speed(),
                mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],
                ip[0],ip[1],ip[2],ip[3],
                0,0,0,0,
                nm[0],nm[1],nm[2],nm[3],
                gw[0],gw[1],gw[2],gw[3],
                eth_link_status(),
                eth_link_full_duplex(),
                net_mtu(),
                net_ip_packets_received(),
                net_ip_errors(),
                net_ip_packets_dropped(),
                net_ip_packets_sent());
        write(fdes, buffer, length);

        free(buffer);
    }
    else
        write(fdes, MEMORY_ERROR, sizeof(MEMORY_ERROR)-1);

    return SHELL_CMD_EXIT;
}

shell_cmd_t sh_netstat_cmd = {
		.name = "netstat",
		.usage = "prints network connection info",
		.cmdfunc = sh_netstat
};

shell_cmd_t sh_ifconfig_cmd = {
        .name = "ifconfig",
        .usage = "prints network interface info",
        .cmdfunc = sh_ifconfig
};

shell_cmd_t sh_wget_cmd = {
        .name = "wget",
        .usage = "very basic wget implementation. saves the url endpoint to the cwd."SHELL_NEWLINE \
        "wget [url]",
        .cmdfunc = sh_wget
};


