#ifndef INTERPRET_H
#define INTERPRET_H

#include "defs.h"
#include "newstring.h"

/* name of most recently entered function, for error reporting */
extern	String	cur_function;

/*
 *	Interpreter for an expression.
 *	See compile.c for the translation schemes.
 */
extern	void	interpret(Expr *action, Expr *expr);

/*
 *	Reduce a value to head normal form
 */
extern	Cell	*evaluate(Cell *value);

#endif
