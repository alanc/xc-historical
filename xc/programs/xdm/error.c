/*
 * error.c
 *
 * Log display manager errors to a file as
 * we generally do not have a terminal to talk to
 */

# include "dm.h"
# include <stdio.h>

static FILE	*errorFile;

InitErrorLog ()
{
	if (!(errorFile = freopen (errorLogFile, "w", stderr)))
		Panic (errorLogFile);
}

LogError (fmt, arg1, arg2, arg3, arg4, arg5)
char	*fmt;
int	arg1, arg2, arg3, arg4, arg5;
{
	fprintf (errorFile, fmt, arg1, arg2, arg3, arg4, arg5);
	fflush (errorFile);
}

LogPanic (fmt, arg1, arg2, arg3, arg4, arg5)
char	*fmt;
int	arg1, arg2, arg3, arg4, arg5;
{
	LogError ("panic: ");
	LogError (fmt, arg1, arg2, arg3, arg4, arg5);
	exit (1);
}

Panic (mesg)
char	*mesg;
{
	write (2, "panic: ", 7);
	write (2, mesg, strlen (mesg));
	exit (1);
}

#undef DEBUG

Debug (fmt, arg1, arg2, arg3, arg4, arg5)
char	*fmt;
int	arg1, arg2, arg3, arg4, arg5;
{
#ifdef DEBUG
	printf (fmt, arg1, arg2, arg3, arg4, arg5);
#endif
}
