/*
 * stats.c -- ADS-B Radio statistics gathering and sending
 * Author: Michael J. Tubby B.Sc. MIET    mike@tubby.org
 *
 * ABSTRACT
 *
 * Deal with sending ADS-B radio stats periodically to the aggregator.
 *
 * These stats are primiarily about what we have observed on the radio
 * channel in terms of messages and counts
 *
 */

#include <string.h>
#include <time.h>

#include "radar.h"


stats_t stats;					/* global for collecting stats */

static int interval;
static int count;


/*
 * stats_init() - initialise the statistics sending every ival interval (seconds)
 */
void stats_init(int ival)
{
        if (ival) {
                time_t ts = time(NULL);

                memset(&stats, 0, sizeof(stats_t));
                stats.start = stats.now = (uint32_t)ts;
                interval = ival;
                count = STATS_INITIAL;			/* first stats after 2 seconds to indicate we're online */
        } else {
                count = 0;
        }
}


/*
 * stats_second() - house keeping
 */
void stats_second(void)
{
        if (count) {
                --count;
                
                if (!count) {
                        time_t ts = time(NULL);
                
                        stats.now = (uint32_t)ts;
                        radar_send_stats();
                        count = interval;
                }
        }
}
