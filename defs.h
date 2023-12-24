/*
 * defs.h -- Definitions of constants used in various places
 */

#ifndef _DEFS_H
#define _DEFS_H 

#define USERNAME_LEN			32		/* Max size of a unix username */
#define GROUPNAME_LEN			32		/* Max size of a unix group name */
#define HOSTNAME_LEN			64		/* Maximum size of a hostname */
#define APIKEY_LEN			18		/* Size of an API key - in ASCII - with 0x prefix */
#define PSK_LEN				64		/* Maximum size of a pre-shared key */

#define MODE_AC_LEN			2		/* Length of Mode-A/C message in bytes */
#define MODE_SS_LEN			7		/* Length of Mode-S Short message in bytes */
#define MODE_ES_LEN			14		/* Length of a Mode-S Extended Squitter in bytes */
#define MLAT_LEN			6		/* Length of the MLAT data in bytes */
#define MAX_DF				32		/* Number of Downlink Formats */

#endif
