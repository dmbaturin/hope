#ifndef VALUE_H
#define VALUE_H

#include "defs.h"
#include "path.h"
#include "num.h"
#include "char.h"
#include "heap.h"

/* data cell classes */
#define	C_NUM		CellClass(0, 0)
#define	C_CHAR		CellClass(0, 1)
#define	C_CONST		CellClass(0, 2)	/* constant */
#define	C_STREAM	CellClass(0, 3)	/* partially read input stream */
#define	C_HOLE		CellClass(0, 4)	/* black hole */
#define	C_CONS		CellClass(1, 0)	/* constructed term */
#define	C_SUSP		CellClass(1, 1)	/* term and environment */
#define C_DIRS		CellClass(1, 2)	/* directions and value */
#define C_UCASE		CellClass(1, 3)	/* upper case */
#define C_LCASE		CellClass(1, 4)	/* lower case */
#define C_PAPP		CellClass(1, 5)	/* partial application */
#define	C_PAIR		CellClass(2, 0)	/* pair and list builder */

/* fields for data cells */
#define	c_arity	c_misc_num			/* PAPP */
#define	c_num	c_union.cu_num			/* Num */
#define	c_char	c_union.cu_char			/* CHAR */
#define	c_file	c_union.cu_file			/* STREAM */
#define	c_cons	c_union.cu_one.co_union.cu_cons	/* CONST, CONS */
#define	c_arg	c_union.cu_one.cu_cell		/* CONS */
#define	c_expr	c_union.cu_one.co_union.cu_expr	/* SUSP, PAPP */
#define	c_code	c_union.cu_one.co_union.cu_code	/* UCASE */
#define	c_lcase	c_union.cu_one.co_union.cu_lcase /* LCASE */
#define	c_env	c_union.cu_one.cu_cell		/* SUSP, UCASE, LCASE, PAPP */
#define	c_path	c_union.cu_one.co_union.cu_path	/* DIRS */
#define	c_val	c_union.cu_one.cu_cell		/* DIRS */
#define	c_left	c_union.cu_two.cu_left		/* PAIR */
#define	c_right	c_union.cu_two.cu_right		/* PAIR */

extern	Cell	*new_num(Num n);
extern	Cell	*new_char(Char c);
extern	Cell	*new_stream(FILE *f);
extern	Cell	*new_cnst(Cons *data_constant);
extern	Cell	*new_cons(Cons *data_constructor, Cell *arg);
extern	Cell	*new_susp(Expr *expr, Cell *env);
extern	Cell	*new_papp(Expr *expr, Cell *env, int arity);
extern	Cell	*new_dirs(Path path, Cell *val);
extern	Cell	*new_ucase(UCase *code, Cell *env);
extern	Cell	*new_lcase(LCase *lcase, Cell *env);
extern	Cell	*new_pair(Cell *left, Cell *right);

#endif
