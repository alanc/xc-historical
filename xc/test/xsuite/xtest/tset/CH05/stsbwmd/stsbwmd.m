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
>>TITLE XSetSubwindowMode CH05
void
XSetSubwindowMode(display, gc, subwindow_mode);
Display	*display = Dsp;
GC	gc;
int	subwindow_mode = IncludeInferiors;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the
.M subwindow_mode
component of the specified GC to the value of the
.A subwindow_mode
argument.
>>STRATEGY
Create window.
Create child window fully obscuring the parent.
Create GC with subwindow_mode = IncludeInferiors, fg = WhitePixel, bg = BlackPixel.
Draw point (0, 0) on ParentWindow.
Verify pixel at (0, 0) is fg using XGetImage and XGetPixel.
Verify pixel at (0, 1) is bg using XGetImage and XGetPixel.
Set subwindow_mode of GC to ClipByChildren with XSetSubwindowMode.
Set pixel at (0, 1) on parent window.
Verify pixel at (0, 1) is bg using XGetImage and XGetPixel.
>>CODE
XVisualInfo *vp;
Window	pwin, cwin;
XGCValues	values;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	pwin = makewin(display, vp);
	cwin = crechild(display, pwin, (struct area *) 0);
	values.foreground = W_FG;
	values.background = W_BG;
	values.subwindow_mode = IncludeInferiors;
	gc = XCreateGC(display, pwin, GCForeground | GCBackground | GCSubwindowMode, &values);

	XDrawPoint(display, pwin, gc, 0, 0);

	if( ! checkpixel(display, cwin, 0, 0, W_FG)) {
		delete("Pixel at (0, 0) was not set to foreground.");
		return;
	} else 
		CHECK;


	if( ! checkpixel(display, cwin, 0, 1, W_BG)) {
		delete("Pixel at (0, 1) was not set to background.");
		return;
	} else 
		CHECK;

	subwindow_mode = ClipByChildren;
	XCALL;

	XDrawPoint(display, pwin, gc, 0, 1);

	if( ! checkpixel(display, cwin, 0, 1, W_BG)) {
		report("Pixel at (0, 1) was not set to background.");
		FAIL;
	} else 
		CHECK;
	
	CHECKPASS(3);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER BadGC
>>ASSERTION Bad A
.ER Value subwindow_mode ClipByChildren IncludeInferiors
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
>># HISTORY cal	Action		Writing code.
