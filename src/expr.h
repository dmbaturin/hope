#ifndef EXPR_H
#define EXPR_H

#include "defs.h"
#include "newstring.h"
#include "table.h"
#include "path.h"
#include "char.h"
#include "num.h"

typedef	Cell	*Function(Cell *value);
typedef	Num	Unary(Num x);
typedef	Num	Binary(Num x, Num y);

struct _Func {
	TabElt	f_linkage;
	short	f_arity;
	SBool	f_explicit_dec;	/* explicitly declared */
	SBool	f_explicit_def;	/* explicitly defined */
	union {
		QType	*fu_qtype;	/* for explicitly declared funcs */
		DefType	*fu_tycons;	/* for implicitly declared funcs */
	} f_union;
	Branch	*f_branch;
	UCase	*f_code;
};
#define	f_name	f_linkage.t_name
#define	f_qtype	f_union.fu_qtype
#define	f_type	f_qtype->qt_type
#define	f_ntvars f_qtype->qt_ntvars
#define	f_tycons f_union.fu_tycons

extern	void	new_fn(String name, QType *qtype);
extern	void	del_fn(Func *fn);
extern	Func	*fn_lookup(String name);
extern	Func	*fn_local(String name);

extern	void	decl_value(String name, QType *qtype);
extern	void	def_value(Expr *formals, Expr *body);

/*
 *	Expressions, including patterns.
 */

enum {
/* kinds of input expression that also appear as patterns */
	E_NUM,		/* integer constant */
	E_CHAR,		/* character constant */
	E_CONS,		/* data structure constructor */
	E_PAIR,		/* pair constructor (,) */
	E_APPLY,	/* function application */
/* kinds of input pattern that don't appear in expressions */
	E_VAR,		/* variable in pattern */
	E_PLUS,		/* p+k pattern */
/* kinds of input expression that don't appear in patterns */
	E_DEFUN,	/* declared function or constant */
	E_LAMBDA,	/* anonymous (lambda) function */
	E_PARAM,	/* variable in expression */
	E_MU,		/* recursive form */
/* variants on APPLY, LAMBDA for building various constructs */
	E_IF,		/* if-then-else (like APPLY) */
	E_WHERE,	/* where clause (like APPLY) */
	E_LET,		/* let ... in clause (like APPLY) */
	E_RWHERE,	/* recursive where clause */
	E_RLET,		/* recursive let ... in clause */
	E_EQN,		/* let/where equation (like LAMBDA) */
	E_PRESECT,	/* (e op) (like LAMBDA) */
	E_POSTSECT,	/* (op e) (like LAMBDA) */
/* expressions used to represent built-in functions */
	E_BUILTIN,	/* lower level of built-in function */
			/* Warning: this is misused by chk_argument() */
			/* you should understand it before changing these */
	E_BU_1MATH,	/* lower level of unary built-in math function */
	E_BU_2MATH,	/* lower level of binary built-in math function */
/* miscellaneous */
	E_RETURN,	/* return from execution */
/* count of the above */
	E_NCLASSES
};

typedef	char	ExprClass;

struct _Expr {
	ExprClass	e_class;
	char	e_misc_num;	/* VAR, PARAM, LAMBDA, APPLY in branch */
	union {	/* grab bag -- see the definitions below */
		Num	eu_num;		/* Num */
		Char	eu_char;	/* CHAR */
		Cons	*eu_const;	/* CONS */
		struct {		/* VAR */
			String	eu_vname;
			Path	eu_dirs;
		} e_v;
		struct {		/* PARAM */
			Expr	*eu_patt;
			Path	eu_where;
		} e_p;
		struct {		/* PAIR, MU */
			Expr	*eu_left;
			Expr	*eu_right;
		} e_pair;
		struct {		/* APPLY */
			Expr	*eu_func;
			Expr	*eu_arg;
		} e_apply;
		struct {		/* PLUS */
			int	eu_incr;
			Expr	*eu_rest;
		} e_plus;
		Func	*eu_defun;	/* DEFUN */
		struct {		/* LAMBDA */
			Branch	*eu_branch;
			UCase	*eu_code;
		} e_lambda;
		Function *eu_fn;	/* BUILTIN */
		Unary	*eu_1math;	/* BU_1MATH */
		Binary	*eu_2math;	/* BU_2MATH */
	} e_union;
};

