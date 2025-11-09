/*
 * beast.h -- Common header file for ADS-B Beast Protocol
 * Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org   
 *
 */

#ifndef _BEAST_H
#define _BEAST_H

#include <stdint.h>
#include <termios.h>

#define BEAST_MAX_FRAME			22		/* maximum size of a Beast data frame */
#define BEAST_BUF_SIZE			1024		/* Beast buffer size */
#define BEAST_ESC			0x1A		/* Escape character used in BEAST frames */
#define BEAST_CONNECT_RETRY		5		/* connection retry interval - 5 seconds */
#define BEAST_SERIAL_PORT_NAME		64		/* size of a serial port device name */
#define BEAST_TCP_PORT			30005		/* BEAST protocol port */


/*
 * enumerated list of operating modes
 */
enum beast_mode {
        BEAST_MODE_NONE,
        BEAST_MODE_SERIAL,
        BEAST_MODE_TCP
};


/*
 * enumerated list of connection states
 */
enum beast_state {
        BEAST_STATE_DISCONNECTED,			/* disconnected state */
        BEAST_STATE_CONNECTED,				/* connected and receiving data */
        BEAST_STATE_RETRY_WAIT				/* waiting to reconnect */
};


/*
 * exported global variables
 */
extern int beast_fd;


/*
 * exported functions
 */
void beast_serial_init(char *, speed_t);
void beast_tcp_init(char *, uint16_t);
void beast_reset_connection(void);
void beast_second(void);
void beast_read(void);
void beast_close(void);

#endif
