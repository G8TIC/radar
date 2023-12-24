/*
 * chain.c -- routines to operate on double linked circular chains
 *
 * chain_init() - initialise a chain (set next and prev to pointer) 
 * chain_insert() - insert an item before the ref provided
 * chain_movebase() - insert an chain of items from one base to another
 * chain_add() - add an item after the ref
 * chain_delete() - delete an item in a chain
 * chain_valid() - is a chain initialised
 * chain_empty_test() - test to see if the chain is empty
 * chain_is_chained() - test to see if object is chained
 * chain_get_next() - get next item in chain
 * chain_get_prev() - get previous item in chain
 * chain_rechain() - re-chain an item at this point (usually after the chain base)
 * chain_flush() - remove all the elements in a chain, just leaving the base
 * chain_new() - create a new chain base in the heap
 * chain_free() - free a chain item (deleting it if it's chained)
 * chain_count() - count the number of items on the chain (not including the base)
 * chain_swap() - swap two items on a chain
 *
 * NB: Please do not include "chain.h" into these routines as it breaks how they work!
 *
 */

#include <stdlib.h>
#include <string.h>

#include "qerror.h"


/* chain definitions */
typedef struct _reft {
	struct _reft *next, *prev;
} reft;


/*
 * error handling
 */
static char erm[] = "chain broken in %s";
#define check(p, ss) if (p == (reft *) 0 || p->prev->next != p || p->next->prev != p) qabort(erm, ss);


/*
 * chain_init() - initialise a chain (set next and prev to pointer) 
 */
void chain_init(reft *p)
{
	p->next = p->prev = p;
}


/*
 * chain_insert() - insert an item before the ref provided
 */
void chain_insert(reft *p, reft *q)
{
	check(p, "insert");
	q->prev = p->prev;
	q->next = p;
	p->prev->next = q;
	p->prev = q;
}


/*
 * chain_add() - add an item after the ref
 */
void chain_add(reft *p, reft *q)
{
	check(p, "add");
	p = p->next;
	chain_insert(p, q);
}


/*
 * chain_delete() - delete an item in a chain
 */
reft *chain_delete(reft *p)
{
	reft *r = p->prev;
	
	check(p, "delete");
	p->prev->next = p->next;
	p->next->prev = p->prev;
	p->next = p->prev = 0;
	return r;
}


/*
 * chain_empty() - test to see if the chain is empty
 */
int chain_empty(reft *base)
{
	if (!base || (base->next == NULL && base->prev == NULL))
		return 1;
	check(base, "chain_empty")
	return base->next == base;
}


/*
 * chain_valid() - test to see if this chain is valid (has been initialised)
 */
int chain_valid(reft *base)
{
	return (base == NULL || base->next == NULL || base->prev == NULL) ? 0 : 1;
}


/*
 * chain_get_next() - get next item in chain
 */
reft *chain_get_next(reft *base, reft *p)
{

	check(base, "next base")
	if (!p)
		return (base->next == base) ? 0 : base->next;

	check(p, "next last ref")
	if (p->next != base)
		return p->next;
	else
		return (reft *) 0;
}


/*
 * chain_get_prev() - get previous item in chain
 */
reft *chain_get_prev(reft *base, reft *p)
{
	check(base, "prev base");
	if (!p)
		return (base->next == base) ? 0 : base->prev;

	check(p, "prev last ref");
	if (p->prev != base)
		return p->prev;
	else
		return (reft *) 0;
}


/*
 * chain_movebase() - move a chain of items from one base to another
 */
void chain_movebase(reft *p, reft *q)
{
	check(p, "movebase");
	q->prev->prev = p->prev;
	q->next->next = p;
	p->prev->next = q->next;
	p->prev = q->prev;
	q->next = q->prev = q;
}


/*
 * chain_rechain() - re-chain an item at this point (usually after the chain base)
 */
void chain_rechain(reft *base, reft *p)
{
	check(base, "rechain base");
	check(p, "rechain last ref");
	chain_delete(p);
	chain_add(base, p);
}


/*
 * chain_swap() - swap two items over on a chain
 */
void chain_swap(reft *p, reft *q)
{
	check(p, "swap p");
	check(q, "swap q");
	chain_delete(q);
	chain_insert(p, q);
}


/*
 * chain_is_chained() - test to see if object is chained
 */
int chain_is_chained(reft *base)
{
	if (base->next == 0 || base->prev == 0 || base->next == base)
		return 0;
	if (base->prev->next != base || base->next->prev != base)
		return 0;
	return 1;
}


/*
 * chain_flush() - remove all the elements in a chain, just leaving the base
 */
void chain_flush(reft *base)
{
	reft *p;

	while (!chain_empty(base)) {
		p = base->next;
		chain_delete(p);
		free(p);
	}
}


/*
 * chain_new() - create a new chain base in the heap
 */
reft * chain_new(void)
{
	reft *p = (reft *) malloc(sizeof(reft));
	if (p)
		chain_init(p);
	return p;
}



/*
 * chain_free() - free a chain item (deleting it if it's chained)
 */
void chain_free(reft *p)
{
	if (chain_is_chained(p))
		chain_delete(p);
	free(p);
}


/*
 * chain_count() - return the count of items on a chain
 */
int chain_count(reft *base)
{
	reft * p;
	int count = 0;

	if (chain_empty(base))
		return 0;
		
	for (p = base->next; p != base; p = p->next)
		++count;

	return count;
}	
