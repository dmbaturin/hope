#ifndef NEWSTRING_H
#define NEWSTRING_H

/*
 *	Heap storage for strings.
 *	Each string is assigned a unique address, so that direct address
 *	comparisons can be used for string equality tests.
 */

typedef	const	char	*String;

extern	void	init_strings(void);
extern	String	newstring(const char *s);
extern	String	newnstring(const char *s, int n);

#endif
