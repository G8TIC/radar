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
static xtimer_t beast_retry;


/*
 * chgconstate() - change connection state with optional debugging
 */
static void chgconstate(enum beaststate newstate)
{
        if (debug > 1)
                printf("chgconstate(): %d -> %d\n", constate, newstate);
        constate = newstate;
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
                        return 1;
                } else {
                        ++telemetry.connect_fail;
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
        
        chgconstate(BEAST_TCP_DISCONNECTED);
}


/*
 * beast_close() - shutdown the BEAST
 */
void beast_tcp_close(void)
{
        if (fd) {
                int rc = close(fd);

                if (rc < 0) {
                        qerror("beast_close(): error calling close(): %s (%d)\n", strerror(errno), errno);
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
                        if (connect_socket()) {
                                /* connection success */
                                chgconstate(BEAST_TCP_CONNECTED);
                                xtimer_stop(&beast_retry);
                        } else {
                                /* connect failed */
                                close(fd);
                                xtimer_start(&beast_retry, BEAST_RETRY);
                                chgconstate(BEAST_TCP_RETRY_WAIT);
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
                                        close(fd);
                                        xtimer_start(&beast_retry, BEAST_RETRY);
                                        chgconstate(BEAST_TCP_RETRY_WAIT);
                        	        ++telemetry.socket_error;
                                        
                                } else if (rc == 0) {
                                        /* timeout no data*/
                                        ; 
                                } else {
                                        /* data available or connection closed - do a read */
                                        size = read(fd, buf, sizeof(buf));

                                        if (size > 0) {
                                                /* we have some data - call beast common input handler to decode */
                                                beast_process_input(buf, size);
                                                
                                        } else {
                                                /* size is zero -> connection closed */
                                                close(fd);
                                                xtimer_start(&beast_retry, BEAST_RETRY);
                                                chgconstate(BEAST_TCP_RETRY_WAIT);
                                                ++telemetry.disconnect;
                                        }                        
                                }
                        }
                        break;
                        
                case BEAST_TCP_RETRY_WAIT:

                        usleep(1000000);		/* wait 100mS before polling timer */
                        
                        if (xtimer_expired(&beast_retry)) {
                                /* when the timer expires try to connect again */
                                chgconstate(BEAST_TCP_DISCONNECTED);
                        }
                        break;
        }
}

