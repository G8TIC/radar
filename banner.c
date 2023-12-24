
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

        sprintf(buf, "Automatic Dependent Surveillance–Broadcast (ADS-B) RADAR Feed Service - Version %d.%02d-%d (%s %s)\n%s\nBuilt by %s@%s with GCC %s from git %s-%s-%s\n",
                VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,  __DATE__, __TIME__, COPYRIGHT, BUILD_USER, BUILD_HOST, __VERSION__, GIT_VERSION, GIT_COUNT, GIT_ID);

        return buf;
}


