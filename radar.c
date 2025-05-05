/*
 * radar.c -- ADS-B receiver feeder V2 for the 1090MHz UK network
 * Author: Michael J. Tubby B.Sc. MIET  mike.tubby@1090mhz.uk / mike@tubby.org
 *
 *
 * ABSTRACT
 *
 * This 'radar' feeder software runs as a daemon on your local system and and
 * connects to the ADS-B service on your device running dump1090/readsb using
 * the BEAST binary protocol (preferred) or AVR ASCII protocol (fallback mode)
 * and extracts messages of interest (mainly Extended Squitter messages), converts
 * them to UDP/IP and forwards them to the 1090MHz UK network aggregator.
 *
 * The code implements local de-duplication over a 3 second window to remove
 * duplicate/un-necessary messages and reduce transmissions by approximately
 * 30-35% compared with blindly sending all messages.  This saves both network
 * bandwidth and processing load at the aggregator.
 *
 * By default we send only Extended Equitter DF17, DF18 and DF19 but can enable
 * can enbable DF20/DF21 intergator/Comm-B responses, DF16 altitude and DF22
 * Military use as/when/if we need them.
 *
 * For more details on ADS-B and DF types see:
 *
 *	https://mode-s.org/decode/book-the_1090mhz_riddle-junzi_sun.pdf
 *
 *
 * SUPPORTED DEVICES
 *
 * Radar supports TLS-SDR type dongles using dump1090 or readsb and supports
 * USB connections to dedicated Mode-S BEAST recivers and GNS 5892/5894 using
 * HULC (modified Beast protocol).
 *
 *
 * ENCRYPTION AND AUTHENTICATION
 *
 * Radar messages are broadcast un-encrypted by aircraft and hence there seems to
 * be little point encrypting the mesasges on the wire, however for system security
 * we want to check the integrity of messages and authenticate the sender is genuine.
 *
 * We prove the integrity and authenticity of each message using a 64-bit digital
 * signature called an "authentication tag" - this is a truncated HMAC-SHA256
 * digest of the message or "signature".
 *
 * Providing the originator and recipient use a unique and private pass-phrase then
 * then the signature provides message integrity and authentication so we can trust
 * both the content and the sender.
 *
 * The digital signature should protect data in transit against message corruption,
 * tampering, forgery, spoofing and replay attack. 
 *
 * Refer to authtag.c for more details.
 *
 *
 * TELEMETRY
 *
 * This software gathers a small amount of telemetry about the system that it is
 * running on in order for us to understand performance issues and track down bugs.
 *
 * No personal information is gathered.  See TELEMTERY.md for more details.
 *
 *
 * STATISTICS
 *
 * This software gathers information about amount of ADS-B traffic and messages
 * seen on the radio channel by your receiver along with message rates in order
 * for us to characterise the radio channel and understand the data being received.
 *
 * See STATS.md for more details.
 *
 *
 * WEBSITE
 *
 * For information on the 1090MHz UK radar network please visit:
 *
 *	https://1090mhz.uk
 *
 *
 * SOURCE REPOSITORY
 *
 * The official repository for this code is now Github - see:
 *
 *	https://github.com/G8TIC/radar
 *
 *
 *
 * COPYRIGHT
 *
 * Copyright (C) 2023 by Michael J. Tubby B.Sc. MIET and 1090MHz Solutions Ltd.
 * trading as "1090MHz UK" - All Rights Reserved.
 *
 * 
 * LICENSE
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or at your option any later version.
 *
 * A copy of the GNU General Public License is included in the file LICENSE.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *
 * USAGE
 *
 * The daemon can be used from the command line as follows:
 *
 *	radar -k <key> -p <pass-phrase> (-d)
 *
 * where:
 *
 *	-k <key>	  sharing key for your station
 *	-h <hostname>	  destination hostname for aggregator, defaults to adsb-in.1090mhz.uk
 *      -p <pass-phrase>  pre-shared key for message authentication, defaults to "secret"
 *	-r <ipaddr>	  address of device that provides ADS-B source if not localhost
 *	-e                forward everything (Mode-A/C, Mode-S, and all Extended Squitter)
 *	-u <user>	  user name to run under, e.g. 'nobody'
 *	-g <group>	  group name to run under, e.g. 'nogroup'
 *	-q <qos>	  IP Quality of Service using DiffServe values 0-63
 *	-d		  daemonise use this for SysV init systems (systemd does this)
 *	-x|xx|xxx	  run with debug (-xx for more debug)
 *	-f		  produce forwarding stats one per second (foreground only)
 *	-s <seconds>	  send radio channel stats every period (default 900 = 15 min)
 *	-t <seconds>	  send system telemetry every period (default 900 = 15 min)
 *	-m		  enable multiframe sending (more efficient but adds latency)
 *	-i <ms>           multiframe forwaring interval/timeout (milliseconds)
 *	-v		  print version number and exit
 *
 * but normally runs as a service.
 *
 *
 * SHARING KEY
 *
 * As an absolute minimum you need an API/Skaring Key - these are 64-bit random numbers
 * that identify your radar station to us but mean nothing to anyone else.
 *
 * Sharing keys are represented as 16 hex nibbles in uppercase with the leading 0x - for
 * example:
 *
 *                        0x79441BC23EDA3F17
 *
 *
 * PASS-PHRASE/SECRET
 *
 * Radar V2 uses a 64-bit Authentication Tag on each message to protect data in transit
 * from corruption, forgery and replay attacks.
 *
 * If both parties set a pre-shared key/pass-phrase then the aggregator can authenticate
 * the originator and we know the message cannot have been spoofed.
 *
 * The system defaults to using the pass-phrase "secret" - please contact us to set a better
 * one - we usually generate pass phrases at random.org
 *
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <sys/time.h>
#include <linux/socket.h>
#include <linux/ip.h>
#include <termios.h>
#include <sys/timerfd.h>
#include <sys/poll.h>

#include "radar.h"
#include "banner.h"
#include "version.h"
#include "beast.h"
#include "udp.h"
#include "dupe.h"
#include "authtag.h"
#include "ustime.h"
#include "mstime.h"
#include "hex.h"
#include "qerror.h"


/*
 * global variables
 */
