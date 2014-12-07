#ifndef SOURCE_H
#define SOURCE_H

#include "defs.h"

extern	const	Byte	*inptr;

extern	void	init_source(FILE *src, Bool gen_listing);
extern	void	enterfile(FILE *f);
extern	Bool	interactive(void);
extern	Bool	getline(void);

#ifdef	RE_EDIT
extern	void	set_script(const char *filename);
extern	void	restart(const char *script_file);
#endif

#endif
