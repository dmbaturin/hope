#include "defs.h"
#include "memory.h"
#include "module.h"
#include "source.h"
#include "error.h"
#ifdef unix
#include "plan9args.h"
#endif

#ifdef NLS
#include <locale.h>
#endif

global	Bool	restricted;	/* disable file I/O */
global	int	time_limit;	/* evaluation time limit in seconds */

global	const	char	*const	*cmd_args;

global int
main(int argc, const char *const argv[])
{
	Bool	gen_listing;	/* generate a listing on stderr */
	const	char	*source_file;
	FILE	*src;
#ifdef RE_EDIT
	const	char	*script_file;

	script_file = NULL;
#endif
	source_file = NULL;
	gen_listing = restricted = FALSE;
	time_limit = 0;
#ifdef unix
	ARGBEGIN {
		case 'f': source_file = ARGF();
		when 'l': gen_listing = TRUE;
		when 'r': restricted = TRUE;
		when 't': time_limit = atoi(ARGF());
#ifdef RE_EDIT
		when 's': script_file = ARGF();
#endif
		otherwise:
			fprintf(stderr, "usage: %s -lr -f file -t nsecs\n",
				argv0);
			return 1;
	} ARGEND
	cmd_args = argv;
#else
	cmd_args = argv+1;
#endif

	if (source_file != NULL) {
		src = fopen(source_file, "r");
		if (src == NULL) {
			fprintf(stderr, "%s: can't read file '%s'\n",
				argv0, source_file);
			return 1;
		}
	} else
		src = stdin;

#ifdef NLS
	(void)setlocale (LC_ALL, "");
#endif
	init_memory();
	init_strings();
	init_lex();
	init_source(src, gen_listing);

#ifdef RE_EDIT
	if (script_file != NULL)
		set_script(script_file);	/* re-entry after an edit */
#endif
	mod_init();		/* begin standard module */
	preserve();
	(void)yyparse();	/* read commands from files and user */
	heap_stats();
	if (source_file != NULL)
		fclose(src);
	return 0;
}

#ifdef RE_EDIT
/*
 *	Restart Hope, reading from the script_file.
 *	The neatest way would be to just longjmp back to the start,
 *	but it's too much trouble to get the external data back into
 *	the right state, or to make sure the program doesn't depend on
 *	its initial state.  So, we just exec ourselves again, passing
 *	the script_file as an argument.
 */
global void
restart(const char *script_file)
{
	(void)execlp(argv0, argv0, "-s", script_file, (char *)0);
	error(FATALERR, "cannot restart");
}
#endif
