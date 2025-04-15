/*
 * beast_tcp.c -- Implement the ADS-B BEAST protocol over TCP
 * Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org
 *
 * ABSTRACT
 *
 * Make a TCP/IP connection to the BEAST protocol on localhost:30005 from Dump1090
 * and it's friends and parse frames de-escaping them.
 *
 * Look for message type 0x33 which is RSSI + MLAT + Extended Squitter and pass
 * these up to radar_send() for forwarding to the aggregator.
 *
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

#include "radar.h"
#include "defs.h"
#include "beast.h"
#include "beast_tcp.h"
#include "telemetry.h"
#include "hex.h"
#include "xtimer.h"
#include "qerror.h"


/*
 * external variables
 */
extern int debug;


/*
 * local variables
 */
static enum beaststate constate;
static int fd;
static char hostname[HOSTNAME_LEN+1];
static struct sockaddr_in saddr;
//static xtimer_t beast_retry;
static int retry_counter;

/*
 * chgconstate() - change connection state with optional debugging
 */
static void chgconstate(enum beaststate newstate)
{
        if (debug)
                printf("chgconstate(): %d -> %d\n", constate, newstate);
        constate = newstate;
}


/*
 * reset_connection() - reset the TCP connection after an error
 */
static void reset_connection(void)
{
        if (fd) {
                close(fd);
                fd = 0;
        }

        if (debug)
                printf("reset_connection(): BEAST connection reset... start retry timer...\n");
        
//        xtimer_start(&beast_retry, BEAST_RETRY);

        retry_counter = BEAST_CONNECT_RETRY;

        chgconstate(BEAST_TCP_RETRY_WAIT);
}


/*
 * connect_socket() - attempt to make a TCP connection
 */
static int connect_socket(void)
{
        struct hostent *hostinfo;

        fd = socket(AF_INET, SOCK_STREAM, 0);

        if (fd < 0)
                qerror("connect_socket(): Could not create socket\n");

        memset(&saddr, 0, sizeof(saddr)); 
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(BEAST_TCP_PORT);

        hostinfo = gethostbyname(hostname);

        if (hostinfo != NULL) {
        
                memcpy(&saddr.sin_addr.s_addr, hostinfo->h_addr, hostinfo->h_length);

                if (connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)) >= 0) {
                        ++telemetry.connect_success;
                        
                        if (debug)
                                printf("connect_socket(): Connected to BEAST source\n");

                        return 1;
                } else {
                        ++telemetry.connect_fail;
                        
                        if (debug)
                                printf("connect_socket(): Connect to BEAST source FAILED: %s (%d)\n", strerror(errno), errno);
                        
                        return 0;
                }
        }
        
        return 0;
}


/*
 * beast_tcp_init() - initialise BEAST conenction over TCP
 */
void beast_tcp_init(char *addr)
{
        memset(&stats, 0, sizeof(stats_t));
        chgconstate(0);

        beast_init();

        strncpy(hostname, addr, HOSTNAME_LEN);

        fd = 0;
        
        chgconstate(BEAST_TCP_DISCONNECTED);
}


/*
 * beast_tcp_close() - shutdown the BEAST connection
 */
void beast_tcp_close(void)
{
        if (fd) {
                int rc = close(fd);

                if (debug && rc < 0) {
                        printf("beast_tcp_close(): error calling close(): %s (%d)\n", strerror(errno), errno);
                }
                
                fd = 0;
        }                
}


/*
 * beast_tcp_run() - run the BEAST protocol connection
 */
void beast_tcp_run(void)
{
        fd_set set;
        struct timeval timeout;
        static uint8_t buf[BEAST_MAX_READ];
        int size, rc;

        switch (constate) {

                case BEAST_TCP_DISCONNECTED:
                        /* attemtp to connect to BEAST source */
                        if (connect_socket()) {
                                /* connection success */
                                chgconstate(BEAST_TCP_CONNECTED);
                                //xtimer_stop(&beast_retry);
                        } else {
                                /* connect failed */
                                reset_connection();
                        }
                        break;

                case BEAST_TCP_CONNECTED:
                        {
                                FD_ZERO(&set);
                                FD_SET(fd, &set);

                                timeout.tv_sec = 0;
                                timeout.tv_usec = BEAST_SELECT_TIMEOUT;
                                
                                rc = select(fd+1, &set, NULL, NULL, &timeout);

                                if (rc < 0) {
                                        /* error on connection */
                                        reset_connection();
                        	        ++telemetry.socket_error;
                                        
                                } else if (rc == 0) {
                                        /* timeout no data - nothing to do here */
                                        ; 
                                } else {
                                        /* data available or connection closed - do a read to find out which */
                                        size = read(fd, buf, sizeof(buf));

                                        if (size > 0) {
                                                /* we have data - call beast common input handler to decode */
                                                beast_process_input(buf, size);
                                                ++telemetry.socket_reads;
                                                
                                        } else if (size == 0) {
                                                /* size is zero -> EOF -> connection closed by peer */
                                                reset_connection();
                                                ++telemetry.disconnect;
                                        } else {
                                                /* size is negative -> error on socket */
                                                reset_connection();
                                                ++telemetry.socket_error;
                                        }
                                }
                        }
                        break;

#if 0
                case BEAST_TCP_RETRY_WAIT:
                        if (xtimer_expired(&beast_retry)) {
                                /* when the timer expires try to connect again */
                                chgconstate(BEAST_TCP_DISCONNECTED);
                        } else {
                                /* wait 100mS before polling timer again (stop loadave being silly) */
                                usleep(100000);
                        }  
                        break;
#endif

                case BEAST_TCP_RETRY_WAIT:
                        break;


        }
}


/*
 * beast_tcp_second() - house keeping
 */
void beast_tcp_second(void)
{
        /* if we're waiting to reconnect change state when the countdown expires */
        if (constate == BEAST_TCP_RETRY_WAIT) {
        
                if (retry_counter) {
                        --retry_counter;

                        if (!retry_counter) {
        
                                if (debug)
                                        printf("beast_tcp_second(): change state to allow re-connect\n");
                                        
                                chgconstate(BEAST_TCP_DISCONNECTED);
                        }
                }
        }
}


