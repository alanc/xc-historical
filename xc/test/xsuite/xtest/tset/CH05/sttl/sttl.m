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
>>TITLE XSetTile CH05
void
XSetTile(display, gc, tile)
Display *display = Dsp;
GC gc;
Pixmap tile;
>>ASSERTION Good A
A call to xname sets the
.M tile
component of the specified GC to the value of the 
.A tile
argument.
>>STRATEGY
Create window, size W_STDWIDTHxW_STDHEIGHT (>=1x1), with
	bg = background_pixel = W_BG.
Create 1x1 pixmap, tile1.
Create 1x1 pixmap, tile2.
Create GC with fg = W_BG.
Set pixel at (0, 0) in tile2 to W_BG with XDrawPoint.
Set GC component fg = W_FG using XSetForeground.
Set pixel at (0, 0) in tile1 to W_FG with XDrawPoint.
Create GC with fill_style = FillTiled, tile = tile1.
Draw a filled rectangle (0, 0) (1, 1) with XFillRectangle.
Verify pixel at (0, 0) is W_FG with XGetImage and XGetPixel.
Set GC component tile = tile2 using XSetTile.
Draw a filled rectangle (0, 0) (1, 1) with XFillRectangle.
Verify pixel at (0, 0) is W_BG with XGetImage and XGetPixel.
>>CODE
XVisualInfo	*vp;
Window		win;
XGCValues	values;
Pixmap		tile1, tile2;
GC		tgc;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp); /* background_pixel = W_BG */
	tile1 = XCreatePixmap( display, win, 1, 1, vp->depth); 	
	tile2 = XCreatePixmap( display, win, 1, 1, vp->depth); 	

	values.foreground = W_BG;

	tgc = XCreateGC(display, win, GCForeground, &values);
	XDrawPoint(display, tile2, tgc, 0, 0);

	XSetForeground(display, tgc, W_FG);
	XDrawPoint(display, tile1, tgc, 0, 0);

	values.fill_style = FillTiled;
	values.tile = tile1;
	gc = XCreateGC(display, win, GCFillStyle | GCTile, &values);
	XFillRectangle(display, win, gc, 0, 0, 1, 1);

	if( ! checkpixel(display, win, 0, 0, W_FG)) {
		delete("Pixel at (0, 0) was not set to foreground.");
		return;
	} else 
		CHECK;

	tile = tile2;
	XCALL;

	XFillRectangle(display, win, gc, 0, 0, 1, 1);

	if( ! checkpixel(display, win, 0, 0, W_BG)) {
		report("Pixel at (0, 0) was not set to background.");
		FAIL;
	} else 
		CHECK;

	CHECKPASS(2);
>>ASSERTION Bad A
.ER Match gc-drawable-screen
>>ASSERTION Bad A
.ER Match gc-drawable-depth
>>ASSERTION Bad A
.ER Alloc
>>ASSERTION Bad A
.ER GC
>>ASSERTION Bad A
.ER BadPixmap
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
>># HISTORY kieron Completed	Please re-check assertions marked NOTE.
>># HISTORY cal Action 		Writing code.
