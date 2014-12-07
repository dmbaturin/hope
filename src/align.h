#ifndef ALIGN_H
#define ALIGN_H

/*
 *	Determine the alignment of doubles on the current machine.
 */

typedef struct { char c; double d; } PtrRec;

#define ALIGNMENT ((int)&(((PtrRec *)0)->d))

/* If that doesn't work, the following is conservative: */
/* #define ALIGNMENT sizeof(double) */

#endif
