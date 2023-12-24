/*
 * stats.h -- stats for the ADS-B / radio
 *
 * WARNING/NOTE/CAUTION
 *
 * We cannot use time_t directly in messages due to architecture differences!
 *
 */

#ifndef _STATS_H
#define _STATS_H

#include <stdint.h>
#include "defs.h"

#define STATS_INTERVAL		900		/* 900 seconds = 15 minutes */


/*
 * statistics structure
 */
typedef struct {
        uint32_t start;				/* start-up time for station */
        uint32_t now;				/* time for this message */

        uint64_t rx_mode_ac;			/* Mode-A/C redeived count */
        uint64_t rx_mode_ss;			/* Mode-S Short received count */
        uint64_t rx_mode_es;			/* Mode-S Extended received count */

        uint64_t rx_df[MAX_DF];			/* Mode-S Downlink formats received */

        uint64_t dupe_ac;			/* Duplicate Mode-A/C messages */
        uint64_t dupe_ss;			/* Duplicate Mode-S Short messages */
        uint64_t dupe_es;			/* Duplicate Mode-S Extended messages */
        uint64_t dupes;				/* Overall duplicates */

        uint64_t tx_keepalive;			/* Keepalive messages sent */
        uint64_t tx_mode_ac;			/* Mode-A/C messages transmitted */
        uint64_t tx_mode_ss;			/* Mode-S Short messages transmitted */
        uint64_t tx_mode_es;			/* Mode-S Extended messages transmitted */
        uint64_t tx_stats;			/* Number of Stats messages (these) transmitted */

        uint64_t tx_count;			/* Total number of transmissions */
        uint64_t tx_bytes;			/* Total number of bytes transmitted */

} __attribute__((packed)) stats_t;


extern stats_t stats;


/*
 * exported functions
 */
void stats_init(int);
void stats_second(void);
void stats_send(void);

#endif

