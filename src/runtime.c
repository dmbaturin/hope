#include "defs.h"
#include "heap.h"
#include "stack.h"
#include "memory.h"
#include "type_check.h"
#include "error.h"

/* define this to get various statistics */
/* #define STATS */

/*
 *	These two constants give the ratio between stack and heap used
 *	when the run-time space is divided up.  Adjust to taste.
 */
#define	STACK	1
#define	HEAP	5

#define	TopStack	((StkElt *)base_temp)
#define	BaseHeap	((Cell *)top_string)

global	StkElt	*stack;
global	StkElt	*last_update;
local	StkElt	*stack_limit;
local	Cell	*heap, *heap_limit;

#ifdef	STATS
local	long	max_heap;	/* max. size of heap */
local	long	max_stack;	/* max. size of stack */
local	int	collections;	/* no. of garbage collections */
local	long	gc_time;	/* amount of time spent collecting garbage */

#define	StackOverflow	"stack overflow"
#define	FixedHeapOverflow	"fixed heap overflow"
#define	HeapOverflow	"heap overflow"
#define	NearlyThrashing	"pre-emptive heap overflow"

#else

#define	StackOverflow	"out of memory"
#define	FixedHeapOverflow	"out of memory"
#define	HeapOverflow	"out of memory"
#define	NearlyThrashing	"out of memory"
#endif

local	Cell	*free_list;
local	long	num_free;	/* number of free cells */

/*
 *	Set up the heap and free list, but not garbage collection.
 *	Required before any calls to new_cell().
 */
global void
start_heap(void)
{
	heap = BaseHeap;
	free_list = NOCELL;
	num_free = 0;
	/*
	 * Split the available space half-and-half between the heap and
	 * temporary table space.
	 * A revised split will be used by start_stack().
	 */
	lim_temp += (base_temp - lim_temp)/2;
}

/*
 *	Set up the stack, and garbage collection on the heap.
 *	Required before any calls to Push().
 *	start_heap() should have been already called.
 */
global void
start_stack(void)
{
	/* Split the available space between the heap and stack. */
	heap_limit = BaseHeap +
		((char *)TopStack - (char *)BaseHeap)*HEAP /
			((HEAP+STACK)*sizeof(Cell));
	/* Check that heap hasn't grown too big already. */
	if (heap_limit < heap)
		error(EXECERR, FixedHeapOverflow);

	stack = TopStack;
	stack_limit = stack -
		((char *)TopStack - (char *)heap_limit)/sizeof(StkElt);
	last_update = NULL;
}

global void
chk_stack(int required)
{
#ifdef STATS
	if ((char *)TopStack - (char *)stack > max_stack)
		max_stack = (char *)TopStack - (char *)stack;
#endif
	if (stack - required < stack_limit)
		error(EXECERR, StackOverflow);
}

global Cell *
new_cell(int class)
{
	Cell	*cell;

	if (num_free == 0) {
		/*
		 * Do it by enlarging the heap.
		 * This section should only be used during type checking,
		 * as during execution chk_heap() should have allocated
		 * enough free cells.
		 */
		cell = heap++;
		if ((char *)heap > lim_temp) {
			heap--;
			error(EXECERR, FixedHeapOverflow);
		}
	} else {	/* get it from the free list */
		cell = free_list;
		free_list = cell->c_foll;
		num_free--;
	}
	cell->c_class = class;
	return cell;
}

local	void	gc(Cell *current);
local	void	reach(Cell *cell);

/*
 *	Make sure that the free list has the required number of cells.
 *	This should only be called during execution, when the cells
 *	in use are those reachable from the run-time stack and
 *		current - the value being evaluated
 *		expr_type - the type of the current expression
 *	It can't be called during type checking, because then references
 *	to cells are scattered all over the C stack.  So we assume that
 *	type checking can be done without a garbage collection.
 */
global void
chk_heap(Cell *current, int required)	/* required no. of free cells */
{
	while (num_free < required)
		/* expand the heap, if possible */
		if (heap < heap_limit) {
			heap->c_foll = free_list;
			free_list = heap++;
			num_free++;
		} else {	/* try to collect garbage */
			gc(current);
			if (num_free < required)
				error(EXECERR, HeapOverflow);
			return;
		}
}

#ifdef STATS
#include <sys/param.h>
#include <sys/types.h>
#include <sys/times.h>
#endif

/*
 * Collect garbage, the mark-sweep scheme.
 */

/*
 *	If you don't get this many, give up, to prevent thrashing.
 *	This number must be at least 1.
 */
#define	MIN_RECOVERED	100

#define	GC_Mark(cell)	((cell)->c_class |= GC_MARK)
#define	GC_UnMark(cell)	((cell)->c_class &= ~GC_MARK)
#define	GC_Marked(cell)	((cell)->c_class & GC_MARK)

local void
gc(Cell *current)
{
	Cell	*cp;
	StkElt	*save_stack, *save_last_update;
#ifdef STATS
	struct	tms	before, after;
#endif

#ifdef STATS
	times(&before);
#endif
	/* mark each cell in the heap */
	for (cp = BaseHeap; cp != heap; cp++)
		GC_Mark(cp);
	/* unmark reachable cells */
	reach(expr_type);
	reach(current);
	save_stack = stack;
	save_last_update = last_update;
	while (stack != TopStack)
		reach(IsUpdate() ? PopUpdate() : Pop());
	stack = save_stack;
	last_update = save_last_update;
	/* add unreachable cells to the free list */
	num_free = 0;
	free_list = NOCELL;
	for (cp = BaseHeap; cp != heap; cp++)
		if (GC_Marked(cp)) {
			cp->c_foll = free_list;
			free_list = cp;
			num_free++;
		}
#ifdef STATS
	times(&after);
	(void)fprintf(stdout,
		"[garbage collection: %ld cells recovered in %ld ms]\n",
		num_free,
		(after.tms_utime - before.tms_utime)*1000/HZ);
	if (((heap - BaseHeap) - num_free)*sizeof(Cell) > max_heap)
		max_heap = ((heap - BaseHeap) - num_free)*sizeof(Cell);
	collections++;
	gc_time += after.tms_utime - before.tms_utime;
#endif
	if (num_free < MIN_RECOVERED)
		error(EXECERR, NearlyThrashing);
}

local void
reach(Cell *cell)
{
	while (cell != NOCELL && GC_Marked(cell)) {
		GC_UnMark(cell);
		switch (CellArity(cell->c_class)) {
		case 0:
			return;
		when 1:
			cell = cell->c_sub;
		when 2:
			reach(cell->c_sub1);
			cell = cell->c_sub2;
		otherwise:
			NOT_REACHED;
		}
		/* reach(cell); return */
	}
}

global void
heap_stats(void)
{
#ifdef	STATS
	(void)fprintf(stdout,
		"text: %ld (string) + %ld (structures) = %ld bytes\n",
		(long)(top_string - base_memory),
		(long)(top_memory - base_temp),
		(long)((top_string - base_memory) +
			(top_memory - base_temp)));
	(void)fprintf(stdout,
		"dynamic: %ld (heap) + %ld (stack) = %ld bytes\n",
		max_heap, max_stack, max_heap + max_stack);
	if (collections != 0)
		(void)fprintf(stdout,
			"%d garbage collections, average %ldms\n",
			collections, gc_time*1000/HZ/collections);
#endif
}
