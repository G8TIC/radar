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
 *	-a                force use of legacy AVR protocol otherwise we use BEAST
 *	-r <ipaddr>	  address of device that provides ADS-B source if not localhost
 *	-e <level>        set the level of Extended Squitter sent
 *	-u <user>	  user name to run under, e.g. 'nobody'
 *	-g <group>	  group name to run under, e.g. 'nogroup'
 *	-q <qos>	  IP Quality of Service using DiffServe values 0-63
 *	-d		  daemonise use this for SysV init systems (systemd does this)
 *	-x|xx|xxx	  run with debug (-xx for more debug)
 *	-f		  produce forwarding stats one per second (foreground only)
 *	-s <seconds>	  send radio channel stats every period (default 900 = 15 min)
 *	-t <seconds>	  send system telemetry every period (default 900 = 15 min)
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
 * Sharing keys are represented as 16 hex nibbles in uppercase with the leacing 0x - for
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
 * the originator and we know the message cannot have been* spoofed either.
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

#include "radar.h"
#include "chain.h"
#include "banner.h"
#include "xtimer.h"
#include "avr.h"
#include "beast.h"
#include "beast_tcp.h"
#include "beast_serial.h"
#include "dupe.h"
#include "authtag.h"
#include "ustime.h"
#include "mstime.h"
#include "hex.h"
#include "qerror.h"

#define TIMER_INTERVAL		100L				/* 100 milliseconds */


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
int gotkey = 0;
int send_es = 2;						/* Send DF17, 18, 19, 20 and 21 by default */
int send_ss = 0;
int send_ac = 0;
int stats_interval = STATS_INTERVAL;
int telemetry_interval = TELEMETRY_INTERVAL;
uint64_t key;
char hostname[HOSTNAME_LEN+1] = "adsb-in.1090mhz.uk";
char psk[PSK_LEN+1] = "secret";
char localaddress[HOSTNAME_LEN+1] = "127.0.0.1";
uint32_t seq = 1;
int udpfd;
xtimer_t timer;
char username[USERNAME_LEN+1] = "nobody";
char groupname[GROUPNAME_LEN+1] = "nogroup";
int max_lookups = 0;
int qos = 0;
stats_t stats;							/* stats for reporting to central aggregator */
uint32_t dupe_ss_count = 0;
uint32_t dupe_es_count = 0;
uint32_t send_count = 0;
uint32_t byte_count = 0;
char serport[BEAST_SERIAL_PORT_NAME+1] = "/dev/ttyUSB0";


/*
 * cleanup() - the mess we've made
 */
void cleanup(void)
{
        /* close connection */
        switch (protocol) {
                case RADAR_PROTOCOL_BEAST_TCP:
                        beast_tcp_close();
                        break;
                
                case RADAR_PROTOCOL_AVR:
                        avr_close();
                        break;
                        
                case RADAR_PROTOCOL_BEAST_SERIAL:
                case RADAR_PROTOCOL_GNS_SERIAL:
                        beast_serial_close();
                        break;
        }

        /* close down UDP */
        if (udpfd) {
                fsync(udpfd);
                close(udpfd);
                udpfd = 0;
        }
}


/*
 * Signal handler
 */
