#include "defs.h"
#include "type_check.h"
#include "expr.h"
#include "deftype.h"
#include "cons.h"
#include "pr_expr.h"
#include "pr_type.h"
#include "type_value.h"
#include "pr_ty_value.h"
#include "functor_type.h"
#include "op.h"
#include "error.h"
#include "exceptions.h"

global	Cell	*expr_type;	/* last inferred type */

	/* Types of local program variables, allocated with new_vars() */
local	Cell	**next_vtype;
	/* Types of variables local to a pattern or parameter */
local	Cell	**local_var;
	/* Local variables at each level */
local	Cell	***variables;

local	void	match_type(String name, Cell *inferred, QType *declared);

local	Cell	*ty_expr(Expr *expr);
local	Cell	*ty_pattern(Expr *pattern, int level);
local	Cell	*ty_if(Expr *expr);
local	Cell	*ty_eqn(Branch *branch, Expr *expr);
local	Cell	*ty_rec_eqn(Branch *branch, Expr *expr);
local	Cell	*ty_mu_expr(Expr *muvar, Expr *body);
local	Cell	*ty_list(Branch *branch);
local	Cell	*ty_branch(Branch *branch);
local	Cell	*ty_formals(Expr *formals, Cell *type);

local	DefType	*get_functor(Expr *expr);

local	void	init_vars(void);
local	void	new_vars(int n);
local	void	del_vars(void);

local	void	show_argument(Expr *func, Expr *arg, Cell *arg_type);
local	void	show_expr_type(Expr *expr, Cell *type);
local	void	show_expr(Expr *expr);
local	void	show_branch(Branch *branch);

global Bool
chk_func(Branch *branch, Func *fn)
{
	Cell	*inferred;

	if (setjmp(execerror))
		return FALSE;
	init_vars();
	inferred = ty_branch(branch);
	match_type(fn->f_name, inferred, fn->f_qtype);
	return TRUE;
}

/*
 *	Check that the inferred type is compatible with (at least as
 *	general as) the declared type.
 */
local void
match_type(String name, Cell *inferred, QType *declared)
{
	if (! instance(declared->qt_type, declared->qt_ntvars, inferred)) {
		start_err_line();
		(void)fprintf(errout, "  declared type: ");
		pr_qtype(errout, declared);
		(void)fprintf(errout, "\n");
		start_err_line();
		(void)fprintf(errout, "  inferred type: ");
		pr_ty_value(errout, inferred);
		(void)fprintf(errout, "\n");
		error(TYPEERR, "'%s': does not match declaration", name);
	}
}

global Bool
ty_instance(Type *type1, Natural ntvars1, Type *type2, Natural ntvars2)
{
	if (setjmp(execerror))
		return FALSE;
	init_vars();
	return instance(type1, ntvars1, copy_type(type2, ntvars2, FALSE));
}

/*
 *	Top level: must have
 *		lambda input => expr: list char -> T
 *
 *	Side effect: set expr_type to T.
 */
global void
chk_expr(Expr *expr)
{
	init_vars();
	new_vars(0);
	*next_vtype++ = new_list_type(new_const_type(character));
	expr_type = ty_expr(expr);
	del_vars();
}

global void
chk_list(Expr *expr)
{
	chk_expr(expr);
	if (! unify(expr_type, new_list_type(new_tvar()))) {
		show_expr_type(expr, expr_type);
		error(TYPEERR, "a 'write' expression must produce a list");
	}
}

