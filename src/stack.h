#ifndef STACK_H
#define STACK_H

/*
 *	The run-time cell stack.
 */

#include "defs.h"

/*
 * enable stack -- required for any calls to Push().
 * start_heap() must have already been called.
 */
extern	void	start_stack(void);

typedef	union	_StkElt	StkElt;
union	_StkElt {
	Cell	*stk_value;
	StkElt	*stk_update;	/* pointer to update frame */
};

extern	StkElt	*stack;

#define FORCE_MARK      NOCELL

#define	Push(cell)	((--stack)->stk_value = (cell))
#define	Pop()		(stack++->stk_value)
#define	Top()		(stack->stk_value)
#define	Pop_void()	(stack++)

/*
 * Update frames:
 *	IsUpdate()	there is an update frame on the top of the stack.
 *	PushUpdate(cp)	push an update frame pointing to cp onto the stack.
 *	PopUpdate()	pop the update from on top of the stack, returning
 *			its cell pointer.
 *
 * Update frames on the stack consist of:
 *	a pointer to the cell to be updated.
 *	a pointer to the next update frame on the stack.
 * pushed onto the stack in that order.
 */
#define	UPD_FRAME 2	/* size of a frame on the stack */

extern	StkElt	*last_update;	/* top update frame */

#define	IsUpdate()	(stack == last_update)
#define	PushUpdate(cell) (\
		Push(cell),\
		(--stack)->stk_update = last_update,\
		last_update = stack)
/* IsUpdate() should be true when PopUpdate() is called */
#define	PopUpdate() (\
		last_update = stack++->stk_update,\
		Pop())

extern	void	chk_stack(int required);

#endif
