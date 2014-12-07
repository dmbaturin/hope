#ifndef ERROR_H
#define ERROR_H

/* error categories - these are also indices for an array in error() (qv) */
enum {
	LEXERR,		/* lexical error */
	SYNERR,		/* syntax error */
	SEMERR,		/* semantic error, except type errors */
	TYPEERR,	/* type conflict */
	EXECERR,	/* run-time error */
	USERERR,	/* user error */
	FATALERR,	/* fatal error */
	LIBERR,		/* library error */
	INTERR		/* internal error */
};

extern	FILE	*errout;	/* initialized by start_err_line() */

extern	void	start_err_line(void);
	/* call before line written to errout */
extern	void	error(int errtype, const char *fmt, ...);
/*
 * If errtype >= TYPEERR, error() will not return.
 * Also, any error in a system module is treated as a library error.
 */
extern	void	yyerror(const char *msg);

#ifdef	DEBUG
#	define	ASSERT(x)	if (! (x)) \
		error(INTERR, "assertion failed: \"%s\", line %d\n", \
			__FILE__, __LINE__)
#else
#	define	ASSERT(x)
#endif

extern	Bool	erroneous; /* error already reported in the current line */

extern	Bool	recovering(void);	/* Yacc is recovering */

#endif
