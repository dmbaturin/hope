#ifndef	DEFS_H
#define	DEFS_H
/*
 * A lazy Hope interpreter
 *
 * Ross Paterson
 */

/*
 * Global compilation options -- adjust to taste
 *
 * See also:
 *	REALS (in num.h)
 *	UCS and UTF_LIBS (in char.h)
 *	STATS (in runtime.c)
 *	TRACE (in interpret.c)
 */

/* set this to facilitate debugging, by retaining symbols
#define DEBUG
*/

/* set NLS to enable Natural language support
#define NLS
*/

/* no need to touch anything else */

#include "config.h"

/*
 * Standard Definitions
 */

#include <stdio.h>

#ifdef	MSDOS
#	define	msdos
#endif

#ifdef	msdos
#	undef	unix
#endif

#ifdef	unix
#	define	RE_EDIT
#endif

#include <stdlib.h>
#include <string.h>

#ifdef	DEBUG
#	define	local
#else
#	define	local	static
#endif
#define	global

typedef	unsigned int	Natural;
typedef	unsigned char	Byte;
typedef	int	Bool;
typedef	char	SBool;
#define	TRUE	1
#define	FALSE	0

#define	when	break; case
#define	or	: case
#define	otherwise	break; default

#define	repeat	for (;;)
#define	until(c)	if (c) break

#define	SIZE(array)	(sizeof(array)/sizeof(array[0]))

#include "error.h"

/* a sop to keep dumb checkers happy (and an extra debugging check) */
#define	NOT_REACHED	ASSERT(FALSE); exit(1)

/*
 * System-dependent stuff
 */

#ifdef	RE_EDIT
extern	void	edit(const char *name);
#else
#	define	edit(name)
#endif

extern	void	init_lex(void);
extern	int	yyparse(void);
extern	int	yylex(void);

/*
 *	Command-line flags
 */
extern	Bool	restricted;	/* disable file I/O */
extern	int	time_limit;	/* evaluation time limit in seconds */
				/* default = 0 (no limit) */

extern	const	char	*const	*cmd_args;	/* other arguments */

#include "structs.h"

/*
 *	Unix functions.
 */

#ifdef	HAVE_UNISTD_H
#	include <unistd.h>
#	ifndef	HAVE_REMOVE
#		define	remove	unlink
#	endif
#endif

#endif
