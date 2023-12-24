/*
 * ustime() - return unix epoc time in micro-seconds
 * Author: Michael J. Tubby mike{@tubby.org    Date: 22-OCT-2023
 *
 * X86/AMD-64 and ARM aarch64 operate differently because we can have
 * a 64-bit ARM system with a 32-bit runtime and hence we have to use
 * explicit cast to 64-bit to get the right outcomes on all platforms.
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#include "ustime.h"

/*
 * ustime() - return unix epoc time in micro-seconds
 */
uint64_t ustime(void)
{
        struct timeval tv;
        uint64_t us;

        gettimeofday(&tv, NULL);
        us = (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
        
        return us;
}
