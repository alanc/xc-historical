/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */
/*LINTLIBRARY*/
/*
 * Reporting functions these implement a higher level
 * on top of tet_infoline.
 * 
 *  REPORT -- report(char *fmt, ...)
 *    A description of something that went wrong.
 *  TRACE -- trace(char *fmt, ...)
 *    A 'I am here' or description of somthing that happened
 *    that is not an error in itself.
 *  DEBUG -- debug(char *fmt, ...)
 *    Debug lines -- Interface to be decided
 *  CHECK -- ???
 *    Path trace line.
 */

#include	"stdio.h"
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"

#if TEST_ANSI
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#define	LINELEN	1024

static int 	DebugLevel = 0;

extern	struct	config	config;

/*VARARGS1*/

void
#if TEST_ANSI
report(char *fmt, ...)
#else
report(fmt, va_alist)
char	*fmt;
va_dcl
#endif
{
char	buf[LINELEN];
va_list	args;

#if TEST_ANSI
	va_start(args, fmt);
#else
	va_start(args);
#endif

	(void) strcpy(buf, "REPORT:");
	(void) vsprintf(buf+strlen("REPORT:"), fmt, args);
	tet_infoline(buf);

	va_end(args);

}

/*VARARGS1*/

void
#if TEST_ANSI
trace(char *fmt, ...)
#else
trace(fmt, va_alist)
char	*fmt;
va_dcl
#endif
{
char	buf[LINELEN];
va_list	args;

	if (config.option_no_trace)
		return;

#if TEST_ANSI
	va_start(args, fmt);
#else
	va_start(args);
#endif

	(void) strcpy(buf, "TRACE:");
	(void) vsprintf(buf+strlen("TRACE:"), fmt, args);
	tet_infoline(buf);

	va_end(args);
}

/*VARARGS1*/

void
#if TEST_ANSI
check(char *fmt, ...)
#else
check(fmt, va_alist)
char	*fmt;
va_dcl
#endif
{
char	buf[LINELEN];
va_list	args;

	if (config.option_no_check)
		return;

#if TEST_ANSI
	va_start(args, fmt);
#else
	va_start(args);
#endif

	(void) strcpy(buf, "CHECK:");
	(void) vsprintf(buf+strlen("CHECK:"), fmt, args);
	tet_infoline(buf);

	va_end(args);
}

/*VARARGS2*/

void
#if TEST_ANSI
debug(int lev, char *fmt, ...)
#else
debug(lev, fmt, va_alist)
int 	lev;
char	*fmt;
va_dcl
#endif
{
char	buf[LINELEN];
va_list	args;

	if (lev > DebugLevel)
		return;

#if TEST_ANSI
	va_start(args, fmt);
#else
	va_start(args);
#endif

	(void) strcpy(buf, "DEBUG:");
	(void) vsprintf(buf+strlen("DEBUG:"), fmt, args);
	tet_infoline(buf);

	va_end(args);
}

/*
 * This formats its arguments as in report().  It also issues the result
 * code MIT_TET_ABORT.  This causes the TCM to give up, and also the TCC if
 * running under it.  It should be used when something is so badly wrong
 * that there is no point continuing any more tests.
 */

/*VARARGS1*/

void
#if TEST_ANSI
tccabort(char *fmt, ...)
#else
tccabort(fmt, va_alist)
char	*fmt;
va_dcl
#endif
{
char	buf[LINELEN];
va_list	args;

#if TEST_ANSI
	va_start(args, fmt);
#else
	va_start(args);
#endif

	(void) strcpy(buf, "REPORT:");
	(void) vsprintf(buf+strlen("REPORT:"), fmt, args);
	tet_infoline(buf);

	va_end(args);

	tet_result(MIT_TET_ABORT);

}

void
setdblev(n)
int 	n;
{
	DebugLevel = n;
}

int
getdblev()
{
	return DebugLevel;
}
