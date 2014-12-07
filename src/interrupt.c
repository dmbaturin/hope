#include "defs.h"
#include "interrupt.h"
#include "error.h"

#include <signal.h>

local	RETSIGTYPE	onintr(int sig);
local	RETSIGTYPE	onalarm(int sig);

global void
disable_interrupt(void)
{
#ifdef	unix
	(void)signal(SIGALRM, SIG_IGN);
#endif
	(void)signal(SIGINT, SIG_IGN);
}

global void
enable_interrupt(void)
{
#ifdef	unix
	if (time_limit > 0) {
		(void)signal(SIGALRM, onalarm);
		alarm(time_limit);
	}
#endif
	(void)signal(SIGINT, onintr);
}

/*ARGSUSED*/
local RETSIGTYPE
onintr(int sig)
{
	disable_interrupt();
	error(EXECERR, "interrupted");
}

/*ARGSUSED*/
local RETSIGTYPE
onalarm(int sig)
{
	disable_interrupt();
	error(EXECERR, "time limit exceeded");
}
