/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: stplnmsk.m,v 1.5 92/06/11 16:19:07 rws Exp $
 */
>>TITLE XSetPlaneMask CH05
void
XSetPlaneMask(display, gc, plane_mask)
Display *display = Dsp;
GC gc;
unsigned long plane_mask;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the 
.M plane_mask
component of the specified GC to the value of the
.A plane_mask
argument.
>>STRATEGY
Create window.
Create GC with bg = BlackPixel, fg = WhitePixel, plane_mask = 0.
Verify pixel at (0, 0) is bg with XGetImage and XGetPixel.
Set pixel at (0, 0) with XDrawPoint.
Verify pixel at (0, 0) is bg with XGetImage and XGetPixel.
Set plane_mask component of GC to AllPlanes with XSetPlaneMask.
Set pixel at (0, 0) with XDrawPoint.
Verify pixel at (0, 0) is fg with XGetImage and XGetPixel.
>>CODE
XVisualInfo *vp;
XGCValues values;
Window win;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);
	values.foreground = W_FG;
	values.background = W_BG;
	values.plane_mask = 0;

	gc = XCreateGC(display, win, (GCPlaneMask | GCForeground | GCBackground), &values);

	if( ! checkpixel(display, win, 0, 0, W_BG)) {
		delete("Pixel at (0, 0) was not set to background.");
		return;
	} else
		CHECK;

	XDrawPoint(display, win, gc, 0, 0);

	if( ! checkpixel(display, win, 0, 0, W_BG)) {
		delete("Pixel at (0, 0) was not left as background.");
		return;
	} else
		CHECK;

	plane_mask = AllPlanes;
	XCALL;

	XDrawPoint(display, win, gc, 0, 0);
	if( ! checkpixel(display, win, 0, 0, W_FG)) {
		report("Pixel at (0, 0) was not set to foreground.");
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(3);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER GC
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
>># HISTORY cal Action		Writing code.
