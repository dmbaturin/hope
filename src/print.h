#ifndef	PRINT_H
#define	PRINT_H

#include "defs.h"
#include "op.h"

/*
 * Defining the following names affects the number of extra parentheses
 * added to the output representation of expressions, values and types
 * (beyond the minimal correct parenthesization), as follows:
 *
 *	name		means: always put parentheses around
 *	----------------------------------------------------
 *	PAR_TUPLES	tuples, even at the top level, e.g. (1, 2, 3).
 *	PAR_INFIX	infix arguments to other infix operators,
 *			e.g. (x * y) + z
 *	PAR_ARGUMENT	arguments of prefix functions, e.g. f (x).
 *
 * Note that the parser always accepts the minimal parenthesization.
 */

#define PAR_TUPLES
/* #define PAR_INFIX */
/* #define PAR_ARGUMENT */

/*
 *	The range of precedences
 */

/* weakest binding: always needs parentheses */
#define	PREC_INFIX	(MINPREC-6)
#define	PREC_COMMA	(MINPREC-5)
#define	PREC_LAMBDA	(MINPREC-4)
#define	PREC_MU		PREC_LAMBDA
#define	PREC_LET	(MINPREC-3)
#define	PREC_WHERE	(MINPREC-2)
#define	PREC_IF		(MINPREC-1)

/* User-defined infix operators: MINPREC..MAXPREC */

#define	PREC_APPLY	(MAXPREC+1)
#define	PREC_ATOMIC	(MAXPREC+2)
/* strongest binding: never needs parentheses */

/*
 *	Context precedences
 */

#define	InnerPrec(prec,context)	((prec) < (context) ? PREC_INFIX : (context))

/* top-level expression, pattern or value */
#ifdef	PAR_TUPLES
#	define	PREC_BODY	PREC_LAMBDA
#else
#	define	PREC_BODY	PREC_COMMA
#endif

/* the left and right arguments of an infix operator */
#ifdef	PAR_INFIX
#	define	LeftPrec(op)	(MAXPREC+1)
#	define	RightPrec(op)	(MAXPREC+1)
#else
#	define	LeftPrec(op)	(op->op_prec + (op->op_assoc != ASSOC_LEFT))
#	define	RightPrec(op)	(op->op_prec + (op->op_assoc != ASSOC_RIGHT))
#endif

/*
 * formal parameter of a LAMBDA.
 * If LAMBDA's are are changed to have multiple arguments (cf. yyparse()),
 * this should be changed to PREC_APPLY+1.
 */
#define	PREC_FORMAL	PREC_BODY

/* the argument of a function, in prefix format */
#ifdef	PAR_ARGUMENT
#	define	PREC_ARG	(PREC_ATOMIC+1)
#else
#	define	PREC_ARG	(PREC_APPLY+1)
#endif

#endif