local Cell *
ty_expr(Expr *expr)
{
	Cell	*type1, *type2;
	DefType	*dt;

	switch (expr->e_class) {
	case E_NUM:
		return new_const_type(num);
	when E_CHAR:
		return new_const_type(character);
	when E_DEFUN:
		if ((dt = get_functor(expr)) != NULL)
			return functor_type(dt);
		return copy_type(expr->e_defun->f_type,
				expr->e_defun->f_ntvars, FALSE);
	when E_CONS:
		/*
		 * Restricted types of list and string syntax:
		 */
		if (expr == e_nil)	/* [] : list alpha */
			return new_list_type(new_tvar());
		if (expr == e_cons) {
			/* alpha # list alpha -> list alpha */
			type1 = new_tvar();
			type2 = new_list_type(type1);
			return new_func_type(
					new_prod_type(type1, type2),
					type2);
		}
		return copy_type(expr->e_const->c_type,
				expr->e_const->c_ntvars, FALSE);
	when E_LAMBDA or E_PRESECT or E_POSTSECT:
		return ty_list(expr->e_branch);
	when E_PARAM:
		if ((dt = get_functor(expr)) != NULL)
			return functor_type(dt);
		return ty_pattern(expr->e_patt, expr->e_level);
	when E_PLUS:
		type1 = new_const_type(num);
		type2 = ty_expr(expr->e_rest);
		if (! unify(type1, type2)) {
			show_expr(expr);
			show_expr_type(expr->e_rest, type2);
			error(TYPEERR, "argument has wrong type");
		}
		return type1;
	when E_VAR:
		/*
		 *	... , x: t, ... |- x: t
		 */
		return local_var[(int)expr->e_var];
	when E_PAIR:
		/*
		 *	A |- e1: t1
		 *	A |- e2: t2
		 *	-------------
		 *	A |- e1, e1: t1 # t2
		 */
		return new_prod_type(ty_expr(expr->e_left),
				     ty_expr(expr->e_right));
	when E_IF:
		return ty_if(expr);
	when E_WHERE or E_LET:
		return ty_eqn(expr->e_func->e_branch, expr->e_arg);
	when E_RWHERE or E_RLET:
		return ty_rec_eqn(expr->e_func->e_branch, expr->e_arg);
	when E_MU:
		return ty_mu_expr(expr->e_muvar, expr->e_body);
	when E_APPLY:
		/*
		 *	A |- e1: t2 -> t
		 *	A |- e2: t2
		 *	-------------
		 *	A |- (e1 e2): t
		 */
		type1 = ty_expr(expr->e_func);
		type2 = ty_expr(expr->e_arg);
		if (! unify(type1, new_func_type(type2, new_tvar()))) {
			show_expr(expr);
			show_expr_type(expr->e_func, type1);
			show_argument(expr->e_func, expr->e_arg, type2);
			error(TYPEERR, "argument has wrong type");
		}
		return deref(type1)->c_targ2;
	otherwise:
		NOT_REACHED;
	}
}

local Cell *
ty_pattern(Expr *pattern, int level)
{
	local_var = variables[level];
	return ty_expr(pattern);
}

local Cell *
ty_if(Expr *expr)
{
	Cell	*type1, *type2;
	Expr	*if_expr, *then_expr, *else_expr;

	if_expr = expr->e_func->e_func->e_arg;
	then_expr = expr->e_func->e_arg;
	else_expr = expr->e_arg;
	type1 = ty_expr(if_expr);
	if (! unify(type1, new_const_type(truval))) {
		show_expr_type(if_expr, type1);
		error(TYPEERR, "predicate is not a truth value");
	}

	type1 = ty_expr(then_expr);
	type2 = ty_expr(else_expr);
	if (! unify(type1, type2)) {
		show_expr_type(then_expr, type1);
		show_expr_type(else_expr, type2);
		error(TYPEERR, "conflict between branches of conditional");
	}
	return type1;
}

/*
 *	A' |- pat: t1
 *	A, A' |- val: t2
 *	A |- exp: t1
 *	--------------------
 *	A |- LET pat == exp IN val: t2
 */
local Cell *
ty_eqn(Branch *branch, Expr *expr)
{
	Cell	*pat_type, *exp_type, *val_type;

	new_vars(branch->br_formals->e_nvars);
	pat_type = ty_pattern(branch->br_formals->e_arg, 0);
	val_type = ty_expr(branch->br_expr);
	del_vars();
	exp_type = ty_expr(expr);
	if (! unify(pat_type, exp_type)) {
		show_expr_type(branch->br_formals->e_arg, pat_type);
		show_expr_type(expr, exp_type);
		error(TYPEERR, "sides of equation have conflicting types");
	}
	return val_type;
}

/*
 *	A' |- pat: t1
 *	A, A' |- val: t2
 *	A, A' |- exp: t1
 *	--------------------
 *	A |- LETREC pat == exp IN val: t2
 */
local Cell *
ty_rec_eqn(Branch *branch, Expr *expr)
{
	Cell	*pat_type, *exp_type, *val_type;

	new_vars(branch->br_formals->e_nvars);
	pat_type = ty_pattern(branch->br_formals->e_arg, 0);
	val_type = ty_expr(branch->br_expr);
	exp_type = ty_expr(expr);
	del_vars();
	if (! unify(pat_type, exp_type)) {
		show_expr_type(branch->br_formals->e_arg, pat_type);
		show_expr_type(expr, exp_type);
		error(TYPEERR, "sides of equation have conflicting types");
	}
	return val_type;
}

/*
 *	A' |- pat: t
 *	A, A' |- exp: t
 *	--------------------
 *	A |- MU pat => exp : t
 */
