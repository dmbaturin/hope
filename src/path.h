#ifndef PATH_H
#define PATH_H

#include "defs.h"

typedef char	*Path;
#define MAXPATH		40

enum {
	P_END,
	P_LEFT,
	P_RIGHT,
	P_STRIP,
	P_PRED,
	P_UNROLL,
/* count of the above */
	P_NCLASSES
};

#define	P_EMPTY		""

#define p_equal(p1,p2)	(strcmp(p1, p2) == 0)
#define p_less(p1,p2)	(strcmp(p1, p2) < 0)
#define	p_empty(p)	(*(p) == P_END)
#define p_pop(p)	((p)+1)
#define p_top(p)	(*(p))

/* new paths, overwritten by next call */
extern	Path	p_new(void);
extern	Path	p_reverse(Path p);

/* to be used only on a path supplied by p_new() or p_push() */
extern	Path	p_push(int dir, Path p);

/* temporary storage for a number of paths */
extern	void	p_init(char *buf, int size);
extern	Path	p_save(Path p);

/* permanent storage for paths */
extern	Path	p_stash(Path p);

#endif
