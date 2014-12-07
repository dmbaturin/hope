/*
 *	Printing of computed values (fully evaluated)
 */

#include "defs.h"
#include "pr_value.h"
#include "expr.h"
#include "cons.h"
#include "value.h"
#include "stack.h"
#include "path.h"
#include "op.h"
#include "print.h"
#include "interpret.h"
#include "pr_expr.h"
#include "error.h"
#include "names.h"

#define	set_env(e)	(chk_stack(1), Push(e))
#define	cur_env()	Top()
#define	clr_env()	Pop_void()

local	void	safe_pr_value(FILE *f, Cell *value, int context);
local	void	real_pr_value(FILE *f, Cell *value, int context);
local	void	pr_vlist(FILE *f, Cell *value);
local	Bool	is_vlist(Cell *value);
local	Bool	is_vstring(Cell *value);
local	Cell	*get_actual(int level, Path path);
local	void	safe_pr_f_value(FILE *f, String name, Cell *arg, int context);
local	void	pr_f_value(FILE *f, String name, int nargs, Cell *arg, int context);
local	void	pr_papp(FILE *f, Expr *expr, Cell *env, int nargs, int context);
local	void	pr_f_papp(FILE *f, String name, Cell *env, int nargs, int context);
local	int	prec_value(Cell *value);

global void
pr_value(FILE *f, Cell *value)
{
	safe_pr_value(f, value, PREC_BODY);
}

global void
pr_f_match(Func *defun, Cell *env)
{
	start_err_line();
	pr_f_papp(errout, defun->f_name, env, defun->f_arity, PREC_BODY);
	(void)fprintf(errout, "\n");
}

global void
pr_l_match(Expr *func, Cell *env)
{
	start_err_line();
	if (func->e_class == E_EQN) {
		pr_c_expr(errout, func->e_branch->br_formals->e_arg,
			0, PREC_BODY);
		(void)fprintf(errout, " %s ", n_eq);
		safe_pr_value(errout, evaluate(env->c_left), PREC_BODY);
	} else	/* LAMBDA */
		pr_papp(errout, func, env, func->e_arity, PREC_BODY);
	(void)fprintf(errout, "\n");
}

/*
 *	Print a value.
 *	The value is first pushed on the stack, so that it isn't treated
 *	as garbage.
 */
local void
safe_pr_value(FILE *f, Cell *value, int context)
{
	chk_stack(1);
	Push(value);
	real_pr_value(f, value, context);
	Pop_void();
}

local void
real_pr_value(FILE *f, Cell *value, int context)
{
	int	prec;

	prec = prec_value(value);
	if (prec < context)
		(void)fprintf(f, "(");
	switch (value->c_class) {
	case C_NUM:
		(void)fprintf(f, NUMfmt, value->c_num);
	when C_CHAR:
		(void)fprintf(f, "'");
		pr_char(f, value->c_char);
		(void)fprintf(f, "'");
	when C_CONST:
		(void)fprintf(f, "%s", value->c_cons->c_name);
	when C_CONS:
		if (is_vlist(value))
			pr_vlist(f, value);
		else
			pr_f_value(f, value->c_cons->c_name,
				value->c_cons->c_nargs,
				value->c_arg, InnerPrec(prec, context));
	when C_PAIR:
		real_pr_value(f, value->c_left, PREC_COMMA+1);
		(void)fprintf(f, ", ");
		real_pr_value(f, value->c_right, PREC_COMMA);
	when C_SUSP:
		set_env(value->c_env);
		pr_c_expr(f, value->c_expr, 0, InnerPrec(prec, context));
		clr_env();
	when C_PAPP:
		switch (value->c_expr->e_class) {
		case E_DEFUN:
			pr_f_papp(f, value->c_expr->e_defun->f_name,
				value->c_env,
				value->c_expr->e_defun->f_arity -
					value->c_arity,
				InnerPrec(prec, context));
		when E_CONS:
			pr_f_papp(f, value->c_expr->e_const->c_name,
				value->c_env,
				value->c_expr->e_const->c_nargs -
					value->c_arity,
				InnerPrec(prec, context));
		otherwise:	/* LAMBDA and the like */
			pr_papp(f, value->c_expr, value->c_env,
				value->c_expr->e_arity - value->c_arity,
				InnerPrec(prec, context));
		}
	otherwise:
		NOT_REACHED;
	}
	if (prec < context)
		(void)fprintf(f, ")");
}

local void
pr_papp(FILE *f, Expr *expr, Cell *env, int nargs, int context)
{
	if (nargs == 0) {
		set_env(env);
		pr_c_expr(f, expr, 0, context);
		clr_env();
	} else {
		if (PREC_APPLY < context)
			(void)fprintf(f, "(");
		pr_papp(f, expr, env->c_right, nargs-1, PREC_APPLY);
		(void)fprintf(f, " ");
		safe_pr_value(f, evaluate(env->c_left), PREC_ARG);
		if (PREC_APPLY < context)
			(void)fprintf(f, ")");
	}
}

