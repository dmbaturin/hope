#include "defs.h"
#include "value.h"

global Cell *
new_pair(Cell *left, Cell *right)
{
	Cell	*cp;

	cp = new_cell(C_PAIR);
	cp->c_left = left;
	cp->c_right = right;
	return cp;
}

global Cell *
new_dirs(Path path, Cell *val)
{
	Cell	*cp;

	cp = new_cell(C_DIRS);
	cp->c_path = path;
	cp->c_val = val;
	return cp;
}

global Cell *
new_cons(Cons *data_constructor, Cell *arg)
{
	Cell	*cp;

	cp = new_cell(C_CONS);
	cp->c_cons = data_constructor;
	cp->c_arg = arg;
	return cp;
}

global Cell *
new_susp(Expr *expr, Cell *env)
{
	Cell	*cp;

	cp = new_cell(C_SUSP);
	cp->c_expr = expr;
	cp->c_env = env;
	return cp;
}

global Cell *
new_papp(Expr *expr, Cell *env, int arity)
{
	Cell	*cp;

	cp = new_cell(C_PAPP);
	cp->c_expr = expr;
	cp->c_env = env;
	cp->c_arity = arity;
	return cp;
}

global Cell *
new_ucase(UCase *code, Cell *env)
{
	Cell	*cp;

	cp = new_cell(C_UCASE);
	cp->c_code = code;
	cp->c_env = env;
	return cp;
}

global Cell *
new_lcase(LCase *lcase, Cell *env)
{
	Cell	*cp;

	cp = new_cell(C_LCASE);
	cp->c_lcase = lcase;
	cp->c_env = env;
	return cp;
}

global Cell *
new_cnst(Cons *data_constant)
{
	Cell	*cp;

	cp = new_cell(C_CONST);
	cp->c_cons = data_constant;
	return cp;
}

global Cell *
new_num(Num n)
{
	Cell	*cp;

	cp = new_cell(C_NUM);
	cp->c_num = n;
	return cp;
}

global Cell *
new_char(Char c)
{
	Cell	*cp;

	cp = new_cell(C_CHAR);
	cp->c_char = c;
	return cp;
}

global Cell *
new_stream(FILE *f)
{
	Cell	*cp;

	cp = new_cell(C_STREAM);
	cp->c_file = f;
	return cp;
}
