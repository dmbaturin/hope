#include "defs.h"
#include "number.h"
#include "expr.h"
#include "cons.h"
#include "pr_expr.h"
#include "error.h"
#include "path.h"

/*
 *	Resolve identifiers.
 */

local	Bool	nv_branch(Branch *branch);
local	Bool	nv_rec_eqn(Branch *branch, Expr *arg);
local	Bool	nv_mu_expr(Expr *muvar, Expr *body);
local	Bool	nv_pattern(Expr *p, Path path);
local	Bool	nv_constructor(Expr *p, int level, Path *pathp);
local	Bool	nv_expr(Expr *expr);
local	Bool	nv_var(Expr *expr);

local	int	arity_formals(Expr *formals);

/*
 *	Management of variable scopes
 */
local	Expr	**base_var;
local	Expr	**next_var;

local	Expr	***base_level;
local	Expr	***ref_level;

local	Bool	enter_scope(Expr *formals);
local	void	leave_scope(Expr *formals);

global Bool
nr_branch(Branch *branch)
{
	Expr	*varlist[MAX_VARIABLES];
	Expr	**scope[MAX_SCOPES];

	ref_level = base_level = scope;
	scope[0] = next_var = base_var = varlist;
	return nv_branch(branch);
}

local Bool
nv_branch(Branch *branch)
{
	if (! enter_scope(branch->br_formals))
		return FALSE;
	if (! nv_expr(branch->br_expr))
		return FALSE;
	leave_scope(branch->br_formals);
	return TRUE;
}

local Bool
nv_rec_eqn(Branch *branch, Expr *arg)
{
	if (! enter_scope(branch->br_formals))
		return FALSE;
	if (! nv_expr(branch->br_expr) || ! nv_expr(arg))
		return FALSE;
	leave_scope(branch->br_formals);
	return TRUE;
}

local Bool
nv_mu_expr(Expr *muvar, Expr *body)
{
	if (! enter_scope(muvar))
		return FALSE;
	if (! nv_expr(body))
		return FALSE;
	leave_scope(muvar);
	return TRUE;
}

local Bool
enter_scope(Expr *formals)
{
	if (formals != NULL && formals->e_class == E_APPLY) {
		/*
		 * The first argument in the reverse-order list
		 * is the innermost scope.
		 */
		if (! enter_scope(formals->e_func))
			return FALSE;
		if (! nv_pattern(formals->e_arg, p_new()))
			return FALSE;
		formals->e_nvars = next_var - *ref_level;
		if (ref_level - base_level == MAX_SCOPES-1) {
			error(SEMERR, "too many nested lambdas/lets/wheres");
			return FALSE;
		}
		*++ref_level = next_var;
	}
	return TRUE;
}

local void
leave_scope(Expr *formals)
{
	ref_level -= arity_formals(formals);
	next_var = *ref_level;
}

local Bool
nv_pattern(Expr *p, Path path)
{
	Expr	**vp;
	int	i;
	Cons	*cp;
	Expr	*arg;

	if (p == NULL)		/* error in apply_pat() */
		return FALSE;
	switch (p->e_class) {
	case E_NUM or E_CHAR:
		return TRUE;
	when E_PAIR:
		return nv_pattern(p->e_left, p_push(P_LEFT, path)) &&
			nv_pattern(p->e_right, p_push(P_RIGHT, path));
	when E_APPLY:
		if (p->e_func->e_class == E_VAR &&
		    p->e_func->e_vname == newstring("+") &&
		    p->e_arg->e_class == E_PAIR &&
		    p->e_arg->e_right->e_class == E_NUM) {
			/* change to a PLUS */
			arg = p->e_arg;
			p->e_class = E_PLUS;
			p->e_incr = (int)(arg->e_right->e_num);
			p->e_rest = arg->e_left;
			for (i = 0; i < p->e_incr; i++)
				path = p_push(P_PRED, path);
			return nv_pattern(p->e_rest, path);
		}
		return nv_constructor(p, 0, &path);
	when E_VAR:
		if ((cp = cons_lookup(p->e_vname)) != NULL &&
		    cp->c_nargs == 0) {
			p->e_class = E_CONS;
			p->e_const = cp;
			return TRUE;
		}
		if (p->e_vname != newstring("_"))
			for (vp = *ref_level; vp != next_var; vp++)
				if ((*vp)->e_vname == p->e_vname) {
					error(SEMERR,
						"%s: occurs twice in pattern",
						p->e_vname);
					return FALSE;
				}
		p->e_var = next_var - *ref_level;
		p->e_dirs = p_stash(p_reverse(path));
		if (next_var - base_var == MAX_VARIABLES-1) {
			error(SEMERR, "too many variables in patterns");
			return FALSE;
		}
		*next_var++ = p;
		return TRUE;
	when E_CONS:
		if (p->e_const->c_nargs == 0)
			return TRUE;
	otherwise:
		NOT_REACHED;
	}
	start_err_line();
	(void)fprintf(errout, "  ");
	pr_expr(errout, p);
	(void)fprintf(errout, "\n");
	error(SEMERR, "illegal pattern");
	return FALSE;
}

