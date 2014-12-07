#ifndef PR_EXPR_H
#define PR_EXPR_H

#include "defs.h"
#include "newstring.h"
#include "char.h"

/*
 *	Printing of functions.
 */

extern	void	pr_fundef(FILE *f, Func *fn);

/*
 *	Printing of expressions.
 *
 *	level = no. of environment levels supplied by the expression.
 *	Others are fetched from the current environment with get_actual().
 */

extern	void	pr_expr(FILE *f, Expr *expr);
extern	void	pr_c_expr(FILE *f, Expr *expr, int level, int context);
extern	void	pr_char(FILE *f, Char c);
extern	void	pr_branch(FILE *f, Branch *branch, int level);

/*
 *	If expr amounts to an identifier, return it, else NULL.
 */
extern	String	expr_name(Expr *expr, int level);

#endif
