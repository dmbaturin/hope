#ifndef MODULE_H
#define MODULE_H

#include "defs.h"
#include "newstring.h"

extern	void	mod_init(void);
extern	String	mod_name(void);	/* name of current module */
extern	Bool	mod_system(void);	/* in a system module? */
extern	void	mod_use(String name);
extern	void	mod_save(String name);
extern	void	mod_dump(FILE *f);
extern	void	mod_file(char *buf, String name);
extern	void	mod_fetch(void);
extern	void	mod_finish(void);
extern	void	mod_private(void);
extern	void	display(void);

#endif
