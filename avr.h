/*
 * avr.h -- Header file for the legacy AVR Protocol
 * Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org   
 *
 */

#ifndef _AVR_H
#define _AVR_H

#include "defs.h"

#define AVR_PORT			30002			/* AVR protocol port */
#define AVR_RETRY			5000L			/* connection retry interval - 5 seconds */
#define AVR_SELECT_TIMEOUT		10000L

#define AVR_MAX_DATA			MODE_ES_LEN
#define AVR_MAX_FRAME			(AVR_MAX_DATA * 2)	/* maximum size of an AVR data frame 28 niddbles -> 14 bytes*/
#define AVR_MAX_READ			1024			/* maximum TCP read size */

/*
 * enumerated list of connection states
 */
enum avrstate {
        AVR_STATE_DISCONNECTED,			/* idle state */
        AVR_STATE_CONNECTED,				/* connected and receiving data */
        AVR_STATE_RETRY_WAIT				/* waiting to reconnect */
};

/*
 * exported functions
 */
void avr_init(char *);
void avr_close(void);
void avr_run(void);
void avr_second(void);
uint16_t avr_get_pps(void);

#endif
