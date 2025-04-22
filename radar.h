/*
 * radar.h -- a definition for ADS-B receiving stations ("radar")
 * Author: Michael J. Tubby B.Sc. MIET    mike@tubby.org
 */

#ifndef _RADAR_H
#define _RADAR_H

#include <stdio.h>
#include <stdint.h>

#include "defs.h"
#include "stats.h"
#include "telemetry.h"
#include "authtag.h"


#define RADAR_PORT				5997

#define RADAR_PROTOCOL_NONE			0
#define RADAR_PROTOCOL_BEAST_TCP		1
#define RADAR_PROTOCOL_BEAST_SERIAL		2
#define RADAR_PROTOCOL_GNS_SERIAL		3

#define RADAR_OPCODE_RESERVED			0x00
#define RADAR_OPCODE_MODE_AC			0x01
#define RADAR_OPCODE_MODE_S			0x02
#define RADAR_OPCODE_MODE_ES			0x03
#define RADAR_OPCODE_MULTIFRAME			0x04
#define RADAR_OPCODE_KEEPALIVE			0x80
#define RADAR_OPCODE_SYSTEM_TELEMETRY		0x81
#define RADAR_OPCODE_RADIO_STATS		0x82
#define RADAR_OPCODE_CONFIG_REQ			0xC1
#define RADAR_OPCODE_CONGIG_ACK			0xC2

#define RADAR_MAX_MULTIFRAME			32
#define RADAR_FORWARD_INTERVAL			50			/* milliseconds */


/*
 * some useful macros
 */
#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifndef pow2
#define pow2(a) (a*a)
#endif



extern int protocol;


typedef struct {
        uint8_t mlat[MLAT_LEN];			/* Multi-lateration timestamp */
        uint8_t rssi;        			/* Received signal strength indication */
        uint8_t data[MODE_ES_LEN];		/* data */
}  __attribute__((packed)) mode_es_frame_t;



/*
 * radar message type:  Generic message (header)
 */
typedef struct {
        uint64_t key;                           /* API key for this radar station */
        uint64_t ts;                            /* Timestamp (uS) */
        uint32_t seq;                           /* Message sequence number */
        uint8_t opcode;				/* opcode - message type */
        uint8_t data[];				/* variable length data */
} __attribute__((packed)) radar_msg_t;


/*
 * radar message type: Mode-A/C ident/altitude (2 bytes)
 */
typedef struct {
        uint64_t key;                           /* API key for this radar station */
        uint64_t ts;                            /* Timestamp (uS) */
        uint32_t seq;                           /* Message sequence number */
        uint8_t opcode;				/* Opcode: message type */
        uint8_t mlat[MLAT_LEN];			/* Multi-lateration timestamp */
        uint8_t rssi;        			/* Received signal strength indication */
        uint8_t data[MODE_AC_LEN];		/* data */
        uint8_t atag[AUTHTAG_LEN];		/* Authentication tag */
} __attribute__((packed)) radar_mode_ac_t;


/*
 * radar message type: Mode-S Short Squitter (7 bytes)
 */
typedef struct {
        uint64_t key;                           /* API key for this radar station */
        uint64_t ts;                            /* Timestamp (uS) */
        uint32_t seq;                           /* Message sequence number */
        uint8_t opcode;				/* Opcode: message type */
        uint8_t mlat[MLAT_LEN];			/* Multi-lateration timestamp */
        uint8_t rssi;        			/* Received signal strength indication */
        uint8_t data[MODE_SS_LEN];		/* data */
        uint8_t atag[AUTHTAG_LEN];		/* Authentication tag */
} __attribute__((packed)) radar_mode_ss_t;


/*
 * radar message type: Mode-S Extended Squitter (14 bytes)
 */
typedef struct {
        uint64_t key;                           /* API key for this radar station */
        uint64_t ts;                            /* Message timestamp (uS) */
        uint32_t seq;                           /* Message sequence number */
        uint8_t opcode;				/* Opcode: message type */
        uint8_t mlat[MLAT_LEN];			/* Multi-lateration timestamp */
        uint8_t rssi;        			/* Received signal strength indication */
        uint8_t data[MODE_ES_LEN];		/* data */
        uint8_t atag[AUTHTAG_LEN];		/* Authentication tag */
} __attribute__((packed)) radar_mode_es_t;


/*
 * radar message type: keepalive for when there is no data
 */
typedef struct {
        uint64_t key;                           /* API key for this radar station */
        uint64_t ts;                            /* Timestamp (uS) */
        uint32_t seq;                           /* Message sequence number */
        uint8_t opcode;				/* Opcode: message type */
        uint8_t ver_hi;
        uint8_t ver_lo;
        uint8_t patch;
        uint8_t atag[AUTHTAG_LEN];		/* Authentication tag */
} __attribute__((packed)) radar_keepalive_t;


/*
 * radar message type: stats - statistics about messages observed on the radio channel (see stats.c,h)
 */
typedef struct {
        uint64_t key;                           /* API key for this radar station */
        uint64_t ts;                            /* Timestamp (uS) */
        uint32_t seq;                           /* Message sequence number */
        uint8_t opcode;				/* Opcode: message type */
        stats_t stats;				/* Radar (radio channel) statistics */
        uint8_t atag[AUTHTAG_LEN];		/* Authentication tag */
} __attribute__((packed)) radar_stats_t;


/*
 * radar message type: telemetry about operation of the receiver system (see telemetry.c,h)
 */
typedef struct {
        uint64_t key;                           /* API key for this radar station */
        uint64_t ts;                            /* Timestamp (uS) */
        uint32_t seq;                           /* Message sequence number */
        uint8_t opcode;				/* Opcode: message type */
        telemetry_t telemetry;			/* Receiver platform telemetry */
        uint8_t atag[AUTHTAG_LEN];		/* Authentication tag */
} __attribute__((packed)) radar_telemetry_t;


/*
 * es_msg_t type - one extended squitter sub-message - 21 bytes
 */
typedef struct {
        uint8_t mlat[MLAT_LEN];			/* Multi-lateration timestamp */
        uint8_t rssi;        			/* Received signal strength indication */
        uint8_t data[MODE_ES_LEN];		/* data */
} __attribute__((packed)) es_t;


/*
 * radar message type: Mode-S Extended Squitter (14 bytes)
 */
typedef struct {
        uint64_t key;                           /* API key for this radar station */
        uint64_t ts;                            /* Message timestamp (uS) */
        uint32_t seq;                           /* Message sequence number */
        uint8_t opcode;				/* Opcode: message type */
        uint8_t num;
        es_t es[RADAR_MAX_MULTIFRAME];
        uint8_t atag[AUTHTAG_LEN];		/* Authentication tag */
} __attribute__((packed)) radar_multiframe_t;


/*
 * external functions
 */
void radar_process(uint8_t mlat[MLAT_LEN], uint8_t rssi, uint8_t *buf, int size);
void radar_send_keepalive(void);
void radar_send_stats(void);
void radar_send_telemetry(void);

#endif
