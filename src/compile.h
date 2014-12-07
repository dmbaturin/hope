#ifndef COMPILE_H
#define COMPILE_H

#include "defs.h"

extern	UCase	*comp_branch(UCase *old_body, Branch *branch);

/* Compile all the LAMBDAs in expr. */
extern	void	comp_expr(Expr *expr);

#endif
