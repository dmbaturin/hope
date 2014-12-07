#ifndef EVAL_H
#define EVAL_H

#include "defs.h"

/*
 *	Evaluation of expressions.
 */

extern	void	eval_expr(Expr *expr);
extern	void	wr_expr(Expr *expr, const char *file);

#endif
