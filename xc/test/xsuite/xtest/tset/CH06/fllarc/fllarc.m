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
>>TITLE XFillArc CH06
void

Display	*display = Dsp;
Drawable d;
GC		gc;
int 	x = 20;
int 	y = 20;
unsigned int 	width = 60;
unsigned int 	height = 60;
int 	angle1 = 0;
int 	angle2 = 270*64;
>>ASSERTION Good A
A call to xname draws a single filled circular or elliptical arc in the drawable
.A d
as specified by
.A x ,
.A y ,
.A width ,
.A height ,
.A angle1
and
.A angle2 .
>>STRATEGY
Draw arc
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

>># Next 3 assertions used to be included by >>INCLUDE ../fllarcs/arc.inc
>># Consequently the wording was wrong - I hand included and corrected.
>># DPJ Cater 17/3/91
>>ASSERTION Good A
When the GC component
.M arc-mode
is
.S ArcChord ,
then a call to xname fills the region closed by the infinitely thin path
described by the specified arc and the single line segment joining the two
endpoints of the arc.
>>STRATEGY
Set arc-mode to ArcChord
Draw arcs
Pixmap check.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XSetArcMode(display, gc, ArcChord);

		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
When the GC component
.M arc-mode
is
.S ArcPieSlice , 
then a call to xname fills the region closed by the infinitely thin path
described by the specified arc and
the two line segments joining the endpoints of the arc with the centre
point.
>>STRATEGY
Set arc-mode to ArcPieSlice
Draw arcs.
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XSetArcMode(display, gc, ArcPieSlice);

		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
A call to xname does not draw a pixel more than once.
>>STRATEGY
Draw with gc function set to GXcopy
Draw with gc function set to GXxor
Verify that window is blank
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XCALL;

		XSetFunction(display, gc, GXxor);

		XCALL;

		if (checkarea(display, d, (struct area *)0, W_BG, W_BG, CHECK_IN))
			CHECK;
		else {
			report("Pixels drawn more than once");
			FAIL;
		}
	}

	CHECKPASS(nvinf());

>>ASSERTION gc
On a call to xname the GC components
.M function ,
.M plane-mask ,
.M fill-style ,
.M arc-mode ,
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
