/*
 * beast.h -- Common header file for ADS-B Beast Protocol
 * Author: Michael J. Tubby B.Sc. MIET  mike@tubby.org   
 *
 */

#ifndef _BEAST_H
#define _BEAST_H

#include <stdint.h>

#define BEAST_MAX_FRAME			22		/* maximum size of a Beast data frame */
#define BEAST_MAX_READ			1024		/* maximum TCP read size */
#define BEAST_ESC			0x1A		/* Escape character used in BEAST frames */


/*
 * exported functions
 */
void beast_init(void);
void beast_process_input(uint8_t *, int);
void beast_second(void);
uint16_t beast_get_pps(void);


#endif
