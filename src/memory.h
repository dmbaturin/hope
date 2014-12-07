#ifndef MEMORY_H
#define MEMORY_H

/*
 *	A large chunk of memory is allocated once at the start.
 *	Subsequently, this area is divided as follows:
 *
 *				      Pointers		Allocation functions
 *				      --------		--------------------
 * High	------------------------- <-- top_memory
 *	| Table space:		|
 *	| structures and code	|
 *	|-----------------------| <-- base_table
 *	| temporary table space	|			t_alloc(n)
 *	|-----------------------| <-- base_temp
 *	| Run-time stack	|			Push(cell)
 *	|-----------------------| <-- stack
 *	|	   |		|
 *	|	   v		|
 *	|			|
 *	|			|
 *	|	   ^		|
 *	|	   |		|
 *	|-----------------------| <-- heap
 *	| Heap			|			new_cell(class)
 *	|-----------------------| <-- top_string
 *	| String space		|			s_alloc(n)
 * Low	------------------------- <-- base_memory
 *
 * There is also a pointer lim_temp, which is either equal to top_string
 * or, when the heap but not the stack is enabled, a fence between heap
 * and base_temp.
 */

extern	void	init_memory(void);	/* set up everything */
extern	void	preserve(void);
/* make temporary table space permanent, re-enable s_alloc() and t_alloc() */
extern	void	clean_slate(void);
/* discard temporary table space, re-enable s_alloc() and t_alloc() */

/*
 * Other calls:
 *	start_heap()	enable calls to new_cell(), disabling s_alloc().
 *	start_stack()	enable calls to Push() and chk_heap(),
 *			disabling t_alloc().
 */

extern	void	*s_alloc(Natural nbytes);
extern	void	*t_alloc(Natural nbytes);

extern	void	heap_stats(void);

#define	NEWARRAY(type,size)	((type *)t_alloc((Natural)sizeof(type)*(size)))
#define	NEW(type)	NEWARRAY(type, 1)

extern	char	*base_memory, *top_memory;
extern	char	*top_string, *base_table, *base_temp;
extern	char	*lim_temp;

#endif
