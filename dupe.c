/*
 * dupe.c -- input processing: duplicate message checking
 */

#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "defs.h"
#include "ustime.h"
#include "qerror.h"
#include "uthash.h"
#include "dupe.h"


extern int debug;


/*
 * a list of duplicate messages
 */
dupe_ss_t *dupe_ss = NULL;
dupe_es_t *dupe_es = NULL;


/*
 * dupe_check_ss() - duplicate message checking for Extended Squitter
 */
int dupe_check_ss(uint8_t *ss)
{
        dupe_ss_t *dp;				/* dupe pointer */
                
        HASH_FIND(hh, dupe_ss, ss, MODE_SS_LEN, dp);

        if (dp) {

                return 1;

        } else {
                dp = malloc(sizeof(dupe_ss_t));

                if (dp) {
                        dp->ts = ustime();
                        memcpy(dp->ss, ss, MODE_SS_LEN);
                        HASH_ADD(hh, dupe_ss, ss, MODE_SS_LEN, dp);
                } else {
                        qabort("dupe_check_ss(): malloc() failed - out of memory\n");
                }
 
                return 0;
        }
}


/*
 * dupe_check_es() - duplicate message checking for Extended Squitter
 */
int dupe_check_es(uint8_t *es)
{
        dupe_es_t *dp;				/* dupe pointer */
                
        HASH_FIND(hh, dupe_es, es, MODE_ES_LEN, dp);

        if (dp) {

                return 1;

        } else {
                dp = malloc(sizeof(dupe_es_t));

                if (dp) {
                        dp->ts = ustime();
                        memcpy(dp->es, es, MODE_ES_LEN);
                        HASH_ADD(hh, dupe_es, es, MODE_ES_LEN, dp);
                } else {
                        qabort("dupe_check_es(): malloc() failed - out of memory\n");
                }
 
                return 0;
        }
}



/*
 * clean_ss() - clean up the duplicate Short Squitter objects
 */
static int clean_ss(uint64_t now)
{
        dupe_ss_t *dp, *tmp;
        
        int count = 0;
        
        HASH_ITER(hh, dupe_ss, dp, tmp) {        		/* delete-safe itteration */
        
                uint64_t age = now - dp->ts;			/* age of message */
        
                if (age > DUPE_MAX_SS) {			/* too old then delete it */
                        HASH_DEL(dupe_ss, dp);
                        free(dp);
                        ++count;
                }
        }

        return count;
}


/*
 * clean_es() - clean up the duplicate Extended Squitter objects
 */
static int clean_es(uint64_t now)
{
        dupe_es_t *dp, *tmp;
        
        int count = 0;
        
        HASH_ITER(hh, dupe_es, dp, tmp) {        		/* delete-safe itteration */
        
                uint64_t age = now - dp->ts;			/* age of message */
        
                if (age > DUPE_MAX_ES) {			/* too old then delete it */
                        HASH_DEL(dupe_es, dp);
                        free(dp);
                        ++count;
                }
        }
        
        return count;
}


/*
 * dupe_clean() - clean the duplicate queues
 *
 * Called at 10Hz from the house keeping timer
 */
int dupe_clean(void)
{
        uint64_t now = ustime();
        int count_ss, count_es, count;
        
        count_ss = clean_ss(now);
        count_es = clean_es(now);

        count = count_ss + count_es;
        
        if (debug > 2 && count)
                printf("dupe_clean(): deleted %d SS and %d ES\n", count_ss, count_es);

        return count;
}

