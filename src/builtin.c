#include "defs.h"
#include "builtin.h"
#include "deftype.h"
#include "cons.h"
#include "value.h"
#include "expr.h"
#include "cases.h"
#include "interpret.h"
#include "stream.h"
#include "output.h"
#include "error.h"

#define	MAX_TMP_STRING	1024

#ifndef REALS
#	define	int_div	divide
#endif

local	void	def_builtin(const char *name, Function *fn);
local	void	def_1math(const char *name, Unary *fn);
local	void	def_2math(const char *name, Binary *fn);
local	Bool	check_arity(Type *type, int n);

local	Cell	*ord(Cell *arg);
local	Cell	*chr(Cell *arg);
local	Cell	*num2str(Cell *arg);
local	Cell	*str2num(Cell *arg);
local	Cell	*user_error(Cell *arg);

local	Num	plus(Num x, Num y);
local	Num	minus(Num x, Num y);
local	Num	times(Num x, Num y);
local	Num	divide(Num x, Num y);
local	Num	int_div(Num x, Num y);
local	Num	mod(Num x, Num y);

global void
init_builtins(void)
{
	def_builtin("ord",	ord		);
	def_builtin("chr",	chr		);
	def_builtin("read",	open_stream 	);
	def_builtin("num2str",	num2str 	);
	def_builtin("str2num",	str2num 	);
	def_builtin("error",	user_error 	);

	def_builtin("print",	print_value 	);
	def_builtin("write_element",	write_value 	);

	def_2math("+",		plus		);
	def_2math("-",		minus		);
	def_2math("*",		times		);
	def_2math("/",		divide		);
	def_2math("div",	int_div		);
	def_2math("mod",	mod		);

#ifdef REALS
#ifdef HAVE_LIBM
	def_1math("acos",	acos		);
	def_1math("asin",	asin		);
	def_1math("atan",	atan		);
	def_2math("atan2",	atan2		);
	def_1math("ceil",	ceil		);
	def_1math("cos",	cos		);
	def_1math("cosh",	cosh		);
	def_1math("exp",	exp		);
	def_1math("abs",	fabs		);
	def_1math("floor",	floor		);
	def_1math("log",	log		);
	def_1math("log10",	log10		);
	def_2math("pow",	pow		);
	def_1math("sin",	sin		);
	def_1math("sinh",	sinh		);
	def_1math("sqrt",	sqrt		);
	def_1math("tanh",	tanh		);
#endif
#ifdef HAVE_ATANH
	def_1math("acosh",	acosh		);
	def_1math("asinh",	asinh		);
	def_1math("atanh",	atanh		);
#endif
#ifdef HAVE_ERF
	def_1math("erf",	erf		);
	def_1math("erfc",	erfc		);
#endif
#ifdef HAVE_HYPOT
	def_2math("hypot",	hypot		);
#endif
#endif
}

local void
def_builtin(const char *name, Function *fn)
{
	Func	*bu;

	bu = fn_lookup(newstring(name));
	if (bu == NULL)
		error(LIBERR, "'%s': undeclared built-in", name);
	bu->f_code = strict(builtin_expr(fn));
	bu->f_arity = 1;
	bu->f_branch = NULL;
}

local void
def_1math(const char *name, Unary *fn)
{
	Func	*bu;

	bu = fn_lookup(newstring(name));
	if (bu == NULL)
		error(LIBERR, "'%s': undeclared built-in", name);
	if (! check_arity(bu->f_type, 1))
		error(LIBERR, "'%s': built-in has wrong type", name);
	bu->f_code = strict(bu_1math_expr(fn));
	bu->f_arity = 1;
	bu->f_branch = NULL;
}

local void
def_2math(const char *name, Binary *fn)
{
	Func	*bu;

	bu = fn_lookup(newstring(name));
	if (bu == NULL)
		error(LIBERR, "'%s': undeclared built-in", name);
	if (! check_arity(bu->f_type, 2))
		error(LIBERR, "'%s': built-in has wrong type", name);
	bu->f_code = strict(bu_2math_expr(fn));
	bu->f_arity = 1;
	bu->f_branch = NULL;
}

local Bool
check_arity(Type *type, int n)
{
	if (! (type->ty_class == TY_CONS &&
	       type->ty_deftype == function &&
	       type->ty_secondarg->ty_deftype == num))
		return FALSE;
	for (type = type->ty_firstarg; n-- > 1; type = type->ty_secondarg)
		if (! (type->ty_class == TY_CONS &&
		       type->ty_deftype == product &&
		       type->ty_firstarg->ty_deftype == num))
			return FALSE;
	return type->ty_class == TY_CONS && type->ty_deftype == num;
}

/*
 *	Implementations of built-in functions.
 */

local Cell *
ord(Cell *arg)
{
	return new_num((Num)(arg->c_char));
}

local Cell *
chr(Cell *arg)
{
	if (arg->c_num < Zero || arg->c_num > (Num)MaxChar) {
		start_err_line();
		(void)fprintf(errout, "  %s(", cur_function);
		(void)fprintf(errout, NUMfmt, arg->c_num);
		(void)fprintf(errout, ")\n");
		error(EXECERR, "value out of range");
	}
	return new_char((Char)(arg->c_num));
}

local Cell *
num2str(Cell *arg)
{
	Byte	strval[MAX_TMP_STRING];

	(void)sprintf((char *)strval, NUMfmt, arg->c_num);
	return c2hope(strval);
}

local Cell *
str2num(Cell *arg)
{
	Byte	strval[MAX_TMP_STRING];

	hope2c(strval, MAX_TMP_STRING, arg);
	return new_num(atoNUM((char *)strval));
}

local Cell *
user_error(Cell *arg)
{
	char	strval[MAX_TMP_STRING];

	hope2c((Byte *)strval, MAX_TMP_STRING, arg);
	error(USERERR, "%s", strval);
	NOT_REACHED;
}

/*
 *	Convert a C string to a string value.
 */
global Cell *
c2hope(const Byte *str)
{
	const	Byte	*sp;
	Cell	*cp;
	int	len;

	len = strlen((const char *)str);
	chk_heap(NOCELL, 3*len+1);
	cp = new_cnst(nil);
	for (sp = str+len-1; sp >= str; sp--)
		cp = new_cons(cons, new_pair(new_char(*sp), cp));
	return cp;
}

/*
 *	Convert a string value to a C string.
 *	It is an error if the string has more than n-1 characters.
 */
global void
hope2c(Byte *s, int n, Cell *arg)
{
	for ( ; n > 0 && arg->c_class == C_CONS; arg = arg->c_arg->c_right) {
		*s++ = arg->c_arg->c_left->c_char;
		n--;
	}
	if (n == 0)
		error(EXECERR, "%s: string too long", cur_function);
	*s = '\0';
}

/*
 *	Arithmetic operators.
 */

local Num plus(Num x, Num y)	{ return x + y; }
local Num minus(Num x, Num y)	{ return x - y; }
local Num times(Num x, Num y)	{ return x * y; }

local Num
divide(Num x, Num y)
{
	if (y == Zero)
		error(EXECERR, "attempt to divide by zero");
	return x / y;
}

#ifdef REALS
local Num
int_div(Num x, Num y)
{
	if (y == Zero)
		error(EXECERR, "attempt to divide by zero");
	return floor(x/y);
}

local Num
mod(Num x, Num y)
{
	if (y == Zero)
		error(EXECERR, "attempt to divide by zero");
	return fmod(x, y);
}
#else
local Num
mod(Num x, Num y)
{
	if (y == Zero)
		error(EXECERR, "attempt to divide by zero");
	return x % y;
}
#endif
