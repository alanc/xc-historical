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
>>TITLE XSetClipOrigin CH05
void
XSetClipOrigin(display, gc, clip_x_origin, clip_y_origin)
Display *display = Dsp;
GC gc;
int clip_x_origin = -1;
int clip_y_origin = -1;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the
.M clip_x_origin
and
.M clip_y_origin
components of the specified GC to the values of the
.A clip_x_origin
and
.A clip_y_origin
arguments.
>>STRATEGY
Create window.
Create 1x1 pixmap.
Create GC with clip_origin = (0,0), clip_mask = pixmap, bg = BlackPixel, fg = WhitePixel.
Set pixel at (2,2) to fg with XDrawPoint.
Verify pixel at (2,2) is bg using XGetImage and XGetPixel.
Set clip_origin to (2,2) with XSetClipOrigin.
Set pixel at (2,2) to fg with XDrawPoint.
Verify pixel at (2,2) is fg using XGetImage and XGetPixel.
>>CODE
XVisualInfo	*vp;
Window		win;
XGCValues	values;
Pixmap		pmap;
GC		pgc;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);
	pmap = XCreatePixmap( display, win, 1, 1, 1); 	

	values.foreground = W_FG;
	values.background = W_BG;
	values.clip_mask = pmap;
	values.clip_x_origin = 0;
	values.clip_y_origin = 0;

	pgc = XCreateGC(display, pmap, GCForeground | GCBackground, &values);
	XFillRectangle(display, pmap, pgc, 0,0, 1,1);

	gc = XCreateGC(display, win, GCForeground | GCBackground | GCClipMask | GCClipXOrigin | GCClipYOrigin, &values);
	XDrawPoint(display,win,gc,2,2);

	if( ! checkpixel(display, win, 2, 2, W_BG)) {
		delete("Pixel at (2,2) was not set to background.");
		return;
	} else 
		CHECK;

	clip_x_origin = 2;
	clip_y_origin = 2;
	XCALL;

	XDrawPoint(display,win,gc,2,2);

	if( ! checkpixel(display, win, 2, 2, W_FG)) {
		report("Pixel at (2,2) was not set to foreground.");
		FAIL;
	} else 
		CHECK;
	
	CHECKPASS(2);

>>ASSERTION Bad A
.ER Alloc
>>ASSERTION Bad A
.ER GC
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
>># HISTORY cal Action		Writing code.
