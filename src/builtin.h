#ifndef BUILTIN_H
#define BUILTIN_H

#include "defs.h"

extern	void	init_builtins(void);

/* conversions between string representations */

/* Convert a C string to a string value. */
extern	Cell	*c2hope(const Byte *str);

/*
 *	Convert a string value to a C string.
 *	It is an error if the string has more than n-1 characters.
 */
extern	void	hope2c(Byte *s, int n, Cell *arg);

#endif
