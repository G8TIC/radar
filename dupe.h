/*
 * dupe.h -- input de-duplicator
 */

#ifndef _DEPE_H
#define _DUPE_H

#include <string.h>
#include <stdint.h>
#include <time.h>

#include "defs.h"
#include "uthash.h"

#define DUPE_MAX_ES	3000000		/* 3,000,000uS -> 3 seconds */
#define DUPE_MAX_SS     3000000         /* 3,000,000uS -> 3 seconds */

int dupe_check_ss(uint8_t *);
int dupe_check_es(uint8_t *);
int dupe_clean(void);

#endif