void signal_handler(int signal)
{
        switch (signal) {
                case SIGHUP:
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


#if 0
/*
 * radar_send() - Send an arbitary message to the aggregator
 */
void radar_send(uint8_t opcode, uint8_t *data, int len)
{
        int size = sizeof(radar_msg_t) + len + AUTHTAG_LEN;			/* message size */
        radar_msg_t * mp = malloc(size);					/* buffer for message */
                
        if (mp) {
                uint8_t *p = mp->data;						/* point to start of data area */
                
                memset(mp, 0, size);						/* start clean */

                mp->key = key;							/* API key */
                mp->ts = ustime();						/* timestamp uS */
                mp->seq = seq++;						/* sequence number */

                mp->opcode = opcode;						/* copy opcode */
                
                memcpy(p, data, len);						/* copy data */
                p += len;
                        
                authtag_sign(p, AUTHTAG_LEN, (void *)mp, size-AUTHTAG_LEN);	/* add auth tag */
                p += AUTHTAG_LEN;
                
                send(udpfd, mp, size, 0);					/* send message */
                        
                free(mp);
                
                ++stats.sent;
        } else {
                qabort("radar_send(): ran out of memory!\n");
        }
}
#endif



/*
 * udp_send() - send a UDP/IP message to the aggregator
 */
static void udp_send(void *buf, int size)
{
        /* send to aggregator */                	
        send(udpfd, buf, size, 0);						/* send message */

        /* update stats */
        ++stats.tx_count;
        stats.tx_bytes += size;
#if 1
        /* debug dump */
        if (debug)
                hex_dump("UDP", buf, size);
#endif
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
        
        memcpy(msg.data, "HELLO", 5);
        
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
        ++stats.tx_stats;
        ++stats.tx_count;
        stats.tx_bytes += sizeof(radar_telemetry_t);
                
        /* local stats */
        ++send_count;
        byte_count += sizeof(radar_telemetry_t);
}


/*
 * radar_process() - process a radar message from AVR or BEAST input
 */
void radar_process(uint8_t mlat[MLAT_LEN], uint8_t rssi, uint8_t *data, int len)
{
        if (len == MODE_ES_LEN) {						/* Mode-S Extended message (14 bytes) */
                uint8_t df = data[0] >> 3;					/* downlink format */

                //printf("radar_process(): df=%d size=%d\n", df, len);

                /*
                 * forward of downlink formats set by the -e <level> option on the command line
                 *
                 * Level-0 -> Send nothing
                 *
                 * Level-1 (default) -> Send DF17, DF18, DF19:
                 *
                 *    DF17:    Extended Squitter
                 *    DF18:    Extended Squitter, Supplementary
                 *    DF19:    Military Extended Squitter
                 *
                 * Level-2 -> Send DF17, DF18, DF19 *plus* DF20, DF21, DF22 :
                 *
                 *    DF20/21: Comm. B Altitude and Ident Reply
                 *    DF22:    Military use (undefined)
                 *
                 * Level-3 -> Send DF17-19, DF20-22 *plus* DF16
                 *
                 * Level-4 -> Send everything (all Extended Squitter, current and future)
                 *
                 */                 
                if ( (send_es >= 1 && (df >= 17 && df <= 19)) || (send_es >= 2 && (df >= 20 && df <= 22)) || (send_es >= 3 && df == 16) || (send_es >= 4) ){
                        int dupe;
                        
                        dupe = dupe_check_es(data);
                        
                        if (dupe) {
                                if (debug > 2)
                                        printf("radar_process(): not sending duplicate ES\n");

                                ++dupe_es_count;
                                ++stats.dupe_es;
                                ++stats.dupes;

                        } else {
                                radar_mode_es_t buf;

                                memcpy(buf.mlat, mlat, MLAT_LEN);		/* copy over MLAT */
                                buf.rssi = rssi;				/* copy RSSI */
                                memcpy(buf.data, data, MODE_ES_LEN);		/* extended squitter */
                                
                                send_mode_es(&buf);
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
                        memcpy(buf.data, data, MODE_AC_LEN);		/* Mode-A/C short */
                         
                        send_mode_ac(&buf);
                }

                ++stats.rx_mode_ac;
        }
}


/*
 * main program
 */
int main(int argc, char *argv[])
{
        struct hostent *hostinfo;
        struct sockaddr_in src, dst;
        int rc, i;

        /*
         * catch signals
         */
        signal(SIGINT,  signal_handler);
        signal(SIGTERM, signal_handler);

        /*
         * parse command line args
         */
        while ((rc = getopt(argc, argv, "k:l:r:h:p:u:g:s:t:q:n:e:S:abBGfvdcyxh?")) >= 0) {
                switch (rc) {

                case 'a':
                        protocol = RADAR_PROTOCOL_AVR;
                        break;

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
                        send_es = atoi(optarg);
                        if (send_es >= 5 || qos < 0)
                                qerror("radar: Mode-S Extended Squitter level out of range (valid: 0-5)\n");
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
                
                case 'n':
                        max_lookups = atoi(optarg);
                        break;

                case 'x':
                        debug++;
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
                        printf("  -a                 : use AVR protocol rather than BEAST\n");
                        printf("  -b                 : use BEAST protocol\n");
                        printf("  -B                 : use Mode-S BEAST via USB connection\n");
                        printf("  -G                 : use GNS 5892/5894T HULC via serial connection\n");
                        printf("  -c                 : enable sending Mode-A/C message (not recommended)\n");
                        printf("  -y                 : enable sending Mode-S Short messages (not recommended)\n");
                        printf("  -e <level>         : control which Mode-S Extended Squitter DF codes are sent (default = 1)\n");
                        printf("  -l <ip addr>       : IP address of local dump1090/readsb server (default: 127.0.0.1)\n");
                        printf("  -s <seconds>       : Set the radio stats interval (default 900)\n");
                        printf("  -t <seconds>       : Set the telemetry interval (default 900)\n");
                        printf("  -d                 : run as daemon (detach from controlling tty)\n");
                        printf("  -f                 : print forwarding stats once per second\n");
                        printf("  -u <uid|username>  : set the UID or username for the process\n");
                        printf("  -g <gid|group>     : set the GID or group name for the process\n");
                        printf("  -q <qos>           : set the DSCP/IP ToS quality of service\n");
                        printf("  -n <number>        : number of DNS lookup attempts (default: infinite)\n");
                        printf("  -v                 : display version information and exit\n");
                        printf("  -x|xx|xxx          : set debug level\n");
                        printf("  -B                 : set protocol to Beast over serial (for Mode-S BEAST receiver)\n");
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
                qerror("radar: no protocol specified, use -a -b -B or -G");

        if (!gotkey || key == 0)
                qerror("radar: must specify your API key with -k <key>\n");

        if (isdaemon && debug)
                qerror("radar: cannot perform debug in background\n");
        
        if (isdaemon && dostats)
                qerror("radar: cannot output stats in background\n");

        if (send_es == 0)
                printf("Warning: Sending Mode-S Extended Squitter is suppressed - are you sure?\n");

        
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
         * Perform DNS look-up for destination host ...
         *
         * We may wait here for an extended time if a user has had a power cut and the Raspberry Pi has
         * rebooted before the broadband comes back online, hence try every 2 seconds indefinately - or
         * for a limited number of attempts if -n <number> is specified ...
         *
         */
        for (i = 0; ;i++) {
                hostinfo = gethostbyname(hostname);
    
                if (hostinfo) {
                        if (debug)
                                printf("Destination is %s (%s) port %u\n", hostinfo->h_name, inet_ntoa(*(struct in_addr*)hostinfo->h_addr), RADAR_PORT);
                        
                        break;
                } else {
                        if (debug)
                                printf("DNS lookup for %s failed ... retrying ...\n", hostname);
                        sleep(2);	                        
                }
                
                if (max_lookups && i >= max_lookups)
                        qerror("radar: error resolving hostname: %s  errno: %s (%d)\n", hostname, hstrerror(h_errno), h_errno);
        }

        /*
         * UDP socket connection.  We use a UDP socket connection for efficiency - this means we
         * lookup and set up the UDP session parameters in the kernel once and then just keep sending ...
         */

        /* make a UDP socket */
        if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                qerror("radar: could not create UDP socket\n");
        } 

        /* if QoS is specified then set it */
        if (qos) {
                const int iptos = qos << 2;		/* QoS in top 6-bits of the IP header */
                rc = setsockopt(udpfd, IPPROTO_IP, IP_TOS, &iptos, sizeof(iptos));
        
                if (rc == -1)
                        printf("setsockopt(): for UDP QoS returned: %s %d\n", strerror(errno), errno);
        }

        /* setup my end (source) */ 
        memset(&dst, 0, sizeof(src));
        src.sin_family = AF_INET;
        src.sin_addr.s_addr = htonl(INADDR_ANY);
        src.sin_port = htons(RADAR_PORT);

        /* setup their end (destination) */
        memset(&dst, 0, sizeof(dst));
        dst.sin_family = AF_INET;
        dst.sin_addr = *(struct in_addr*) hostinfo->h_addr;
        dst.sin_port = htons(RADAR_PORT);

        /* connect the socket */
        rc = connect(udpfd, (struct sockaddr*)&dst,sizeof(dst));

        if (rc < 0) {
                 qerror("radar: could not connect UDP socket\n");
        
        } else {
                if (debug)
                        printf("UDP socket connected\n");
        }

        /*
         * initialise stats gathering (must do before starting AVR or BEAST)
         */
        stats_init(stats_interval);
        telemetry_init(telemetry_interval);

        /*
         * initialise authentication key
         */
        authtag_init(psk);

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
                        beast_tcp_init(localaddress);
                        if (dostats)
                                printf("Using BEAST over TCP on %s:%d (preferred)\n", localaddress, BEAST_TCP_PORT);
                        break;
                
                case RADAR_PROTOCOL_AVR:
                        avr_init(localaddress);
                        if (dostats)
                                printf("Using AVR protocol on %s:%d (fallback)\n", localaddress, AVR_PORT);
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
        xtimer_start(&timer, TIMER_INTERVAL);

        /*
         * forward traffic ...
         */
        do {
                switch (protocol) {

                        case RADAR_PROTOCOL_BEAST_TCP:
                                beast_tcp_run();
                                break;
                
                        case RADAR_PROTOCOL_AVR:
                                avr_run();
                                break;
                        
                        case RADAR_PROTOCOL_BEAST_SERIAL:
                        case RADAR_PROTOCOL_GNS_SERIAL:
                                beast_serial_run();
                                break;
                }        


                if (xtimer_expired(&timer)) {			/* 100 mS timer */

                        dupe_clean();				/* clean duplicates */
                        
                        ++i;

                        if (i >= 10) {

                                /* if we've sent no packets in the last second, send a keep alive */
                                if (send_count == 0)
                                        radar_send_keepalive();
                        
                                /* do approprioate housekeeping */
                                switch (protocol) {
                                        case RADAR_PROTOCOL_BEAST_TCP:
                                        case RADAR_PROTOCOL_BEAST_SERIAL:
                                        case RADAR_PROTOCOL_GNS_SERIAL:
                                                beast_second();
                                                break;
                
                                        case RADAR_PROTOCOL_AVR:
                                                avr_second();
                                                break;
                                }        

                                /* foreground stats */
                                if (dostats) {
                                        printf("Packets forwarded: %3u   Not forwarded (dupes): %3u  Bytes per second: %5u\n", send_count, dupe_ss_count+dupe_es_count, byte_count);
                                }

                                /* clear the per-second stats */                                
                                send_count = dupe_ss_count = dupe_es_count = byte_count = 0;
                                
                                /* do radio stats and device telemetry */
                                stats_second();
                                telemetry_second();

                                /* reset counter */                                
                                i = 0;
                        }
                        
                        /* restart the 100mS interval timer */
                        xtimer_start(&timer, TIMER_INTERVAL);
                }

        } while (!ending);

        /*
         * clean up and finish
         */
        cleanup();
        exit(EXIT_SUCCESS);
}
