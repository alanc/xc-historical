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
#include	"tet_api.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"
#include	"XstlibInt.h"

#if TEST_ANSI
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#define	LINELEN	1024

/*
 * Routines to control deleting of tests.
 *
 * xproto lib version to avoid pulling in Xlib functions like XSync or
 * the connection opening stuff along with references to Dsp etc.
 * (refer to libxtest versions of same to see what's happening)
 * Use Xst_delete_count instead of a static boolean.
 */

extern	int 	tet_thistest;

/*
 * Call this routine to delete the current test.  If there is a message
 * it is output using report().  If not then a generic message is output
 * as a fail safe measure.
 */
/*VARARGS1*/

void
#if TEST_ANSI
delete(char *mess, ... )
#else
delete(mess, va_alist)
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

	Xst_delete_count++;

	if (mess && *mess) {
		(void) vsprintf(buf, mess, args);
		report(buf);
	} else
		report("Test deleted");

	/*
	 * Can not call tet_result from within a startup routine.
	 */
	if (tet_thistest)
		tet_result(TET_UNRESOLVED);
}

/*
 * Return True if deleted, False if not.
 */
int
isdeleted()
{
	return(Xst_delete_count > 0);
}

/*
 * Reset the delete count back to 0.
 */
void
resetdelete()
{
	Xst_delete_count = 0;
}

/*
 * Cancel all remaining tests in this test case.  They will
 * not be initiated by the TCM.  The current test is not affected,
 * that should be marked as deleted with delete() if that is desired.
 * Can also be called from startup routines - none of the tests will be
 * run.
 */
void
cancelrest(reason)
char	*reason;
{
extern	int 	ntests;
int 	i;

	for (i = tet_thistest+1; i <= ntests; i++)
		tet_delete(i, reason);
}
