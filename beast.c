/*
 * beast.c -- Implement the ADS-B BEAST protocol
 * Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org
 *
 * ABSTRACT
 *
 * Implement the BEAST binary block-mode protocol over a TCP/IP
 * connection, a USB serial connection or a physical serial port.
 *
 * Make a TCP/IP connection to a source of the BEAST protocol on 
 * TCP/localhost.30005 as provided by Readsb and Dump1090 or from a
 * read or virtual serial port for Mode-S-Beast hardware.
 *
 * Parse frames de-escaping them and look for Extended Squitter
 * (message type 0x33) which is MLAT + RSSI + 14-bytes of data and pass
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
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

#include "radar.h"
#include "defs.h"
#include "beast.h"
#include "telemetry.h"
#include "hex.h"
#include "qerror.h"


#ifdef DEBUG
#define DEBUG_BEAST
#endif


/*
 * external variables
 */
extern int debug;


/*
 * global variables
 */
int beast_fd = -1;

/*
 * local variables
 */
static enum beast_mode mode = BEAST_MODE_NONE;
static int state;
static uint16_t pps = 0;
static enum beast_state constate;
static char hostname[HOSTNAME_LEN+1];
static uint16_t port;
static struct sockaddr_in saddr;
static int retry_count;
static char dev[BEAST_SERIAL_PORT_NAME+1];
static speed_t speed;
static int obs_count = 0;
static uint8_t buf[BEAST_MAX_FRAME+1];
static uint8_t *op = buf;




/*
 * chgconstate() - change connection state with optional debugging
 */
static void chgconstate(enum beast_state new)
{
#ifdef DEBUG_BEAST
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
#ifdef DEBUG_BEAST
        if (debug > 4)
                printf("chgstate(): %d -> %d\n", state, new);
#endif
        state = new;
}



/*
 * reset_parser() - reset the BEAST frame parser
 */
static void reset_parser(void)
{
        op = buf;
        chgstate(0);
}


/*
 * process_frame() - process a decoded (de-escaped) BEAST frame
 */
static void process_frame(uint8_t *bp, int size)
{
        if ( (bp[0] == 0x31 && size == 10) || (bp[0] == 0x32 && size == 15) || (bp[0] == 0x33 && size == 22) ) {
                radar_process_beast_frame(&bp[1], bp[7], &bp[8], size-8);
                obs_count = BEAST_OBS_COUNT;
                ++telemetry.frames_good;
                ++pps;
        } else {
                ++telemetry.frames_bad;
        }
}


/*
 * process_input() - process a chunk of BEAST protocol input from a TCP or serial connection
 */
static void process_input(uint8_t *bp, ssize_t size)
{
        uint8_t b;
        int sz;

#ifdef DEBUG_BEAST
        printf("process_input(): size: %zd\n", size);
#endif

        ++telemetry.socket_reads;
        telemetry.bytes_read += size;

        /*
         * process a hunk of data from the Beast TCP connection and decode and 
         * pass frames up to process_frame()
         */
         while (size--) {
                b = *bp++;

                sz = op - buf;

                if (sz > BEAST_MAX_FRAME) {
                        ++telemetry.frames_bad;
                        reset_parser();
                        sz = 0;
                }
                
#ifdef DEBUG_BEAST
                printf("process_input(): b=%02X sz=%d state=%d\n", b, sz, state);
#endif

                switch (state) {
                        case 0:							/* wait for first instance of Escape */
                                if (b == BEAST_ESC) {
                                        chgstate(1);
                                        op = buf;
                                } else {
                                        ;
                                }
                                break;

                        case 1:
                                if (b >= 0x31 && b <= 0x33) {
                                        *op++ = b;
                                        chgstate(2);
                                } else if (b == BEAST_ESC) {
                                        /* Still waiting for a frame type. */
                                        op = buf;
                                } else {
                                        reset_parser();
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

                                                if (b >= 0x31 && b <= 0x33) {
                                                        *op++ = b;
                                                        chgstate(2);		/* next frame */
                                                } else {
                                                        chgstate(1);
                                                }
                                        } else {
                                                reset_parser();			/* error reset */
                                        }
                                }
                                break;
                                
                        default:
                                ++telemetry.frames_bad;
                                reset_parser();
                                break;
                }                
        }
}


/*
 * beast_reset_connection() - reset the TCP connection after an error
 */
void beast_reset_connection(void)
{
        if (beast_fd >= 0) {
                close(beast_fd);
                beast_fd = -1;
        }
        
        reset_parser();

        retry_count = BEAST_CONNECT_RETRY;
        chgconstate(BEAST_STATE_RETRY_WAIT);
}


/*
 * connect_serial() - attempt to make a connection
 */
static int connect_serial(void)
{
        int fd;
        struct termios term;

        fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);

        if (fd < 0) {
                ++telemetry.connect_fail;
                return -1;
        }

        if (tcgetattr(fd, &term) < 0) {
                close(fd);
                ++telemetry.connect_fail;
                return -1;
        }

        term.c_iflag = IGNBRK;
        term.c_oflag = 0;
        term.c_lflag = 0;

        term.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
        term.c_cflag |= CS8 | CREAD | CLOCAL | CRTSCTS;

        if (cfsetispeed(&term, speed) < 0 ||
            cfsetospeed(&term, speed) < 0 ||
            tcsetattr(fd, TCSAFLUSH, &term) < 0) {
                close(fd);
                ++telemetry.connect_fail;
                return -1;
        }

        if (tcflush(fd, TCIFLUSH) < 0) {
                close(fd);
                ++telemetry.connect_fail;
                return -1;
        }

        ++telemetry.connect_success;
        return fd;
}



