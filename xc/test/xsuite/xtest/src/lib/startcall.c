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

#include	"xtest.h"
#include	"stdio.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"tet_api.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * This routine is similar to startcall except that no XSync() occurs.
 * Various tests assume that this routine merely sets up the error
 * handler stuff.  This routine should not generate protocol requests.
 */
void
_startcall(disp)
Display	*disp;
{
extern	int	error_status();
	/* Reset the error status */
	reseterr();

	/*
	 * Set error handler to trap errors that occur on this call
	 * Should be LAST.
	 */
	XSetErrorHandler(error_status);
}

/*
 * This routine contains setup procedures that should be called
 * before each time that the routine under test is called.
 */
void
startcall(disp)
Display	*disp;
{
	XSync(disp, True);

	_startcall(disp);
}

/*
 * This routine is similar to endcall() except that XSync() is
 * not called.
 * Various tests assume that this routine merely un-sets up the error
 * handler stuff.  This routine should not generate protocol requests.
 */
void
_endcall(disp)
Display	*disp;
{
extern	int 	unexp_err();

	/*
	 * Go back to the unexpected error handler.
	 */
	XSetErrorHandler(unexp_err);

	/* A debuging aid - pause for CR after displaying window */
	if (config.debug_pause_after) {
	int 	c;
	extern	int 	tet_thistest;

		printf("Test %d: Hit return to continue...", tet_thistest);
		fflush(stdout);
		while ((c = getchar()) != '\n' && c != EOF)
			;
	}
}

/*
 * The endcall() routine does all common cleanup code that
 * should be called after the routine under test has been
 * called.  XSync() needs to be called before this.
 */
void
endcall(disp)
Display	*disp;
{

	XSync(disp, False);

	_endcall(disp);
}
