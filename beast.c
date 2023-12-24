/*
 * beast.c -- Implement the ADS-B BEAST protocol
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
static int state;
static int fd;
static char hostname[HOSTNAME_LEN+1];
static struct sockaddr_in saddr;
static xtimer_t beast_retry;
static uint16_t pps;


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
 * chgstate() - change protocol state with optional debugging
 */
static void chgstate(int new)
{
        if (debug > 2)
                printf("chgstate(): %d -> %d\n", state, new);
        state = new;
}


/*
 * process_frame() - process a decoded (de-escaped) BEAST frame
 */
static void process_frame(uint8_t *bp, int size)
{
        if (bp[0] >= 0x31 && bp[0] <= 0x33) {
                radar_process(&bp[1], bp[7], &bp[8], size-8);
                ++pps;
        }
}


/*
 * process_input() - process a hunk of BEAST protocol input from the TCP connection
 */
static void process_input(uint8_t *bp, int size)
{
        static uint8_t buf[BEAST_MAX_FRAME];
        static uint8_t *op = buf;
        uint8_t b;
        int sz;

        //printf("process_input(): size: %d\n", size);

        ++telemetry.socket_reads;
        telemetry.bytes_read += size;

        /*
         * process a hunk of data from the Beast TCP connection and decode and 
         * pass frames up to process_frame()
         */
         while (size--) {
                b = *bp++;

                sz = op - buf;

                if (sz > sizeof(buf)) {
                        op = buf;
                        chgstate(0);
                }

                // printf("process_input(): b=%02X sz=%d state=%d\n", b, sz, state);
                
                switch (state) {
                        case 0:							/* wait for first instance of Escape */
                                if (b == BEAST_ESC) {
                                        chgstate(1);
                                        op = buf;
                                } else {
                                        ;
                                }
                                break;
                        
                        case 1:							/* look for start of frame */
                                if (b >= 0x31 && b <= 0x33) {
                                        *op++ = b;
                                        chgstate(2);				/* start of frame */
                                } else {
                                        chgstate(0);				/* all other chars including Escape */
                                }
                                break;
                                
                        case 2:							/* inside frame */
                                if (b == BEAST_ESC) {
                                        chgstate(3);				/* seen an Escape inside the frame */
                                } else {
                                        *op++ = b;				/* copy bytes */
                                }
                                break;
                                
                        case 3:
                                if (b == BEAST_ESC) {				/* Escaped, Escape or end of frame ? */
                                        *op++ = BEAST_ESC;
                                        chgstate(2);
                                } else {
                                        if (sz) {
                                                process_frame(buf, sz);		/* process frame */
                                                op = buf;
                                                ++telemetry.frames_good;

                                                if (b >= 0x31 && b <= 0x33) {
                                                        *op++ = b;
                                                        chgstate(2);		/* next frame */
                                                } else {
                                                        chgstate(1);
                                                }
                                        } else {
                                                chgstate(0);			/* error reset */
                                                ++telemetry.frames_bad;
                                        }
                                }
                                break;
                }                
        }
}


/*
 * connect_socket() - attempt to make a connection - factorised code
 */
static int connect_socket(void)
{
        struct hostent *hostinfo;

        fd = socket(AF_INET, SOCK_STREAM, 0);

        if (fd < 0)
                qerror("connect_socket(): Could not create socket\n");

        memset(&saddr, 0, sizeof(saddr)); 
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(BEAST_PORT);

        hostinfo = gethostbyname(hostname);

        if (hostinfo != NULL) {
        
                memcpy(&saddr.sin_addr.s_addr, hostinfo->h_addr, hostinfo->h_length);

                if (connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)) >= 0) {
                        ++telemetry.connect_success;
                        return 1;
                } else {
                        return 0;
                }
        }
        
        return 0;
}


/*
 * beast_init() - initialise the BEAST
 */
int beast_init(char *addr)
{
        int rc;

        memset(&stats, 0, sizeof(stats_t));
        chgconstate(0);
        chgstate(0);
        pps = 0;

        strncpy(hostname, addr, HOSTNAME_LEN);
        
        rc = connect_socket();
        
        if (rc) {
                /* connection success */
                chgconstate(BEAST_STATE_CONNECTED);
                return 1;
        } else {
                return 0;
        }        
}


/*
 * beast_close() - shutdown the BEAST
 */
void beast_close(void)
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
 * beast_run() - run the BEAST protocol connection
 */
void beast_run(void)
{
        fd_set set;
        struct timeval timeout;
        static uint8_t buf[BEAST_MAX_READ];
        int size, rc;

        switch (constate) {

                case BEAST_STATE_DISCONNECTED:
                        if (connect_socket()) {
                                /* connection success */
                                chgconstate(BEAST_STATE_CONNECTED);
                                xtimer_stop(&beast_retry);
                                ++telemetry.connect_success;
                        } else {
                                /* connect failed */
                                //printf("beast_run(): Connect failed: %s (%d)\n", strerror(errno), errno);

                                close(fd);
                                xtimer_start(&beast_retry, BEAST_RETRY);
                                chgconstate(BEAST_STATE_RETRY_WAIT);
                                ++telemetry.connect_fail;
                        }
                        break;
                        
                case BEAST_STATE_CONNECTED:
                        {
                                FD_ZERO(&set);
                                FD_SET(fd, &set);

                                timeout.tv_sec = 0;
                                timeout.tv_usec = 10000;		/* 10mS */
                                
                                rc = select(fd+1, &set, NULL, NULL, &timeout);

                                if (rc < 0) {
                                        /* error */
                                        //perror("select");
                                        close(fd);
                                        xtimer_start(&beast_retry, BEAST_RETRY);
                                        chgconstate(BEAST_STATE_RETRY_WAIT);
                        	        ++telemetry.socket_error;
                                        
                                } else if (rc == 0) {
                                        /* timeout */
                                        ; 
                                } else {
                                        /* data available or connection closed - do a read */
                                        size = read(fd, buf, sizeof(buf));

                                        if (size > 0) {
                                                /* we have some data */
                                                process_input(buf, size);
                                                
                                        } else {
                                                /* size is zero -> connection shutdown */
                                                close(fd);
                                                xtimer_start(&beast_retry, BEAST_RETRY);
                                                chgconstate(BEAST_STATE_RETRY_WAIT);
                                                ++telemetry.disconnect;
                                        }                        
                                }
                        }
                        break;
                        
                case BEAST_STATE_RETRY_WAIT:
                        if (xtimer_expired(&beast_retry)) {
                                /* when the timer expires try to connect again */
                                chgstate(0);
                                chgconstate(BEAST_STATE_DISCONNECTED);
                        }
                        break;
        }
}


/*
 * beast_second() - housekeeping
 */
void beast_second(void)
{
        telemetry.packets_per_second = pps;
        pps = 0;
}

