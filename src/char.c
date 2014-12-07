#ifdef UCS

#include "defs.h"
#include "char.h"

/*
 * The UTF-FSS (aka UTF-2) encoding of UCS, as described in the following
 * quote from Ken Thompson's utf-fss.c:
 *
 * Bits  Hex Min  Hex Max  Byte Sequence in Binary
 *   7  00000000 0000007f 0vvvvvvv
 *  11  00000080 000007FF 110vvvvv 10vvvvvv
 *  16  00000800 0000FFFF 1110vvvv 10vvvvvv 10vvvvvv
 *  21  00010000 001FFFFF 11110vvv 10vvvvvv 10vvvvvv 10vvvvvv
 *  26  00200000 03FFFFFF 111110vv 10vvvvvv 10vvvvvv 10vvvvvv 10vvvvvv
 *  31  04000000 7FFFFFFF 1111110v 10vvvvvv 10vvvvvv 10vvvvvv 10vvvvvv 10vvvvvv
 *
 * The UCS value is just the concatenation of the v bits in the multibyte
 * encoding.  When there are multiple ways to encode a value, for example
 * UCS 0, only the shortest encoding is legal.
 */

/* invalid sequences are ignored */

global Char
FetchChar(const Byte **p)
{
	Char	c;
	int	extras;
	unsigned bit;

	extras = 0;
	c = *(*p)++;
	if ((c & 0x80) == 0)		/* ASCII character */
		return c;
	/* how many extra bytes? */
	extras = 1;
	for (bit = 0x20; (c & bit) != 0; bit >>= 1)
		extras++;
	if (bit > 0)
		c &= bit-1;
	while (extras-- > 0)
		c = (c<<6) | *(*p)++&0x3f;
	return c;
}

global void
BackChar(const Byte **p)
{
	while ((*--*p & 0xc0) == 0x80)
		;
}

global long
GetChar(FILE *f)
{
	int	c;
	Char	wc;
	int	extras;
	unsigned bit;

	extras = 0;
	while ((c = getc(f)) != EOF) {
		if ((c & 0x80) == 0)		/* ASCII character */
			return (long)c;
		if ((c & 0xc0) == 0x80) {	/* tail character */
			if (extras > 0) {	/* in the right place */
				wc = (wc<<6) | c&0x3f;
				if (--extras == 0)
					return (long)wc;
			}
		} else {				/* head of sequence */
			/* how many extra bytes? */
			wc = (Char)c;
			extras = 1;
			for (bit = 0x20; (wc & bit) != 0; bit >>= 1)
				extras++;
			if (bit > 0)
				wc &= bit-1;
		}
	}
	return EOF;
}

global void
PutChar(Char *wc, FILE *f)
{
	Char	tmp;
	int	extras;

	if ((wc & ~0x7f) == 0) {
		(void)putc(wc, f);
		return;
	}
	/* how many extra bytes are required? */
	extras = 1;
	for (tmp = wc >> 11; tmp != 0; tmp >>= 5)
		extras++;
	/* put header Byte */
	(void)putc(0xff&(0x7f80 >> extras) | (wc >> (extras*6)), f);
	/* put tail bytes */
	while (extras-- != 0)
		(void)putc(0x80|0x3f&(wc >> (extras*6)), f);
}
#endif /* UCS */
