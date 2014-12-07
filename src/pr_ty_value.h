#ifndef PR_TY_VALUE_H
#define PR_TY_VALUE_H

/*
 *	Printing of inferred types.
 */

#include "defs.h"

global void init_pr_ty_value(void);

global void pr_ty_value(FILE *f, Cell *type);

#endif
