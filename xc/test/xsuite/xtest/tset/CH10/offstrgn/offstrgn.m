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
>>TITLE XOffsetRegion CH10

XOffsetRegion(r, dx, dy)
Region	r;
int	dx;
int	dy;
>>ASSERTION Good A
A call to xname moves the region
.A r
by
.A dx
in the x direction and
.A dy
in the y direction.
>>STRATEGY
Create a window using XCreateWindow.
Create a gc using XCreateGC.
Set the gc function to GXxor using XSetFunction.
Set the foreground pixel of the gc to W_FG ^ W_BG using XSetForeground.
Draw a rectangle using XFillRectangle.
Create a region using XCreateRegion.
Verify that the call did not return NULL.
Set the region to a rectangle of the same dimension but a different position as the drawn one using XUnionRectWithRegion.
Offset the region to lie over the same position as the drawn rectangle using xname.
Set the clipmask of the gc to the region using XSetRegion.
Fill the entire drawable using XFillRectangle.
Verify that the entire window is W_BG.
>>CODE
XVisualInfo		*vi;
Window			win;
GC			gc;
unsigned int    	width;
unsigned int    	height;
static XRectangle	cliprect = { 23, 13, 53, 63 };
static XRectangle	rect = { -1, -2, 53, 63 };

	resetvinf(VI_WIN);
	nextvinf(&vi);
	win = makewin(Dsp, vi); /* Makes a window with bg W_BG. */

	gc = makegc(Dsp, win);
	XSetFunction(Dsp, gc, GXxor);
	XSetForeground(Dsp, gc, W_BG^W_FG);
	XFillRectangle(Dsp, win, gc, cliprect.x, cliprect.y, cliprect.width, cliprect.height);
	if( checkarea(Dsp, win, (struct area *) NULL, W_BG, W_BG, CHECK_ALL|CHECK_DIFFER) != 0) {
		delete("All pixels were set to W_BG before calling %s", TestName);
		return;
	} else
		PASS;

	if( (r = XCreateRegion()) == (Region) NULL) {
		delete("XCreateRegion() returned NULL.");
		return;
	} else
		CHECK;

	XUnionRectWithRegion(&rect, r, r);
	dx = cliprect.x - rect.x;
	dy = cliprect.y - rect.y;
	XCALL;
	XSetRegion(Dsp, gc, r);
	getsize(Dsp, win, &width, &height);
	XFillRectangle(Dsp, win, gc, 0,0, width, height);

	if( checkarea(Dsp, win, (struct area *) NULL, W_BG, W_BG, CHECK_ALL) == 0) {
		report("Some pixels were not set to W_BG after calling %s", TestName);
		FAIL;
	} else
		PASS;
	
	XFlush(Dsp);
	sleep(5);
	XDestroyRegion(r);