#define	e_num	e_union.eu_num		/* Num */
#define	e_char	e_union.eu_char		/* CHAR */
#define	e_const	e_union.eu_const	/* CONS */
#define	e_vname	e_union.e_v.eu_vname	/* VAR */
#define	e_var	e_misc_num		/* VAR */
#define	e_dirs	e_union.e_v.eu_dirs 	/* VAR */
#define	e_patt	e_union.e_p.eu_patt	/* PARAM */
#define	e_level	e_misc_num		/* PARAM */
#define	e_where	e_union.e_p.eu_where	/* PARAM */
#define	e_left	e_union.e_pair.eu_left	/* PAIR */
#define	e_right	e_union.e_pair.eu_right	/* PAIR */
#define	e_func	e_union.e_apply.eu_func	/* APPLY */
#define	e_arg	e_union.e_apply.eu_arg	/* APPLY */
#define	e_nvars	e_misc_num		/* APPLY in branch */
#define	e_incr	e_union.e_plus.eu_incr	/* PLUS */
#define	e_rest	e_union.e_plus.eu_rest	/* PLUS */

#define	e_defun	e_union.eu_defun	/* DEFUN */
#define	e_arity	e_misc_num		/* LAMBDA */
#define	e_branch e_union.e_lambda.eu_branch	/* LAMBDA */
#define	e_code	e_union.e_lambda.eu_code	/* LAMBDA */

#define	e_muvar	e_union.e_pair.eu_left	/* MU */
#define	e_body	e_union.e_pair.eu_right	/* MU */

#define	e_fn	e_union.eu_fn		/* BUILTIN */
#define	e_1math	e_union.eu_1math	/* BU_1MATH */
#define	e_2math	e_union.eu_2math	/* BU_2MATH */

/* expression constructors */
extern	Expr	*char_expr(Char c);
extern	Expr	*text_expr(const Byte *text, int n);
extern	Expr	*num_expr(Num n);
extern	Expr	*cons_expr(Cons *constr);
extern	Expr	*id_expr(String name);
extern	Expr	*dir_expr(Path where);
extern	Expr	*pair_expr(Expr *left, Expr *right);
extern	Expr	*apply_expr(Expr *func, Expr *arg);
extern	Expr	*func_expr(Branch *branches);
extern	Expr	*ite_expr(Expr *if_expr, Expr *then_expr, Expr *else_expr);
extern	Expr	*let_expr(Expr *pattern, Expr *body, Expr *subexpr, Bool recursive);
extern	Expr	*where_expr(Expr *subexpr, Expr *pattern, Expr *body, Bool recursive);
extern	Expr	*mu_expr(Expr *muvar, Expr *body);
extern	Expr	*presection(String operator, Expr *arg);
extern	Expr	*postsection(String operator, Expr *arg);

extern	Expr	*builtin_expr(Function *fn);
extern	Expr	*bu_1math_expr(Unary *fn);
extern	Expr	*bu_2math_expr(Binary *fn);

extern	void	init_argv(void);

/*
 *	Internal names of some constructor expressions.
 */

extern	Expr	*e_true, *e_false;
/* the next two are used for building lists, and force a list type */
extern	Expr	*e_cons, *e_nil;
extern	Func	*f_id;

struct _Branch {
	Expr	*br_formals;	/* parameters in an APPLY-list */
	Expr	*br_expr;	/* the body */
	Branch	*br_next;	/* next branch in lambda or defined fn */
};

/* branch constructors */
extern	Branch	*new_branch(Expr *formals, Expr *expr, Branch *next);
extern	Branch	*new_unary(Expr *pattern, Expr *expr, Branch *next);

/* the following are checked by nr_branch() */
#define	MAX_VARIABLES	80	/* max. no. of variables visible */
#define	MAX_SCOPES	40	/* max. no. of nested lambdas */

#endif
