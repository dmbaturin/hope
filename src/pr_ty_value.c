/*
 *	Printing of inferred types.
 */

#include "defs.h"
#include "pr_ty_value.h"
#include "typevar.h"
#include "deftype.h"
#include "type_value.h"
#include "op.h"
#include "print.h"
#include "names.h"
#include "error.h"

local	void	pr_c_ty_value(FILE *f, Cell *type, int context);
local	int	n_ty_precedence(Cell *type);

local	Bool	occurs(Cell *type1, Cell *type2);

/* number of known type variables (including mu's) */
local	int	var_count;

global void
init_pr_ty_value(void)
{
	var_count = 0;
}

global void
pr_ty_value(FILE *f, Cell *type)
{
	pr_c_ty_value(f, type, PREC_BODY);
}

/*
 *	Print a type.
 *	The occurs check for mu-types makes this quadratic, but I can't
 *	think of anything better (and maybe it's not too bad).
 */
local void
pr_c_ty_value(FILE *f, Cell *type, int context)
{
	Op	*op;
	int	prec;
	Bool	is_mu;
	DefType	*tcons;
	Cell	*targ;

	type = deref(type);
	is_mu = type->c_class == C_VOID || occurs(type, type);
	prec = is_mu ? PREC_MU : n_ty_precedence(type);

	if (prec < context)
		(void)fprintf(f, "(");

	if (is_mu) {
		var_count++;
		type->c_varno = var_count;
		(void)fprintf(f, "%s ", n_mu);
		tv_print(f, (Natural)(type->c_varno - 1));
		(void)fprintf(f, " %s ", n_gives);
	}

	switch (type->c_class) {
	case C_TVAR:
		if (type->c_varno == 0) {
			var_count++;
			type->c_varno = var_count;
		}
		tv_print(f, (Natural)(type->c_varno - 1));
	when C_VOID:
		tv_print(f, (Natural)(type->c_varno - 1));
	when C_TCONS:
		ASSERT( type->c_abbr->c_class == C_TSUB );
		tcons = type->c_abbr->c_tcons;
		targ = type->c_abbr->c_targ;
		ASSERT( tcons->dt_arity == 0 || targ->c_class == C_TLIST );
		/* mark it as a VAR in case we encounter it recursively */
		type->c_class = C_TVAR;
		if (tcons->dt_arity == 2 && tcons->dt_tupled &&
		    (op = op_lookup(tcons->dt_name)) != NULL) {
						/* infix */
			pr_c_ty_value(f, targ->c_head, LeftPrec(op));
			(void)fprintf(f, " %s ", tcons->dt_name);
			pr_c_ty_value(f, targ->c_tail->c_head, RightPrec(op));
		} else if (tcons->dt_tupled) {
			(void)fprintf(f, "%s (", tcons->dt_name);
			pr_c_ty_value(f, targ->c_head, PREC_BODY);
			for (targ = targ->c_head;
			     targ != NOCELL;
			     targ = targ->c_tail) {
				ASSERT( targ->c_class == C_TLIST );
				(void)fprintf(f, ", ");
				pr_c_ty_value(f, targ->c_head, PREC_BODY);
			}
			(void)fprintf(f, ")");
		} else {
			(void)fprintf(f, "%s", tcons->dt_name);
			for ( ; targ != NOCELL; targ = targ->c_tail) {
				ASSERT( targ->c_class == C_TLIST );
				(void)fprintf(f, " ");
				pr_c_ty_value(f, targ->c_head, PREC_ARG);
			}
		}
		type->c_class = C_TCONS;
	otherwise:
		NOT_REACHED;
	}

	if (prec < context)
		(void)fprintf(f, ")");
}

/*
 *	Does type1 occur as a proper sub-type of type2?
 *	(both are dereferenced.)
 */
local Bool
occurs(Cell *type1, Cell *type2)
{
	Cell	*arg;
	Cell	*type;

	if (type2->c_class == C_TCONS) {
		/* mark it in case we encounter it recursively */
		type2->c_class = C_VISITED;
		for (arg = type2->c_abbr->c_targ;
		     arg != NOCELL;
		     arg = arg->c_tail) {
			ASSERT( arg->c_class == C_TLIST );
			type = deref(arg->c_head);
			if (type1 == type || occurs(type1, type)) {
				type2->c_class = C_TCONS;
				return TRUE;
			}
		}
		type2->c_class = C_TCONS;
	}
	return FALSE;
}

local int
n_ty_precedence(Cell *type)
{
	Op	*op;
	DefType	*tcons;

	switch (type->c_class) {
	case C_VOID:
		return PREC_MU;
	case C_TCONS:
		tcons = type->c_abbr->c_tcons;
		if (tcons->dt_arity == 0)
			return PREC_ATOMIC;
		if (tcons->dt_arity == 2 &&
		    (op = op_lookup(tcons->dt_name)) != NULL)
			return op->op_prec;
		return PREC_APPLY;
	when C_TVAR:
		return PREC_ATOMIC;
	otherwise:
		NOT_REACHED;
	}
}
