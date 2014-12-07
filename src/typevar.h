#ifndef TYPEVAR_H
#define TYPEVAR_H

#include "defs.h"
#include "newstring.h"

/*
 *	Type variables.
 */
typedef	String	TVar;

extern	void	tv_declare(String name);
extern	Bool	tv_lookup(String name);
extern	void	tv_print(FILE *f, Natural n);

#endif
