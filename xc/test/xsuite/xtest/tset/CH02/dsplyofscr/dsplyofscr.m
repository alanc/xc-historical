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
>>TITLE XDisplayOfScreen CH02
Display *
XDisplayOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the display of the screen
.A screen .
>>STRATEGY
Obtain the root window of the screen using XRootWindowOfScreen.
Obtain the display of the screen using xname.
Obtain the root window of the display using XDefaultRootWindow.
Verify that the two IDs are the same.
>>CODE
Display	*display;
Window	dw, sw;

	display = XCALL;
	sw = XRootWindowOfScreen(screen);
	dw = DefaultRootWindow(display);

	if( sw != dw ) {
		report("The associated root windows of the screen and display are not the same.");
		FAIL;
	} else
		PASS;
