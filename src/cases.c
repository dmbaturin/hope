#include "defs.h"
#include "cases.h"
#include "expr.h"
#include "char_array.h"
#include "memory.h"

local	LCase	*copy_lcase(LCase *old);
local	UCase	*new_reference(UCase *node);

/*
 *	Upper case expressions.
 */

global UCase *
ucase(int level, Path path, LCase *cases)
{
	UCase	*code;

	code = NEW(UCase);
	code->uc_class = UC_CASE;
	code->uc_references = 1;
	code->uc_level = level;
	code->uc_path = path;
	code->uc_cases = cases;
	return code;
}

global UCase *
f_nomatch(Func *defun)
{
	UCase	*code;

	code = NEW(UCase);
	code->uc_class = UC_F_NOMATCH;
	code->uc_defun = defun;
	return code;
}

global UCase *
l_nomatch(Expr *who)
{
	UCase	*code;

	code = NEW(UCase);
	code->uc_class = UC_L_NOMATCH;
	code->uc_who = who;
	return code;
}

global UCase *
success(Expr *body, int size)
{
	UCase	*code;

	code = NEW(UCase);
	code->uc_class = UC_SUCCESS;
	code->uc_body = body;
	code->uc_size = size;
	return code;
}

global UCase *
strict(Expr *real)
{
	UCase	*code;

	code = NEW(UCase);
	code->uc_class = UC_STRICT;
	code->uc_real = real;
	return code;
}

global UCase *
copy_ucase(UCase *old)
{
	UCase	*new;

	new = NEW(UCase);
	new->uc_class = old->uc_class;
	switch (old->uc_class) {
	case UC_CASE:
		new->uc_references = 1;
		new->uc_level = old->uc_level;
		new->uc_path = old->uc_path;
		new->uc_cases = copy_lcase(old->uc_cases);
	when UC_F_NOMATCH:
		new->uc_defun = old->uc_defun;
	when UC_L_NOMATCH:
		new->uc_who = old->uc_who;
	when UC_SUCCESS:
		new->uc_body = old->uc_body;
		new->uc_size = old->uc_size;
	when UC_STRICT:
		new->uc_real = old->uc_real;
	otherwise:
		NOT_REACHED;
	}
	return new;
}

/*
 *	Lower case expressions.
 */

global LCase *
alg_case(Natural arity, UCase *def)
{
	LCase	*lcase;
	Natural	i;

	lcase = NEW(LCase);
	lcase->lc_class = LC_ALGEBRAIC;
	lcase->lc_arity = arity;
	lcase->lc_limbs = NEWARRAY(UCase *, arity);
	for (i = 0; i < arity; i++)
		lcase->lc_limbs[i] = def;
	return lcase;
}

global LCase *
num_case(UCase *def)
{
	LCase	*lcase;

	lcase = alg_case((Natural)3, def);
	lcase->lc_class = LC_NUMERIC;
	return lcase;
}

global LCase *
char_case(UCase *def)
{
	LCase	*lcase;

	lcase = NEW(LCase);
	lcase->lc_class = LC_CHARACTER;
	lcase->lc_arity = 256;		/* number of characters */
	lcase->lc_c_limbs = ca_new(def);
	return lcase;
}

local LCase *
copy_lcase(LCase *old)
{
	LCase	*new;
	int	i;

	new = NEW(LCase);
	new->lc_class = old->lc_class;
	new->lc_arity = old->lc_arity;
	switch (old->lc_class) {
	case LC_ALGEBRAIC or LC_NUMERIC:
		new->lc_limbs = NEWARRAY(UCase *, old->lc_arity);
		for (i = 0; i < old->lc_arity; i++)
			new->lc_limbs[i] = new_reference(old->lc_limbs[i]);
	when LC_CHARACTER:
		new->lc_c_limbs = ca_copy(old->lc_c_limbs);
		ca_map(new->lc_c_limbs, new_reference);
	otherwise:
		NOT_REACHED;
	}
	return new;
}

local UCase *
new_reference(UCase *node)
{
	if (node->uc_class == UC_CASE)
		node->uc_references++;
	return node;
}
