#include "defs.h"
#include "pr_expr.h"
#include "expr.h"
#include "cons.h"
#include "op.h"
#include "print.h"
#include "pr_value.h"
#include "names.h"

local	void	pr_f_expr(FILE *f, String name, Expr *arg,
				int level, int context);
local	Bool	is_list(Expr *expr);
local	void	pr_elist(FILE *f, Expr *expr, int level);
local	Bool	is_string(Expr *expr);
local	void	pr_string(FILE *f, Expr *expr);
local	void	pr_lambda(FILE *f, Branch *branch, int level);
local	void	pr_formals(FILE *f, Expr *formals);
local	void	pr_presection(FILE *f, Expr *expr, int level);
local	void	pr_postsection(FILE *f, Expr *expr, int level);
local	int	precedence(Expr *expr);

local	Bool	in_definition;		/* initially FALSE */

/*
 *	Printing of functions.
 */

global void
pr_fundef(FILE *f, Func *fn)
{
	Branch	*br;

	in_definition = TRUE;
	for (br = fn->f_branch; br != NULL; br = br->br_next) {
		(void)fprintf(f, "%s ", n_valof);
		pr_c_expr(f, br->br_formals, 0, PREC_BODY);
		(void)fprintf(f, " %s ", n_is);
		pr_c_expr(f, br->br_expr, fn->f_arity, PREC_BODY);
		(void)fprintf(f, ";\n");
	}
	in_definition = FALSE;
}

/*
 *	Printing of expressions.
 *
 *	level = no. of environment levels supplied by the expression.
 *	Others are fetched from the current environment with get_actual().
 */

global void
pr_expr(FILE *f, Expr *expr)
{
	pr_c_expr(f, expr, MAX_SCOPES, PREC_BODY);
}

global void
pr_c_expr(FILE *f, Expr *expr, int level, int context)
{
	int	prec;
	String	name;

	prec = precedence(expr);
	if (prec < context)
		(void)fprintf(f, "(");
	switch (expr->e_class) {
	case E_PAIR:
		pr_c_expr(f, expr->e_left, level, PREC_COMMA+1);
		(void)fprintf(f, ", ");
		pr_c_expr(f, expr->e_right, level, PREC_COMMA);
	when E_APPLY:
		if (is_list(expr))
			if (is_string(expr))
				pr_string(f, expr);
			else
				pr_elist(f, expr, level);
		else {
			name = expr_name(expr->e_func, level);
			if (name != NULL)
				pr_f_expr(f, name, expr->e_arg,
					level, InnerPrec(prec, context));
			else {
				pr_c_expr(f, expr->e_func, level, PREC_APPLY);
				(void)fprintf(f, " ");
				pr_c_expr(f, expr->e_arg, level, PREC_ARG);
			}
		}
	when E_IF:
		(void)fprintf(f, "%s ", n_if);
		pr_c_expr(f, expr->e_func->e_func->e_arg, level, PREC_BODY);
		(void)fprintf(f, " %s ", n_then);
		pr_c_expr(f, expr->e_func->e_arg, level, PREC_BODY);
		(void)fprintf(f, " %s ", n_else);
		pr_c_expr(f, expr->e_arg, level, PREC_IF);
	when E_LET:
		(void)fprintf(f, "%s ", n_let);
		pr_c_expr(f, expr->e_func->e_branch->br_formals->e_arg,
			level+1, PREC_BODY);
		(void)fprintf(f, " %s ", n_eq);
		pr_c_expr(f, expr->e_arg, level, PREC_BODY);
		(void)fprintf(f, " %s ", n_in);
		pr_c_expr(f, expr->e_func->e_branch->br_expr,
			level+1, PREC_LET);
	when E_RLET:
		(void)fprintf(f, "%s ", n_letrec);
		pr_c_expr(f, expr->e_func->e_branch->br_formals->e_arg,
			level+1, PREC_BODY);
		(void)fprintf(f, " %s ", n_eq);
		pr_c_expr(f, expr->e_arg, level+1, PREC_BODY);
		(void)fprintf(f, " %s ", n_in);
		pr_c_expr(f, expr->e_func->e_branch->br_expr,
			level+1, PREC_LET);
	when E_WHERE:
		pr_c_expr(f, expr->e_func->e_branch->br_expr,
			level+1, PREC_WHERE);
		(void)fprintf(f, " %s ", n_where);
		pr_c_expr(f, expr->e_func->e_branch->br_formals->e_arg,
			level+1, PREC_BODY);
		(void)fprintf(f, " %s ", n_eq);
		pr_c_expr(f, expr->e_arg, level, PREC_WHERE);
	when E_RWHERE:
		pr_c_expr(f, expr->e_func->e_branch->br_expr,
			level+1, PREC_WHERE);
		(void)fprintf(f, " %s ", n_whererec);
		pr_c_expr(f, expr->e_func->e_branch->br_formals->e_arg,
			level+1, PREC_BODY);
		(void)fprintf(f, " %s ", n_eq);
		pr_c_expr(f, expr->e_arg, level+1, PREC_WHERE);
	when E_MU:
		(void)fprintf(f, "%s ", n_mu);
		pr_formals(f, expr->e_muvar);
		(void)fprintf(f, " %s ", n_gives);
		pr_c_expr(f, expr->e_body, level+1, PREC_MU);
	when E_LAMBDA:
		pr_lambda(f, expr->e_branch, level + expr->e_arity);
	when E_PRESECT:
		if (in_definition)
			pr_presection(f, expr->e_branch->br_expr, level+1);
		else
			pr_lambda(f, expr->e_branch, level+1);
	when E_POSTSECT:
		if (in_definition)
			pr_postsection(f, expr->e_branch->br_expr, level+1);
		else
			pr_lambda(f, expr->e_branch, level+1);
	when E_NUM:
		(void)fprintf(f, NUMfmt, expr->e_num);
	when E_CHAR:
		(void)fprintf(f, "'");
		pr_char(f, expr->e_char);
		(void)fprintf(f, "'");
	when E_DEFUN:
		(void)fprintf(f, "%s", expr->e_defun->f_name);
	when E_CONS:
		if (expr == e_nil)
			(void)fprintf(f, "[]");
		else
			(void)fprintf(f, "%s", expr->e_const->c_name);
	when E_PARAM:
		if (expr->e_level < level)
			pr_c_expr(f, expr->e_patt,
				0, InnerPrec(prec, context));
		else
			pr_actual(f, expr->e_level - level,
				expr->e_where, InnerPrec(prec, context));
	when E_PLUS:
		pr_c_expr(f, expr->e_rest, level, prec);
		(void)fprintf(f, " + %d", expr->e_incr);
	when E_VAR:
		(void)fprintf(f, "%s", expr->e_vname);
	otherwise:
		NOT_REACHED;
	}
	if (prec < context)
		(void)fprintf(f, ")");
}

