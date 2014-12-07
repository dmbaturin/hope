#ifndef DEFTYPE_H
#define DEFTYPE_H

#include "defs.h"
#include "newstring.h"
#include "table.h"
#include "typevar.h"

/*
 *	Defined types.
 */
struct _DefType {
	TabElt	dt_linkage;
	char	dt_arity;
	char	dt_syn_depth;	/* depth of synonym definitions */
				/* a depth of 0 implies a data type */
	SBool	dt_private;	/* definition of type is private */
	SBool	dt_tupled;
	unsigned char dt_index;	/* zero for global deftypes */
				/* index+1 for local ones */
	TypeList *dt_varlist;
	TypeList *dt_oldvarlist;
	union {
		Cons	*dtu_cons;	/* list of constants and constructors */
		Type	*dtu_type;	/* synonymous type */
	} dt_union;
};
#define	dt_name	dt_linkage.t_name
#define	dt_next	dt_linkage.t_next
#define	dt_cons	dt_union.dtu_cons
#define	dt_type	dt_union.dtu_type

#define	IsAbsType(dt)	((dt)->dt_syn_depth == 0 && (dt)->dt_cons == NULL)
#define	IsDataType(dt)	((dt)->dt_syn_depth == 0 && (dt)->dt_cons != NULL)
#define	IsSynType(dt)	((dt)->dt_syn_depth > 0)

extern	void	dt_declare(DefType *dt);
extern	DefType	*dt_lookup(String name);
extern	DefType	*dt_local(String name);
extern	void	fix_synonyms(void);

extern	void	start_dec_type(void);
extern	DefType	*new_deftype(String name, Bool tupled, TypeList *vars);

extern	void	abstype(DefType *deftype);
extern	void	type_syn(DefType *deftype, Type *type);
extern	void	decl_type(DefType *deftype, Cons *conslist);

extern	TVar	alpha;
extern	DefType	*product, *function, *list, *num, *truval, *character;

/*
 *	Types
 */

/* maximum depth of mu quantifiers in a type (not checked) */
#define	MAX_MU_DEPTH	8
/* maximum depth of type synonyms (checked) */
#define	MAX_SYN_DEPTH	50

#define	MAX_TVARS_IN_TYPE	40	/* (checked) */

struct _TypeList {
	Type	*ty_head;
	TypeList *ty_tail;
};

typedef	enum {
	TY_VAR,		/* type variable */
	TY_CONS,	/* type constructor applied to zero or more types */
	TY_MU		/* recursive type */
} TypeClass;

struct _Type {
	char	ty_class;	/* small TypeClass */
	SBool	ty_misc_bool;	/* for CONS */
	union {
		struct {
			TVar	tyu_var;
			SBool	tyu_mu_bound;	/* bound by a mu */
			unsigned char	tyu_index;
			SBool	tyu_pos;	/* used positively */
			SBool	tyu_neg;	/* used negatively */
		} tyu_v;
		struct {
			DefType	*tyu_deftype;
			TypeList *tyu_args;
		} tyu_def;
		struct {
			TVar	tyu_muvar;
			Type	*tyu_body;
		} tyu_mu;
	} ty_union;
};
#define	ty_tupled	ty_misc_bool
#define	ty_var		ty_union.tyu_v.tyu_var
#define	ty_mu_bound	ty_union.tyu_v.tyu_mu_bound
#define	ty_index	ty_union.tyu_v.tyu_index
#define	ty_neg		ty_union.tyu_v.tyu_neg
#define	ty_pos		ty_union.tyu_v.tyu_pos
#define	ty_deftype	ty_union.tyu_def.tyu_deftype
#define	ty_args		ty_union.tyu_def.tyu_args
#define	ty_firstarg	ty_union.tyu_def.tyu_args->ty_head
#define	ty_secondarg	ty_union.tyu_def.tyu_args->ty_tail->ty_head
#define	ty_muvar	ty_union.tyu_mu.tyu_muvar
#define	ty_body		ty_union.tyu_mu.tyu_body

/*
 *	ty_index:
 *	If a type variable is mu-bound, a de Bruijn number
 *	Otherwise:
 *		If in a type definition, the parameter index
 *		If in a value declaration, a variable number
 */

extern	Type	*new_type(String ident, Bool tupled, TypeList *args);
extern	Type	*new_tv(TVar tvar);
extern	Type	*def_type(DefType *dt, TypeList *args);
extern	Type	*pair_type(Type *type1, Type *type2);
extern	Type	*func_type(Type *type1, Type *type2);
extern	TypeList *cons_type(Type *type, TypeList *typelist);
extern	void	enter_mu_tv(String ident);
extern	Type	*mu_type(Type *body);

extern	void	remember_type(DefType *dt);
extern	void	check_type_defs(void);

/*
 *	Types qualified by a list of type constructors,
 *	for use in value declarations.
 */

struct _QType {
	unsigned char	qt_ntvars;	/* no. of type variables in type */
	Type	*qt_type;
};

extern	QType	*qualified_type(Type *type);

#endif
