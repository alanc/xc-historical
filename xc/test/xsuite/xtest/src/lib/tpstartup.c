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
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

extern	Display	*Dsp;

#ifdef DEBUG
static int 	brkval;
#endif

/*
 * Actions to take at the beginning of a test purpose.
 */
void
tpstartup()
{
#if TEST_ANSI
extern  int     unexp_err(Display *, XErrorEvent *ep);
extern  int     io_err(Display *);
#else
extern  int     unexp_err();
extern  int     io_err();
#endif

	(void) XSetErrorHandler(unexp_err);
	(void) XSetIOErrorHandler(io_err);

	resetdelete();

	if (Dsp)
		XResetScreenSaver(Dsp);
#ifdef DEBUG
	brkval = sbrk(0);
#endif

	/* Do this here in case tpcleanup did not get called */
	freereg();

	/*
	 * Clean up the common display client.
	 */
	if (Dsp)
		XSync(Dsp, True);
}

/*
 * Actions to take at the end of a test purpose.
 */
void
tpcleanup()
{
	/* Free all registered resources */
	freereg();

	/*
	 * Try to ensure that everything that is going to happen for
	 * this test happens now, rather than popping up in the next test.
	 */
	if (Dsp)
		XSync(Dsp, False);
#ifdef DEBUG
	{
	int 	newbrk;
	extern	int 	tet_thistest;

		newbrk = sbrk(0);
		debug(1, "brk value was %d after test %d", newbrk, tet_thistest);
		if (newbrk > brkval)
			printf("brk value increased to %d (+%d) after test %d\n"
				, newbrk, newbrk-brkval, tet_thistest);
	}
#endif
}
