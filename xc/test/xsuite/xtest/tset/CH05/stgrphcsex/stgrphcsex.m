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
>>TITLE XSetGraphicsExposures CH05
void
XSetGraphicsExposures(display, gc, graphics_exposures)
Display *display = Dsp;
GC gc;
Bool graphics_exposures = False;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the
.M graphics_exposures
component of the specified GC to the value of the
.A graphics_exposures
argument.
>>STRATEGY
Create window.
Create child window partially obscuring parent.
Create GC with graphics_exposures = True.
Flush event queue with XSync.
Copy from parent window using XCopyArea.
Verify that a GraphicsExpose event was generated, 
Set graphics_exposures = False with XSetGraphicsExposures.
Flush event queue with XSync.
Copy from parent window using XCopyArea.
Verify that no event was generated.
>>CODE
XEvent event;
XVisualInfo *vp;
Window	pwin, cwin;
XGCValues	values;
struct area ar;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	pwin = makewin(display, vp);
	ar.x = ar.y = 10;
	ar.width = ar.height = 10;
	cwin = crechild(display, pwin, &ar);
	values.foreground = W_FG;
	values.background = W_BG;
	values.graphics_exposures = True;
	gc = XCreateGC(display, pwin, GCForeground | GCBackground| GCGraphicsExposures, &values);

	XSync(display, True);
	XCopyArea(display, pwin, pwin, gc, 5, 5, 10, 10, 0, 0);
	XSync(display, False);

	if( getevent(display, &event) == 0 ) {
		delete("No graphics expose event was generated.");
		return;
	}

	if(event.type == GraphicsExpose)
		CHECK;
	else {
		delete("Event was not of type GraphicsExpose.");
		return;
	}

	graphics_exposures = False;
	XCALL;	

	XSync(display, True);
	XCopyArea(display, pwin, pwin, gc, 5, 5, 10, 10, 0, 0);
	XSync(display, False);

	if(getevent(display, &event) != 0) {
		report("An Event was generated.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER GC
>>ASSERTION Bad A
.ER Value graphics_exposures True False
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY cal	Action		Writing code.
