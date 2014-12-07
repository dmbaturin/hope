#ifndef BAD_RECTYPE_H
#define BAD_RECTYPE_H

#include "defs.h"

/*
 *	type is an illegal body for head if it contains a use of head
 *	with different arguments.
 */
extern	Bool	bad_rectype(DefType *head, Type *type);

#endif
