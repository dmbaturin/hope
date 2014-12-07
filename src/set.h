#ifndef SET_H
#define SET_H

#include "defs.h"

#define BYTESIZE	8
#define WORDSIZE	(BYTESIZE*sizeof(Natural))

/* declare a set of [0..size-1] */
#define	SET(name,size)	Natural	name[((size)+WORDSIZE-1)/WORDSIZE]
#define	NWords(set)	((Natural)(sizeof(set)/sizeof(Natural)))

typedef	Natural	*SetPtr;	/* local reference to a set */

extern	void	set_clear(SetPtr s, Natural n);
extern	Natural	set_card(SetPtr s, Natural n);
extern	void	set_union(SetPtr s1, Natural n1, SetPtr s2, Natural n2);

#define ADD(set,n)	(set[(n)/WORDSIZE] |= 1<<((n)%WORDSIZE))
#define REMOVE(set,n)	(set[(n)/WORDSIZE] &= ~(1<<((n)%WORDSIZE)))
#define MEMBER(set,n)	((set[(n)/WORDSIZE] & 1<<((n)%WORDSIZE)) != 0)

/* the following do NOT work on SetPtrs */
#define	CLEAR(set)	set_clear(set, NWords(set))
#define	CARD(set)	set_card(set, NWords(set))
#define	UNION(set1,set2) set_union(set1, NWords(set1), set2, NWords(set2))

#endif
