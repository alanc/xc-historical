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
>>TITLE XScreenOfDisplay CH02
Screen *
XScreenOfDisplay(display, screen_number)
Display	*display = Dsp;
int	screen_number = DefaultScreen(Dsp);
>>ASSERTION Good A
A call to xname returns a pointer to the
.S Screen
structure indicated by the
.A screen_number
argument.
>>STRATEGY
Obtain the default screen using xname.
Obtain the root window ID of the default screen using XRootWindowOfScreen.
Obtain the root window ID of the default display using XDefaultRootWindow.
Verify that the two IDs are the same.
>>CODE
Window	drw, srw;
Screen	*scr;

	scr = XCALL;
	srw = XRootWindowOfScreen(scr);	
	drw = XDefaultRootWindow(display);

	if(drw != srw) {
		report("%s() returns a screen whose root window is not the same as that of the associated display.",TestName);
		report("(0x%lx instead of 0x%lx)", (long) srw, (long) drw);
		FAIL;
	} else
		PASS;
