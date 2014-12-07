#include "defs.h"
#include "deftype.h"
#include "cons.h"
#include "memory.h"
#include "bad_rectype.h"
#include "functors.h"
#include "expr.h"
#include "polarity.h"
#include "type_check.h"
#include "error.h"

/*
 *	Defined Types.
 */

/* Internal names of some types and constructors. */
global	DefType *product, *function, *list, *num, *truval, *character;
global	Cons	*nil, *cons, *succ, *true, *false;

/*
 *	The type currently being defined.
 *	(not yet recorded in the tables, in case of error,
 *	except in the case of an abstract type being defined)
 */
local	DefType	*cur_deftype;		/* type currently being defined */
local	TypeList *cur_varlist;		/* its new formal parameters */
local	Type	*cur_newtype;		/* application of the above */
local	Bool	already_defined;	/* current type is already defined */

local	String	mu_names[MAX_MU_DEPTH];	/* stack of mu type variables */
local	String	*mu_top;

local	Type	*current_newtype(void);
local	Bool	args_repeated(TypeList *varlist);
local	Bool	is_header(Type *type, DefType *deftype);
local	int	ty_length(TypeList *typelist);

local	int	nr_type(Type *type);
local	void	nv_type(Type *type);
local	int	nv_tvar(String name);

local	Type	*multi_pair_type(TypeList *args);
local	Type	*multi_func_type(TypeList *args, Type *result);

global void
start_dec_type()
{
	cur_deftype = NULL;
	mu_top = mu_names;
}

global DefType *
new_deftype(String name, Bool tupled, TypeList *vars)
{
	DefType *dt;
	TypeList *varp;
	int	arity;
	int	i;

	dt = dt_local(name);
	arity = ty_length(vars);
	already_defined = dt != NULL;
	if (already_defined) {
		if (! IsAbsType(dt)) {
			error(SEMERR, "'%s': attempt to redefine type", name);
			return dt;
		}
		if (arity != dt->dt_arity) {
			error(SEMERR,
				"'%s': wrong number of type arguments",
				name);
			return dt;
		}
		if (tupled != dt->dt_tupled) {
			error(SEMERR,
				"'%s': different argument syntax", name);
			return dt;
		}
	}
	else {
		if (arity > MAX_TVARS_IN_TYPE)
			error(SEMERR, "'%s': too many type arguments", name);
		dt = NEW(DefType);
		dt->dt_name = name;
		dt->dt_syn_depth = 0;
		dt->dt_index = 0;
		dt->dt_tupled = tupled;
		dt->dt_cons = NULL;
		dt->dt_arity = arity;
		dt->dt_varlist = NULL;
	}

	for (varp = vars, i = 0; varp != NULL; varp = varp->ty_tail, i++)
		varp->ty_head->ty_index = i;

	cur_varlist = vars;
	cur_deftype = dt;
	cur_newtype = NULL;
	mu_top = mu_names;
	return dt;
}

local Type *
current_newtype(void)
{
	ASSERT( cur_deftype != NULL );
	if (cur_newtype == NULL)
		cur_newtype = def_type(cur_deftype, cur_varlist);
	return cur_newtype;
}

global void
abstype(DefType *deftype)
{
	if (erroneous || already_defined)
		return;

	/* definition is OK -- add it to the table */
	set_polarities(cur_varlist);
	deftype->dt_varlist = cur_varlist;
	dt_declare(deftype);
	preserve();
}

global void
type_syn(DefType *deftype, Type *type)
{
	if (erroneous || args_repeated(cur_varlist))
		return;

	if (is_header(type, deftype)) {
		error(SEMERR, "'%s': left-recursive type definition",
			deftype->dt_name);
		return;
	}

	if (bad_rectype(deftype, type))
		return;

	start_polarities(deftype, cur_varlist);
	compute_polarities(type);
	finish_polarities();

	if (already_defined &&
	    ! check_polarities(deftype->dt_varlist, cur_varlist))
		return;

	/* definition is OK -- add it to the table */
	deftype->dt_varlist = cur_varlist;
	deftype->dt_type = type;
	while (type->ty_class == TY_MU)
		type = type->ty_body;
	ASSERT( type->ty_class == TY_VAR || type->ty_class == TY_CONS );
	deftype->dt_syn_depth = 1 +
		(type->ty_class == TY_VAR ? 0 :
			type->ty_deftype->dt_syn_depth);
	if (deftype->dt_syn_depth > MAX_SYN_DEPTH)
		error(SEMERR, "type synonyms nested too deeply");

	if (already_defined)
		fix_synonyms();
	else
		dt_declare(deftype);
	def_functor(deftype);
	preserve();
}

