#include "defs.h"
#include "interpret.h"
#include "expr.h"
#include "cases.h"
#include "cons.h"
#include "char_array.h"
#include "value.h"
#include "stack.h"
#include "error.h"
#include "interrupt.h"
#include "output.h"
#include "pr_value.h"
#include "path.h"
#include "stream.h"

/*
 * The interpreter uses an extension of Krivine's machine.
 *
 * In Krivine's machine (as yet unpublished), a value is a deBruijn-form
 * lambda expression paired with an environment, which is a list of
 * values.  The machine state has 2 components: the value currently being
 * executed, and the stack, a list of values comprising its arguments.
 * Start with the expression to be evaluated, with an empty environment
 * and stack.  The transitions are as follows:
 *
 *	Current value		Stack		New current	New stack
 *	=============		=====		===========	=========
 *	(Var n, env)		s	->	env@n		s
 *	(Apply fun arg, env)	s	->	(fun, env)	(arg, env)::s
 *	(Lambda body, env)	a::s	->	(body, a::env)	s
 *	(Lambda body, env)	[]	->	stop and unravel
 *
 * Extensions involved here:
 * - Call-by-need evaluation (a simplified form of the corresponding
 *   enhancement to the TIM):
 *	(1) when a variable is entered, push an update frame pointing to
 *	    the value onto the stack.
 *	(2) when taking an argument for a Lambda, if there is an update
 *	    frame on the top of the stack, update the cell pointed to
 *	    with the current value.
 * - Cells representing data values: when these become current, they
 *   remove themselves, entering the first argument on the stack.
 *   Again, if there is an update frame in the way, the cell it points to
 *   is updated.
 * - an extra kind of mark on the stack: FORCE_MARK.  When this is at
 *   the the top of the stack, it indicates that the current value is
 *   to be evaluated to head normal form.  Thus if it is a constructor
 *   or pair, its sub-value(s) must also be forced.
 * - Fetching a sub-part of an argument: DIR nodes (cf number.c, path.c).
 * - Pattern matching: CASE nodes (cf compile.c).
 */

#ifdef TRACE
#	define	SHOW(fmt)	(void)fprintf(stderr, fmt)
#	define	SHOW2(fmt,arg)	(void)fprintf(stderr, fmt, arg)
#else
#	define	SHOW(fmt)
#	define	SHOW2(fmt,arg)
#endif

local	void	run(Cell *current);
local	Cell	*take(Cell *current);
local	void	chk_argument(Cell *arg);

global	String	cur_function;	/* for error reporting */

#define	NULL_ENV	NOCELL

/* push an update frame and enter p */
#define	EnterUpdate(p)	(current = (p), PushUpdate(current))

/* force evaluation (and update) of p */
#define	Force(p)	(Push(FORCE_MARK), EnterUpdate(p))

/*
 * The following two constants are properties of the abstract machine below,
 * the stream handler and the various builtins.  If any of these change,
 * these constants may also need to change.
 */
#define	MAX_NEWS	4	/* max. no. of cells required by any step */
#define	MAX_PUSHES	(2+2*UPD_FRAME)
				/* max. amount of stack growth on any step */

/* #define MORE_STATS */
#ifdef MORE_STATS
local	int	do_cell[C_NCLASSES];
local	int	do_expr[E_NCLASSES];
local	int	do_dir[P_NCLASSES];
#endif

/*
 *	Interpreter for an expression.
 *	See compile.c for the translation schemes.
 */
