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
>>TITLE XFillRectangle CH06
void

Display	*display = Dsp;
Drawable d;
GC		gc;
int 	x = 20;
int 	y = 30;
unsigned int 	width = 70;
unsigned int 	height = 30;
>>ASSERTION Good A
A call to xname fills the rectangle specified by
.A x ,
.A y ,
.A width
and 
.A height
in the drawable
.A d .
>>STRATEGY
Draw rectangle.
Call checkarea to verify result.
>>CODE
XVisualInfo	*vp;
struct	area	area;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XCALL;

		setarea(&area, x, y, width, height);
		if (checkarea(display, d, &area, W_FG, W_BG, CHECK_ALL))
			CHECK;
		else {
			report("XFillRectangle failed");
			FAIL;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION def
A call to xname fills the rectangle as if a four-point
.S FillPolygon
protocol request were specified in the order
[x, y], [x+width, y], [x+width, y+height], [x, y+height].
>>ASSERTION Good A
A call to xname does not draw a pixel more than once.
>>STRATEGY
Set GC Function to GXxor
Draw rectangle.
Verify that each pixel is set in the area.
>>CODE
XVisualInfo	*vp;
struct	area	area;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);

		XCALL;

		setarea(&area, x, y, width, height);
		if (checkarea(display, d, &area, W_FG, 0, CHECK_IN))
			CHECK;
		else {
			report("XFillRectangle failed");
			FAIL;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION gc
On a call to xname the GC components
.M function ,
.M plane-mask ,
.M fill-style ,
.M subwindow-mode ,
.M clip-x-origin ,
.M clip-y-origin ,
and 
.M clip-mask
are used.
>>ASSERTION gc
On a call to xname the GC mode-dependent components
.M foreground ,
.M background ,
.M tile ,
.M stipple ,
.M tile-stipple-x-origin
and
.M tile-stipple-y-origin
are used.
>>ASSERTION Bad A
.ER BadDrawable
>>ASSERTION Bad A
.ER BadGC
>>ASSERTION Bad A
.ER BadMatch inputonly
>>ASSERTION Bad A
.ER BadMatch gc-drawable-depth
>>ASSERTION Bad A
.ER BadMatch gc-drawable-screen
>># HISTORY steve Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
