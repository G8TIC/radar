
#include <string.h>
#include <stdio.h>

#include "banner.h"
#include "version.h"


/*
 * banner() - return the sign-on banner in a string
 */
char * banner(void)
{
        static char buf[500];

        sprintf(buf, "Automatic Dependent Surveillance–Broadcast (ADS-B) RADAR Feed Service - Version %d.%02d-%d (%s %s)\n%s\nBuilt with GCC %s\n",
                VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,  __DATE__, __TIME__, COPYRIGHT, __VERSION__);

        return buf;
}


