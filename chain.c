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
 * chain_insert_sorted() - insert an item into a sorted list
 * chain_sort() - sort a chain using quicksort 
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


#if 0

/*
 * FIXME the versions below are newer than the rest of our codebase expects
 * and are incompatible, hence I have retained the old symantics for now
 * Mike -- 21-FEB-2022
 *
 */


/*
 * chain_new() - create a new chain base or item in the heap
 */
void * chain_new(int size)
{
	reft *p = malloc(size > sizeof(reft) ? size : sizeof(reft));
	if (p)
		chain_init(p);
	return p;
}


/*
 * chain_newz() - do the same as chain_newz, but zeroise the whole thing
 */
void * chain_newz(int size)
{
	reft *p = malloc(size > sizeof(reft) ? size : sizeof(reft));
	if (p) {
		memset(p, 0, size);
		chain_init(p);
	}
	return p;
}

#endif


/*
 * chain_free() - free a chain item (deleting it if it's chained)
 */
void chain_free(reft *p)
{
	if (chain_is_chained(p))
		chain_delete(p);
	free(p);
}

#if 0
/*
 * chain_count() - count the number of items on the chain (not including the base)
 */
int chain_count(reft *base)
{
	reft *p = (void *)0;
	int count = 0; 

	while((p = chain_get_next(base, p))) 
		++count;

	return count;
}
#endif

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


/*
 * chain_insert_sorted() - insert an object to a sorted chain
 */
void chain_insert_sorted(reft *base, reft *p, int (*cmp)(reft *, reft *))
{
	reft * it;
	for (it = base->next; it != base; it = it->next) {
		if (cmp(p, it) < 0)
			break;
	}
	chain_insert(it, p);
}



/****
**
** Chain sorting using QuickSort
**
**
****/


/*
 * Nobody at Thorcom appears to want to admit having written this code ;-)
 *
 * I have tidied it up and got it working (not because there was a bug in the code) after a
 * misunderstanding of what the call-out to the compare routine has to do, more specifically
 * what it has to return, that being three states (less than, equal or greater than) like
 * strcmp() does.
 *
 * Now I have my understanding 'aligned' with the implementation, it works.
 *
 * I have tidied up the function order and formal parameters to use modern style.
 *
 * --Mike 21/03/2022
 *
 */


/*
 * chain_sort_partition() - internal part of the partition sort
 */
static reft * chain_sort_partition(reft *begin, reft *end, reft *pivot, int (*cmp)(reft *, reft *))
{
	reft * it, * newbegin;
	if (pivot != begin) {
		/* unlink the pivot */
		pivot->prev->next = pivot->next;
		pivot->next->prev = pivot->prev;

		/* move it to the beginning */
		pivot->prev = begin->prev;
		pivot->next = begin;
		begin->prev->next = pivot;
		begin->prev = pivot;
	}

	newbegin = pivot;

	/* move everything less than the pivot to before the pivot */
	for (it = begin; it != end; it = it->next) {
		if (cmp(it, pivot) < 0)	{
			reft * newit = it;
			/* prev <-> begin <-> it <-> */
			/* prev <-> it <-> begin <-> */
			it->prev->next = it->next;
			it->next->prev = it->prev;
			it = it->prev;

			/* move it to the beginning */
			newit->prev = newbegin->prev;
			newit->next = newbegin;
			newbegin->prev->next = newit;
			newbegin->prev = newit;

			newbegin = newit;
		}
	}

	return newbegin;
}


/*
 * chain_sort_impl() - some subfunction of quick sort
 */
static void chain_sort_impl(reft *begin, reft *end, int (*cmp)(reft *, reft *))
{
	/* check for empty list or list of just one element */
	if (begin != end) {
		reft * pivot = (cmp(begin, end->prev) < 0) ? begin : end->prev;
		begin = chain_sort_partition(begin, end, pivot, cmp);
		chain_sort_impl(begin, pivot, cmp);
		chain_sort_impl(pivot->next, end, cmp);
	}
}


/*
 * chain_sort() - Sort a chain of items using the quick ssort alogorithm.
 *
 * Parameters:
 *
 *	base:	the base of the chain that is to be sorted
 *	cmp:	a function to be called to perform the comparision (must return -1, 0 or 1)
 *
 */
void chain_sort(reft *base, int (*cmp)(reft *, reft *))
{
	chain_sort_impl(base->next, base, cmp);
}

