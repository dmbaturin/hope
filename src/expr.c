#include "defs.h"
#include "expr.h"
#include "cons.h"
#include "memory.h"
#include "cases.h"
#include "number.h"
#include "compile.h"
#include "type_check.h"
#include "error.h"
#include "path.h"

/*
 *	Functions, Expressions and Patterns.
 *
 *	All semantic checks here occur at the top level, so sub-expressions
 *	are always defined.
 */

/* Internal names of some constructor expressions */
global	Expr	*e_true, *e_false, *e_cons, *e_nil;
global	Func	*f_id;

/* the following is different from any String */
local	const	char	bound_variable[] = "x'";

global Expr *
char_expr(Char c)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_CHAR;
	expr->e_char = c;
	return expr;
}

global Expr *
text_expr(const Byte *text, int n)
{
	const	Byte	*s;
	Expr	*expr;

	expr = e_nil;
	for (s = text+n-1; s >= text; s--)
		expr = apply_expr(e_cons, pair_expr(char_expr(*s), expr));
	return expr;
}

global Expr *
num_expr(Num n)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_NUM;
	expr->e_num = n;
	return expr;
}

global Expr *
cons_expr(Cons *constr)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_CONS;
	expr->e_const = constr;
	return expr;
}

/*
 *	An identifier occurring as an expression.
 *	Call it a variable for now; we'll find out what it really is later.
 */
global Expr *
id_expr(String name)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_VAR;
	expr->e_vname = name;
	return expr;
}

global Expr *
dir_expr(Path where)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_PARAM;
	expr->e_level = 0;
	expr->e_where = p_stash(p_reverse(where));
	return expr;
}

global Expr *
pair_expr(Expr *left, Expr *right)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_PAIR;
	expr->e_left = left;
	expr->e_right = right;
	return expr;
}

global Expr *
apply_expr(Expr *func, Expr *arg)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_APPLY;
	expr->e_func = func;
	expr->e_arg = arg;
	return expr;
}

global Expr *
func_expr(Branch *branches)
{
	Expr	*expr;
	Expr	*formals;

	expr = NEW(Expr);
	expr->e_class = E_LAMBDA;
	expr->e_branch = branches;
	expr->e_arity = 0;
	/* use the first branch for the arity: checked later in nv_expr() */
	for (formals = branches->br_formals;
	     formals != NULL && formals->e_class == E_APPLY;
	     formals = formals->e_func)
		expr->e_arity++;
	return expr;
}

/*
 *	Representation of various other structures.
 */

global Expr *
ite_expr(Expr *if_expr, Expr *then_expr, Expr *else_expr)
{
	Expr	*expr;

	expr = apply_expr(apply_expr(apply_expr(
					id_expr(newstring("if_then_else")),
					if_expr),
				then_expr),
			else_expr);
	expr->e_class = E_IF;
	return expr;
}

global Expr *
let_expr(Expr *pattern, Expr *body, Expr *subexpr, Bool recursive)
{
	Expr	*expr;

	expr = apply_expr(func_expr(
				new_unary(pattern, subexpr, (Branch *)0)),
			body);
	expr->e_class = recursive ? E_RLET : E_LET;
	expr->e_func->e_class = E_EQN;
	return expr;
}

global Expr *
where_expr(Expr *subexpr, Expr *pattern, Expr *body, Bool recursive)
{
	Expr	*expr;

	expr = apply_expr(func_expr(
				new_unary(pattern, subexpr, (Branch *)0)),
			body);
	expr->e_class = recursive ? E_RWHERE : E_WHERE;
	expr->e_func->e_class = E_EQN;
	return expr;
}

global Expr *
mu_expr(Expr *muvar, Expr *body)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_MU;
	expr->e_muvar = apply_expr((Expr *)0, muvar);
	expr->e_body = body;
	return expr;
}

global Expr *
presection(String operator, Expr *arg)
{
	Expr	*expr;

	expr = func_expr(new_unary(
			id_expr(bound_variable),
			apply_expr(id_expr(operator),
				pair_expr(arg, id_expr(bound_variable))),
			(Branch *)0));
	expr->e_class = E_PRESECT;
	return expr;
}

