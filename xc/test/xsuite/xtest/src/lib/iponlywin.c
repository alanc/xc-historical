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

#include	"stdlib.h"

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"tet_api.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * Create an input-only window on the given display
 * The window has size 5x5 at location 1, 1.
 */
Window
iponlywin(disp)
Display	*disp;
{
Window	w;
XSetWindowAttributes atts;

	atts.override_redirect = config.debug_override_redirect;
	w = XCreateWindow(disp
		, DefaultRootWindow(disp)
		, 1
		, 1
		, 5
		, 5
		, 0
		, 0
		, InputOnly
		, (Visual *)CopyFromParent
		, CWOverrideRedirect
		, &atts
		);
	regid(disp, (union regtypes *)&w, REG_WINDOW);
	return(w);
}
