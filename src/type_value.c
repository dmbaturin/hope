#include "defs.h"
#include "type_value.h"
#include "deftype.h"
#include "type_check.h"
#include "error.h"

/* maximum no. of instantiations in a single call to unify() (not checked) */
#define	MAX_INSTANTIATIONS	80

typedef struct {
	Cell	*location;
	Cell	old_value;
} Trail;

typedef struct {
	DefType	*type_syn;
	Cell	*type_args;
	Cell	*type_value;
} Memo;

local	Bool	real_unify(Cell *type1, Cell *type2);
local	void	assign(Cell *var, Cell *type);
local	void	assign_no_trail(Cell *var, Cell *type);

local	void	identify_types(Cell *type1, Cell *type2);
local	void	expand_aux(Cell *type, Memo *last_type_memo);
local	Bool	same_args(Cell *tp1, Cell *tp2);

local	Cell	*cp_type(Type *type, Cell *type_arg);
local	Cell	*cp_type_aux(Type *type, Cell *type_arg, Cell **mu_top);
local	Cell	*cp_list(TypeList *typelist, Cell *type_arg, Cell **mu_top);

local	Cell	*type_var_list(Natural n, Cell *(*elt)(void));
local	Cell	*type_arg_lookup(Cell *type_arg, Natural n);

local	void	add_trail(Cell *cp);
local	void	untrail(Trail *tp);

/* stack of cells changed by the current unification */
local	Trail	*top_trail;

/*
 *	Is type an instance of inf_type?
 *	i.e. can they be unified without instantiating the variables
 *	of type?
 */
global Bool
instance(Type *type, Natural ntvars, Cell *inf_type)
{
	return unify(inf_type, copy_type(type, ntvars, TRUE));
}

/*
 *	Unification of regular type terms.
 *	Proceeds by direct modification of type cells.
 *	If the unification is unsuccessful, nothing is changed.
 */
global Bool
unify(Cell *type1, Cell *type2)
{
	Trail	trail[MAX_INSTANTIATIONS];

	top_trail = trail;
	if (real_unify(type1, type2))
		return TRUE;
	/* unification has failed: undo any instantiations */
	untrail(trail);
	return FALSE;
}

local Bool
real_unify(Cell *type1, Cell *type2)
{
	DefType	*tcons1, *tcons2;
	Cell	*targ1, *targ2;

	type1 = deref(type1);
	type2 = deref(type2);
	if (type1 == type2)
		return TRUE;
	/* if either is a variable, succeed by instantiation */
	if (type1->c_class == C_TVAR) {
		assign(type1, type2);
		return TRUE;
	}
	if (type2->c_class == C_TVAR) {
		assign(type2, type1);
		return TRUE;
	}
	/* fail if different frozen variables */
	if (type1->c_class == C_FROZEN || type2->c_class == C_FROZEN)
		return FALSE;
	/* if either is void, the other must also be void */
	if (type1->c_class == C_VOID)
		return type2->c_class == C_VOID;
	if (type2->c_class == C_VOID)
		return FALSE;
	/* both are data constructed types */
	ASSERT( type1->c_class == C_TCONS );
	ASSERT( type1->c_full->c_class == C_TSUB );
	ASSERT( type2->c_class == C_TCONS );
	ASSERT( type2->c_full->c_class == C_TSUB );
	tcons1 = type1->c_full->c_tcons;
	tcons2 = type2->c_full->c_tcons;
	ASSERT( tcons1->dt_syn_depth == 0 );
	ASSERT( tcons2->dt_syn_depth == 0 );
	if (tcons1 != tcons2)
		return FALSE;	/* different data type constructors */
	/*
	 * Unification of regular trees:
	 * equate the cons codes before looking at args.
	 * (This will be undone if something fails.)
	 */
	targ1 = type1->c_full->c_targ;
	targ2 = type2->c_full->c_targ;
	identify_types(type1, type2);
	/* same type constructor (implies same no. of arguments) */
	while (targ1 != NOCELL) {
		ASSERT( targ2 != NOCELL );
		ASSERT( targ1->c_class == C_TLIST );
		ASSERT( targ2->c_class == C_TLIST );
		if (! real_unify(targ1->c_head, targ2->c_head))
			return FALSE;
		targ1 = targ1->c_tail;
		targ2 = targ2->c_tail;
	}
	ASSERT( targ2 == NOCELL );
	return TRUE;
}

/*
 * Identify two constructed types, by making the shallower a reference
 * to the deeper.
 * (Doing it that way gives more compact presentations of inferred types.)
 */