int ending = 0;
int isdaemon = 0;
int protocol = 0;
int dosyslog = 0;
int dologfile = 0;
int dostats = 0;
int debug = 0;
int restart = 0;
int gotkey = 0;
int send_ss = 0;
int send_ac = 0;
int multiframe = 0;
int forward_interval = RADAR_FORWARD_INTERVAL;			/* milliseconds */
int rebind = 0;
int everything = 0;
int stats_interval = STATS_INTERVAL;
int telemetry_interval = TELEMETRY_INTERVAL;
uint64_t key;
char hostname[HOSTNAME_LEN+1] = UDP_HOST;
char psk[PSK_LEN+1] = "secret";
char localaddress[HOSTNAME_LEN+1] = "127.0.0.1";
uint16_t port = BEAST_TCP_PORT;
uint32_t seq = 1;
char username[USERNAME_LEN+1] = "nobody";
char groupname[GROUPNAME_LEN+1] = "nogroup";
int qos = 0;
stats_t stats;							/* stats for reporting to central aggregator */
uint32_t dupe_ss_count = 0;
uint32_t dupe_es_count = 0;
uint32_t send_count = 0;
uint32_t byte_count = 0;
char serport[BEAST_SERIAL_PORT_NAME+1] = "/dev/ttyUSB0";
int num;


typedef struct {
        uint8_t mlat[MLAT_LEN];					/* Multi-lateration timestamp */
        uint8_t rssi;        					/* Received signal strength indication */
        uint8_t data[MODE_ES_LEN];				/* data */
} esdata_t;


esdata_t esdata[RADAR_MAX_MULTIFRAME];


/*
 * cleanup() - the mess we've made
 */
void cleanup(void)
{
        /* close beast connection */
        beast_close();

        /* close down UDP */
        udp_close();
}


/*
 * Signal handler
 */
void signal_handler(int signal)
{
        if (debug)
                printf("signal_handler(): called with signal=%d", signal);

        switch (signal) {
                case SIGHUP:
                        ++restart;
                        break;

                case SIGTERM:
                        ++ending;
                        break;

                case SIGINT:
                        ++ending;
                        break;
        }
}


/*
 * get_uid() - return the UID for a user by name or number by performing the
 * appropriate lookup in the /etc/password database, or return 0 for not found.
 */
uid_t get_uid(const char * name)
{
        struct passwd *pwd = (void *)0;
        int i = atoi(name);

        if (i) {
                pwd = getpwuid((uid_t)i);
        } else {
                pwd = getpwnam(name);
        }

        return (pwd) ? pwd->pw_uid : (uid_t)0;
}


