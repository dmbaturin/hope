#ifndef OUTPUT_H
#define OUTPUT_H

/*
 *	The builtins "print" and "write_element".
 */

#include "defs.h"

extern	Expr	*e_return, *e_print, *e_wr_list;

extern	void	init_print(void);

/*
 *	Print value and inferred type on standard output
 */
extern	Cell	*print_value(Cell *value);

/*
 *	Direct a list-valued output to the terminal or a file
 */

extern	void	open_out_file(const char *name);
extern	void	save_out_file(void);
extern	void	close_out_file(void);

extern	Cell	*write_value(Cell *value);

#endif