/*
 *	Is the expansion of type headed by deftype?
 */
local Bool
is_header(Type *type, DefType *deftype)
{
	for (;;) {
		switch (type->ty_class) {
		case TY_VAR:
			return FALSE;
		when TY_MU:
			type = type->ty_body;
		when TY_CONS:
			if (type->ty_deftype == deftype)
				return TRUE;
			if (! IsSynType(type->ty_deftype))
				return FALSE;
			type = type->ty_deftype->dt_type;
		otherwise:
			NOT_REACHED;
		}
	}
}

global void
decl_type(DefType *deftype, Cons *conslist)
{
	Cons	*ct;
	Type	*type;
	Natural	c_index;
	Func	*fn;

	if (erroneous || args_repeated(cur_varlist))
		return;

	start_polarities(deftype, cur_varlist);
	c_index = 0;
	for (ct = conslist; ct != NULL; ct = ct->c_next) {
		if (cons_local(ct->c_name) != NULL) {
			error(SEMERR, "'%s': attempt to redefine constructor",
				ct->c_name);
			return;
		}

		ct->c_nargs = 0;
		for (type = ct->c_type;
		     type->ty_deftype == function;
		     type = type->ty_secondarg) {
			if (bad_rectype(deftype, type->ty_firstarg))
				return;
			compute_polarities(type->ty_firstarg);
			ct->c_nargs++;
		}

		ct->c_index = c_index++;
		ct->c_ntvars = deftype->dt_arity;

		if ((fn = fn_local(ct->c_name)) != NULL) {
			/* fulfilling a declaration */
			if (fn->f_code != NULL) {
				error(SEMERR,
					"'%s': attempt to redefine value identifier",
					ct->c_name);
				return;
			}
			/* BUG: implicitly defined fns not checked */
			if (! fn->f_explicit_dec &&
			    ! ty_instance(fn->f_type, fn->f_ntvars,
					ct->c_type, ct->c_ntvars)) {
				error(SEMERR,
					"'%s': type does not match declaration",
					ct->c_name);
				return;
			}
		}
	}
	finish_polarities();

	if (already_defined &&
	    ! check_polarities(deftype->dt_varlist, cur_varlist))
		return;

	/* definition is OK -- add it to the table */
	deftype->dt_varlist = cur_varlist;
	deftype->dt_cons = conslist;

	for (ct = conslist; ct != NULL; ct = ct->c_next)
		if ((fn = fn_local(ct->c_name)) != NULL) {
			def_value(id_expr(fn->f_name), cons_expr(ct));
			fn->f_explicit_def = FALSE;
		}
	if (! already_defined)
		dt_declare(deftype);
	def_functor(deftype);
	preserve();
}

/*
 *	Check a list of type variables for repetitions.
 */
local Bool
args_repeated(TypeList *varlist)
{
	TypeList *vp;

	for ( ; varlist != NULL; varlist = varlist->ty_tail)
		for (vp = varlist->ty_tail; vp != NULL; vp = vp->ty_tail)
			if (vp->ty_head->ty_var == varlist->ty_head->ty_var) {
				error(SEMERR, "'%s': parameter is repeated",
					vp->ty_head->ty_var);
				return TRUE;
			}
	return FALSE;
}

/*
 *	Lists of data constructors.
 */

global Cons *
constructor(String name, Bool tupled, TypeList *args)
{
	Cons	*c;

	c = NEW(Cons);
	c->c_name = name;
	c->c_type = tupled ?
			func_type(multi_pair_type(args), current_newtype()) :
			multi_func_type(args, current_newtype());
	c->c_next = NULL;
	return c;
}

global Cons *
alt_cons(Cons *constr, Cons *next)
{
	constr->c_next = next;
	return constr;
}

/*
 *	Type structures.
 */

local int
ty_length(TypeList *typelist)
{
	int	len;

	len = 0;
	for ( ; typelist != NULL; typelist = typelist->ty_tail)
		len++;
	return len;
}

/*
 *	Type constructor application.
 *	If tupled is TRUE, there must be at least 2 arguments.
 */
