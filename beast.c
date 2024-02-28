/*
 * beast.c -- Implement the ADS-B BEAST protocol over TCP
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
#include "beast_serial.h"
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
static int state;
static uint16_t pps;



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
 * beast_process_input() - process a chunk of BEAST protocol input from a TCP or serial connection
 */
void beast_process_input(uint8_t *bp, int size)
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
 * beast_second() - housekeeping
 */
void beast_second(void)
{
        telemetry.packets_per_second = pps;
        pps = 0;
}


/*
 * beast_init() - initialise common Beast stuff
 */
void beast_init(void)
{
        chgstate(0);
        pps = 0;
}

