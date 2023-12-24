/*
 * chain.h -- Operations on double-linked lists
 * Authors: Dirk Koopman G1TLH, Mike Tubby G8TIC, et. al (c) circa 1994
 *
 * NB: Many of the functions declared here have formal parameters of type 'void *'
 * where the actual parameter is a user defined type or object that has the 'reft'
 * as it's first element -- this is deliberate and designed/implemented this way.
 * Please do not change this.
 *
 * Include this header in your application code but DO NOT include it in to chain.c
 * or you will suffer a load of pain!
 *
 */

#ifndef _CHAIN_H
#define _CHAIN_H

/*
 * reft: the chain reference type with next and prev pointers
 */
typedef struct _reft
{
	struct _reft *next, *prev;
} reft;


/*
 * external functions
 */
extern void chain_init(reft *);
extern void chain_insert(reft *, void *);
extern void chain_movebase(reft *, reft *);
extern void chain_add(reft *, void *);
extern void *chain_delete(void *);
extern void *chain_get_next(reft *, void *);
extern void *chain_get_prev(reft *, void *);
extern void chain_rechain(reft *, void *);
extern void chain_swap(void *, void *);
extern int  chain_empty(reft *);
extern int  chain_valid(reft *);
extern int  chain_is_chained(void *);
extern void chain_flush(reft *);
extern void *chain_new(void);
extern void chain_free(void *);
extern int  chain_count(reft *);

#endif
