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
>>TITLE XSetRegion CH10

XSetRegion(display, gc, r)
Display	*display = Dsp;
GC	gc;
Region	r = XCreateRegion();
>>SET tpcleanup strgncleanup
>>EXTERN
static void
strgncleanup()
{
	XDestroyRegion(r);
	tpcleanup();
}

>>ASSERTION Good A
A call to xname sets the
.M clip_mask
component of the
.A gc
argument to the region
.A r .
>>STRATEGY
Create a region using XCreateRegion.
Set the region to a rectangle using XUnionRectWithRegion.
Create a drawable using XCreateWindow.
Create a gc using XCreateGC.
Draw the clip mask rectangle using XFillRectangle.
Set the graphics function of the GC to GXxor using XSetFunction.
Set the foreground pixel of the gc to W_BG ^ W_FG using XSetForeground.
Set the clip mask of the gc to the region using xname.
Set every pixel in the drawable using XFillRectangle.
Verify that every pixel in the drawable is set to W_BG.
Destroy the region using XDestroyRegion.
>>CODE
XVisualInfo		*vi;
Window			win;
GC			gc;
unsigned int    	width;
unsigned int    	height;
static XRectangle	cliprect2 = { 23, 13, 53, 63 };
static XRectangle	cliprect = { 0,0, 1,1};

	resetvinf(VI_WIN);
	nextvinf(&vi);
	win = makewin(display, vi); /* Makes a window with bg W_BG. */

	gc = makegc(display, win);
	XSetFillStyle(display, gc, FillSolid);
	XSetFunction(display, gc, GXxor);
	XSetForeground(display, gc, W_BG^W_FG);

	XUnionRectWithRegion(&cliprect, r, r);
	XFillRectangle(display, win, gc, cliprect.x, cliprect.y, cliprect.width, cliprect.height);

	if( checkarea(display, win, (struct area *) NULL, W_BG, W_BG, CHECK_ALL|CHECK_DIFFER)){
		delete("XFillRectangle() did set any pixels to non-background values.");
		return;
	} else
		CHECK;

	XCALL;
	getsize(display, win, &width, &height);
	XFillRectangle(display, win, gc, 0,0, width, height);

	if( checkarea(display, win, (struct area *) NULL, W_BG, W_BG, CHECK_ALL) == 0){
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(2);

>>ASSERTION Bad A
.ER BadGC