local void
identify_types(Cell *type1, Cell *type2)
{
	ASSERT( type1->c_class == C_TCONS );
	ASSERT( type1->c_abbr->c_class == C_TSUB );
	ASSERT( type1->c_full->c_class == C_TSUB );
	ASSERT( type2->c_class == C_TCONS );
	ASSERT( type2->c_abbr->c_class == C_TSUB );
	ASSERT( type2->c_full->c_class == C_TSUB );
	if (type1->c_abbr->c_tcons->dt_syn_depth <
	    type2->c_abbr->c_tcons->dt_syn_depth)
		assign(type1, type2);
	else
		assign(type2, type1);
}

/*
 *	Follow a chain of instantiated variables to either a constructor or
 *	an uninstantiated variable.
 */
global Cell *
deref(Cell *cell)
{
	while (cell->c_class == C_TREF)
		cell = cell->c_tref;
	return cell;
}

/*
 *	Assign a dereferenced term to a cell.
 */
local void
assign(Cell *var, Cell *type)
{
	add_trail(var);
	if (type == var)
		var->c_class = C_VOID;
	else {
		var->c_class = C_TREF;
		var->c_tref = type;
	}
}

local void
assign_no_trail(Cell *abbr, Cell *full)
{
	ASSERT( abbr->c_class == C_TCONS );
	if (abbr == full)
		abbr->c_class = C_VOID;
	else {
		if (full->c_class == C_TCONS &&
		    full->c_abbr->c_tcons->dt_syn_depth <
		    abbr->c_abbr->c_tcons->dt_syn_depth)
			full->c_abbr = abbr->c_abbr;
		abbr->c_class = C_TREF;
		abbr->c_tref = full;
	}
}

/*
 *	The trail.
 */

local void
add_trail(Cell *cp)
{
	top_trail->location = cp;
	top_trail->old_value = *cp;
	top_trail++;
}

local void
untrail(Trail *tp)
{
	while (top_trail > tp) {
		top_trail--;
		*(top_trail->location) = top_trail->old_value;
	}
}

/*
 *	Memoized expansion of type synonyms.
 */

local	Memo	*first_type_memo;

global Cell *
expand_type(Cell *type)
{
	Memo	type_memo[MAX_SYN_DEPTH];

	first_type_memo = type_memo;
	expand_aux(type, type_memo);
	return deref(type);
}

local void
expand_aux(Cell *type, Memo *last_type_memo)
{
	DefType	*tcons;
	Cell	*targ;
	Memo	*memo;
	Cell	*newtype;

	type = deref(type);
	if (type->c_class != C_TCONS)
		return;
	ASSERT( type->c_abbr->c_class == C_TSUB );
	ASSERT( type->c_full->c_class == C_TSUB );
	tcons = type->c_full->c_tcons;
	targ = type->c_full->c_targ;
	if (tcons->dt_syn_depth == 0) {
		/* data type constructor: expand the arguments */
		/* mark it in case we encounter it recursively */
		type->c_class = C_VISITED;
		for ( ; targ != NOCELL; targ = targ->c_tail) {
			ASSERT( targ->c_class == C_TLIST );
			expand_aux(targ->c_head, last_type_memo);
		}
		type->c_class = C_TCONS;
	} else {	/* type synonym: expand it */
		/* have we expanded this one before? */
		for (memo = first_type_memo; memo != last_type_memo; memo++)
			if (tcons == memo->type_syn &&
			    same_args(targ, memo->type_args)) {
				assign_no_trail(type, deref(memo->type_value));
				return;
			}

		/* No?  Then do the expansion */
		newtype = cp_type(tcons->dt_type, targ);
		assign_no_trail(type, newtype);

		/* remember it for next time */
		last_type_memo->type_syn = tcons;
		last_type_memo->type_args = targ;
		last_type_memo->type_value = newtype;

		expand_aux(newtype, last_type_memo+1);
	}
}

local Bool
same_args(Cell *tp1, Cell *tp2)	/* known to have the same length */
{
	while (tp1 != NOCELL) {
		ASSERT( tp2 != NOCELL );
		ASSERT( tp1->c_class == C_TLIST );
		ASSERT( tp2->c_class == C_TLIST );
		if (tp1->c_head != tp2->c_head)
			return FALSE;
		tp1 = tp1->c_tail;
		tp2 = tp2->c_tail;
	}
	ASSERT( tp2 == NOCELL );
	return TRUE;
}

/*
 *	Make a temporary copy of a type.
 */
