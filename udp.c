/*
 * udp.c -- Routines for sending UDP and managing protocol errors
 * Author: Mike Tubby G8TIC mike@tubby.org  14th April 2025
 *
 * ABSTRACT
 *
 * UDP routines moved to a separate module with it's own state-machine to provide
 * a management layer because the previous code was brittle and had no error recovery.
 *
 * We now run a state-machine that manages DNS look-ups and error recovery.
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <linux/socket.h>
#include <linux/ip.h>


#include "defs.h"
#include "udp.h"
#include "hex.h"
#include "stats.h"


/*
 * external variables
 */
extern int debug;

/*
 * local variables
 */
static struct hostent *hostinfo;
static struct sockaddr_in src, dst;
static enum udpstate state;
static int fd;
static char hostname[HOSTNAME_LEN+1];
static int qos;
static int retry = 0;
static int rebind_interval = 0;
static int rebind = 0;

/*
 * chgstate() - change state with optional debugging
 */
static void chgstate(enum udpstate newstate)
{
        if (debug)
                printf("udp chgstate(): %d -> %d\n", state, newstate);
                
        state = newstate;
}


/*
 * reset_connection() - reset the UDP connection after an error
 */
static void reset_connection(void)
{
        if (fd) {
                close(fd);
                fd = 0;
        }

        if (debug)
                printf("reset_connection(): start retry timer...\n");

        retry = UDP_RETRY;

        chgstate(UDP_RETRY_WAIT);
}


/*
 * host_lookup() - perform DNS lookup on destination hostname
 */
static int host_lookup(void)
{
        hostinfo = gethostbyname(hostname);
    
        if (hostinfo) {
                if (debug)
                        printf("host_lookup(): Destination is %s (%s) port %u\n", hostinfo->h_name, inet_ntoa(*(struct in_addr*)hostinfo->h_addr), UDP_PORT);
                return 1;
        
        } else {
                if (debug)
                        printf("host_lookup(): error resolving hostname: %s  errno: %s (%d)\n", hostname, hstrerror(h_errno), h_errno);
        }
        
        return 0;
}


/*
 * make_socket() - make a UDP socket for sending data
 */
static int make_socket(void)
{
        int rc;

        /* make a UDP socket */
        if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        
                if (debug)
                        printf("make_socket(): socket(): %s (%d)\n", strerror(errno), errno);
                        
                return 0;
        } 

        /* if QoS is specified then set it */
        if (qos) {
                const int iptos = qos << 2;		/* QoS in top 6-bits of the IP header */

                if ((rc = setsockopt(fd, IPPROTO_IP, IP_TOS, &iptos, sizeof(iptos)) < 0)) {
        
                        if (debug)
                                printf("make_socket(): setsockopt(): %s (%d)\n", strerror(errno), errno);
                                
                        return 0;
                }
        }

        /* setup my end (source) */ 
        memset(&dst, 0, sizeof(src));
        src.sin_family = AF_INET;
        src.sin_addr.s_addr = htonl(INADDR_ANY);
        src.sin_port = 0;

        /* setup their end (destination) */
        memset(&dst, 0, sizeof(dst));
        dst.sin_family = AF_INET;
        dst.sin_addr = *(struct in_addr*) hostinfo->h_addr;
        dst.sin_port = htons(UDP_PORT);

        return 1;
}


/*
 * connect_socket() - connect the socket so we can send on it
 */
static int connect_socket(void)
{
        int rc;

        /* connect the socket */
        if ((rc = connect(fd, (struct sockaddr*)&dst,sizeof(dst)) < 0)) {

                if (debug)
                        printf("connect_socket(): %s (%d)\n", strerror(errno), errno);
                        
                return 0;
        } 

        return 1;
}


/*
 * udp_send() - send a UDP/IP message to the aggregator
 */
void udp_send(void *buf, int size)
{
        if (state == UDP_CONNECTED) {
                int rc;

                rc = send(fd, buf, size, 0);

                if (rc >= 0) {
                        /* send succeeded */
                
                        /* update stats */
                        ++stats.tx_count;
                        stats.tx_bytes += size;

                        /* debug dump */
                        if (debug > 2)
                                hex_dump("UDP", buf, size);
                } else {
                        /* send failed */
                        if (debug)
                                printf("udp_send(): %s (%d)", strerror(errno), errno);
                                
                        reset_connection();
                }
        }                
}


/*
 * udp_init() - initialise the UDP sub-system
 */
void udp_init(char * host, int qs, int rb)
{
        strcpy(hostname, host);
        qos = qs;
        rebind_interval = rb;
        fd = 0;
        chgstate(UDP_IDLE);
}


/*
 * udp_second() - run the UDP finite state-machine from teh housekeeping timer
 */
void udp_second(void)
{
        switch (state) {
                case UDP_IDLE:
                        chgstate(UDP_WAIT_LOOKUP);
                        break;

                case UDP_WAIT_LOOKUP:
                        if (host_lookup())
                                chgstate(UDP_WAIT_CONNECT);
                        else
                                reset_connection();
                        break;

                case UDP_WAIT_CONNECT:
                        if (make_socket() && connect_socket()) {
                                if (rebind_interval)
                                        rebind = rebind_interval;
                                chgstate(UDP_CONNECTED);
                        } else {
                                reset_connection();
                        }
                        break;
                
                case UDP_CONNECTED:
                        if (rebind) {
                        
                                --rebind;
                        
                                if (!rebind) {

                                        close(fd);					/* close existing fd */
                                        
                                        if (make_socket() && connect_socket()) {	/* make new socket and connect it */
                                        
                                                if (rebind_interval)			/* reset interval timer */
                                                        rebind = rebind_interval;
                                        
                                                chgstate(UDP_CONNECTED);		/* un-necessary but useful in debug */
                                                
                                        } else {
                                                reset_connection();
                                        }
                                }
                        }
                        break;

                case UDP_RETRY_WAIT:
                        if (retry)
                                --retry;
                        if (!retry)
                                chgstate(UDP_IDLE);
                        break;
        }
}


/*
 * udp_close() - shutdown UDP
 */
void udp_close(void)
{
        if (fd) {
                close(fd);
                fd = 0;
        }
}


/*
 * udp_reset() - reset the UDP session
 */
void udp_reset(void)
{
        reset_connection();
}

