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
 * Scribble on the drawable.  A series of vertical lines are drawn starting
 * at (0,0) then at (5,0) (10,0) etc.
 */
void
pattern(disp, d)
Display	*disp;
Drawable	d;
{
unsigned int 	width;
unsigned int 	height;
int 	x;
GC  	gc;

	gc = XCreateGC(disp, d, 0, (XGCValues*)0);
	XSetState(disp, gc, W_FG, W_BG, GXcopy, AllPlanes);

	getsize(disp, d, &width, &height);

	for (x = 0; x < width; x += 5)
		XDrawLine(disp, d, gc, x, 0, x, height);
	
	XFreeGC(disp, gc);
}

/*
 * Check that the pattern that is drawn in pattern() is unchanged.
 * This is done by direct pixel validation with GetImage.
 * If ap is non-NULL then validation is restricted to that area
 * with the origin the origin of the area.
 */
Status
checkpattern(disp, d, ap)
Display	*disp;
Drawable	d;
struct	area	*ap;
{
XImage	*imp;
int 	x, y;
unsigned long	pix;
struct	area	area;
	
	if (ap == (struct area *)0) {
		ap = &area;
		ap->x = ap->y = 0;
		getsize(disp, d, &ap->width, &ap->height);
	}

	imp = XGetImage(disp, d, ap->x, ap->y, ap->width, ap->height, AllPlanes, ZPixmap);
	if (imp == (XImage*)0) {
		report("Get Image failed in checkpattern()");
		return(False);
	}

	for (y = 0; y < ap->height; y++) {
		for (x = 0; x < ap->width; x++) {
			pix = XGetPixel(imp, x, y);
			if (pix != ((x%5 == 0)? W_FG: W_BG)) {
				report("Bad pixel in pattern at (%d, %d)", x, y);
				return(False);
			}
		}
	}
	return(True);
}
