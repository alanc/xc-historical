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
>>TITLE XFillRectangles CH06
void

Display	*display = Dsp;
Drawable	d;
GC		gc;
XRectangle	*rectangles = defrects;
int 	nrectangles = sizeof(defrects)/sizeof(XRectangle);
>>EXTERN

static	XRectangle	defrects[] = {
	{2, 2, 20, 10},
	{30, 30, 5, 5},
	{0, 30, 20, 40},
	{70, 50, 20, 20},
};
>>ASSERTION Good A
A call to xname fills
.A nrectangles
rectangles
specified by
.A rectangles
in the drawable
.A d .
>>STRATEGY
Draw rectangles
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XCALL;

		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION def
A call to xname fills each rectangle as if a four-point
.S FillPolygon
protocol request were specified for each rectangle in the order
[x, y], [x+width, y], [x+width, y+height], [x, y+height].
>>ASSERTION Good A
A call to xname does not draw a pixel more than once in any given rectangle.
>>STRATEGY
Set GC function to xor.
Draw rectangles
Check that rectangles have no holes.
>>CODE
XVisualInfo	*vp;
XRectangle	*rp;
struct	area	area;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);

		XCALL;

		for (rp = rectangles; rp < &rectangles[nrectangles]; rp++) {
			setarea(&area, rp->x, rp->y, rp->width, rp->height);
			if (checkarea(display, d, &area, W_FG, 0, CHECK_IN))
				CHECK;
			else {
				report("Pixels drawn twice in rectangle at (%d, %d)", rp->x, rp->y);
				FAIL;
			}
		}
	}
	CHECKPASS(nrectangles*nvinf());

>>ASSERTION Good A
When rectangles intersect, then the intersecting pixels are
drawn multiple times.
>>STRATEGY
Set GC function to GXxor.
Draw intersecting rectangles.
Check that the overlapping region is all unset.
>>CODE
XVisualInfo	*vp;
static	XRectangle recs[] = {
	{0, 0, 60, 40},
	{10, 10, 60, 40},
};
struct	area	area;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);
		rectangles = recs;
		nrectangles = 2;

		XCALL;

		setarea(&area, 10, 10, 50, 30);
		if (checkarea(display, d, &area, W_BG, 0, CHECK_IN))
			CHECK;
		else {
			report("Intersecting area was not all drawn twice");
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
