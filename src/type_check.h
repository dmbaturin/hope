#ifndef TYPE_CHECK_H
#define TYPE_CHECK_H

#include "defs.h"

extern	Cell	*expr_type;	/* last inferred type */

extern	Bool	chk_func(Branch *branch, Func *fn);

extern	Bool	ty_instance(Type *type1, Natural ntvars1,
			    Type *type2, Natural ntvars2);

/*
 *	Top level: must have
 *		lambda input => expr: list char -> T
 *
 *	Side effect: set expr_type to T.
 */
extern	void	chk_expr(Expr *expr);

extern	void	chk_list(Expr *expr);

#endif