/*
 * connect_socket() - attempt to make a TCP connection
 */
static int connect_socket(void)
{
        int fd;
        struct hostent *hostinfo;

        fd = socket(AF_INET, SOCK_STREAM, 0);

        if (fd < 0)
                qerror("connect_socket(): Could not create socket\n");

        memset(&saddr, 0, sizeof(saddr)); 
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(port);

        hostinfo = gethostbyname(hostname);

        if (hostinfo == NULL) {
                /* error in DNS lookup */
                
#ifdef DEBUG_BEAST
                if (debug)
                        printf("connect_socket(): Unable to resolve %s: %s\n", hostname, hstrerror(h_errno));
#endif

                close(fd);
                ++telemetry.connect_fail;
        
        } else {
        
                memcpy(&saddr.sin_addr.s_addr, hostinfo->h_addr, hostinfo->h_length);

                if (connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)) >= 0) {
                        ++telemetry.connect_success;

#ifdef DEBUG_BEAST                        
                        if (debug)
                                printf("connect_socket(): Connected to BEAST source: %s:%d\n", inet_ntoa(saddr.sin_addr), port);
#endif

                        return fd;
                } else {
                
#ifdef DEBUG_BEAST
                        int save = errno;
#endif
                
                        close(fd);
                        ++telemetry.connect_fail;

#ifdef DEBUG_BEAST                        
                        if (debug)
                                printf("connect_socket(): Connect to BEAST source: %s:%d failed %s (%d)\n", inet_ntoa(saddr.sin_addr), port, strerror(save), save);
#endif
                }
        }
        
        return -1;
}


/*
 * beast_read() - called from main when poll() indicates that there's something
 * to be read from a Beast device (TCP or serial)
 */
void beast_read(void)
{
        if (beast_fd >= 0) {
                ssize_t size;
                uint8_t buf[BEAST_BUF_SIZE];
        
                /* data available or connection closed - do a read to find out which */
                size = read(beast_fd, buf, sizeof(buf));

                if (size > 0) {
                        /* we have data - call beast common input handler to decode */
                        process_input(buf, size);

                } else if (size == 0) {
                        /* size is zero -> EOF -> connection closed by peer */
                        beast_reset_connection();
                        ++telemetry.disconnect;
                } else {
                        /* size is negative -> error on socket */
                        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                                return;
        
                        beast_reset_connection();
                        ++telemetry.socket_error;
                }
        }
}


/*
 * beast_serial_init() - initialise the BEAST connection to a serial port
 */
void beast_serial_init(char *port, speed_t spd)
{
        mode = BEAST_MODE_SERIAL;
        strncpy(dev, port, BEAST_SERIAL_PORT_NAME);
        dev[BEAST_SERIAL_PORT_NAME] = '\0';
        speed = spd;
        chgconstate(BEAST_STATE_DISCONNECTED);
}


/*
 * beast_tcp_init() - initialise BEAST conenction over TCP
 */
void beast_tcp_init(char *addr, uint16_t prt)
{
        mode = BEAST_MODE_TCP;
        strncpy(hostname, addr, HOSTNAME_LEN);
        hostname[HOSTNAME_LEN] = '\0';
        port = prt;
        chgconstate(BEAST_STATE_DISCONNECTED);
}


/*
 * beast_close() - shutdown the BEAST connection
 */
void beast_close(void)
{
        if (beast_fd >= 0) {
                close(beast_fd);
                beast_fd = -1;
        }                
        
        reset_parser();
        obs_count = 0;
        chgconstate(BEAST_STATE_DISCONNECTED);
}


/*
 * beast_second() - house keeping
 */
void beast_second(void)
{
        switch (constate) {

                case BEAST_STATE_DISCONNECTED:
                         /* attempt to connect or reconnect to the BEAST source */
                        if (mode == BEAST_MODE_TCP) {
                                int fd;

                                fd = connect_socket();
                        
                                if (fd >= 0) {
                                        /* connect success */
                                        beast_fd = fd;
                                        chgconstate(BEAST_STATE_CONNECTED);

                                        /* reset parser */
                                        reset_parser();
                                        
                                        /* reset obs counter */
                                        obs_count = BEAST_OBS_COUNT;
                                } else {
                                        /* connect failed */
                                        beast_reset_connection();
                                }

                        } else if (mode == BEAST_MODE_SERIAL) {
                                int fd;
                                
                                fd = connect_serial();
                        
                                if (fd >= 0) {
                                        /* connect success */
                                        beast_fd = fd;
                                        chgconstate(BEAST_STATE_CONNECTED);
                                        
                                        /* reset parser */
                                        reset_parser();
                                        
                                        /* reset obs counter */
                                        obs_count = BEAST_OBS_COUNT;
                                } else {
                                        /* connect failed */
                                        beast_reset_connection();
                                }
                        }
                        break;

                case BEAST_STATE_CONNECTED:
                        /* watch the obs counter */
                        if (obs_count) {
                                --obs_count;
                                
                                if (!obs_count) {
                                        /* no longer handling frames - reset */
                                        beast_reset_connection();
                                }                                        
                        }
                        break;

                case BEAST_STATE_RETRY_WAIT:
                        /* count down and retry */                
                        if (retry_count) {
                                --retry_count;
                                if (!retry_count) {
                                
#ifdef DEBUG_BEAST
                                        if (debug)
                                                printf("beast_second(): change state to allow re-connect\n");
#endif
                                        chgconstate(BEAST_STATE_DISCONNECTED);
                                }
                        }
                        break;
        }
        
        telemetry.packets_per_second = pps;
        pps = 0;
}
