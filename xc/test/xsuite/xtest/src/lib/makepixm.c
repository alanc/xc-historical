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
#include	"tet_api.h"
#include	"pixval.h"

#define	BORDER	1

/*
 * Creates a general purpose pixmap that can be used within the
 * test suite.  The pixmap is cleared to W_BG.
 */
Pixmap
makepixm(disp, vp)
Display	*disp;
XVisualInfo	*vp;
{
GC  	gc;
Pixmap	pm;

	pm = XCreatePixmap(disp, DRW(disp), W_STDWIDTH, W_STDHEIGHT, vp->depth);
	gc = makegc(disp, pm);

	/*
	 * Set the forground to the background pixel so that we can clear the
	 * pixmap using fill rectangle.
	 */
	XSetForeground(disp, gc, W_BG);
	XFillRectangle(disp, pm, gc, 0, 0, W_STDWIDTH+1, W_STDHEIGHT+1);

	/* register pixmap, will be conditional */
	regid(disp, (union regtypes *)&pm, REG_PIXMAP);

	return(pm);
}
