/*
 *	plan 9 argument parsing
 */
#define	ARGBEGIN	for((argv0? 0: (argv0= *argv)),argv++,argc--;\
			    argv[0] && argv[0][0]=='-' && argv[0][1];\
			    argc--, argv++) {\
				const char *_args, *_argt;\
				char _argc;\
				_args = &argv[0][1];\
				if(_args[0]=='-' && _args[1]==0){\
					argc--; argv++; break;\
				}\
				_argc=0;while(*_args) switch(_argc= *_args++)
#define	ARGEND		}
#define	ARGF()		(_argt=_args, _args="",\
				(*_argt? _argt: argv[1]? (argc--, *++argv): ""))
#define	ARGC()		_argc

const char *argv0;	/* extern in Plan 9 */
