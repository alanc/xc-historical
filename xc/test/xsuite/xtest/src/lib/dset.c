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

/*
 * Clear a drawable to the pixel W_BG, it uses fill rectangle so
 * that it will work on pixmaps.
 */
void
dclear(disp, d)
Display	*disp;
Drawable d;
{
void dset();

	dset(disp, d, W_BG);
}

/*
 * Set every pixel in a drawable to pix, it uses fill rectangle so that it
 * works on pixmaps.
 */
void
dset(disp, d, pix)
Display	*disp;
Drawable d;
unsigned long	pix;
{
unsigned int 	width, height;
GC		gc;

	getsize(disp, d, &width, &height);
	gc = makegc(disp, d);
	XSetForeground(disp, gc, pix);
	XFillRectangle(disp, d, gc, 0, 0, width+1, height+1);
	/* XFreeGC(disp, gc); */
}
