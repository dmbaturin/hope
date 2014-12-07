#ifndef TYPE_VALUE_H
#define TYPE_VALUE_H

#include "defs.h"
#include "heap.h"

extern	Bool	unify(Cell *type1, Cell *type2);
extern	Bool	instance(Type *type, Natural ntvars, Cell *inf_type);
extern	Cell	*deref(Cell *type);
extern	Cell	*copy_type(Type *type, Natural ntvars, Bool frozen);
extern	Cell	*expand_type(Cell *type);

extern	Cell	*new_func_type(Cell *from, Cell *to);
extern	Cell	*new_prod_type(Cell *left, Cell *right);
extern	Cell	*new_list_type(Cell *element);
extern	Cell	*new_const_type(DefType *dt);

/* type cell classes */
#define	C_TVAR		CellClass(0, 8)
#define	C_VOID		CellClass(0, 9)
#define	C_FROZEN	CellClass(0, 10)
#define	C_VISITED	CellClass(0, 11)
#define	C_TSUB		CellClass(1, 8)
#define	C_TREF		CellClass(1, 9)
#define	C_TLIST		CellClass(2, 8)
#define	C_TCONS		CellClass(2, 9)

/* fields for type structures */
#define	c_tcons	c_union.cu_one.co_union.cu_tcons /* TSUB */
#define	c_targ	c_union.cu_one.cu_cell		/* TSUB */
#define	c_tref	c_union.cu_one.cu_cell		/* TREF */
#define	c_varno	c_misc_num			/* TVAR, TCONS */

#define	c_targ1	c_targ->c_full->c_head		/* TCONS */
#define	c_targ2	c_targ->c_full->c_tail->c_head	/* TCONS */

#define	c_abbr	c_union.cu_two.cu_left		/* TSYN */
#define	c_full	c_union.cu_two.cu_right		/* TSYN */
#define	c_head	c_union.cu_two.cu_left		/* TLIST */
#define	c_tail	c_union.cu_two.cu_right		/* TLIST */

extern	Cell	*new_tvar(void);
extern	Cell	*new_tsub(DefType *tcons, Cell *targ);
extern	Cell	*new_tref(Cell *tref);
extern	Cell	*new_void(void);
extern	Cell	*new_frozen(void);
extern	Cell	*new_tsyn(Cell *abbr, Cell *full);
extern	Cell	*new_tlist(Cell *left, Cell *right);
extern	Cell	*new_tcons(DefType *tcons, Cell *targ);

#endif
