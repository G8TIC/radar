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
#include "telemetry.h"
#include "hex.h"
#include "qerror.h"


/*
 * external variables
 */
extern int debug;


/*
 * global variables
 */
int beast_fd;

/*
 * local variables
 */
static enum beast_mode mode = BEAST_MODE_NONE;
static int state;
static uint16_t pps;
static enum beast_state constate;
static char hostname[HOSTNAME_LEN+1];
static uint16_t port;
static struct sockaddr_in saddr;
static int retry_counter;
static char dev[BEAST_SERIAL_PORT_NAME+1];
static speed_t speed;


/*
 * chgconstate() - change connection state with optional debugging
 */
static void chgconstate(enum beast_state new)
{
#if 0
        if (debug > 4)
                printf("chgstate(): %d -> %d\n", constate, new);
#endif
        constate = new;
}


/*
 * chgstate() - change beast protocol state with optional debugging
 */
static void chgstate(int new)
{
#if 0
        if (debug > 4)
                printf("chgstate(): %d -> %d\n", state, new);
#endif
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
 * process_input() - process a chunk of BEAST protocol input from a TCP or serial connection
 */
void process_input(uint8_t *bp, int size)
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


static void common_init(void)
{
        beast_fd = 0;
        pps = 0;
        chgconstate(BEAST_DISCONNECTED);
        retry_counter = 1;
}


/*
 * beast_reset_connection() - reset the TCP connection after an error
 */
void beast_reset_connection(void)
{
        if (beast_fd) {
                close(beast_fd);
                beast_fd = 0;
        }

        if (debug)
                printf("beast_reset_connection(): BEAST connection reset... start retry timer...\n");
        
        retry_counter = BEAST_CONNECT_RETRY;

        chgconstate(BEAST_RETRY_WAIT);
}


/*
 * connect_serial() - attempt to make a connection - factorised code
 */
static int connect_serial(void)
{
        beast_fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);

        if (beast_fd > 0) {
                struct termios term;
        
                tcgetattr(beast_fd, &term);			/* get old port settings */
                        
                term.c_iflag = term.c_oflag = term.c_lflag = 0;
                        
                // term.c_cflag |= B921600;		/* 921Kbps baud rate */
                // term.c_cflag |= B3000000;

                term.c_cflag |= speed;

                term.c_cflag |= CREAD;			/* enable receiver */
                term.c_cflag |= CS8;			/* 8-bit data */
                term.c_cflag |= CLOCAL;			/* No modem controls */
                term.c_cflag |= CRTSCTS;		/* hardware flow control */
                term.c_iflag |= IGNBRK;			/* ignore Break on input */

                term.c_cc[VMIN] = 0;			/* we're using non-blocking so don't set these */
                term.c_cc[VTIME] = 0;

                tcsetattr(beast_fd, TCSAFLUSH, &term);	/* set attribues and flush input */
                tcflush(beast_fd, TCIFLUSH);

                ++telemetry.connect_success;
                return beast_fd;
        } else {
                ++telemetry.connect_fail;
                return 0;
        }
}


/*
 * connect_socket() - attempt to make a TCP connection
 */
static int connect_socket(void)
{
        struct hostent *hostinfo;

        beast_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (beast_fd < 0)
                qerror("connect_socket(): Could not create socket\n");

        memset(&saddr, 0, sizeof(saddr)); 
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(port);

        hostinfo = gethostbyname(hostname);

        if (hostinfo != NULL) {
        
                memcpy(&saddr.sin_addr.s_addr, hostinfo->h_addr, hostinfo->h_length);

                if (connect(beast_fd, (struct sockaddr *)&saddr, sizeof(saddr)) >= 0) {
                        ++telemetry.connect_success;
                        
                        if (debug)
                                printf("connect_socket(): Connected to BEAST source\n");

                        return beast_fd;
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
 * beast_read() - called from main when poll() indicates that there's soemthing
 * to be read from a Beast device (TCP or serial) - here they are just reads on
 * a file descriptor
 */
void beast_read(void)
{
        int size;
        uint8_t buf[1024];
        
        /* data available or connection closed - do a read to find out which */
        size = read(beast_fd, buf, sizeof(buf));

        if (size > 0) {
                /* we have data - call beast common input handler to decode */
                process_input(buf, size);
                ++telemetry.socket_reads;

        } else if (size == 0) {
                /* size is zero -> EOF -> connection closed by peer */
                beast_reset_connection();
                ++telemetry.disconnect;
        } else {
                /* size is negative -> error on socket */
                beast_reset_connection();
                ++telemetry.socket_error;
        }
}


/*
 * beast_serial_init() - initialise the BEAST connection to a serial port
 */
void beast_serial_init(char *port, speed_t spd)
{
        mode = BEAST_MODE_SERIAL;
        strncpy(dev, port, BEAST_SERIAL_PORT_NAME);
        speed = spd;
        common_init();
}


/*
 * beast_tcp_init() - initialise BEAST conenction over TCP
 */
void beast_tcp_init(char *addr, uint16_t prt)
{
        mode = BEAST_MODE_TCP;
        strncpy(hostname, addr, HOSTNAME_LEN);
        port = prt;
        common_init();
}


/*
 * beast_close() - shutdown the BEAST connection
 */
void beast_close(void)
{
        if (beast_fd) {
                close(beast_fd);
                beast_fd = 0;
        }                
}


/*
 * beast_second() - house keeping
 */
void beast_second(void)
{
        switch (constate) {

                case BEAST_DISCONNECTED:
                        /* attemtp to connect to BEAST source */
                        
                        if (mode == BEAST_MODE_TCP) {
                                if (connect_socket()) {
                                        /* connect success */
                                        chgconstate(BEAST_CONNECTED);
                                } else {
                                        /* connect failed */
                                        beast_reset_connection();
                                }

                        } else if (mode == BEAST_MODE_TCP) {
                                if (connect_serial()) {
                                        /* connect success */
                                        chgconstate(BEAST_CONNECTED);
                                } else {
                                        /* connect failed */
                                        beast_reset_connection();
                                }
                        }
                        break;

                case BEAST_CONNECTED:
                        break;

                case BEAST_RETRY_WAIT:
                        if (retry_counter) {
                                --retry_counter;
                                if (!retry_counter) {
                                        if (debug)
                                                printf("beast_second(): change state to allow re-connect\n");
                                        chgconstate(BEAST_DISCONNECTED);
                                }
                        }
                        break;
        }
        
        telemetry.packets_per_second = pps;
        pps = 0;
}