global Cell *
copy_type(Type *type, Natural ntvars, Bool frozen)
{
	return expand_type(cp_type(type,
			     type_var_list(ntvars,
					   frozen ? new_frozen : new_tvar)));
}

local Cell *
type_var_list(Natural n, Cell *(*elt)(void))
{
	Cell	*var_list;
	Natural i;

	var_list = NOCELL;
	for (i = 0; i < n; i++)
		var_list = new_tlist((*elt)(), var_list);
	return var_list;
}

/*
 * return a copy of the type, guaranteed not a reference.
 */
local Cell *
cp_type(Type *type, Cell *type_arg)
{
	Cell	*mu_var[MAX_MU_DEPTH];

	return cp_type_aux(type, type_arg, mu_var);
}

local Cell *
cp_type_aux(Type *type, Cell *type_arg, Cell **mu_top)
{
	Cell	*ty_value;

	switch (type->ty_class) {
	case TY_VAR:
		return deref(type->ty_mu_bound ?
				*(mu_top - type->ty_index - 1) :
				type_arg_lookup(type_arg, type->ty_index));
	case TY_MU:
		*mu_top = new_void();
		ty_value = cp_type_aux(type->ty_body, type_arg, mu_top+1);
		if (ty_value != *mu_top) {
			(*mu_top)->c_class = C_TREF;
			(*mu_top)->c_tref = ty_value;
		}
		return ty_value;
	case TY_CONS:
		ty_value = new_tcons(type->ty_deftype,
				cp_list(type->ty_args, type_arg, mu_top));
		return ty_value;
	otherwise:
		NOT_REACHED;
	}
}

local Cell *
type_arg_lookup(Cell *type_arg, Natural n)
{
	Natural	i;

	for (i = 0; i < n; i++) {
		ASSERT( type_arg != NOCELL );
		ASSERT( type_arg->c_class == C_TLIST );
		type_arg = type_arg->c_tail;
	}
	ASSERT( type_arg != NOCELL );
	ASSERT( type_arg->c_class == C_TLIST );
	return type_arg->c_head;
}

local Cell *
cp_list(TypeList *typelist, Cell *type_arg, Cell **mu_top)
{
	return typelist == NULL ? NOCELL :
		new_tlist(cp_type_aux(typelist->ty_head, type_arg, mu_top),
			 cp_list(typelist->ty_tail, type_arg, mu_top));
}

/*
 *	Constructors for type values
 */

global Cell *
new_func_type(Cell *from, Cell *to)
{
	return new_tcons(function, new_tlist(from, new_tlist(to, NOCELL)));
}

global Cell *
new_prod_type(Cell *left, Cell *right)
{
	return new_tcons(product, new_tlist(left, new_tlist(right, NOCELL)));
}

global Cell *
new_list_type(Cell *element)
{
	return expand_type(new_tcons(list, new_tlist(element, NOCELL)));
}

global Cell *
new_const_type(DefType *dt)
{
	return new_tcons(dt, NOCELL);
}

/*
 *	General type value constructors
 */

global Cell *
new_tvar(void)
{
	Cell	*cp;

	cp = new_cell(C_TVAR);
	cp->c_varno = 0;
	return cp;
}

global Cell *
new_tsub(DefType *tcons, Cell *targ)
{
	Cell	*cp;

	cp = new_cell(C_TSUB);
	cp->c_varno = 0;
	cp->c_tcons = tcons;
	cp->c_targ = targ;
	return cp;
}

global Cell *
new_tref(Cell *tref)
{
	Cell	*cp;

	cp = new_cell(C_TREF);
	cp->c_varno = 0;
	cp->c_tref = tref;
	return cp;
}

global Cell *
new_void(void)
{
	Cell	*cp;

	cp = new_cell(C_VOID);
	cp->c_varno = 0;
	return cp;
}

global Cell *
new_frozen(void)
{
	Cell	*cp;

	cp = new_cell(C_FROZEN);
	cp->c_varno = 0;
	return cp;
}

global Cell *
new_tsyn(Cell *abbr, Cell *full)
{
	Cell	*cp;

	cp = new_cell(C_TCONS);
	cp->c_abbr = abbr;
	cp->c_full = full;
	return cp;
}

global Cell *
new_tcons(DefType *tcons, Cell *targ)
{
	Cell	*cp;

	cp = new_tsub(tcons, targ);
	return new_tsyn(cp, cp);
}

global Cell *
new_tlist(Cell *head, Cell *tail)
{
	Cell	*cp;

	cp = new_cell(C_TLIST);
	cp->c_head = head;
	cp->c_tail = tail;
	return cp;
}