local void
pr_f_expr(FILE *f, String name, Expr *arg, int level, int context)
{
	Op	*op;

	if (arg->e_class == E_PARAM)
		if (arg->e_level < level)
			pr_f_expr(f, name, arg->e_patt, 0, context);
		else
			pr_f_actual(f, name,
				arg->e_level - level, arg->e_where, context);
	else if ((op = op_lookup(name)) != NULL)
		if (arg->e_class == E_PAIR) {
			if (op->op_prec < context)
				(void)fprintf(f, "(");
			pr_c_expr(f, arg->e_left, level, LeftPrec(op));
			(void)fprintf(f, " %s ", name);
			pr_c_expr(f, arg->e_right, level, RightPrec(op));
			if (op->op_prec < context)
				(void)fprintf(f, ")");
		} else {
			(void)fprintf(f, "(%s) ", name);
			pr_c_expr(f, arg, level, PREC_ARG);
		}
	else {
		(void)fprintf(f, "%s ", name);
		pr_c_expr(f, arg, level, PREC_ARG);
	}
}

/*
 * An expression is printed as a list if it was input as a list,
 * signified by being built with e_cons (and e_nil).
 */
local Bool
is_list(Expr *expr)
{
	return expr->e_class == E_APPLY && expr->e_func == e_cons;
}

local void
pr_elist(FILE *f, Expr *expr, int level)
{
	(void)fprintf(f, "[");
	repeat {
		pr_c_expr(f, expr->e_arg->e_left, level, PREC_COMMA+1);
		expr = expr->e_arg->e_right;
	until(expr->e_const == nil);
		(void)fprintf(f, ", ");
	}
	(void)fprintf(f, "]");
}

/*
 *	Is expr a string?  (We already know it's a list)
 */
local Bool
is_string(Expr *expr)
{
	while (expr->e_class == E_APPLY &&
	       expr->e_arg->e_left->e_class == E_CHAR)
		expr = expr->e_arg->e_right;
	return expr->e_class == E_CONS;		/* i.e. nil */
}

