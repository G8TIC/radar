/*
 * beast.h -- Header file for ADS-B Beast Protocol
 * Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org   
 *
 */

#ifndef _BEAST_H
#define _BEAST_H

#include <stdint.h>

#define BEAST_PORT			30005		/* BEAST protocol port */
#define BEAST_RETRY			3000L		/* connection retry interval - 3 seconds */
#define BEAST_SELECT_TIMEOUT		10000L		/* 10mS */
#define BEAST_MAX_FRAME			22		/* maximum size of a Beast data frame */
#define BEAST_MAX_READ			1024		/* maximum TCP read size */
#define BEAST_CONNECT_RETRY		2000L		/* connection retry timer in mS */
#define BEAST_ESC			0x1A		/* Escape character used in BEAST frames */


/*
 * enumerated list of connection states
 */
enum beaststate {
        BEAST_STATE_DISCONNECTED,			/* idle state */
        BEAST_STATE_CONNECTED,				/* connected and receiving data */
        BEAST_STATE_RETRY_WAIT				/* waiting to reconnect */
};


/*
 * exported functions
 */
void beast_init(char *);
void beast_close(void);
void beast_run(void);
void beast_second(void);
uint16_t beast_get_pps(void);


#endif
