#include "defs.h"
#include "polarity.h"
#include "deftype.h"
#include "error.h"
#include "names.h"

/*
 *	Polarities of type constructor arguments.
 */

global String
type_arg_name(Type *var, Bool full)
{
	return full ? var->ty_var :
		var->ty_pos ? (var->ty_neg ? var->ty_var : n_pos) :
			var->ty_neg ? n_neg : n_none;
}

global void
set_polarities(TypeList *varlist)
{
	for ( ; varlist != NULL; varlist = varlist->ty_tail)
		if (varlist->ty_head->ty_var == n_none) {
			varlist->ty_head->ty_pos = FALSE;
			varlist->ty_head->ty_neg = FALSE;
		} else {
			varlist->ty_head->ty_pos =
				varlist->ty_head->ty_var != n_neg;
			varlist->ty_head->ty_neg =
				varlist->ty_head->ty_var != n_pos;
		}
}

global Bool
check_polarities(TypeList *decl_vars, TypeList *def_vars)
{
	while (decl_vars != NULL) {
		if (def_vars->ty_head->ty_pos &&
		    ! decl_vars->ty_head->ty_pos) {
			error(SEMERR, "%s: unexpected positive use",
				def_vars->ty_head->ty_var);
			return FALSE;
		}
		if (def_vars->ty_head->ty_neg &&
		    ! decl_vars->ty_head->ty_neg) {
			error(SEMERR, "%s: unexpected negative use",
				def_vars->ty_head->ty_var);
			return FALSE;
		}
		decl_vars = decl_vars->ty_tail;
		def_vars = def_vars->ty_tail;
	}
	return TRUE;
}

/*
 *	Computing polarities of arguments.
 *
 *	We now permit recursive uses to have a permutation of the original
 *	arguments -- cf check_recursion().  This means:
 *	- if arg i occurs in position j in a positive context, then args
 *	  i and j will have the same polarity.
 *	- if arg i occurs in position j in a negative context, then args
 *	  i and j will have opposite polarities.
 *	Hence we maintain equivalence classes of arguments.
 */
#define	MAX_TYCON_ARITY	10	/* max. arity of a type cons. (not checked) */
#define	NIL	(-1)

local	struct {
	int	equiv;	/* towards root of class, or NIL if this is the root */
	int	dual;	/* root: root of dual class if any */
	Bool	pos;	/* root: any of these variables occur positively */
	Bool	neg;	/* root: any of these variables occur negatively */
} ty_arg[MAX_TYCON_ARITY];

local	DefType	*cur_deftype;		/* type currently being defined */
local	TypeList *cur_varlist;		/* its new formal parameters */

local	int	tycon_find(int n);
local	void	tycon_union(int n1, int n2);
local	void	set_equiv(int n1, int n2);
local	void	set_dual(int n1, int n2);
local	void	set_pos(int n);
local	void	set_neg(int n);
local	void	do_polarities(Type *type, Bool pos, Bool neg);

global void
start_polarities(DefType *deftype, TypeList *varlist)
{
	int	i;

	for (i = deftype->dt_arity-1; i >= 0; i--) {
		ty_arg[i].equiv = ty_arg[i].dual = NIL;
		ty_arg[i].pos = ty_arg[i].neg = FALSE;
	}
	cur_deftype = deftype;
	cur_varlist = varlist;
}

global void
compute_polarities(Type *type)
{
	do_polarities(type, TRUE, FALSE);
}

local void
do_polarities(Type *type, Bool pos, Bool neg)
		/* in a positive and/or negative context */
{
	TypeList *formals;
	TypeList *actuals;

	switch (type->ty_class) {
	case TY_VAR:
		if (pos)
			set_pos(type->ty_index);
		if (neg)
			set_neg(type->ty_index);
	when TY_MU:
		/*
		 *	BUG: if the var occurs negatively in the body,
		 *	should set both pos and neg.
		 */
		do_polarities(type->ty_body, pos, neg);
	when TY_CONS:
		if (type->ty_deftype == cur_deftype) {
			for (actuals = type->ty_args,
			     formals = cur_varlist;
			     actuals != NULL;
			     actuals = actuals->ty_tail,
			     formals = formals->ty_tail) {
				if (pos)
					set_equiv(formals->ty_head->ty_index,
						  actuals->ty_head->ty_index);
				if (neg)
					set_dual(formals->ty_head->ty_index,
						 actuals->ty_head->ty_index);
			}
		} else
			for (actuals = type->ty_args,
			     formals = type->ty_deftype->dt_varlist;
			     actuals != NULL;
			     actuals = actuals->ty_tail,
			     formals = formals->ty_tail)
				do_polarities(actuals->ty_head,
					(pos && formals->ty_head->ty_pos) ||
					    (neg && formals->ty_head->ty_neg),
					(neg && formals->ty_head->ty_pos) ||
					    (pos && formals->ty_head->ty_neg));
	otherwise:
		NOT_REACHED;
	}
}

global void
finish_polarities(void)
{
	TypeList *vp;
	int	n, nd;

	for (vp = cur_varlist; vp != NULL; vp = vp->ty_tail) {
		n = tycon_find(vp->ty_head->ty_index);
		nd = ty_arg[n].dual;
		vp->ty_head->ty_pos = ty_arg[n].pos ||
					(nd != NIL && ty_arg[nd].neg);
		vp->ty_head->ty_neg = ty_arg[n].neg ||
					(nd != NIL && ty_arg[nd].pos);
	}
}

/*
 *	For two roots r1, r2,
 *		ty_arg[r1].dual == r2 <=> ty_arg[r2].dual == r1
 */

local int
tycon_find(int n)
{
	while (ty_arg[n].equiv != NIL)
		n = ty_arg[n].equiv;
	return n;
}

local void
tycon_union(int n1, int n2)
{
	n1 = tycon_find(n1);
	n2 = tycon_find(n2);
	if (n1 != n2) {
		ty_arg[n2].equiv = n1;
		ty_arg[n1].pos = ty_arg[n1].pos || ty_arg[n2].pos;
		ty_arg[n1].neg = ty_arg[n1].neg || ty_arg[n2].neg;
		if (ty_arg[n1].dual == NIL)
			ty_arg[n1].dual = ty_arg[n2].dual;
		else if (ty_arg[n2].dual != NIL)
			tycon_union(ty_arg[n1].dual, ty_arg[n2].dual);
	}
}

local void
set_equiv(int n1, int n2)
{
	tycon_union(n1, n2);
}

local void
set_dual(int n1, int n2)
{
	n1 = tycon_find(n1);
	n2 = tycon_find(n2);
	if (ty_arg[n1].dual != NIL)
		tycon_union(ty_arg[n1].dual, n2);
	else if (ty_arg[n2].dual != NIL)
		tycon_union(ty_arg[n2].dual, n1);
	else {	/* both NIL */
		ty_arg[n1].dual = n2;
		ty_arg[n2].dual = n1;
	}
}

local void
set_pos(int n)
{
	ty_arg[tycon_find(n)].pos = TRUE;
}

local void
set_neg(int n)
{
	ty_arg[tycon_find(n)].neg = TRUE;
}
