#include "defs.h"
#include "bad_rectype.h"
#include "deftype.h"
#include "cons.h"
#include "error.h"

#define	MAX_ARG_STACK	200	/* max. size of type arg stack (not checked) */

typedef enum {	/* return types for check_recursion() */
	REC_NONE,	/* type contains no uses of the constructor */
	REC_OK,		/* type contains correct uses of the constructor */
	REC_FAIL	/* type contains an error (already reported) */
} RecType;

local	RecType	check_recursion(DefType *head, Type *type);
local	Bool	visited(DefType *deftype);

/*
 *	Stack of type synonyms whose expansions we are in.
 *	Runs upwards.
 */
local	DefType	**dt_base;
local	DefType	**dt_top;

/*
 *	Stack of arguments of type synonyms whose expansions we are in.
 *	Arguments which refer to arguments of the head constructor are
 *	represented by their argument number in that constructor.
 *	Complex arguments are represented as -1, which cannot match.
 *	Runs downwards.
 */
local	short	*arg_top;

#define	COMPLEX_ARG	(-1)

/*
 *	type is an illegal body for head if it contains a use of head
 *	with different arguments.
 */
global Bool
bad_rectype(DefType *head, Type *type)
{
	int	i;
	short	arg_stack[MAX_ARG_STACK];
	DefType	*dt_stack[MAX_SYN_DEPTH];

	dt_top = dt_base = dt_stack;
	arg_top = arg_stack + MAX_ARG_STACK;

	arg_top -= head->dt_arity;
	for (i = head->dt_arity-1; i >= 0; i--)
		arg_top[i] = i;
	return check_recursion(head, type) == REC_FAIL;
}

local RecType
check_recursion(DefType *head, Type *type)
{
	TypeList *argp, *argp2;
	RecType	result, sub_result;
	int	arity;
	int	i;
	Cons	*ct;

	while (type->ty_class == TY_MU)
		type = type->ty_body;

	if (type->ty_class == TY_VAR || visited(type->ty_deftype))
		return REC_NONE;

	if (type->ty_deftype == head) {
		/*
		 * check that the arguments are a permutation of those
		 * on the left side.  It suffices to check that they are
		 * all variables and none occurs twice.
		 */
		for (argp = type->ty_args;
		     argp != NULL;
		     argp = argp->ty_tail) {
			if (argp->ty_head->ty_class != TY_VAR) {
				error(SEMERR,
					"%s: recursive use with non-variable argument",
					head->dt_name);
				return REC_FAIL;
			}
			for (argp2 = type->ty_args;
			     argp2 != argp;
			     argp2 = argp2->ty_tail)
				if (argp2->ty_head->ty_index ==
						argp->ty_head->ty_index) {
					error(SEMERR,
						"%s: recursive use with different arguments",
						head->dt_name);
					return REC_FAIL;
				}
		}
		return REC_OK;
	}

	arity = type->ty_deftype->dt_arity;

	*dt_top++ = type->ty_deftype;
	for (argp = type->ty_args, i = 0;
	     argp != NULL;
	     argp = argp->ty_tail, i++)
		arg_top[i - arity] = argp->ty_head->ty_class == TY_VAR ?
			arg_top[argp->ty_head->ty_index] : COMPLEX_ARG;
	arg_top -= arity;

	/* check expansion of type synonym */
	if (IsSynType(type->ty_deftype)) {
		result = check_recursion(head, type->ty_deftype->dt_type);
		if (result == REC_FAIL)
			return REC_FAIL;
	} else {	/* it's a data-type */
		result = REC_NONE;
		for (ct = type->ty_deftype->dt_cons;
		     ct != NULL;
		     ct = ct->c_next)
			if (ct->c_nargs > 0) {
				sub_result = check_recursion(head,
					ct->c_type->ty_firstarg);
				if (sub_result == REC_FAIL)
					return REC_FAIL;
				if (sub_result == REC_OK)
					result = REC_OK;
			}
	}

	/* pop stacks */
	arg_top += arity;
	dt_top--;

	/*
	 * If the expansion of a type constructor contains a recursive
	 * use of the constructor being defined, then defining this
	 * constructor will make that one recursive too.  The situation
	 * is something like
	 *
	 *	abstype a(v1, ..., vn);
	 *	type b(u1, ..., um) == ... a(v1, ..., vn) ...;
	 *	type a(v1, ..., vn) == ... b(u1, ..., um) ...;
	 *
	 * If a and b have different arities, then one of them must have
	 * different arguments for the recursive use; we have already
	 * checked a, so an error is being introduced into b.
	 * If they have the same arities, and a has checked out, then the
	 * two variable lists must be permutations of each other, and b
	 * will also be legal.
	 */
	if (result == REC_OK && arity != head->dt_arity) {
		error(SEMERR,
			"%s: recursive use of '%s' has different arguments",
			head->dt_name,
			type->ty_deftype->dt_name);
		return REC_FAIL;
	}

	/* check arguments of type constructor */
	for (argp = type->ty_args; argp != NULL; argp = argp->ty_tail) {
		sub_result = check_recursion(head, argp->ty_head);
		if (sub_result == REC_FAIL)
			return REC_FAIL;
		if (sub_result == REC_OK)
			result = REC_OK;
	}
	return result;
}

/*
 *	Is deftype on the stack of expanded type synonyms?
 */
local Bool
visited(DefType *deftype)
{
	DefType	**dp;

	for (dp = dt_base; dp != dt_top; dp++)
		if (*dp == deftype)
			return TRUE;
	return FALSE;
}