/*
 * A constructed pattern
 *	(...((c p1) p2) ... pn-1) pn
 * is matched against a value represented as (cf interpret.c):
 *	c(v1, (v2, ... (vn-1, vn)...))
 * so that the path for pi+1 can be derived from that for pi.
 * Hence the wierd bottom-up construction of paths here.
 */
local Bool
nv_constructor(Expr *p, int level, Path *pathp)
{
	Cons	*cp;

	switch (p->e_class) {
	case E_VAR:
		if ((cp = cons_lookup(p->e_vname)) == NULL) {
			error(SEMERR, "'%s': unknown constructor",
				p->e_vname);
			return FALSE;
		}
		if (cp->c_nargs != level) {
			error(SEMERR, "'%s': incorrect arity", cp->c_name);
			return FALSE;
		}
		p->e_class = E_CONS;
		p->e_const = cp;
		*pathp = p_push(cp == succ ? P_PRED : P_STRIP, *pathp);
		return TRUE;
	when E_CONS:
		cp = p->e_const;
		if (cp->c_nargs != level) {
			error(SEMERR, "'%s': incorrect arity", cp->c_name);
			return FALSE;
		}
		*pathp = p_push(cp == succ ? P_PRED : P_STRIP, *pathp);
		return TRUE;
	when E_APPLY:
		if (! nv_constructor(p->e_func, level+1, pathp))
			return FALSE;
		if (level > 0) {
			if (! nv_pattern(p->e_arg, p_push(P_LEFT, *pathp)))
				return FALSE;
			*pathp = p_push(P_RIGHT, *pathp);
			return TRUE;
		}
		/* last argument */
		return nv_pattern(p->e_arg, *pathp);
	otherwise:
		start_err_line();
		(void)fprintf(errout, "  ");
		pr_expr(errout, p);
		(void)fprintf(errout, "\n");
		error(SEMERR, "constructor required");
		return FALSE;
	}
}

local Bool
nv_expr(Expr *expr)
{
	Branch	*br;

	switch (expr->e_class) {
	case E_NUM or E_CHAR or E_CONS:
		return TRUE;
	when E_PAIR:
		return nv_expr(expr->e_left) && nv_expr(expr->e_right);
	when E_APPLY or E_IF or E_WHERE or E_LET:
		return nv_expr(expr->e_func) && nv_expr(expr->e_arg);
	when E_RLET or E_RWHERE:
		return nv_rec_eqn(expr->e_func->e_branch, expr->e_arg);
	when E_MU:
		return nv_mu_expr(expr->e_muvar, expr->e_body);
	when E_LAMBDA or E_EQN or E_PRESECT or E_POSTSECT:
		for (br = expr->e_branch; br != NULL; br = br->br_next) {
			if (arity_formals(br->br_formals) != expr->e_arity) {
				start_err_line();
				(void)fprintf(errout, "  ");
				pr_expr(errout, expr);
				(void)fprintf(errout, "\n");
				error(SEMERR,
					"branches have different arities");
				return FALSE;
			}
			if (! nv_branch(br))
				return FALSE;
		}
		return TRUE;
	when E_VAR:
		return nv_var(expr);
	otherwise:
		NOT_REACHED;
	}
}

local int
arity_formals(Expr *formals)
{
	int	n;

	n = 0;
	for ( ; formals != NULL && formals->e_class == E_APPLY;
	     formals = formals->e_func)
		n++;
	return n;
}

local Bool
nv_var(Expr *expr)
{
	String	name;
	Expr	**vp;
	Expr	***def_level;

	name = expr->e_vname;
	ASSERT( next_var == *ref_level );
	for (vp = next_var-1; vp >= base_var; vp--)
		if ((*vp)->e_vname == name) {
			expr->e_class = E_PARAM;
			expr->e_patt = *vp;
			for (def_level = base_level;
			     *def_level <= vp;
			     def_level++)
				;
			expr->e_level = ref_level - def_level;
			expr->e_where = (*vp)->e_dirs;
			return TRUE;
		}
	/* fiddle: succ constructor becomes succ function */
	if ((expr->e_const = cons_lookup(name)) != NULL &&
	    expr->e_const != succ) {
		expr->e_class = E_CONS;
		return TRUE;
	}
	if ((expr->e_defun = fn_lookup(name)) != NULL) {
		expr->e_class = E_DEFUN;
		return TRUE;
	}
	error(SEMERR, "%s: undefined variable", name);
	return FALSE;
}