/*
 * get_gid() - return the GID for a group by name or number by performing the
 * appropriate lookup in the /etc/group database, or return 0 for not found.
 */
gid_t get_gid(const char * group)
{
        struct group *grp = (void *)0;
        int i = atoi(group);

        if (i) {
                grp = getgrgid((gid_t)i);
        } else {
                grp = getgrnam(group);
        }

        return (grp) ? grp->gr_gid : (gid_t)0;
}


/*
 * clear_buffer() - clear the multi-frame buffer
 */
static void clear_buffer(void)
{
        memset(&esdata, 0, sizeof(esdata));
        num = 0;
}


/*
 * send_mode_ac() - Send a Mode-A/C message to the aggregator
 */
static void send_mode_ac(radar_mode_ac_t *bp)
{
        if (bp) {
                bp->key = key;							/* API key */
                bp->ts = ustime();						/* timestamp uS */
                bp->seq = seq++;						/* sequence number */
                bp->opcode = RADAR_OPCODE_MODE_ES;				/* opcode */
                
                /* add auth tag */
                authtag_sign(bp->atag, AUTHTAG_LEN, bp, sizeof(radar_mode_ac_t)-AUTHTAG_LEN);
                
                /* send to aggregator */
                udp_send(bp, sizeof(radar_mode_ac_t));				/* send message */

                /* stats for aggregator */
                ++stats.tx_mode_ac;
                ++stats.tx_count;
                stats.tx_bytes += sizeof(radar_mode_ac_t);
                
                /* local stats */
                ++send_count;
                byte_count += sizeof(radar_mode_ac_t);
        }
}


/*
 * send_mode_ss() - Send a Mode-S Short Squitter to the aggregator
 */
static void send_mode_ss(radar_mode_ss_t *bp)
{
        if (bp) {
                uint8_t df = bp->data[0] >> 3;
        
                bp->key = key;							/* API key */
                bp->ts = ustime();						/* timestamp uS */
                bp->seq = seq++;						/* sequence number */
                bp->opcode = RADAR_OPCODE_MODE_ES;				/* opcode */
                
                /* add auth tag */
                authtag_sign(bp->atag, AUTHTAG_LEN, bp, sizeof(radar_mode_ss_t)-AUTHTAG_LEN);

                /* send to aggregator */                	
                udp_send(bp, sizeof(radar_mode_ss_t));

                /* stats for aggregator */
                ++stats.tx_mode_ss;
                ++stats.tx_count;
                stats.tx_bytes += sizeof(radar_mode_ss_t);
                
                /* local stats */
                ++send_count;
                byte_count += sizeof(radar_mode_ss_t);
                
                if (debug)
                        printf("send_mode_ss(): df=%d\n", df);
        }
}


/*
 * send_mode_es() - Send a Mode-S Extended Squitter to the aggregator
 */
static void send_mode_es(radar_mode_es_t *bp)
{
        if (bp) {
                bp->key = key;							/* API key */
                bp->ts = ustime();						/* timestamp uS */
                bp->seq = seq++;						/* sequence number */
                bp->opcode = RADAR_OPCODE_MODE_ES;				/* opcode */

                /* add auth tag */
                authtag_sign(bp->atag, AUTHTAG_LEN, bp, sizeof(radar_mode_es_t) - AUTHTAG_LEN);
#if 0
                /*
                 * interference monkey - brake random bits on random occasions to check auth tag works ...
                 */
                int r = rand() % 10;
                
                if (r == 0) {
                        uint8_t *p = (uint8_t *)bp;
                        int bit = rand() % 8;					/* bit to flip */
                        int byte = rand() % sizeof(radar_mode_es_t);		/* byte to flip at */
                        
                        uint8_t mask = 1 << bit;
                        p[byte] ^= mask;
                        printf("send_mode_es(): corrupted byte=%d bit=%d\n", byte, bit);
                }
#endif
                /* send to aggregator */
                udp_send(bp, sizeof(radar_mode_es_t));

                /* stats for aggregator */
                ++stats.tx_mode_es;
                ++stats.tx_count;
                stats.tx_bytes += sizeof(radar_mode_es_t);
                
                /* local stats */
                ++send_count;
                byte_count += sizeof(radar_mode_es_t);
        }
}


