/*
 * telemetry.h -- Telemetry (statistics) about the receiving station
 */

#ifndef _TELEMETRY_H
#define _TELEMETRY_H

#include <stdint.h>
#include <sys/utsname.h>

#include "defs.h"

#define TELEMETRY_INTERVAL	900			/* fifteen minutes */


/*
 * telemetry structure
 */
typedef struct {
        uint32_t start;					/* start-up time for radar software */
        uint32_t now;

        /*
         * Posix UTS name items for Kernel/OS
         */
        char sysname[_UTSNAME_SYSNAME_LENGTH];          /* OS name */
        char nodename[_UTSNAME_NODENAME_LENGTH];        /* Node name */
        char release[_UTSNAME_RELEASE_LENGTH];          /* Release level */
        char version[_UTSNAME_VERSION_LENGTH];          /* Version level of release */
        char machine[_UTSNAME_MACHINE_LENGTH];          /* Machine hardware */

        /*
         * Processor items
         */
        uint8_t cpu_arch;				/* CPU architecture */
        uint8_t cpu_count;				/* Number of CPUs */

        /*
         * system performance and memory
         */
        uint32_t uptime;				/* uptime for system (seconds) */
        uint16_t procs;					/* number of processes */
        uint16_t load[3];				/* CPU load average x 100 */
        uint16_t cpu_temp;				/* CPU temperature in degrees x 10 */
        uint16_t mem_total;				/* total memory (MB) */
        uint16_t mem_free;				/* free memory (MB) */
        uint16_t mem_shared;				/* shared memory (MB) */
        uint16_t mem_cache;				/* cache/buffer memory (MB) */
        uint16_t swap_total;				/* total swap size (MB) */
        uint16_t swap_free;        			/* free swap (MB) */

        /*
         * GCC version used to build code
         */
        uint8_t gcc_major;				/* GCC compiler version */
        uint8_t gcc_minor;
        uint8_t gcc_patch;

        /*
         * Glibc version
         */
        uint8_t glibc_major;
        uint8_t glibc_minor;

        /*
         * Size of data items these a useful architecture cross checks
         */
        uint8_t sizeof_pointer;
        uint8_t sizeof_short;
        uint8_t sizeof_int;
        uint8_t sizeof_long;
        uint8_t sizeof_long_long;
        uint8_t sizeof_time_t;

        /*
         * radar software version
         */
        uint8_t version_major;				/* Radar software version */
        uint8_t version_minor;
        uint8_t version_patch;

        /*
         * radar software performance items
         */
        uint8_t protocol;				/* Protocol: 0:none 1:BEAST 2:AVR */
        uint32_t connect_success;
        uint32_t connect_fail;
        uint32_t disconnect;
        uint32_t socket_error;
        uint32_t socket_reads;
        uint32_t bytes_read;
        uint32_t frames_good;
        uint32_t frames_bad;
        uint16_t packets_per_second;			/* packets per second */

} __attribute__((packed)) telemetry_t;


extern telemetry_t telemetry;


/*
 * exported functions
 */
void telemetry_init(int);
void telemetry_second(void);
void telemetry_send(void);

#endif

