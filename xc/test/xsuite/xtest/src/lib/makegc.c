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

#define	NO_FONT	(~(Font)0)

static	Font	Gcfont = NO_FONT;		/* Default font to place in gc */

/*
 * Make a gc suitable for use with the given drawable.
 * The forground and background will be set to the standard forground
 * and background, function to GXcopy and planes to AllPlanes.
 */
GC
makegc(disp, d)
Display	*disp;
Drawable	d;
{
GC	gc;

	gc = XCreateGC(disp, d, 0, (XGCValues*)0);
	XSetState(disp, gc, W_FG, W_BG, GXcopy, AllPlanes);
	XSetLineAttributes(disp, gc, 1, LineSolid, CapButt, JoinMiter);

	if (Gcfont != NO_FONT)
		XSetFont(disp, gc, Gcfont);

	/* register it */
	regid(disp, (union regtypes *)&gc, REG_GC);

	return(gc);
}

void
setgcfont(font)
Font	font;
{
	Gcfont = font;
}