local void
pr_f_papp(FILE *f, String name, Cell *env, int nargs, int context)
{
	if (nargs == 0)
		if (context > PREC_INFIX && op_lookup(name) != NULL)
			(void)fprintf(f, "(%s)", name);
		else
			(void)fprintf(f, "%s", name);
	else if (nargs == 1)
		safe_pr_f_value(f, name, evaluate(env->c_left), context);
	else {
		if (PREC_APPLY < context)
			(void)fprintf(f, "(");
		pr_f_papp(f, name, env->c_right, nargs-1, PREC_APPLY);
		(void)fprintf(f, " ");
		safe_pr_value(f, evaluate(env->c_left), PREC_ARG);
		if (PREC_APPLY < context)
			(void)fprintf(f, ")");
	}
}

/*
 * Print the value, which has been forced, and is known to be a non-empty list.
 */
local void
pr_vlist(FILE *f, Cell *value)
{
	if (is_vstring(value)) {
		(void)fprintf(f, "\"");
		for ( ; value->c_class == C_CONS;
		     value = value->c_arg->c_right)
			pr_char(f, value->c_arg->c_left->c_char);
		(void)fprintf(f, "\"");
	} else {
		(void)fprintf(f, "[");
		repeat {
			real_pr_value(f, value->c_arg->c_left, PREC_COMMA+1);
			value = value->c_arg->c_right;
		until(value->c_class == C_CONST);
			(void)fprintf(f, ", ");
		}
		(void)fprintf(f, "]");
	}
}

/*
 * Is the value, which has been forced, a list?
 */
local Bool
is_vlist(Cell *value)
{
	while (value->c_class == C_CONS && value->c_cons == cons)
		value = value->c_arg->c_right;
	return value->c_class == C_CONST && value->c_cons == nil;
}

/*
 * Is the value, which has been forced and is known to be a list, a string?
 */
local Bool
is_vstring(Cell *value)
{
	while (value->c_class == C_CONS) {
		if (value->c_arg->c_left->c_class != C_CHAR)
			return FALSE;
		value = value->c_arg->c_right;
	}
	return TRUE;
}

local Cell *
get_actual(int level, Path path)
{
	Cell	*env;

	env = cur_env();
	while (level-- > 0)
		env = env->c_right;
	return evaluate(new_dirs(path, env->c_left));
}

/*
 *	Print actual parameter, taking its value from the environment.
 */
global void
pr_actual(FILE *f, int level, Path path, int context)
{
	safe_pr_value(f, get_actual(level, path), context);
}

global void
pr_f_actual(FILE *f, String name, int level, Path path, int context)
{
	safe_pr_f_value(f, name, get_actual(level, path), context);
}

local void
safe_pr_f_value(FILE *f, String name, Cell *arg, int context)
{
	chk_stack(1);
	Push(arg);
	pr_f_value(f, name, 1, arg, context);
	Pop_void();
}

local void
pr_f_value(FILE *f, String name, int nargs, Cell *arg, int context)
		/* a constructor arg is actually several */
{
	Op	*op;

	if ((op = op_lookup(name)) != NULL) {
		if (arg->c_class == C_PAIR) {
			if (op->op_prec < context)
				(void)fprintf(f, "(");
			real_pr_value(f, arg->c_left, LeftPrec(op));
			(void)fprintf(f, " %s ", name);
			real_pr_value(f, arg->c_right, RightPrec(op));
			if (op->op_prec < context)
				(void)fprintf(f, ")");
		} else {
			(void)fprintf(f, "(%s) ", name);
			real_pr_value(f, arg, PREC_ARG);
		}
	} else {
		(void)fprintf(f, "%s", name);
		while (nargs-- > 1) {
			(void)fprintf(f, " ");
			real_pr_value(f, arg->c_left, PREC_ARG);
			arg = arg->c_right;
		}
		(void)fprintf(f, " ");
		real_pr_value(f, arg, PREC_ARG);
	}
}

global String
val_name(int level, Path path)
{
	Cell	*value;

	value = get_actual(level, path);
	switch (value->c_class) {
	case C_SUSP:
		switch (value->c_expr->e_class) {
		case E_CONS:
			return value->c_expr->e_const->c_name;
		when E_DEFUN:
			return value->c_expr->e_defun->f_name;
		otherwise:
			return NULL;
		}
	when C_PAPP:
		if (value->c_expr->e_class == E_DEFUN &&
		    value->c_arity == value->c_expr->e_defun->f_arity)
			return value->c_expr->e_defun->f_name;
		else
			return NULL;
	otherwise:
		return NULL;
	}
}

local int
prec_value(Cell *value)
{
	switch (value->c_class) {
	case C_NUM or C_CHAR or C_CONST:
		return PREC_ATOMIC;
	when C_CONS:
		return PREC_APPLY;
	when C_PAPP:
		switch (value->c_expr->e_class) {
		case E_DEFUN:
			if (value->c_expr->e_defun->f_arity > value->c_arity)
				return PREC_APPLY;
			else
				return PREC_ATOMIC;
		when E_CONS:
			if (value->c_expr->e_const->c_nargs > value->c_arity)
				return PREC_APPLY;
			else
				return PREC_ATOMIC;
		otherwise:
			return PREC_APPLY;
		}
	when C_PAIR:
		return PREC_COMMA;
	otherwise:
		NOT_REACHED;
	}
}
