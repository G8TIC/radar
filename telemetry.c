/*
 * telemetry.c -- statistics gathering about operation of the receiver
 * Author: Michael J. Tubby B.Sc. MIET    mike@tubby.org
 *
 *
 * ABSTRACT
 *
 * Collect and report information about the operating system, environment and
 * operation of the radar software to better understand system performance and
 * determine bugs or failure modes with our software, in line with our policy
 * of continued improvement but while remaining 
 *
 *
 * WHAT WE COLLECT
 *
 * We collect functional and operational information about the receriver system
 * and its environment such as the kernel version, the version og GCC used to
 * compile the software, the status of system memory, processor family, CPU
 * temperature and load averages.
 *
 * Please refer to the data structure in telemetry.h for full details.
 * PRIVACY CONCERNS
 *
 * We do not collect any Personally Identifiable Information (PII) or information
 * about your network - for example we do not collect:
 *
 *	any IP addresses or details of other computers
 *	usernames or passwords
 *	documents, files, photos
 *
 * or any information or data that does not concern us.
 *
 */

#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <gnu/libc-version.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "version.h"
#include "radar.h"
#include "beast.h"
#include "avr.h"
#include "arch.h"
#include "telemetry.h"

#define MB			(1024*1024)

#define GCC_MAJOR		__GNUC__
#define GCC_MINOR		__GNUC_MINOR__
#define GCC_PATCH		__GNUC_PATCHLEVEL__

#define MAX_THERMAL_ZONE	15
#define X86_THERMAL_NAME	"x86_pkg_temp"
#define ARM_THERMAL_NAME	"cpu-thermal"


extern int debug;
extern int protocol;
 
telemetry_t telemetry;
static int interval;
static int countdown;
static char path[64];
static FILE * tempf = NULL;


/*
 * find_thermal_zone() - auto-detect the thermal zone based on it's type name
 */
static int find_thermal_zone(void)
{
        FILE * fin;
        int i;

        for (i = 0; i <= MAX_THERMAL_ZONE; i++) {
                char path[50];
                char buf[20];
                
                sprintf(path, "/sys/class/thermal/thermal_zone%d/type", i);
                
                fin = fopen(path, "r");
                
                if (fin) {
                        int rc;
                        
                        rc = fread(buf, sizeof(buf), 1, fin);
                        rc = rc;					/* hack since fread on sysfs returns zero */
                        
                        /* use string comparison with size to ignore the newline on the end of the name returned by fread() */                
                        if (strncmp(buf, ARM_THERMAL_NAME, strlen(ARM_THERMAL_NAME)) == 0 || strncmp(buf, X86_THERMAL_NAME, strlen(X86_THERMAL_NAME)) == 0) {
                                fclose(fin);
                                return i;                
                        }
                                        
                        fclose(fin);
                }
        }
        
        return -1;
}


/*
 * setup_temp_zone() - use the user-provide zone number, if provided or auto-detect
 */
static void setup_temp_zone(void)
{
        int zone = find_thermal_zone();

        if (zone >= 0) {
                sprintf(path, "/sys/class/thermal/thermal_zone%d/temp", zone);
                tempf = fopen(path, "r");
        }
}


/*
 * cpu_temp() - return CPU temperature in deci-degrees (degrees-C x 10)
 * as a short integer.
 */
static uint16_t cpu_temp(void)
{
        if (tempf) {
                int tmp;
        
                rewind(tempf);
                
                if (fscanf(tempf, "%d", &tmp) > 0)
                        return (uint16_t)(tmp /100);
        }
        
        return 0;
}


/*
 * telemetry_update() - send a telemetry update to the aggregator
 */
void telemetry_update(void)
{
        struct sysinfo info;
        int rc;
        
        rc = sysinfo(&info);        

        if (rc == 0) {

                /* current time */
                telemetry.now = (uint32_t)time(NULL);

                /* system uptime in seconds */
                telemetry.uptime = info.uptime;

                /* report memory utilisation in megabytes */
                telemetry.mem_total = info.totalram / MB;
                telemetry.mem_free = info.freeram / MB;
                telemetry.mem_shared = info.sharedram / MB;
                telemetry.mem_cache = info.bufferram / MB;
                telemetry.swap_total = info.totalswap / MB;
                telemetry.swap_free = info.freeswap / MB;
        
                /* report number of processes */
                telemetry.procs = info.procs;

                /* report load averages usinging integer approximation */
                telemetry.load[0] = info.loads[0] / 655;
                telemetry.load[1] = info.loads[1] / 655;
                telemetry.load[2] = info.loads[2] / 655;

                /* report CPU temperature (deci-degrees) */
                telemetry.cpu_temp = cpu_temp();

                /* has to be here for reasons of ordering */
                telemetry.protocol = protocol;
        
                /* send telemetry */
                radar_send_telemetry();
        }
}


/*
 * telemetry_init() - initialise telemetry transmission if we have a non-zero interval
 *
 * zone allows us to pass the thermal zone number
 *
 */
void telemetry_init(int ival)
{
        if (ival) {
                /* initialise telemetry by getting constants */
                struct utsname uts;
                int rc;

                memset(&telemetry, 0, sizeof(telemetry_t));

                setup_temp_zone();
        
                rc = uname(&uts);
                
                if (rc == 0) {
                        memcpy(telemetry.sysname, uts.sysname, _UTSNAME_SYSNAME_LENGTH);
                        memcpy(telemetry.nodename, uts.nodename, _UTSNAME_NODENAME_LENGTH);
                        memcpy(telemetry.release, uts.release, _UTSNAME_RELEASE_LENGTH);
                        memcpy(telemetry.version, uts.version, _UTSNAME_VERSION_LENGTH);
                        memcpy(telemetry.machine, uts.machine, _UTSNAME_MACHINE_LENGTH);
                }

                telemetry.start = (uint32_t)time(NULL);

                telemetry.version_major = VERSION_MAJOR;
                telemetry.version_minor = VERSION_MINOR;
                telemetry.version_patch = VERSION_PATCH;
       
                telemetry.gcc_major = GCC_MAJOR;
                telemetry.gcc_minor = GCC_MINOR;
                telemetry.gcc_patch = GCC_PATCH;

                telemetry.glibc_major = __GLIBC__;
                telemetry.glibc_minor = __GLIBC_MINOR__;

                telemetry.cpu_arch = arch_type();
                telemetry.cpu_count = get_nprocs();
                
                telemetry.sizeof_pointer = sizeof(void *);
                telemetry.sizeof_short = sizeof(short);
                telemetry.sizeof_int = sizeof(int);
                telemetry.sizeof_long = sizeof(long);
                telemetry.sizeof_long_long = sizeof(long long);
                telemetry.sizeof_time_t = sizeof(time_t);
                
                interval = ival;
               
                countdown = 10;				/* send first telemetry after 10 seconds */

        } else {
                /* no telemetry */
                countdown = 0;
        }
}


/*
 * telemetry_second() - run the telemetry from house-keeping
 */
void telemetry_second(void)
{
        if (countdown) {
                --countdown;
                
                if (!countdown) {
                        telemetry_update();
                        countdown = interval;
                }
        }
}


void telemetry_close(void)
{
        fclose(tempf);
}
