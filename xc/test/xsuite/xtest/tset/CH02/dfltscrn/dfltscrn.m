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
>>SET   macro
>>TITLE XDefaultScreen CH02
int
XDefaultScreen(display)
Display	*display = Dsp;
>>ASSERTION Good A
A call to xname returns the screen number which was passed to the
.S XOpenDisplay
call that returned the
.A display
argmuent.
>>STRATEGY
Obtain the default screen number using xname.
Obtain the default screen using XScreenOfDisplay.
Obtain the root window ID of the default screen using XRootWindowOfScreen.
Obtain the root window ID of the default display using XDefaultRootWindow.
Verify that the root window IDs are the same.
>>CODE
Window	drw, srw;
Screen	*scr;
int	scrn;

	scrn = XCALL;
	scr = XScreenOfDisplay(Dsp, scrn);
	srw = XRootWindowOfScreen(scr);
	drw = XDefaultRootWindow(Dsp);

	if(drw != srw) {
		report("%s() returns a screen number whose root window is not the same as that of the associated display.",TestName);
		FAIL;
	} else
		PASS;
