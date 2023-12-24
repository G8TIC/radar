/*
 * xtimer.h -- Platform independant timer code
 * Author: Michael J Tubby B.Sc. G8TIC   Date: 27-DEC-1994    Version: 01.00
 */

#include <stdio.h>

#include "mstime.h"
#include "xtimer.h"


/*
 * xtimer_clear() : Initialise & clear an individual timer
 */
void xtimer_clear(xtimer_t * t)
{
	t->started = 0;
	t->expires = 0;
	t->state = TIMER_STOPPED;
}


/*
 * xtimer_start() : Start a specified timer running for a period
 */
void xtimer_start(xtimer_t * t, uint32_t interval)
{
	t->started = mstime();
	t->expires = t->started + (uint64_t)interval;
	t->state = TIMER_RUNNING;
}


/*
 * xtimer_stop() : Stop a timer
 */
void xtimer_stop(xtimer_t * t)
{
	t->state = TIMER_STOPPED;
}


/*
 * xtimer_running() : Check whether a timer is running
 */
int xtimer_running(xtimer_t * t)
{
	return (t->state == TIMER_RUNNING);
}


/*
 * xtimer_stopped() : Check whether a timer is stopped
 */
int xtimer_stopped(xtimer_t * t)
{
	return (t->state == TIMER_STOPPED);
}


/*
 * xtimer_expired() : Test if the timer has expired. If the timer requested
 * has not been started return false.
 */
int xtimer_expired(xtimer_t * t)
{
	if (t->state == TIMER_RUNNING) {
		uint64_t now = mstime();

		if (now >= t->expires) {
			t->state = TIMER_EXPIRED;
			return 1;
		}
	}

	return 0;
}


/*
 * xtimer_delay() - blocking wait on a timer
 */
void xtimer_delay(uint32_t delay)
{
	xtimer_t t;

	xtimer_start(&t, delay);
	
	while (!xtimer_expired(&t))
		;
}

