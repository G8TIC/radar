/*
 * xtimer.h -- header file for platform independant timer code
 * Author: Michael J Tubby B.Sc. G8TIC   Date: 27-DEC-1994    Version: 01.00
 */

#ifndef _XTIMER_H
#define _XTIMER_H

#include <stdint.h>

enum xtimer_state {
	TIMER_STOPPED,
	TIMER_RUNNING,
	TIMER_EXPIRED
};

typedef struct {
	enum xtimer_state state;
	int64_t started;
	int64_t expires;
} xtimer_t;

extern void xtimer_start(xtimer_t *, uint32_t);
extern void xtimer_clear(xtimer_t *);
extern void xtimer_stop(xtimer_t *);
extern int xtimer_expired(xtimer_t *);
extern int xtimer_running(xtimer_t *);
extern int xtimer_stopped(xtimer_t *);
extern void xtimer_delay(uint32_t);

#endif
