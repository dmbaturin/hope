#include "defs.h"
#include "eval.h"
#include "expr.h"
#include "error.h"
#include "compile.h"
#include "interpret.h"
#include "stream.h"
#include "number.h"
#include "output.h"
#include "type_check.h"
#include "exceptions.h"

local	Bool	create_environment(Expr *expr);

/*
 *	Evaluation of expressions.
 */

global	jmp_buf	execerror;

local Bool
create_environment(Expr *expr)
{
	return nr_branch(new_unary(id_expr(newstring("input")),
			expr,
			(Branch *)0));
}

global void
eval_expr(Expr *expr)
{
	if (erroneous)
		return;
	if (create_environment(expr)) {
		reset_streams();
		if (! setjmp(execerror)) {
			chk_expr(expr);
			comp_expr(expr);
			interpret(e_print, expr);
		}
		close_streams();
	}
}

global void
wr_expr(Expr *expr, const char *file)
{
	if (erroneous)
		return;
	if (create_environment(expr)) {
		reset_streams();
		if (! setjmp(execerror)) {
			open_out_file(file);
			chk_list(expr);
			comp_expr(expr);
			interpret(e_wr_list, expr);
			save_out_file();
		} else
			close_out_file();
		close_streams();
	}
}
