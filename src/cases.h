#ifndef CASES_H
#define CASES_H

#include "defs.h"
#include "path.h"

/*
 *	The upper part of case constructs.
 */

enum {
	UC_CASE,	/* upper level of case expression */
	UC_F_NOMATCH,	/* no-match error on function */
	UC_L_NOMATCH,	/* no-match error on lambda */
	UC_SUCCESS,	/* body expression */
	UC_STRICT	/* strict function */
};

struct _UCase {
	short	uc_class;
	union {
		struct {		/* CASE */
			short	ucu_references;
			short	ucu_level;
			Path	ucu_path;
			LCase	*ucu_cases;
		} uc_case;
		Func	*ucu_defun;	/* F_NOMATCH */
		Expr	*ucu_expr;	/* L_NOMATCH, STRICT */
		struct {		/* SUCCESS */
			int	ucu_size;
			Expr	*ucu_body;
		} uc_success;
	} uc_union;
};
#define	uc_references	uc_union.uc_case.ucu_references	/* CASE */
#define	uc_level	uc_union.uc_case.ucu_level	/* CASE */
#define	uc_path	uc_union.uc_case.ucu_path	/* CASE */
#define	uc_cases uc_union.uc_case.ucu_cases	/* CASE */
#define	uc_defun uc_union.ucu_defun		/* F_NOMATCH */
#define	uc_who	uc_union.ucu_expr		/* L_NOMATCH */
#define	uc_real	uc_union.ucu_expr		/* STRICT */
#define	uc_body	uc_union.uc_success.ucu_body	/* SUCCESS */
#define	uc_size	uc_union.uc_success.ucu_size	/* SUCCESS */

extern	UCase	*ucase(int level, Path path, LCase *cases);
extern	UCase	*f_nomatch(Func *defun);
extern	UCase	*l_nomatch(Expr *who);
extern	UCase	*success(Expr *body, int size);
extern	UCase	*strict(Expr *real);
extern	UCase	*copy_ucase(UCase *old);

/*
 *	The lower part of case constructs.
 */

enum {
	LC_ALGEBRAIC,	/* algebraic data type */
	LC_NUMERIC,	/* numbers -- <0, 0, succ(n) */
	LC_CHARACTER	/* characters */
};

struct _LCase {
	short	lc_class;
	int	lcu_arity;
	union {
		UCase	**lcu_limbs;		/* ALGEBRAIC, NUMERIC */
		CharArray *lcu_c_limbs;	/* CHARACTER */
	} lc_union;
};
#define	lc_arity	lcu_arity
#define	lc_limbs	lc_union.lcu_limbs
#define	lc_c_limbs	lc_union.lcu_c_limbs

/* indexes for number cases */
enum { LESS, EQUAL, GREATER };

extern	LCase	*alg_case(Natural arity, UCase *def);
extern	LCase	*num_case(UCase *def);
extern	LCase	*char_case(UCase *def);

#endif
