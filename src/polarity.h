#ifndef POLARITY_H
#define POLARITY_H

/*
 *	Polarities of type constructor arguments.
 */

#include "defs.h"
#include "newstring.h"

extern	String	type_arg_name(Type *var, Bool full);

extern	void	set_polarities(TypeList *varlist);

extern	Bool	check_polarities(TypeList *decl_vars, TypeList *def_vars);

extern	void	start_polarities(DefType *deftype, TypeList *varlist);
extern	void	compute_polarities(Type *type);
extern	void	finish_polarities(void);

#endif
