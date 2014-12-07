#ifndef	NUM_H
#define	NUM_H

#include "defs.h"

/*
 *	The representation of the type `num'.
 */

/* define this to use real numbers (well, double) */
#define REALS

#ifdef	REALS
#	include <float.h>
#	include <math.h>
#	define	Num	double
#	define	atoNUM	atof
#	define	NUMfmt	"%.*g", DBL_DIG
#	define	Zero	0.0
#else
#	define	Num	long
#	define	atoNUM	atol
#	define	NUMfmt	"%ld"
#	define	Zero	0L
#endif

extern	Num	atoNUM(const char *s);

#endif