/*
 * radar_send_keepalive() - send a keepalive message when there's been
 * no other traffic in the last second
 */
void radar_send_keepalive(void)
{
        radar_keepalive_t msg;
        
        msg.key = key;
        msg.ts = ustime();
        msg.seq = seq++;
        msg.opcode = RADAR_OPCODE_KEEPALIVE;
        
        /* software version number */
        msg.ver_hi = VERSION_MAJOR;
        msg.ver_lo = VERSION_MINOR;
        msg.patch = VERSION_PATCH;
        
        /* add auth tag */
        authtag_sign(&msg.atag[0], AUTHTAG_LEN, &msg, sizeof(radar_keepalive_t) - AUTHTAG_LEN);
                
        /* send to aggregator */
        udp_send(&msg, sizeof(radar_keepalive_t));

        /* stats for aggregator */
        ++stats.tx_stats;
        ++stats.tx_count;
        stats.tx_bytes += sizeof(radar_keepalive_t);
}


/*
 * radar_send_stats() - send statistics about the radio channel
 */
void radar_send_stats(void)
{
        radar_stats_t msg;
        
        msg.key = key;
        msg.ts = ustime();
        msg.seq = seq++;
        msg.opcode = RADAR_OPCODE_RADIO_STATS;
        msg.stats = stats;
        
        /* add auth tag */
        authtag_sign(&msg.atag[0], AUTHTAG_LEN, &msg, sizeof(radar_stats_t) - AUTHTAG_LEN);
                
        /* send to aggregator */
        udp_send(&msg, sizeof(radar_stats_t));

        /* stats for aggregator */
        ++stats.tx_stats;
        ++stats.tx_count;
        stats.tx_bytes += sizeof(radar_stats_t);
                
        /* local stats */
        ++send_count;
        byte_count += sizeof(radar_stats_t);
}


/*
 * radar_send_telemetry() - send telemetry about the receiver station itself
 */
void radar_send_telemetry(void)
{
        radar_telemetry_t msg;
        
        msg.key = key;
        msg.ts = ustime();
        msg.seq = seq++;
        msg.opcode = RADAR_OPCODE_SYSTEM_TELEMETRY;
        msg.telemetry = telemetry;
        
        /* add auth tag */
        authtag_sign(&msg.atag[0], AUTHTAG_LEN, &msg, sizeof(radar_telemetry_t) - AUTHTAG_LEN);
                
        /* send to aggregator */
        udp_send(&msg, sizeof(radar_telemetry_t));

        /* stats for aggregator */
        ++stats.tx_telemetry;
        ++stats.tx_count;
        stats.tx_bytes += sizeof(radar_telemetry_t);
                
        /* local stats */
        ++send_count;
        byte_count += sizeof(radar_telemetry_t);
}


/*
 * radar_send_multiframe() - send several Extended Squitter frames in a single UDP/IP message for improved efficiency
 */
void radar_send_multiframe(void)
{
        if (num) {
                uint8_t buf[1024];
                uint8_t *bp = buf;
                uint64_t ts = ustime();
                int i, sz;

                if (debug)
                        printf("radar_send_multiframe(): num=%d\n", num);

                memset(&buf, 0, sizeof(buf));
        
                memcpy(bp, &key, sizeof(key));				/* API key */
                bp += sizeof(key);
                
                memcpy(bp, &ts, sizeof(ts));				/* time stamp */
                bp += sizeof(ts);
                
                memcpy(bp, &seq, sizeof(seq));				/* sequence number */
                bp += sizeof(seq);
                seq++;
                
                *bp++ = RADAR_OPCODE_MULTIFRAME;			/* opcode */

                *bp++ = (uint8_t)num;					/* item count */
                
                for (i=0; i<num; ++i) {					/* copy ES messages to buffer */
                        memcpy(bp, &esdata[i].mlat, MLAT_LEN);
                        bp += MLAT_LEN;
                        
                        *bp++ = esdata[i].rssi;
                        
                        memcpy(bp, &esdata[i].data, MODE_ES_LEN);
                        bp += MODE_ES_LEN;
                }

                /* size to be signed/auth tagged */
                sz = bp - buf;

                /* add auth tag */
                authtag_sign(bp, AUTHTAG_LEN, &buf, sz);
                
                /* bump size to include the auth tag */
                sz += AUTHTAG_LEN;

                /* send to aggregator */
                udp_send(&buf, sz);

                /* stats for aggregator */
                ++stats.tx_mode_multi;
                ++stats.tx_count;
                stats.tx_bytes += sz;
                
                /* local stats */
                ++send_count;
                byte_count += sz;

                /* reset buffer */
                clear_buffer();
        }
}


