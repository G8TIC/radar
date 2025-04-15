/*
 * beast_serial.c -- Implement the ADS-B BEAST protocol over a serial port
 * Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org
 *
 * ABSTRACT
 *
 * Make an RS232 connection to the a Mode-S Beast receiver using a USB serial port
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/time.h>
#include <errno.h>
#include <termios.h>

#include "radar.h"
#include "defs.h"
#include "beast.h"
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
static char dev[BEAST_SERIAL_PORT_NAME+1];
static speed_t speed;
static enum beast_serial_state constate;
static int fd;
static xtimer_t retry;


/*
 * chgconstate() - change connection state with optional debugging
 */
static void chgconstate(enum beast_serial_state newstate)
{
        if (debug > 1)
                printf("chgconstate(): %d -> %d\n", constate, newstate);
        constate = newstate;
}


/*
 * connect_serial() - attempt to make a connection - factorised code
 */
static int connect_serial(void)
{
        fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);

        if (fd > 0) {
                struct termios term;
        
                tcgetattr(fd, &term);			/* get old port settings */
                        
                term.c_iflag = term.c_oflag = term.c_lflag = 0;
                        
//                term.c_cflag |= B921600;		/* 921Kbps baud rate */
//                term.c_cflag |= B3000000;

                term.c_cflag |= speed;

                term.c_cflag |= CREAD;			/* enable receiver */
                term.c_cflag |= CS8;			/* 8-bit data */
                term.c_cflag |= CLOCAL;			/* No modem controls */
                term.c_cflag |= CRTSCTS;		/* hardware flow control */
                term.c_iflag |= IGNBRK;			/* ignore Break on input */

                term.c_cc[VMIN] = 0;			/* we're using non-blocking so don't set these */
                term.c_cc[VTIME] = 0;

                tcsetattr(fd, TCSAFLUSH, &term);	/* set attribues and flush input */
                tcflush(fd, TCIFLUSH);

                ++telemetry.connect_success;
                return fd;
        } else {
                ++telemetry.connect_fail;
                return 0;
        }
}


/*
 * beast_serial_init() - initialise the BEAST connection to a serial port
 */
void beast_serial_init(char *port, speed_t spd)
{
        memset(&stats, 0, sizeof(stats_t));
        chgconstate(0);
        
        strncpy(dev, port, BEAST_SERIAL_PORT_NAME);
        
        speed = spd;

        beast_init();

        chgconstate(BEAST_SERIAL_DISCONNECTED);
}


/*
 * beast_serial_close() - shutdown the BEAST
 */
void beast_serial_close(void)
{
        if (fd) {
                int rc = close(fd);

                if (rc < 0) {
                        qerror("beast_serial_close(): error calling close(): %s (%d)\n", strerror(errno), errno);
                }
                
                fd = 0;
        }                
}


/*
 * beast_serial_run() - run the BEAST protocol connection
 */
void beast_serial_run(void)
{
        fd_set set;
        struct timeval timeout;
        static uint8_t buf[BEAST_MAX_READ];
        int size, rc;

        switch (constate) {

                case BEAST_SERIAL_DISCONNECTED:
                        if (connect_serial()) {
                                /* connection success */
                                chgconstate(BEAST_SERIAL_CONNECTED);
                                xtimer_stop(&retry);
                        } else {
                                /* connect failed */
                                close(fd);
                                xtimer_start(&retry, BEAST_RETRY);
                                chgconstate(BEAST_SERIAL_RETRY_WAIT);
                        }
                        break;
                        
                case BEAST_SERIAL_CONNECTED:
                        {
                                FD_ZERO(&set);
                                FD_SET(fd, &set);

                                timeout.tv_sec = 0;
                                timeout.tv_usec = BEAST_SELECT_TIMEOUT;
                                
                                rc = select(fd+1, &set, NULL, NULL, &timeout);

                                if (rc < 0) {
                                        /* error on connection */
                                        close(fd);
                                        xtimer_start(&retry, BEAST_RETRY);
                                        chgconstate(BEAST_SERIAL_RETRY_WAIT);
                        	        ++telemetry.socket_error;
                                        
                                } else if (rc == 0) {
                                        /* timeout no data*/
                                        ; 
                                } else {
                                        /* data available or connection closed - do a read */
                                        size = read(fd, buf, sizeof(buf));

                                        if (size > 0) {

                                                //printf("beasr_serial(): read %d bytes\n", size);

                                                /* we have some data - call beast common input handler to decode */
                                                beast_process_input(buf, size);
                                                
                                        } else {
                                                /* size is zero -> connection closed */
                                                close(fd);
                                                xtimer_start(&retry, BEAST_RETRY);
                                                chgconstate(BEAST_SERIAL_RETRY_WAIT);
                                                ++telemetry.disconnect;
                                        }                        
                                }
                        }
                        break;
                        
                case BEAST_SERIAL_RETRY_WAIT:
                        if (xtimer_expired(&retry)) {
                                /* when the timer expires try to connect again */
                                chgconstate(BEAST_SERIAL_DISCONNECTED);
                        } else {
                                /* wait 100mS before polling timer again */
                                usleep(100000);
                        }
                        break;
        }
}

