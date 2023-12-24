/*
 * hex.c -- simple utilities for working with hexadecial
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "hex.h"


/*
 * hex_dump() - dump out a buffer in hex with a prompt
 */
void hex_dump(const char * prompt, uint8_t * bp, int size)
{
	int i;

	printf("%s (size %d): ", prompt, size);

	for (i=0; i<size; ++i)
		printf("%02X ", bp[i]);

	printf("\n");
}


/*
 * hex_digits() - test is a string contains only hex digits
 */
int hex_digits(char * buf, int max)
{
        int i;
        
        for (i = 0; i < max; i++) {
                if (!isxdigit(buf[i]))
                        return 0;        
        }

        return 1;
}


/*
 * hex_parse() - parse a hexaecimal string from 'in' and put the binary
 * equivalent at 'out', returning the output length or zero on error
 */
int hex_parse(uint8_t * out, char *in)
{
        char * p;
        uint8_t * q;

        for (p=in, q=out; *p; p+=2, q++) {
                if (sscanf(p, "%02hhx", q) != 1)
                        return 0;
        }
        return (q - out);
}
