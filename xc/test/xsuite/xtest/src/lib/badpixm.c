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

#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include	"pixval.h"

/*
 * Return a bad pixmap id on display disp by creating
 * a pixmap and freeing it
 */
Pixmap
badpixm(disp)
Display	*disp;
{
Pixmap	pm;
Window	w;

	w = DefaultRootWindow(disp);
	pm = XCreatePixmap(disp, w, 100, 100, 1);

	XFreePixmap(disp, pm);

	return(pm);
}
