/*
 * chain.h chain base definitions
 *
 * NB: Many of the functions declared here have formal parameters
 * of 'void *' which the actual parameter is a user defined type
 * or object that has the 'reft' at the start.  This is the (deliberate)
 * hack that makes this work. Please do not change this.
 *
 */

#ifndef _CHAIN_H
#define _CHAIN_H

typedef struct _reft
{
	struct _reft *next, *prev;
} reft;

#define chain_is_empty chain_empty
#define chain_empty_test chain_empty
#define is_chain_empty chain_empty
#define is_chained chain_is_chained

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
//extern reft * chain_insert_sorted(reft *, reft *, int (*cmp)(reft *, reft *));
//extern void chain_sort(reft *, int (*cmp)(reft *, reft *));
extern reft * chain_insert_sorted(reft *, void *, int (*cmp)(void *, void *));
extern void chain_sort(reft *, int (*cmp)(void *, void *));


#endif