/*
 * radar_process() - process a radar message from BEAST input
 */
void radar_process(uint8_t mlat[MLAT_LEN], uint8_t rssi, uint8_t *data, int len)
{
        if (len == MODE_ES_LEN) {						/* Mode-S Extended message (14 bytes) */
                uint8_t df = data[0] >> 3;					/* downlink format */

                if ( (df >= 17 && df <= 22) || everything ){
                        int dupe;
                        
                        dupe = dupe_check_es(data);				/* duplicate check */
                        
                        if (dupe) {
                                ++dupe_es_count;
                                ++stats.dupe_es;
                                ++stats.dupes;

                        } else {
                        
                                if (multiframe) {
                                        /* 
                                         * in multiframe mode we store ES data here and send when we have either
                                         * reached the buffer limit or the multiframe forwarding timeout
                                         */
                                        memcpy(&esdata[num].mlat, mlat, MLAT_LEN);
                                        esdata[num].rssi = rssi;
                                        memcpy(&esdata[num].data, data, MODE_ES_LEN);
                                        
                                        ++num;

                                        if (num >= RADAR_MAX_MULTIFRAME)	/* buffer full? send now */
                                                radar_send_multiframe();
                                                
                                } else {
                                        radar_mode_es_t buf;

                                        memcpy(buf.mlat, mlat, MLAT_LEN);
                                        buf.rssi = rssi;
                                        memcpy(buf.data, data, MODE_ES_LEN);

                                        send_mode_es(&buf);
                                }
                        }
                }
                
                ++stats.rx_mode_es;
                ++stats.rx_df[df];
                
        } else if (len == MODE_SS_LEN) {					/* Mode-S Short message (7 bytes) */
                uint8_t df = data[0] >> 3;					/* downlink format */

                if (send_ss) {
                        int dupe;
                
                        dupe = dupe_check_ss(data);

                        if (dupe) {
                                if (debug > 2)
                                        printf("radar_process(): not sending duplicate SS\n");
                                
                                ++dupe_ss_count;
                                ++stats.dupe_ss;
                                ++stats.dupes;

                        } else {
                                radar_mode_ss_t buf;

                                memcpy(buf.mlat, mlat, MLAT_LEN);          /* copy over MLAT */
                                buf.rssi = rssi;                                /* copy RSSI */
                                memcpy(buf.data, data, MODE_SS_LEN);	/* Short squitter */
                         
                                send_mode_ss(&buf);
                        }
                }

                ++stats.rx_mode_ss;
                ++stats.rx_df[df];
        
        } else if (len == MODE_AC_LEN) {

                if (send_ac) {
                        radar_mode_ac_t buf;

                        memcpy(buf.mlat, mlat, MLAT_LEN);			/* copy over MLAT */
                        buf.rssi = rssi;					/* copy RSSI */
                        memcpy(buf.data, data, MODE_AC_LEN);			/* Mode-A/C short */
                         
                        send_mode_ac(&buf);
                }

                ++stats.rx_mode_ac;
        }
}


/*
 * house_keeping() - called once per second from a timer
 */
static void house_keeping(void)
{
        /* clean duplicates */
        dupe_clean();
                        
        /* if we've sent no packets in the last second, send a keep alive */
        if (send_count == 0)
                radar_send_keepalive();
                        
        /* restart UDP as a result of SIGHUP */
        if (restart) {
                udp_reset();
                restart = 0;
        }

        /* Beast housekeeping */
        beast_second();

        /* UDP housekeeping */
        udp_second();

        /* foreground stats */
        if (dostats) {
                printf("Packets forwarded: %3u   Not forwarded (dupes): %3u  Bytes per second: %5u\n", send_count, dupe_ss_count+dupe_es_count, byte_count);
        }

        /* clear the per-second stats */                                
        send_count = dupe_ss_count = dupe_es_count = byte_count = 0;
                                
        /* do radio stats and device telemetry */
        stats_second();
        telemetry_second();
}


/*
 * main program
 */