global Type *
new_type(String name, Bool tupled, TypeList *args)
{
	Type	*type;
	DefType	*deftype;
	TypeList *tparam;
	String	*mu_ptr;

	if (args == NULL) { /* nullary constructor: may be type variable */
		/* is it bound by a mu quantifier? */
		for (mu_ptr = mu_top-1; mu_ptr >= mu_names; mu_ptr--)
			if (*mu_ptr == name) {
				type = new_tv(name);
				type->ty_mu_bound = TRUE;
				type->ty_index = mu_top - mu_ptr - 1;
				return type;
			}
		/* in a type definition, must be a parameter */
		if (cur_deftype != NULL) {
			for (tparam = cur_varlist;
			     tparam != NULL;
			     tparam = tparam->ty_tail)
				if (name == tparam->ty_head->ty_var)
					return tparam->ty_head;
		/* in a value declaration, must be a declared type var */
		} else if (tv_lookup(name))
			return new_tv(name);
	}

	if (cur_deftype != NULL && name == cur_deftype->dt_name)
		deftype = cur_deftype;
	else if ((deftype = dt_lookup(name)) == NULL) {
		error(SEMERR, "'%s' is not a defined type", name);
		deftype = truval;	/* dummy */
	}
	if (deftype->dt_arity != ty_length(args))
		error(SEMERR,
			"'%s': wrong number of type arguments", name);
	else if (deftype->dt_tupled != tupled)
		error(SEMERR, "'%s': different argument syntax", name);

	type = NEW(Type);
	type->ty_class = TY_CONS;
	type->ty_tupled = tupled;
	type->ty_deftype = deftype;
	type->ty_args = args;
	return type;
}

global Type *
def_type(DefType *dt, TypeList *args)
{
	Type	*type;

	type = NEW(Type);
	type->ty_class = TY_CONS;
	type->ty_deftype = dt;
	type->ty_args = args;
	return type;
}

/*
 *	Enter a mu scope.
 */
global void
enter_mu_tv(String name)
{
	*mu_top++ = name;
}

/*
 *	Leave a mu scope, building the mu type.
 */
global Type *
mu_type(Type *body)
{
	Type	*type;

	type = NEW(Type);
	type->ty_class = TY_MU;
	type->ty_muvar = *--mu_top;
	type->ty_body = body;
	return type;
}

global Type *
new_tv(TVar tvar)
{
	Type	*type;

	type = NEW(Type);
	type->ty_class = TY_VAR;
	type->ty_mu_bound = FALSE;
	type->ty_var = tvar;
	return type;
}

global TypeList *
cons_type(Type *type, TypeList *typelist)
{
	TypeList *tl;

	if (type == NULL)
		return typelist;
	tl = NEW(TypeList);
	tl->ty_head = type;
	tl->ty_tail = typelist;
	return tl;
}

global Type *
pair_type(Type *type1, Type *type2)
{
	return def_type(product,
		cons_type(type1, cons_type(type2, (TypeList *)NULL)));
}

global Type *
func_type(Type *type1, Type *type2)
{
	return def_type(function,
		cons_type(type1, cons_type(type2, (TypeList *)NULL)));
}

local Type *
multi_pair_type(TypeList *args)
{
	return args->ty_tail == NULL ? args->ty_head :
		pair_type(args->ty_head, multi_pair_type(args->ty_tail));
}

local Type *
multi_func_type(TypeList *args, Type *result)
{
	return args == NULL ? result :
		func_type(args->ty_head,
			multi_func_type(args->ty_tail, result));
}

/*
 *	Qualified types
 */

global QType *
qualified_type(Type *type)
{
	QType	*qtype;

	qtype = NEW(QType);
	qtype->qt_type = type;
	qtype->qt_ntvars = nr_type(type);
	if (qtype->qt_ntvars > MAX_TVARS_IN_TYPE)
		error(SEMERR, "too many type variables");
	return qtype;
}

/*
 *	Numbering of type variables.
 */

local	String	*base_vars_seen;
local	String	*last_vars_seen;

local int
nr_type(Type *type)
{
	String	vars_seen[MAX_TVARS_IN_TYPE];

	last_vars_seen = base_vars_seen = vars_seen;
	nv_type(type);
	return last_vars_seen - vars_seen;
}

local void
nv_type(Type *type)
{
	TypeList *argp;

	switch (type->ty_class) {
	case TY_VAR:
		if (! type->ty_mu_bound)
			type->ty_index = nv_tvar(type->ty_var);
	when TY_MU:
		nv_type(type->ty_body);
	when TY_CONS:
		for (argp = type->ty_args; argp != NULL; argp = argp->ty_tail)
			nv_type(argp->ty_head);
	otherwise:
		NOT_REACHED;
	}
}

local int
nv_tvar(String name)
{
	String	*varp;

	*last_vars_seen = name;		/* also serves as a sentinel */
	for (varp = base_vars_seen; *varp != name; varp++)
		;
	if (varp == last_vars_seen)
		last_vars_seen++;
	return varp - base_vars_seen;
}
