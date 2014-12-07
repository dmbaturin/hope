#ifndef CHAR_H
#define	CHAR_H

#include "defs.h"

/*
 * By default, the char type refers to an 8-bit character set whose
 * first 128 positions are the ASCII characters, e.g. ASCII itself,
 * or any of the ISO 8859 alphabets.
 *
 * Set UCS to make the char type refer to the universal character
 * set ISO 10646-1 UCS-2 (a.k.a Unicode).  The first 256 positions in
 * this character set coincide with ISO 8859-1 (Latin-1).  For external
 * purposes, this will be encoded using the UTF (a.k.a. FSS-UTF, UTF-2)
 * Byte encoding.  (UTF encodes ASCII characters as themselves.)
 *
 * If UCS is defined, you can also define UTF_LIBS to use special symbols
 * in the syntax, but this will need special versions of the libraries.
 */
/* #define UCS */
/* #define UTF_LIBS */

/* SChar is best for arrays, Char for variables */
#ifdef UCS
typedef	unsigned short	SChar;
#else
typedef	Byte	SChar;
#endif
typedef	unsigned int	Char;

#ifdef UCS
#define	MaxChar	((Char)0xfffdL)
#else
#define	MaxChar	((Char)0xff)
#endif

extern	Char	FetchChar(const Byte **p);
extern	void	BackChar(const Byte **p);
extern	long	GetChar(FILE *f);
extern	void	PutChar(Char c, FILE *f);

#ifndef UCS
#define	FetchChar(p)	(*(*p)++)
#define	BackChar(p)	((*p)--)
#define	GetChar(f)	getc(f)
#define	PutChar(ch,f)	((void)putc(ch, f))
#endif

/* character classes */
#include <ctype.h>

#ifdef UCS
/* extra characters are treated as either punctuation of alphabetic */
#define	ExtPunct(c)	(0x2000 <= (c) && (c) <= 0x3020)

#define IsCntrl(c)	(isascii(c) && iscntrl(c))
#define IsSpace(c)	(isascii(c) && isspace(c))
#define	IsDigit(c)	(isascii(c) && isdigit(c))
#define	IsPunct(c)	(isascii(c) ? ispunct(c) : ExtPunct(c))
#define	IsAlpha(c)	(isascii(c) ? isalpha(c) : ! ExtPunct(c))
#define	IsAlNum(c)	(isascii(c) ? isalnum(c) : ! ExtPunct(c))
#else /* ! UCS */
#define IsCntrl(c)	iscntrl(c)
#define IsSpace(c)	isspace(c)
#define	IsDigit(c)	isdigit(c)
#define	IsPunct(c)	ispunct(c)
#define	IsAlpha(c)	isalpha(c)
#define	IsAlNum(c)	isalnum(c)
#endif /* UCS */

#endif
