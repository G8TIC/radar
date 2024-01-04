/*
 * avr.c -- Implement the ADS-B AVR protocol
 * Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org
 *
 * ABSTRACT
 *
 * Make a TCP/IP connection to the AVR protocol on localhost:30002 from Dump1090
 * and it's friends and parse frames converting them to binary and putting them
 * on the input queue.
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
#include "avr.h"
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
static int constate;
static int state;
static int fd;
static struct sockaddr_in saddr;
static xtimer_t avr_retry;
static char hostname[HOSTNAME_LEN+1];
static uint8_t mlat[MLAT_LEN];
static uint8_t rssi;
static uint16_t pps;


/*
 * chgconstate() - change connection state with debugging
 */
static void chgconstate(enum avrstate newstate)
{
        if (debug)
                printf("chgconstate(): %d -> %d\n", constate, newstate);
        constate = newstate;
}


/*
 * chgstate() - change protocol state with debugging
 */
static void chgstate(int newstate)
{
        if (debug >= 2)
                printf("chgstate(): %d -> %d\n", state, newstate);
        state = newstate;
}


/*
 * process_frame() - process an AVH hex frame
 */
static void process_frame(char *bp)
{
        if (strlen(bp) <= AVR_MAX_FRAME) {
                uint8_t buf[AVR_MAX_DATA];
                int sz;
                
                sz = hex_parse(buf, bp);		/* parse AVR hext to binary */

                if (sz) {
                        radar_process(mlat, rssi, buf, sz);

                        ++telemetry.frames_good;
                }
                
                ++pps;
                
        } else {
                ++telemetry.frames_bad;
        }
}


/*
 * process_input() - process a hunk of AVR protocol input from the TCP connection
 */
static void process_input(uint8_t *bp, int size)
{
        static char buf[50];
        static char *op = buf;
        uint8_t b;
        int sz;

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
                        case 0:							/* wait for leading star character */
                                if (b == '*') {
                                        op = buf;
                                        chgstate(1);
                                } else {
                                        ;
                                }
                                break;
                        
                        case 1:							/* read chars */
                                if (b == ';') {
                                        *op = 0;
                                        process_frame(buf);
                                        ++telemetry.frames_good;
                                        chgstate(0);
                                } else if (sz <= 30) {
                                        *op++ = b;
                                } else {
                                        chgstate(0);
                                        ++telemetry.frames_bad;
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
        saddr.sin_port = htons(AVR_PORT);

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
 * avr_init() - initialise the AVR connection
 */
void avr_init(char *addr)
{
        memset(mlat, 0, MLAT_LEN);			/* no MLAT from AVR */
        rssi = 0xFF;					/* no RSSI from AVR */

        memset(&stats, 0, sizeof(stats_t));
        chgconstate(0);
        chgstate(0);
        pps = 0;

        strncpy(hostname, addr, HOSTNAME_LEN);
        
        chgconstate(AVR_STATE_DISCONNECTED);
}


/*
 * avr_close() - shutdown the AVR connection
 */
void avr_close(void)
{
        if (fd) {
                int rc = close(fd);

                if (rc < 0) {
                        qerror("avr_close(): error calling close(): %s (%d)\n", strerror(errno), errno);
                }
                
                fd = 0;
        }                
}


/*
 * avr_run() - run the AVR protocol connection
 */
void avr_run(void)
{
        fd_set set;
        struct timeval timeout;
        static uint8_t buf[AVR_MAX_READ];
        int size, rc;

        switch (constate) {

                case AVR_STATE_DISCONNECTED:
                        if (connect_socket()) {
                                /* connection success */
                                chgconstate(AVR_STATE_CONNECTED);
                                xtimer_stop(&avr_retry);
                        } else {
                                /* connect failed */
                                close(fd);
                                xtimer_start(&avr_retry, AVR_RETRY);
                                chgconstate(AVR_STATE_RETRY_WAIT);
                        }
                        break;
                        
                case AVR_STATE_CONNECTED:
                        {
                                FD_ZERO(&set);
                                FD_SET(fd, &set);

                                timeout.tv_sec = 0;
                                timeout.tv_usec = AVR_SELECT_TIMEOUT;
                                
                                rc = select(fd+1, &set, NULL, NULL, &timeout);

                                if (rc < 0) {
                                        /* select() error */
                                        close(fd);
                                        xtimer_start(&avr_retry, AVR_RETRY);
                                        chgconstate(AVR_STATE_RETRY_WAIT);
                        	        ++telemetry.socket_error;
                                        
                                } else if (rc == 0) {
                                        /* timeout no data */
                                        ; 
                                } else {
                                        /* data available or connection closed - do a read */
                                        size = read(fd, buf, sizeof(buf));

                                        if (size > 0) {
                                                /* we have some data */
                                                process_input(buf, size);
                                                
                                        } else {
                                                /* size is zero -> connection closed */
                                                close(fd);
                                                xtimer_start(&avr_retry, AVR_RETRY);
                                                chgconstate(AVR_STATE_RETRY_WAIT);
                                                ++telemetry.disconnect;
                                        }                        
                                }
                        }
                        break;

                case AVR_STATE_RETRY_WAIT:
                        if (xtimer_expired(&avr_retry)) {
                                /* when the timer expires try to connect again */
                                chgstate(0);
                                chgconstate(AVR_STATE_DISCONNECTED);
                        }
                        break;
        }
}


/*
 * avr_second() - house keeping
 */
void avr_second(void)
{
        telemetry.packets_per_second = pps;
        pps = 0;
}

