/*
 * beast_serial.h -- Header file for ADS-B Beast Protocol
 * Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org   
 *
 */

#ifndef _BEAST_SERIAL_H
#define _BEAST_SERIAL_H

#include <termios.h>

#define BEAST_SERIAL_PORT_NAME		32		/* BEAST serial port name max length */
#define BEAST_SELECT_TIMEOUT		10000L		/* 10mS */
#define BEAST_MAX_FRAME			22		/* maximum size of a Beast data frame */
#define BEAST_MAX_READ			1024		/* maximum TCP read size */
#define BEAST_ESC			0x1A		/* Escape character used in BEAST frames */


/*
 * enumerated list of connection states
 */
enum beast_serial_state {
        BEAST_SERIAL_DISCONNECTED,			/* idle state */
        BEAST_SERIAL_CONNECTED,				/* connected and receiving data */
        BEAST_SERIAL_RETRY_WAIT				/* waiting to reconnect */
};


/*
 * exported functions
 */
void beast_serial_init(char *, speed_t);
void beast_serial_close(void);
void beast_serial_run(void);
void beast_serial_second(void);


#endif
