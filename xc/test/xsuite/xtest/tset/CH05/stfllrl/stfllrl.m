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
>>TITLE XSetFillRule CH05
void
XSetFillRule(display, gc, fill_rule)
Display *display = Dsp;
GC gc;
int fill_rule = WindingRule;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the
.M fill_rule
component of the specified GC to the value of the
.A fill_rule
argument.
>>STRATEGY
Create a Window.
Create GC with fill_rule = EvenOddRule, bg = BlackPixel, fg = WhitePixel.
Draw (complex) polygon (0,0), (0,1) , (1,1), (1,0, (0,0) , (0,1) , (1,1) , (1,0) using XFillPolygon.
Verify that pixel (0,0) is bg using XGetImage and XGetPixel.
Set fill_rule component of GC to WindingRule using XSetFillRule.
Draw (complex) polygon (0,0), (0,1) , (1,1), (1,0, (0,0) , (0,1) , (1,1) , (1,0) using XFillPolygon.
Verify that pixel (0,0) is fg using XGetImage and XGetPixel.
>>EXTERN
XPoint poly[] = { {0,0} , {0,1}, {1,1}, {1,0}, {0,0}, {0,1}, {1,1}, {1,0} };
>>CODE
XVisualInfo	*vp;
Window		win;
XGCValues	values;
Pixmap		stip;
GC		sgc;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);
	values.foreground = W_FG;
	values.background = W_BG;
	values.fill_rule = EvenOddRule;

	gc = XCreateGC(display, win, GCForeground | GCBackground | GCFillRule, &values);

	XFillPolygon(display, win, gc, poly, sizeof(poly) / sizeof(XPoint), Complex, CoordModeOrigin);

	if( ! checkpixel(display, win, 0, 0, W_BG)) {
		delete("Pixel at (0, 0) was not set to background.");
		return;
	} else 
		CHECK;


	fill_rule = WindingRule;
	XCALL;

	XFillPolygon(display, win, gc, poly, sizeof(poly) / sizeof(XPoint), Complex, CoordModeOrigin);

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
>>ASSERTION Bad A
.ER Value fill_rule EvenOddRule WindingRule
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed      Final checking to do - 21/11/90
>># HISTORY cal Action		Writing code.
