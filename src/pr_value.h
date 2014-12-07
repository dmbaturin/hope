#ifndef PR_VALUE_H
#define PR_VALUE_H

/*
 *	Printing of computed values (fully evaluated)
 */

#include "defs.h"
#include "newstring.h"
#include "path.h"

extern	void	pr_value(FILE *f, Cell *value);

extern	void	pr_f_match(Func *defun, Cell *env);
extern	void	pr_l_match(Expr *func, Cell *env);

/*
 *	Print actual parameter, taking its value from the environment.
 */
extern	void	pr_actual(FILE *f, int level, Path path, int context);
extern	void	pr_f_actual(FILE *f, String name, int level,
			Path path, int context);

extern	String	val_name(int level, Path path);

#endif