global Expr *
postsection(String operator, Expr *arg)
{
	Expr	*expr;

	expr = func_expr(new_unary(
			id_expr(bound_variable),
			apply_expr(id_expr(operator),
				pair_expr(id_expr(bound_variable), arg)),
			(Branch *)0));
	expr->e_class = E_POSTSECT;
	return expr;
}

/*
 *	Kinds of expression used to represent built-in functions.
 */

global Expr *
builtin_expr(Function *fn)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_BUILTIN;
	expr->e_fn = fn;
	return expr;
}

global Expr *
bu_1math_expr(Unary *fn)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_BU_1MATH;
	expr->e_1math = fn;
	return expr;
}

global Expr *
bu_2math_expr(Binary *fn)
{
	Expr	*expr;

	expr = NEW(Expr);
	expr->e_class = E_BU_2MATH;
	expr->e_2math = fn;
	return expr;
}

/*
 *	Branches of lambdas or defined functions.
 */

global Branch *
new_branch(Expr *formals, Expr *expr, Branch *next)
{
	Branch	*branch;

	branch = NEW(Branch);
	branch->br_formals = formals;
	branch->br_expr = expr;
	branch->br_next = next;
	return branch;
}

global Branch *
new_unary(Expr *pattern, Expr *expr, Branch *next)
{
	return new_branch(apply_expr((Expr *)0, pattern), expr, next);
}

/*
 *	Defined value names
 */

global void
decl_value(String name, QType *qtype)
{
	Func	*fn;
	Cons	*cp;

	if (erroneous)
		return;
	if (((fn = fn_local(name)) != NULL && fn->f_explicit_dec) ||
	    ((cp = cons_local(name)) != NULL && cp != succ))
		error(SEMERR, "'%s': value identifier already declared", name);
	else {
		if (fn != NULL)
			del_fn(fn);
		new_fn(name, qtype);
		preserve();
	}
}

global void
def_value(Expr *formals, Expr *body)
{
	Branch	*branch;
	Func	*fn;
	Branch	*br;
	int	arity;
	Expr	*head;

	if (erroneous)
		return;

	/* special treatment of if-then-else */
	if (formals->e_class == E_IF)
		formals->e_class = E_APPLY;

	arity = 0;
	for (head = formals; head->e_class == E_APPLY; head = head->e_func)
		arity++;

	if (head->e_class != E_VAR)
		error(SEMERR, "illegal left-hand-side");
	else if ((fn = fn_local(head->e_vname)) == NULL)
		error(SEMERR, "'%s': value identifier not locally declared",
			head->e_vname);
	else if (fn->f_explicit_def && fn->f_arity != arity)
		error(SEMERR,
			"'%s': attempted redefinition with a different arity",
			head->e_vname);
	else if (fn->f_code != NULL && arity == 0)
		error(SEMERR, "'%s': attempt to redefine value identifier",
			head->e_vname);
	else {	
		branch = new_branch(formals, body, (Branch *)0);	
		/* BUG: implicitly declared functions are not checked */
		if (! nr_branch(branch) ||
		    (fn->f_explicit_dec && ! chk_func(branch, fn)))
			return;		/* some error reported */
		if (! fn->f_explicit_def) {
			fn->f_code = NULL;
			fn->f_branch = NULL;
			fn->f_explicit_def = TRUE;
		}
		head->e_class = E_DEFUN;
		head->e_defun = fn;
		fn->f_arity = arity;
		/* add the branch at the end */
		if (fn->f_branch == NULL)
			fn->f_branch = branch;
		else {
			for (br = fn->f_branch;
			     br->br_next != NULL;
			     br = br->br_next)
				;
			br->br_next = branch;
		}
		/* compile it */
		if (fn->f_code == NULL && arity > 0)
			fn->f_code = f_nomatch(fn);
		fn->f_code = comp_branch(fn->f_code, branch);
		preserve();
	}
}

local	Expr	*textlist(const char *const *sp);

global void
init_argv(void)
{
	def_value(id_expr(newstring("argv")), textlist(cmd_args));
}

local Expr *
textlist(const char *const *sp)
{
	return *sp == NULL ? e_nil :
		apply_expr(e_cons,
			pair_expr(text_expr((const Byte *)*sp, strlen(*sp)),
			textlist(sp+1)));
}
