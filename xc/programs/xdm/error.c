/*
 * error.c
 *
 * Log display manager errors to a file as
 * we generally do not have a terminal to talk to
 */

# include "dm.h"
# include <stdio.h>

InitErrorLog ()
{
	if (access (errorLogFile, 2) == 0)
		freopen (errorLogFile, "w", stderr);
	else
		LogError ("Cannot open errorLogFile %s\n", errorLogFile);
}

LogError (fmt, arg1, arg2, arg3, arg4, arg5)
char	*fmt;
int	arg1, arg2, arg3, arg4, arg5;
{
	fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5);
	fflush (stderr);
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
