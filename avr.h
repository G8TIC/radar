/*
 * avr.h -- Header file for the legacy AVR Protocol
 * Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org   
 *
 */

#ifndef _AVR_H
#define _AVR_H

#include "defs.h"

#define AVR_PORT		30002			/* BEAST protocol port */
#define AVR_RETRY		3000L			/* retry interval for beast connection 3 seconds */

#define AVR_STATE_DISCONNECTED	0			/* connection states */
#define AVR_STATE_CONNECTED	1
#define AVR_STATE_RETRY_WAIT	2

#define AVR_MAX_DATA		MODE_ES_LEN
#define AVR_MAX_FRAME		(AVR_MAX_DATA * 2)	/* maximum size of an AVR data frame 28 niddbles -> 14 bytes*/
#define AVR_MAX_READ		1024			/* maximum TCP read size */

/*
 * exported functions
 */
int avr_init(char *);
void avr_close(void);
void avr_run(void);
void avr_second(void);
uint16_t avr_get_pps(void);

#endif