local void
pr_string(FILE *f, Expr *expr)
{
	(void)fprintf(f, "\"");
	while (expr->e_const != nil) {
		pr_char(f, expr->e_arg->e_left->e_char);
		expr = expr->e_arg->e_right;
	}
	(void)fprintf(f, "\"");
}

global void
pr_char(FILE *f, Char c)
{
	switch (c) {
	case '\007':	(void)fprintf(f, "\\a");
	when '\b':	(void)fprintf(f, "\\b");
	when '\f':	(void)fprintf(f, "\\f");
	when '\n':	(void)fprintf(f, "\\n");
	when '\r':	(void)fprintf(f, "\\r");
	when '\t':	(void)fprintf(f, "\\t");
	when '\013':	(void)fprintf(f, "\\v");
	otherwise:
		if (IsCntrl(c))
			(void)fprintf(f, "\\%03o", c);
		else
			PutChar(c, f);
	}
}

local void
pr_lambda(FILE *f, Branch *branch, int level)
{
	(void)fprintf(f, "%s ", n_lambda);
	while (branch != NULL) {
		pr_branch(f, branch, level);
		branch = branch->br_next;
		if (branch != NULL)
			(void)fprintf(f, " | ");
	}
}

global void
pr_branch(FILE *f, Branch *branch, int level)
{
	pr_formals(f, branch->br_formals);
	(void)fprintf(f, "%s ", n_gives);
	pr_c_expr(f, branch->br_expr, level, PREC_LAMBDA);
}

local void
pr_formals(FILE *f, Expr *formals)
{
	if (formals != NULL && formals->e_class == E_APPLY) {
		pr_formals(f, formals->e_func);
		pr_c_expr(f, formals->e_arg, 0, PREC_FORMAL);
		(void)fprintf(f, " ");
	}
}

local void
pr_presection(FILE *f, Expr *expr, int level)
{
	pr_c_expr(f, expr->e_arg->e_left, level, PREC_COMMA+1);
	(void)fprintf(f, " %s", expr->e_func->e_class == E_DEFUN ?
				expr->e_func->e_defun->f_name :
				expr->e_func->e_const->c_name);
}

local void
pr_postsection(FILE *f, Expr *expr, int level)
{
	(void)fprintf(f, "%s ", expr->e_func->e_class == E_DEFUN ?
				expr->e_func->e_defun->f_name :
				expr->e_func->e_const->c_name);
	pr_c_expr(f, expr->e_arg->e_right, level, PREC_COMMA+1);
}

/*
 *	If expr amounts to an identifier, return it, else NULL.
 */
global String
expr_name(Expr *expr, int level)
{
	switch (expr->e_class) {
	case E_DEFUN:
		return expr->e_defun->f_name;
	when E_CONS:
		return expr->e_const->c_name;
	when E_PLUS:
		return newstring("+");
	when E_VAR:
		return expr->e_vname;
	when E_PARAM:
		if (expr->e_level < level)
			return expr_name(expr->e_patt, 0);
		return val_name(expr->e_level - level, expr->e_where);
	otherwise:
		return NULL;
	}
}

local int
precedence(Expr *expr)
{
	switch (expr->e_class) {
	case E_NUM or E_CHAR:
		return PREC_ATOMIC;
	when E_PAIR:
		return PREC_COMMA;
	when E_LAMBDA:
		return PREC_LAMBDA;
	when E_MU:
		return PREC_MU;
	when E_PRESECT or E_POSTSECT:
		return in_definition ? PREC_INFIX : PREC_LAMBDA;
	when E_WHERE or E_RWHERE:
		return PREC_WHERE;
	when E_LET or E_RLET:
		return PREC_LET;
	when E_IF:
		return PREC_IF;
	when E_APPLY:
		return PREC_APPLY;
	when E_CONS:
		if (op_lookup(expr->e_const->c_name) != NULL)
			return PREC_INFIX;
		else
			return PREC_ATOMIC;
	when E_DEFUN:
		if (op_lookup(expr->e_defun->f_name) != NULL)
			return PREC_INFIX;
		else
			return PREC_ATOMIC;
	when E_PLUS:
		return op_lookup(newstring("+"))->op_prec;
	when E_VAR:
		if (op_lookup(expr->e_vname) != NULL)
			return PREC_INFIX;
		else
			return PREC_ATOMIC;
	when E_PARAM:
		return precedence(expr->e_patt);
	otherwise:
		NOT_REACHED;
	}
}
