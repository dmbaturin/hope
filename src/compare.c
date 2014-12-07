#include "defs.h"
#include "compare.h"
#include "expr.h"
#include "cons.h"
#include "cases.h"
#include "value.h"
#include "path.h"
#include "error.h"

/*
 * Comparisons
 *
 * This is all complicated by the fact that comparisons are done lazily.
 */

local	Expr	*e_cmp, *e_cmppair;
local	Cons	*c_less, *c_equal, *c_greater;

local	Cell	*compare(Cell *arg);
local	Cons	*cmp_args(Cell *first, Cell *second);

/*
 *	Set up comparison code
 *	Call after reading standard module.
 */
global void
init_cmps(void)
{
	Func	*fn;

	/*
	 * The following weird structure causes the 2 arguments of cmp
	 * to be unrolled.
	 * If they are functions, and try to get their arguments, the
	 * routine chk_argument() in interpret.c will detect this structure
	 * and report an error.  Yes, we're talking major kludge, so be
	 * careful about changing it.
	 */
	e_cmp = apply_expr(dir_expr(p_push(P_LEFT, p_new())),
			   apply_expr(dir_expr(p_push(P_RIGHT, p_new())),
				      builtin_expr(compare)));
	fn = fn_lookup(newstring("compare"));
	ASSERT( fn != NULL );
	fn->f_code = success(e_cmp, 0);
	fn->f_arity = 1;
	fn->f_branch = NULL;

	fn = fn_lookup(newstring("cmp_pair"));
	ASSERT( fn != NULL );
	e_cmppair = fn->f_code->uc_body;

	c_less = cons_lookup(newstring("LESS"));
	c_equal = cons_lookup(newstring("EQUAL"));
	c_greater = cons_lookup(newstring("GREATER"));
	ASSERT( c_less != NULL );
	ASSERT( c_equal != NULL );
	ASSERT( c_greater != NULL );
}

/*
 *	Called by the the built-in function "compare", to compare two values.
 *	Comparison of functions should be excluded by the type-checker.
 *	For now, a kludge called chk_argument() in the interpreter does it.
 */
local Cell *
compare(Cell *arg)
{
	switch (arg->c_left->c_class) {
	case C_NUM or C_CHAR or C_CONST:
		return new_cnst(cmp_args(arg->c_left, arg->c_right));
	when C_CONS:
		return arg->c_left->c_cons == arg->c_right->c_cons ?
			new_susp(e_cmp,
				 new_pair(new_pair(arg->c_left->c_arg,
						  arg->c_right->c_arg),
					  NOCELL)) :
			new_cnst(cmp_args(arg->c_left, arg->c_right));
	when C_PAIR:
		return new_susp(e_cmppair, new_pair(arg, NOCELL));
	otherwise:
		NOT_REACHED;
	}
}

local Cons *
cmp_args(Cell *first, Cell *second)
{
	switch (first->c_class) {
	case C_NUM:
		return first->c_num == second->c_num ? c_equal :
			first->c_num < second->c_num ?
					c_less : c_greater;
	when C_CHAR:
		return first->c_char == second->c_char ? c_equal :
			first->c_char < second->c_char ?
					c_less : c_greater;
	when C_CONST:
		return first->c_cons == second->c_cons ? c_equal :
			first->c_cons->c_index < second->c_cons->c_index ?
					c_less : c_greater;
	when C_CONS:
		return first->c_cons->c_index < second->c_cons->c_index ?
					c_less : c_greater;
	otherwise:
		NOT_REACHED;
	}
}
