#include "defs.h"
#include "newstring.h"
#include "memory.h"
#include "align.h"

#define TABSIZ	293		/* should be prime */

typedef	struct _Ident	Ident;
struct _Ident {
	Ident	*link;
	char	text[ALIGNMENT];	/* stub for size purposes */
};
#define	SizeIdent(n)	(sizeof(Ident) - ALIGNMENT + 1 + (n))

local	Ident	*table[TABSIZ];
local	Natural	hash(const Byte *s, int n);

global void
init_strings(void)
{
	Ident	**tp;

	for (tp = table; tp != &table[TABSIZ]; tp++)
		*tp = NULL;
}

global String
newstring(const char *s)
{
	return newnstring(s, strlen(s));
}

global String
newnstring(const char *s, int n)
{
	Ident	*np, **p;

	p = &table[hash((const Byte *)s, n)];
	for (np = *p; np != NULL; np = np->link)
		if (strncmp(np->text, s, n) == 0 && np->text[n] == '\0')
			return np->text;
	np = (Ident *)s_alloc((Natural)SizeIdent(n));
	np->link = *p;
	*p = np;
	np->text[n] = '\0';
	return strncpy(np->text, s, n);
}

#define	A 17
#define	B 89
#define	C 167

local Natural
hash(const Byte *s, int n)
{
	return n <= 0 ? 0 :
		(Natural)(n + A*s[0] + B*s[n/2] + C*s[n-1])%TABSIZ;
}
