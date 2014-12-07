#ifndef NAMES_H
#define NAMES_H

#include "defs.h"
#include "newstring.h"

/* reserved words */
extern	String	n_or, n_valof, n_is, n_eq, n_gives,
		n_abstype, n_data, n_dec,
		n_infix, n_infixr, n_type, n_typevar, n_uses,
		n_else, n_if, n_in, n_lambda, n_let,
		n_letrec, n_mu, n_then, n_where, n_whererec;
/* special type constructor arguments */
extern	String	n_pos, n_neg, n_none;

#endif