global void
interpret(Expr *action, Expr *expr)
{
#ifdef MORE_STATS
	int	i;

	for (i = 0; i < C_NCLASSES; i++)
		do_cell[i] = 0;
	for (i = 0; i < E_NCLASSES; i++)
		do_expr[i] = 0;
	for (i = 0; i < P_NCLASSES; i++)
		do_dir[i] = 0;
#endif
	start_stack();
	enable_interrupt();
	chk_stack(1);
	Push(new_susp(expr, new_pair(new_stream(stdin), NULL_ENV)));
	run(new_susp(action, NULL_ENV));
	disable_interrupt();
#ifdef MORE_STATS
	for (i = 0; i < C_NCLASSES; i++) {
		int	j;

		printf("cell %d: %d\n", i, do_cell[i]);
		if (i == C_SUSP)
			for (j = 0; j < E_NCLASSES; j++)
				printf("\texpr %d: %d\n", j, do_expr[j]);
		if (i == C_DIRS)
			for (j = 0; j < P_NCLASSES; j++)
				printf("\tdir %d: %d\n", j, do_dir[j]);
	}
#endif
}

/*
 *	Reduce a value to head normal form
 */
global Cell *
evaluate(Cell *value)
{
	chk_stack(3 + UPD_FRAME);
	Push(value);	/* so that it isn't forgotten */
	Push(new_susp(e_return, NULL_ENV));
	/* Force(value); */
	Push(FORCE_MARK);
	PushUpdate(value);
	run(value);
	return Pop();
}

