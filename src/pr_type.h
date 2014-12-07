#ifndef PR_TYPE_H
#define PR_TYPE_H

#include "defs.h"

/*
 *	Printing of types.
 */

extern	void	pr_qtype(FILE *f, QType *qtype);

extern	void	pr_type(FILE *f, Type *type);
extern	void	pr_deftype(FILE *f, DefType *dt, Bool full);

#endif
