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

/*
 * Routines to handle reporting of test result codes and associated messages.
 * If there is a message it is output using report().  
 * If not then a generic message is output as a fail safe measure.
 */

/*
 * Call this routine to report the current test as unsupported.  
 */
/*VARARGS1*/

void
#if TEST_ANSI
unsupported(char *mess, ... )
#else
unsupported(mess, va_alist)
char	*mess;
va_dcl
#endif
{
char	buf[LINELEN];
va_list args;

#if TEST_ANSI
	va_start(args, mess);
#else
	va_start(args);
#endif


	if (mess && *mess) {
		(void) vsprintf(buf, mess, args);
		report(buf);
	} else
		report("Test unsupported");

	UNSUPPORTED;
}

/*
 * Call this routine to report the current test as notinuse.  
 */
/*VARARGS1*/

void
#if TEST_ANSI
notinuse(char *mess, ... )
#else
notinuse(mess, va_alist)
char	*mess;
va_dcl
#endif
{
char	buf[LINELEN];
va_list args;

#if TEST_ANSI
	va_start(args, mess);
#else
	va_start(args);
#endif


	if (mess && *mess) {
		(void) vsprintf(buf, mess, args);
		report(buf);
	} else
		report("Test not in use");

	NOTINUSE;
}

/*
 * Call this routine to report the current test as untested.  
 */
/*VARARGS1*/

void
#if TEST_ANSI
untested(char *mess, ... )
#else
untested(mess, va_alist)
char	*mess;
va_dcl
#endif
{
char	buf[LINELEN];
va_list args;

#if TEST_ANSI
	va_start(args, mess);
#else
	va_start(args);
#endif


	if (mess && *mess) {
		(void) vsprintf(buf, mess, args);
		report(buf);
	} else
		report("Test is untested");

	UNTESTED;
}
