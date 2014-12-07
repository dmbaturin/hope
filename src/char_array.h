#ifndef CHAR_ARRAY_H
#define CHAR_ARRAY_H

#include "defs.h"
#include "char.h"

/*
 *	Sparse arrays indexed by characters.
 *	Specification:
 *
 *	index(new(x), c) = x
 *	index(copy(a), c) = index(a, c)
 *	index(assign(a, c', x), c) = if c' = c then x else index(a, c)
 *	index(map(a, f), c) = f(index(a, c))
 */
typedef	UCase	*ArrayElement;
typedef	ArrayElement	EltMap(ArrayElement x);

extern	CharArray	*ca_new(ArrayElement x);
extern	CharArray	*ca_copy(CharArray *a);
extern	ArrayElement	ca_index(CharArray *a, Char c);

extern	void	ca_assign(CharArray *a, Char c, ArrayElement x);
extern	void	ca_map(CharArray *a, EltMap *f);

#endif
