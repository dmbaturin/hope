#ifndef HEAP_H
#define HEAP_H

#include "defs.h"
#include "path.h"
#include "num.h"
#include "char.h"

/*
 * Classes of cells on the heap, arranged by number of cell children
 * they have.  First we list the underlying names, and then the names
 * they are given by the type checker and interpreter.
 * Note: the top bit of this class (GC_MARK) is set, and cleared again,
 * by the garbage collector.
 */
#define	GC_MARK	0100

#define	C_MAXARITY	2
#define	C_CLASSBITS	4
#define	C_NCLASSES	((C_MAXARITY+1)<<C_CLASSBITS)

#define	CellClass(arity,n)	(((arity)<<C_CLASSBITS)|(n))
#define	CellArity(c)		((c)>>C_CLASSBITS)

#define	NOCELL	((Cell *)0)

struct _Cell {
	char	c_class;
	char	c_misc_num;		/* PAPP */
	union {
		Num	cu_num;		/* Num */
		Char	cu_char;	/* CHAR */
		FILE	*cu_file;	/* STREAM */
		struct {	/* unary nodes */
			union {
				DefType	*cu_tcons;	/* TSUB */
				Cons	*cu_cons;	/* CONST, CONS */
				Expr	*cu_expr;	/* SUSP, PAPP */
				Path	cu_path;	/* DIRS */
				UCase	*cu_code;	/* UCASE */
				LCase	*cu_lcase;	/* LCASE */
			} co_union;
			Cell	*cu_cell;
		} cu_one;
		struct {	/* binary nodes */
			Cell	*cu_left, *cu_right;	/* PAIR */
		} cu_two;
	} c_union;
};

/* free list */
#define	c_foll	c_union.cu_one.cu_cell

#define	c_sub	c_union.cu_one.cu_cell
#define	c_sub1	c_union.cu_two.cu_left
#define	c_sub2	c_union.cu_two.cu_right

/*
 *	The heap of cells
 */
extern	void	start_heap(void);
/* required before any calls to new_cell() */
extern	Cell	*new_cell(int class);
extern	void	chk_heap(Cell *current, int required);
extern	void	heap_stats(void);

#endif
