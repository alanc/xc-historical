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
>>TITLE XSetTSOrigin CH05
void
XSetTSOrigin(display, gc, ts_x_origin, ts_y_origin)
Display *display = Dsp;
GC gc;
int ts_x_origin = -1;
int ts_y_origin = -1;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the
.M ts_x_origin
and
.M ts_y_origin
components of the specified GC to the arguments
.A ts_x_origin
and
.A ts_y_origin .
>>STRATEGY
Create Window.  
Create GC with fg = WhitePixel, bg = BlackPixel, fill_style = FillOpaqueTiled,
 tile = | 0 1 |
        | 1 1 |  
Draw a filled rectangle (0, 0) (2, 2) with XFillRectangle.
Verify Pixel at (0, 0) is bg with XGetImage and XGetPixel.
Set Tile/Stipple origin to (1, 1) with XSetTSOrigin.
Draw a filled rectangle (0, 0) (2, 2) with XFillRectangle.
Verify Pixel at (0, 0) is bg with XGetImage and XGetPixel.
>>CODE
XVisualInfo	*vp;
Window		win;
XGCValues	values;
Pixmap		stip;
GC	sgc;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);
	stip = XCreatePixmap( display, win, 2, 2, 1); 	

	values.foreground = 1;
	sgc = XCreateGC(display, stip, GCForeground, &values);
	XFillRectangle(display, stip, sgc, 0, 0, 2, 2);
	XSetForeground(display, sgc, 0);
	XDrawPoint(display, stip, sgc, 0, 0);

	values.foreground = W_FG;
	values.background = W_BG;
	values.fill_style = FillOpaqueStippled;
	values.stipple = stip;

	gc = XCreateGC(display, win, GCForeground | GCBackground | GCFillStyle | GCStipple, &values);
	XFillRectangle(display, win, gc, 0, 0, 2, 2);

	if( ! checkpixel(display, win, 0, 0, W_BG)) {
		delete("Pixel at (0, 0) was not set to background.");
		return;
	} else 
		CHECK;

	ts_x_origin = 1;
	ts_y_origin = 1;
	XCALL;

	XFillRectangle(display, win, gc, 0, 0, 2, 2);
	if( ! checkpixel(display, win, 0, 0, W_FG)) {
		report("Pixel at (0, 0) was not set to foreground.");
		FAIL;
	} else 
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER GC
>># HISTORY cal Completed	Written in new format and style /tree/Xtest/x/root/xtest/lib/libxtest.a
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
>># HISTORY cal Action 		Writing code.
