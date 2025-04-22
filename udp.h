/*
 * udp.h -- UDP routines for sending and managing data connection
 * Author: Mike Tubby G8TIC mike@tubby.org  14th April 2025
 */

#ifndef _UDP_H
#define _UDP_H

#define UDP_HOST		"adsb-in.1090mhz.uk"	/* default host */
#define UDP_PORT		5997			/* if not specified */
#define UDP_RETRY		3			/* retry timer in seconds */


/*
 * enumerated list of UDP states
 */
enum udpstate {
        UDP_IDLE,					/* idle state at start-up and after failure/retry */
        UDP_WAIT_LOOKUP,				/* waiting for DNS look-up to complete */
        UDP_WAIT_CONNECT,				/* wait for socket to be connect()-ed */
        UDP_CONNECTED,					/* Normal run state */
        UDP_RETRY_WAIT					/* Something failed - waiting to restart */
};

/*
 * exported functions
 */
void udp_init(char *, int, int);
void udp_close(void);
void udp_second(void);
void udp_send(void *, int);
void udp_reset(void);

#endif