local void
run(Cell *current)
{
/*
 *	The following variables hold temporary values during the emulation
 *	of particular instructions.
 *	Only current and the stack carry values between instructions.
 */
	Cell	*tmp;		/* general purpose temporary */
	int	itmp;		/* general purpose integer temporary */

/* special names for these.  BEWARE! Slippery when wet! */
#define	arity	itmp		/* arity of partial application */
#define	dir	itmp		/* direction value */
#define	var	itmp		/* variable number */
#define	top	tmp 		/* the top of the argument stack */

	Cell	*env; 		/* environment for suspensions */
	Expr	*expr; 		/* expression for suspensions */
	UCase	*code;
	LCase	*lcase;

    repeat {
	chk_heap(current, MAX_NEWS);
	chk_stack(MAX_PUSHES);
#ifdef MORE_STATS
	do_cell[current->c_class]++;
	if (current->c_class == C_SUSP)
		do_expr[current->c_expr->e_class]++;
	if (current->c_class == C_DIRS)
		do_expr[p_top(current->c_path)]++;
#endif
	switch (current->c_class) {
	case C_HOLE:
		error(EXECERR, "infinite loop");
	when C_NUM:
		SHOW("Num: "); SHOW2(NUMfmt, current->c_num); SHOW("\n");
		top = take(current);
		current = top == FORCE_MARK ? Pop() : top;
	when C_CHAR:
		SHOW2("CHAR: %c\n", current->c_char);
		top = take(current);
		current = top == FORCE_MARK ? Pop() : top;
	when C_CONST:
		SHOW2("CONST: %s\n", current->c_cons->c_name);
		top = take(current);
		current = top == FORCE_MARK ? Pop() : top;
	when C_CONS:
		SHOW2("CONS: %s\n", current->c_cons->c_name);
		top = take(current);
		if (top == FORCE_MARK)
			Force(current->c_arg);
		else	/* top is a normal value */
			current = top;
	when C_PAIR:
		SHOW("PAIR\n");
		top = take(current);
		if (top == FORCE_MARK) {
			tmp = current->c_right;
			Push(FORCE_MARK);
			PushUpdate(tmp);
			Push(tmp);
			Force(current->c_left);
		} else	/* top is a normal value */
			current = top;
	when C_STREAM:
		SHOW("STREAM\n");
		current = read_stream(current);
	when C_DIRS:
		SHOW("DIRS: ");
		dir = p_top(current->c_path);
		current->c_path = p_pop(current->c_path);
		switch (dir) {
		case P_END:
			SHOW("EMPTY\n");
			EnterUpdate(current->c_val);
		when P_LEFT:
			SHOW("LEFT\n");
			current->c_val = current->c_val->c_left;
		when P_RIGHT:
			SHOW("RIGHT\n");
			current->c_val = current->c_val->c_right;
		when P_STRIP:
			SHOW("STRIP\n");
			current->c_val = current->c_val->c_arg;
		when P_PRED:
			SHOW("PRED\n");
			current->c_val = new_num(current->c_val->c_num - 1);
		when P_UNROLL:
			SHOW("UNROLL\n");
			Push(current);
			EnterUpdate(current->c_val);
		otherwise:
			NOT_REACHED;
		}
	when C_SUSP:
		SHOW("SUSP: ");
		env = current->c_env;
		expr = current->c_expr;
		current->c_class = C_HOLE;
		switch (expr->e_class) {
		case E_PAIR:
			SHOW("PAIR\n");
			current = new_pair(new_susp(expr->e_left, env),
					   new_susp(expr->e_right, env));
		when E_APPLY or E_IF or E_LET or E_WHERE:
			SHOW("APPLY\n");
			Push(new_susp(expr->e_arg, env));
			current = new_susp(expr->e_func, env);
		when E_RLET or E_RWHERE:
			SHOW("RLET\n");
			/*
			 * build a cyclic environment:
			 * (LETREC p == e IN e', env) =>
			 *	letrec env' == (e, env')::env in (e', env')
			 */
			env = new_pair(new_susp(expr->e_arg, NULL_ENV), env);
			env->c_left->c_env = env;
			current = new_susp(expr->e_func->e_branch->br_expr,
					env);
		when E_MU:
			SHOW("MU\n");
			/*
			 * another cyclic environment:
			 * (MU p => e, env) =>
			 *	letrec v' == (e, env') and env' == v'::env in
			 *		v'
			 */
			env = new_pair(new_susp(expr->e_body, NULL_ENV), env);
			current = env->c_left;
			current->c_env = env;
		when E_DEFUN:
			SHOW2("DEFUN: %s\n", expr->e_defun->f_name);
			if (expr->e_defun->f_code == NULL)
				error(EXECERR, "%s: undefined name",
					expr->e_defun->f_name);
			current = new_papp(expr, NULL_ENV,
					expr->e_defun->f_arity);
		when E_LAMBDA or E_EQN or E_PRESECT or E_POSTSECT:
			SHOW("LAMBDA\n");
			current = new_papp(expr, env, expr->e_arity);
		when E_NUM:
			SHOW("Num: ");
			SHOW2(NUMfmt, expr->e_num);
			SHOW("\n");
			current = new_num(expr->e_num);
		when E_CHAR:
			SHOW2("CHAR: '%c'\n", expr->e_char);
			current = new_char(expr->e_char);
		when E_CONS:
			SHOW2("CONS: %s\n", expr->e_const->c_name);
			current = expr->e_const->c_nargs == 0 ?
				new_cnst(expr->e_const) :
				new_papp(expr, NULL_ENV,
						expr->e_const->c_nargs);
		when E_PARAM:
			SHOW2("PARAM(%d)\n", expr->e_level);
			for (var = expr->e_level; var > 0; var--)
				env = env->c_right;
			current = new_dirs(expr->e_where, env->c_left);
		when E_BUILTIN:
			SHOW("BUILTIN\n");
			/* Apply the built-in function to var 0,
			 * i.e the first element of the environment
			 */
			current = (*(expr->e_fn))(env->c_left);
		when E_BU_1MATH:
			SHOW("BU_1MATH\n");
			current = new_num((*(expr->e_1math))
					(env->c_left->c_num));
		when E_BU_2MATH:
			SHOW("BU_2MATH\n");
			current = new_num((*(expr->e_2math))
					(env->c_left->c_left->c_num,
					 env->c_left->c_right->c_num));
		when E_RETURN:
			SHOW("RETURN\n");
			return;
		otherwise:
			NOT_REACHED;
		}
	when C_PAPP:
		SHOW2("PAPP(%d)\n", current->c_arity);
		env = current->c_env;
		expr = current->c_expr;
		arity = current->c_arity;
		if (arity == 0) {
			current->c_class = C_HOLE;
			switch (expr->e_class) {
			case E_CONS:
				/*
				 * The internal representation of
				 *	c v1 ... vn-1 vn
				 * is
				 *	c(v1, (v2, ... (vn-1, vn)...))
				 */
				tmp = env->c_left;
				while ((env = env->c_right) != NULL_ENV)
					tmp = new_pair(env->c_left, tmp);
				current = new_cons(expr->e_const, tmp);
			when E_DEFUN:
				current =
					new_ucase(expr->e_defun->f_code, env);
			when E_LAMBDA or E_EQN or E_PRESECT or E_POSTSECT:
				current = new_ucase(expr->e_code, env);
			otherwise:
				NOT_REACHED;
			}
		} else {
			top = take(current);
			if (top == FORCE_MARK)
				current = Pop();
			else { /* top is a normal value */
				chk_argument(top);
				current = new_papp(expr,
						new_pair(top, env), arity-1);
			}
		}
	when C_UCASE:
		SHOW("UCASE: ");
		code = current->c_code;
		env = current->c_env;
		current->c_class = C_HOLE;
		switch (code->uc_class) {
		case UC_F_NOMATCH:
			SHOW("F_NOMATCH\n");
			pr_f_match(code->uc_defun, env);
			error(EXECERR, "no match found");
		when UC_L_NOMATCH:
			SHOW("L_NOMATCH\n");
			pr_l_match(code->uc_who, env);
			error(EXECERR, "no match found");
		when UC_CASE:
			SHOW("CASE\n");
			tmp = env;
			for (var = code->uc_level; var > 0; var--)
				tmp = tmp->c_right;
			tmp = new_dirs(code->uc_path, tmp->c_left);
			Push(tmp);		/* arg to LCASE or NCASE */
			Push(new_lcase(code->uc_cases, env));
			EnterUpdate(tmp);
		when UC_SUCCESS:
			SHOW("SUCCESS\n");
			current = new_susp(code->uc_body, env);
		when UC_STRICT:
			SHOW("STRICT\n");
			/* force the evaluation of var 0,
			 * i.e the first element of the environment,
			 * before continuing with the body (a built-in)
			 */
			Push(new_susp(code->uc_real, env));
			Force(env->c_left);
		otherwise:
			NOT_REACHED;
		}
	when C_LCASE:
		SHOW("LCASE: ");
		lcase = current->c_lcase;
		env = current->c_env;
		current->c_class = C_HOLE;
		switch (lcase->lc_class) {
		case LC_ALGEBRAIC:
			SHOW("LCASE\n");
			top = Pop();		/* arg (now updated) */
			code = lcase->lc_limbs[top->c_cons->c_index];
		when LC_NUMERIC:
			SHOW("NUMERIC\n");
			top = Pop();		/* arg (now updated) */
			code = lcase->lc_limbs[top->c_num < Zero ? LESS :
						top->c_num == Zero ? EQUAL :
							GREATER];
		when LC_CHARACTER:
			SHOW("CHARACTER\n");
			top = Pop();		/* arg (now updated) */
			code = ca_index(lcase->lc_c_limbs, top->c_char);
		otherwise:
			NOT_REACHED;
		}
		current = new_ucase(code, env);
	otherwise:
		(void)fprintf(stderr, "class: %d\n", current->c_class);
		NOT_REACHED;
	}
    }
}

local Cell *
take(Cell *current)
{
	while (IsUpdate())
		*PopUpdate() = *current;	/* perform the update */
	return Pop();
}

/*
 *	Desperate kludge to catch comparison of functions.
 *	Cf init_cmps() in compare.c
 */
local void
chk_argument(Cell *arg)
{
	if (arg->c_class == C_SUSP &&
	    arg->c_expr->e_class == E_APPLY &&
	    arg->c_expr->e_arg->e_class == E_BUILTIN)
		error(EXECERR, "attempt to compare functions");
}
