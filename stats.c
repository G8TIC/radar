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

static int interval;
static int countdown;


void stats_init(int ival)
{
        if (ival) {
                time_t ts = time(NULL);

                memset(&stats, 0, sizeof(stats_t));
                stats.start = (uint32_t)ts;
                interval = ival;
                countdown = ival / 2;
        } else {
                countdown = 0;
        }
}


void stats_second(void)
{
        if (countdown) {
                --countdown;
                
                if (!countdown) {
                        time_t ts = time(NULL);
                
                        stats.now = (uint32_t)ts;
                        radar_send_stats();
                        countdown = interval;
                }
        }
}
