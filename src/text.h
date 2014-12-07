#ifndef	TEXT_H
#define	TEXT_H

#include "defs.h"
#include "char.h"

/*
 * A text literal, guaranteed null-terminated, but may contain extra nulls.
 */

typedef	struct {
	const	SChar	*t_start;
	int	t_length;
} Text;

#endif