int main(int argc, char *argv[])
{
        int rc;
        int timer_fd = 0;
        int forward_fd = 0;
        uint8_t dummybuf[8];

        struct itimerspec spec_second = {		/* 1 second timer for housekeeping */
                { 1, 0 },
                { 1, 0 }
        };
      

        /*
         * catch signals
         */
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGHUP, signal_handler);

        
        /*
         * parse command line args
         */
        while ((rc = getopt(argc, argv, "k:l:r:h:p:u:g:s:t:q:S:P:i:n:mebBGfvdcyxh?")) >= 0) {
                switch (rc) {

                case 'b':
                        protocol = RADAR_PROTOCOL_BEAST_TCP;                
                        break;

                case 'B':
                        protocol = RADAR_PROTOCOL_BEAST_SERIAL;
                        break;

                case 'G':
                        protocol = RADAR_PROTOCOL_GNS_SERIAL;
                        break;

                case 'S':
                        strncpy(serport, optarg, BEAST_SERIAL_PORT_NAME);
                        break;

                case 'k':
                        if (strlen(optarg) > APIKEY_LEN) {
                                qerror("radar: API key too long\n");
                        }
                        key = (uint64_t)strtoll(optarg, NULL, 16);
                        ++gotkey;
                        break;

                case 'p':
                        if (strlen(optarg) > PSK_LEN) {
                                qerror("radar: pass-phrase (PSK) too long (max %d chars)\n", PSK_LEN);
                        }
                        strncpy(psk, optarg, PSK_LEN);
                        break;

                case 'r':
                case 'l':
                        if (strlen(optarg) > HOSTNAME_LEN) {
                                qerror("radar: IP address of server too long\n");
                        }
                        strncpy(localaddress, optarg, HOSTNAME_LEN);
                        break;

                case 'h':
                        if (strlen(optarg) > HOSTNAME_LEN) {
                                qerror("radar: remote hostname too long\n");
                        }
                        strncpy(hostname, optarg, HOSTNAME_LEN);
                        break;

                case 'c':
                        ++send_ac;
                        break;
                        
                case 'y':
                        ++send_ss;
                        break;

                case 'e':
                        ++everything;
                        break;

                case 'm':
                        ++multiframe;
                        break;

                case 'i':
                        forward_interval = atoi(optarg);
                        if (forward_interval < 10 || forward_interval > 250)
                                qerror("radar: multiframe forwarding interval must be in range 10-250mS\n");
                        break;

                case 't':
                        telemetry_interval = atoi(optarg);
                        break;

                case 's':
                        stats_interval = atoi(optarg);
                        break;

                case 'd':
                        isdaemon++;
                        break;
                        
                case 'f':
                        ++dostats;
                        break;

                case 'u':
                        strncpy(username, optarg, USERNAME_LEN);
                        break;

                case 'g':
                        strncpy(groupname, optarg, GROUPNAME_LEN);
                        break;
                
                case 'q':
                        qos = atoi(optarg);
                        if (qos > 63 || qos < 0)
                                qerror("radar: QoS value must in in range 0-63\n");
                        break;
                
                case 'x':
                        debug++;
                        break;

                case 'P':
                        port =(uint16_t)(atoi(optarg));
                        break;

                case 'n':
                        rebind = atoi(optarg);
                        if (rebind > 3600 || rebind < 0)
                                qerror("radar: rebind interval must in in range 0-3600 seconds\n");
                        if (debug)
                                printf("radar: rebind interval = %d seconds\n", rebind);
                        break;

                case 'v':
                        puts(banner());
                        exit(0);
                        break;

                case '?':
                        fputs(banner(), stdout);
                        printf("usage: radar [options]\n\n");
                        printf("  -k <key>           : sharing key (identity) of this receiver station\n");
                        printf("  -h <hostname>      : hostname of central aggregator\n");
                        printf("  -p <psk>           : pre-shared key for HMAC authentication (signing of messages)\n");
                        printf("  -B                 : use Mode-S BEAST via USB connection\n");
                        printf("  -G                 : use GNS 5892/5894T HULC via serial connection\n");
                        printf("  -c                 : enable sending Mode-A/C message (not recommended)\n");
                        printf("  -y                 : enable sending Mode-S Short messages (not recommended)\n");
                        printf("  -e                 : forward everything\n");
                        printf("  -l <ip addr>       : IP address of local dump1090/readsb server (default: 127.0.0.1)\n");
                        printf("  -P <port>          : TCP port number to connect to Beast on (default: 30005)\n");
                        printf("  -m                 : Enable multiframe sending (more efficient but more latency)\n");
                        printf("  -i <ms>            : Forwarding interval in milliseconds for multiframe (range 10-250, default 50)\n");
                        printf("  -s <seconds>       : Set the radio stats interval (default 900)\n");
                        printf("  -t <seconds>       : Set the telemetry interval (default 900)\n");
                        printf("  -d                 : run as daemon (detach from controlling tty)\n");
                        printf("  -f                 : print forwarding stats once per second\n");
                        printf("  -u <uid|username>  : set the UID or username for the process\n");
                        printf("  -g <gid|group>     : set the GID or group name for the process\n");
                        printf("  -q <qos>           : set the DSCP/IP ToS quality of service\n");
                        printf("  -n <seconds>       : time before re-binding UDP socket source port (CGNAT work-around)\n");
                        printf("  -v                 : display version information and exit\n");
                        printf("  -x|xx|xxx          : set debug level\n");
                        printf("  -S <serial port>   : specify serial port for Mode-S Beast connection (default: /dev/ttyUSB0)\n");
                        printf("  -?                 : help (this output)\n");
                        printf("\n");
                        exit(0);
                        break;
                }
        }


        /*
         * check params and sanity
         */

        if (!protocol)
                protocol = RADAR_PROTOCOL_BEAST_TCP;

        if (!gotkey || key == 0)
                qerror("radar: must specify your API key with -k <key>\n");

        if (isdaemon && debug)
                qerror("radar: cannot perform debug in background\n");
        
        if (isdaemon && dostats)
                qerror("radar: cannot output stats in background\n");

        
        /*
         * if process is root drop privs
         */
        if (getuid() == 0) {
                uid_t uid;
                gid_t gid;

                /*
                 * username is taken from the "-u <uid|user>" argument or program name and then converted to numeric UID
                 */
                uid = get_uid(username);

                /*
                 * group name is taken from the "-g <gid|group>" argument or from the fixed "nogroup" name and then converted to numeric GID
                 */
                gid = get_gid(groupname);

                /*
                 * drop to new group id - if it exists
                 *
                 * IMPORTANT: must drop group priv before dropping user priv - it doesn't work the other way round!
                 */
                if (gid) {
                        if (setgid(gid) == 0) {
                                ;
#if 0
                                if (!isdaemon)
                                        printf("radar: setgid(): set GID to %d\n", gid);
#endif
                        } else {
                                qerror("radar: setgid(): unable to drop group privileges to gid: %d - %s (%d)\n", gid, strerror(errno), errno);
                        }
                } else {
                        qerror("radar: setgid(): unable to drop group privileges to gid: %s - GID does not exist\n", groupname);
                }

                /*
                 * drop to new user id
                 */
                if (uid) {
                        if (setuid(uid) == 0) {
                                ;
#if 0
                                if (!isdaemon)
                                         printf("radar: setuid(): set UID to %d\n", uid);
#endif
                        } else {
                                qerror("radar: setuid(): Unable to drop user privileges to uid: %d - %s (%d)\n", uid, strerror(errno), errno);
                        }
                } else {
                        qerror("radar: setgid(): unable to drop group privileges to uid: %s - UID does not exist\n", username);
                }

                /*
                 * security check - should not be able to setuid() back to root!
                 */
                if (setuid(0) != -1)
                        qerror("radar: setuid(): Security failure - was able to setuid back to 'root'\n");
        }

        /*
         * initialise stats gathering (must do before starting beast connection)
         */
        stats_init(stats_interval);
        telemetry_init(telemetry_interval);

        /*
         * initialise authentication key
         */
        authtag_init(psk);

        /*
         * initialise the UDP sub-system
         */
        udp_init(hostname, qos, rebind);

        /*
         * run in background
         */
        if (isdaemon) {
                /* use the GNU method of detaching from the controlling tty */
                if (daemon(0, 0) < 0)
                        qerror("radar: error calling daemon(): %s (%d)\n", strerror(errno), errno);
        }

        /*
         * initialise the appropriate ADS-B protocol source
         */
        switch (protocol) {

                case RADAR_PROTOCOL_BEAST_TCP:
                        beast_tcp_init(localaddress, port);
                        if (dostats)
                                printf("Using BEAST over TCP on %s:%d (preferred)\n", localaddress, BEAST_TCP_PORT);
                        break;
                
                case RADAR_PROTOCOL_BEAST_SERIAL:
                        beast_serial_init(serport, B3000000);
                        if (dostats)
                                printf("Using Mode-S BEAST over serial/USB on device: %s speed: 3Mbps\n", serport);
                        break;

                case RADAR_PROTOCOL_GNS_SERIAL:
                        beast_serial_init(serport, B921600);
                        if (dostats)
                                printf("Using GNS/HULC/BEAST over USB/serial on device: %s speed: 921600bps\n", serport);
                        break;

                default:
                        qerror("Unsupported client protocol");
                        break;
        }

        /*
         * start the housekeeping timer
         */
        timer_fd = timerfd_create(CLOCK_REALTIME,  0);

        if (!timer_fd)
                qerror("unable to create timer!");

        timerfd_settime(timer_fd, 0, &spec_second, NULL);


        /*
         * if using multiframe clear the buffer and start the forwarding timer
         */
        if (multiframe) {
#if 0
                struct itimerspec spec_forward = {		/* radar multi-frame forwarding intervl */
                        { 0, RADAR_FORWARD_INTERVAL },
                        { 0, RADAR_FORWARD_INTERVAL }
                };
#endif
                struct itimerspec spec_forward;
                long nsec = forward_interval * 1000000;

                spec_forward.it_interval.tv_sec = 0;
                spec_forward.it_interval.tv_nsec = nsec;
                spec_forward.it_value.tv_sec = 0;
                spec_forward.it_value.tv_nsec = nsec;

                forward_fd = timerfd_create(CLOCK_REALTIME,  0);

                if (!forward_fd)
                        qerror("unable to create timer!");

                timerfd_settime(forward_fd, 0, &spec_forward, NULL);

                clear_buffer();
        }

        /*
         * forward traffic ...
         */
        do {
                struct pollfd fds[3];
                int nfds = 2;
                int rc;
        
                /* watch house-keeping timer */
                fds[0].fd = timer_fd;
                fds[0].events = POLLIN;

                /* watch forwarding timer */
                fds[1].fd = forward_fd;
                fds[1].events = (multiframe) ? POLLIN : 0;

                /* watch for input, hangups and errors from Beast connection, if active */
                if (beast_fd) {
                        fds[2].fd = beast_fd;
                        fds[2].events = POLLIN|POLLHUP|POLLERR;
                        ++nfds;
                }

                /*
                 * perform poll() for IO status and decode result:
                 *
                 *      >0 : one or more file descriptors has an event
                 *       0 : timeout waiting for event
                 *      <0 : an error occurred - consult errno for reason
                 *
                 */
again:
                rc = poll(fds, nfds, 250);

                if (rc > 0) {
                        /*
                         * poll() input available
                         */
                         
                        /* check house-keeping timer */
                        if (fds[0].revents & POLLIN) {
                                rc = read(timer_fd, dummybuf, 8);
                                rc = rc;
                                house_keeping();
                        }

                        /* check fast forwarding timer (for multframe) */
                        if (multiframe && (fds[1].revents & POLLIN)) {
                                rc = read(forward_fd, dummybuf, 8);
                                rc = rc;
                                
                                if (num)
                                        radar_send_multiframe();
                        }

                        /* check for beast data available and errors */
                        if (beast_fd) {
                                if (fds[2].revents & POLLIN) {
                                        beast_read();
                                } else if (fds[2].revents & POLLHUP || fds[2].revents & POLLERR) {
                                        beast_reset_connection();
                                }
                        }

                } else if (rc == 0) {
                        /*
                         * poll() timed out … nothing to do
                         */
#if 0
                        if (debug)
                                printf("Poll() timeout\n");
#endif
                        ;

                } else {
                        /*
                         * poll error
                         */
                         
                        /* if it was 'interupted system call' then it was a signal - carry on */
                        if (errno == EINTR) {
                                goto again;
                        } else {
                                /* anything else then error exit */
                                qerror("poll() error: %s (%d)\n", strerror(errno), errno);
                        }
                }

        } while (!ending);

        /*
         * clean up and finish
         */
        cleanup();
        exit(EXIT_SUCCESS);
}

