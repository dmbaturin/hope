#include "defs.h"
#include "functor_type.h"
#include "deftype.h"
#include "type_value.h"
#include "error.h"

/*
 *	Generate types of 'functors'.
 */

local	int	num_tvars_in(TypeList *varlist);

local	Cell	*fold_typelist(TypeList *varlist, Cell **targ, Cell *finish,
			Cell *(*fn)(Type *head, Cell *tail, Cell **targ));

local	Cell	*result_domain(Type *head, Cell *tail, Cell **targ);
local	Cell	*result_range(Type *head, Cell *tail, Cell **targ);
local	Cell	*tupled_args(Type *head, Cell *tail, Cell **targ);
local	Cell	*curried_args(Type *head, Cell *tail, Cell **targ);

local	Cell	*functor_arg(Type *tvar, Cell **targ);

global Cell *
functor_type(DefType *dt)
{
	int	ntvars;
	int	i;
	Cell	*result_type;
	Cell	*type_arg[MAX_TVARS_IN_TYPE];

	ntvars = num_tvars_in(dt->dt_varlist);
	for (i = 0; i < ntvars; i++)
		type_arg[i] = new_tvar();

	result_type = new_func_type(
			new_tcons(dt,
				fold_typelist(dt->dt_varlist, type_arg,
						NOCELL, result_domain)),
			new_tcons(dt,
				fold_typelist(dt->dt_varlist, type_arg,
						NOCELL, result_range)));
	return expand_type(
	    dt->dt_tupled ?
		new_func_type(fold_typelist(dt->dt_varlist, type_arg,
						NOCELL, tupled_args),
				result_type) :
		fold_typelist(dt->dt_varlist, type_arg,
				result_type, curried_args));
}

local int
num_tvars_in(TypeList *varlist)
{
	Type	*head;
	int	ntvars;

	ntvars = 0;
	for ( ; varlist != NULL; varlist = varlist->ty_tail) {
		head = varlist->ty_head;
		ntvars += (head->ty_pos ? 1 : 2) * (head->ty_neg ? 1 : 2);
	}
	return ntvars;
}

/*
 *	NB: functor argument types must be generated in order left-to-right
 *	so that they get the right variable numbers.
 */

local Cell *
fold_typelist(TypeList *varlist, Cell **targ, Cell *finish,
		Cell *(*fn)(Type *head, Cell *tail, Cell **targ))
{
	Type	*head;

	if (varlist == NULL)
		return finish;
	head = varlist->ty_head;
	return (*fn)(head,
		     fold_typelist(
			varlist->ty_tail,
			targ + (head->ty_pos ? 1 : 2) * (head->ty_neg ? 1 : 2),
			finish,
			fn),
		     targ);
}

local Cell *
result_domain(Type *head, Cell *tail_type, Cell **targ)
{
	return new_tlist(*targ, tail_type);
}

local Cell *
result_range(Type *head, Cell *tail_type, Cell **targ)
{
	return new_tlist(*(targ + (head->ty_pos && head->ty_neg ? 0 : 1)),
			tail_type);
}

local Cell *
tupled_args(Type *head, Cell *tail_type, Cell **targ)
{
	Cell	*head_type;

	head_type = functor_arg(head, targ);
	return tail_type == NULL ? head_type :
		new_prod_type(head_type, tail_type);
}

local Cell *
curried_args(Type *head, Cell *tail_type, Cell **targ)
{
	return new_func_type(functor_arg(head, targ), tail_type);
}

local Cell *
functor_arg(Type *tvar, Cell **targ)
{
	Cell	*domain_type, *range_type;

	if (tvar->ty_pos) {
		if (tvar->ty_neg)
			domain_type = range_type = *targ;
		else {
			domain_type = *targ;
			range_type = *(targ+1);
		}
	} else {
		if (tvar->ty_neg) {
			domain_type = *(targ+1);
			range_type = *targ;
		}
		else {
			domain_type = *(targ+2);
			range_type = *(targ+3);
		}
	}
	return new_func_type(domain_type, range_type);
}
