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

/*
 * a data type for holding duplicate checks on Short Squitter (7 bytes)
 */
typedef struct {
        uint8_t ss[MODE_SS_LEN];
        uint64_t ts;
        UT_hash_handle hh;
} dupe_ss_t;


/*
 * a data type for holding duplicate checks on Extended Squitter (14 bytes)
 */
typedef struct {
        uint8_t es[MODE_ES_LEN];
        uint64_t ts;
        UT_hash_handle hh;
} dupe_es_t;


int dupe_check_ss(uint8_t *);
int dupe_check_es(uint8_t *);
int dupe_clean(void);

#endif
