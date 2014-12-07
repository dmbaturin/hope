#ifndef CONS_H
#define CONS_H

#include "defs.h"
#include "newstring.h"

struct _Cons {
	String	c_name;
	Type	*c_type;
	unsigned char	c_nargs;
	unsigned char	c_index;
	unsigned char	c_ntvars;
	SBool	c_tupled;
	Cons	*c_next;
};

extern	Cons	*constructor(String name, Bool tupled, TypeList *args);
extern	Cons	*alt_cons(Cons *constr, Cons *next);
extern	Cons	*cons_lookup(String name);
extern	Cons	*cons_local(String name);

extern	Cons	*nil, *cons, *succ, *true, *false;

#endif
