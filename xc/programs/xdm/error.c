/*
 * xdm - display manager daemon
 *
 * $XConsortium: error.c,v 1.14 94/01/18 10:26:49 rws Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * error.c
 *
 * Log display manager errors to a file as
 * we generally do not have a terminal to talk to
 */

# include "dm.h"
# include <stdio.h>
#if NeedVarargsPrototypes
# include <stdarg.h>
#else
/* this type needs to be big enough to contain int or pointer */
typedef long Fmtarg_t;
#endif

/*VARARGS1*/
LogInfo(
#if NeedVarargsPrototypes
    char * fmt, ...)
#else
    fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
    Fmtarg_t arg1, arg2, arg3, arg4, arg5, arg6;
#endif
{
    fprintf (stderr, "xdm info (pid %d): ", getpid());
#if NeedVarargsPrototypes
    {
	va_list args;
	va_start(args, fmt);
	vfprintf (stderr, fmt, args);
	va_end(args);
    }
#else
    fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
    fflush (stderr);
}

/*VARARGS1*/
LogError (
#if NeedVarargsPrototypes
    char * fmt, ...)
#else
    fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
    Fmtarg_t arg1, arg2, arg3, arg4, arg5, arg6;
#endif
{
    fprintf (stderr, "xdm error (pid %d): ", getpid());
#if NeedVarargsPrototypes
    {
	va_list args;
	va_start(args, fmt);
	vfprintf (stderr, fmt, args);
	va_end(args);
    }
#else
    fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
    fflush (stderr);
}

/*VARARGS1*/
LogPanic (
#if NeedVarargsPrototypes
    char * fmt, ...)
#else
    fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
    Fmtarg_t arg1, arg2, arg3, arg4, arg5, arg6;
#endif
{
    fprintf (stderr, "xdm panic (pid %d): ", getpid());
#if NeedVarargsPrototypes
    {
	va_list args;
	va_start(args, fmt);
	vfprintf (stderr, fmt, args);
	va_end(args);
    }
#else
    fprintf (fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
    fflush (stderr);
    exit (1);
}

/*VARARGS1*/
LogOutOfMem (
#if NeedVarargsPrototypes
    char * fmt, ...)
#else
    fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
    Fmtarg_t arg1, arg2, arg3, arg4, arg5, arg6;
#endif
{
    fprintf (stderr, "xdm: out of memory in routine ");
#if NeedVarargsPrototypes
    {
	va_list args;
	va_start(args, fmt);
	vfprintf (stderr, fmt, args);
	va_end(args);
    }
#else
    fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
    fflush (stderr);
}

Panic (mesg)
char	*mesg;
{
    int	i;

    i = creat ("/dev/console", 0666);
    write (i, "panic: ", 7);
    write (i, mesg, strlen (mesg));
    exit (1);
}


/*VARARGS1*/
Debug (
#if NeedVarargsPrototypes
    char * fmt, ...)
#else
    fmt, arg1, arg2, arg3, arg4, arg5, arg6)
    char *fmt;
    Fmtarg_t arg1, arg2, arg3, arg4, arg5, arg6;
#endif
{
    if (debugLevel > 0)
    {
#if NeedVarargsPrototypes
	va_list args;
	va_start(args, fmt);
	vprintf (fmt, args);
	va_end(args);
#else
	printf (fmt, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
	fflush (stdout);
    }
}

InitErrorLog ()
{
	int	i;
	if (errorLogFile[0]) {
		i = creat (errorLogFile, 0666);
		if (i != -1) {
			if (i != 2) {
				dup2 (i, 2);
				close (i);
			}
		} else
			LogError ("Cannot open errorLogFile %s\n", errorLogFile);
	}
}
