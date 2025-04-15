/*
 * authtag.h -- Authentication tag signing of Radar messages
 */

#ifndef _AUTHTAG_H
#define _AUTHTAG_H

#include <stdint.h>

#define AUTHTAG_LEN		8
#define AUTHTAG_KEY_LEN		64		/* inpuit to HMAC-SHA256 */

/*
 * exported functions
 */
void authtag_init(char *);
void authtag_sign(uint8_t *, int, void *, int);
int authtag_check(uint8_t *, int, uint8_t *, int);

#endif