local Cell *
ty_mu_expr(Expr *muvar, Expr *body)
{
	Cell	*pat_type, *exp_type;

	new_vars(muvar->e_nvars);
	pat_type = ty_pattern(muvar->e_arg, 0);
	exp_type = ty_expr(body);
	del_vars();
	if (! unify(pat_type, exp_type)) {
		show_expr_type(muvar->e_arg, pat_type);
		show_expr_type(body, exp_type);
		error(TYPEERR, "pattern and body have conflicting types");
	}
	return exp_type;
}

/*
 *	A |- b1: t
 *	...
 *	A |- bn: t
 *	--------------
 *	A |- (lambda b1 | ... | bn): t
 */
local Cell *
ty_list(Branch *branch)
{
	Cell	*type;
	Branch	*br;

	type = ty_branch(branch);
	for (br = branch->br_next; br != NULL; br = br->br_next)
		if (! unify(type, ty_branch(br))) {
			show_branch(branch);
			error(TYPEERR, "alternatives have incompatible types");
		}
	return type;
}

/*
 *	A1 |- p1: t1
 *	...
 *	An |- pn: tn
 *	A, A1, ..., An |- e: t
 *	-----------------
 *	A |- (p1 ... pn => e): t1 -> ... -> tn -> t
 *
 * Because pn is at the front of the list, and we want to check e after
 * checking the p's, a rather messy recursion is required.
 */
local Cell *
ty_branch(Branch *branch)
{
	Cell	*type;
	Cell	**tp;
	Expr	*formals;

	type = ty_formals(branch->br_formals, NOCELL);
	/* plug the result type */
	for (tp = &type; *tp != NOCELL; tp = &((*tp)->c_targ2))
		;
	*tp = ty_expr(branch->br_expr);
	/* delete all the variables pushed by ty_formals() */
	for (formals = branch->br_formals;
	     formals != NULL && formals->e_class == E_APPLY;
	     formals = formals->e_func)
		del_vars();
	return type;
}

local Cell *
ty_formals(Expr *formals, Cell *type)
{
	Cell	*newtype;

	if (formals == NULL || formals->e_class != E_APPLY)
		return type;
	newtype = new_func_type(NOCELL, type);
	type = ty_formals(formals->e_func, newtype);
	new_vars(formals->e_nvars);
	newtype->c_targ1 = ty_pattern(formals->e_arg, 0);
	return type;
}

local DefType *
get_functor(Expr *expr)
{
	switch (expr->e_class) {
	case E_DEFUN:
		if (! expr->e_defun->f_explicit_dec)
			return expr->e_defun->f_tycons;
		else
			return NULL;
	otherwise:
		return NULL;
	}
}

/*
 *	Type variable scopes.
 */

local void
init_vars(void)
{
static	Cell	*first_vtype[MAX_VARIABLES];
static	Cell	**local_table[MAX_SCOPES];

	start_heap();
	next_vtype = first_vtype;
	variables = local_table + MAX_SCOPES;
	init_pr_ty_value();
}

/*
 *	New scope: allocate and initialize a new type variable
 *	for each program variable introduced in the branch.
 */
local void
new_vars(int n)
{
	*--variables = next_vtype;
	/*
	 *	x1: alpha1, ..., xn: alphan
	 */
	while (n-- > 0)
		*next_vtype++ = new_tvar();
}

local void
del_vars(void)
{
	next_vtype = *variables++;
}

/*
 *	Display various things and their types,
 *	to enlighten the user about a type error.
 */

local void
show_argument(Expr *func, Expr *arg, Cell *arg_type)
{
	String	name;

	if (arg->e_class == E_PAIR &&
	    (name = expr_name(func, MAX_SCOPES)) != NULL &&
	    op_lookup(name) != NULL) {
		arg_type = deref(arg_type);
		show_expr_type(arg->e_left, arg_type->c_targ1);
		show_expr_type(arg->e_right, arg_type->c_targ2);
	}
	else
		show_expr_type(arg, arg_type);
}

local void
show_expr_type(Expr *expr, Cell *type)
{
	start_err_line();
	(void)fprintf(errout, "  ");
	pr_expr(errout, expr);
	(void)fprintf(errout, " : ");
	pr_ty_value(errout, type);
	(void)fprintf(errout, "\n");
}

local void
show_expr(Expr *expr)
{
	start_err_line();
	(void)fprintf(errout, "  ");
	pr_expr(errout, expr);
	(void)fprintf(errout, "\n");
}

local void
show_branch(Branch *branch)
{
	for ( ; branch != NULL; branch = branch->br_next) {
		start_err_line();
		(void)fprintf(errout, "  ");
		pr_branch(errout, branch, MAX_SCOPES);
		(void)fprintf(errout, " : ");
		pr_ty_value(errout, ty_branch(branch));
		(void)fprintf(errout, "\n");
	}
}
