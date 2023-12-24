/*
 * hex.h -- header file for hexadecimal utils
 */

#ifndef _HEX_H
#define _HEX_H

#include <stdint.h>

extern void hex_dump(const char *, uint8_t *, int);
extern int hex_parse(uint8_t * out, char *);
extern int hex_digits(char *, int);

#endif
