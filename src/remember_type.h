#ifndef REMEMBER_TYPE_H
#define REMEMBER_TYPE_H

#include "defs.h"

/*
 *	Remember this one?
 *	Called whenever a type is defined in the Standard module.
 */
extern	void	remember_type(DefType *dt);

/*
 *	Called at the end of the Standard module, to check that all the
 *	types and constructors required internally have been defined.
 */
extern	void	check_type_defs(void);

#endif
