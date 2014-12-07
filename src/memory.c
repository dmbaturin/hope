#include "defs.h"
#include "memory.h"
#include "error.h"
#include "align.h"

/* size of space for compiled code, tables, stack and heap */
#define	MEGABYTE (1024*1024L)
#define	MEMSIZE 16*MEGABYTE

/*
 * The granularity of allocation is the alignment granularity.
 */
#define	RoundDown(n)	((n)/ALIGNMENT*ALIGNMENT)
#define	RoundUp(n)	RoundDown((n) + (ALIGNMENT-1))

global	char	*base_memory, *top_memory;
global	char	*top_string, *base_table, *base_temp;
global	char	*lim_temp;

global void
init_memory(void)
{
	if ((base_memory = (char *)malloc((size_t)MEMSIZE)) == NULL)
		error(FATALERR, "can't allocate memory");
	top_memory = base_memory + RoundDown(MEMSIZE);

	lim_temp = top_string = base_memory;
	base_table = base_temp = top_memory;
}

global void *
s_alloc(Natural n)
{
	char	*start;

	start = top_string;
	top_string += RoundUp(n);
	lim_temp = top_string;
	if (base_temp < lim_temp)
		error(FATALERR, "can't store string: out of memory");
	return (void *)start;
}

global void *
t_alloc(Natural n)
{
	base_temp -= RoundUp(n);
	if (base_temp < lim_temp)
		error(FATALERR, "out of memory");
	return (void *)base_temp;
}

global void
clean_slate(void)
{
	base_temp = base_table;
	lim_temp = top_string;
}

global void
preserve(void)
{
	base_table = base_temp;
	lim_temp = top_string;
}
