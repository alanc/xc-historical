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
>>TITLE XRootWindowOfScreen CH02
Window
XRootWindowOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the Window ID of the root window of the screen
.A screen .
>>STRATEGY
Obtain the root window ID of the default display using XDefaultRootWindow.
Obtain the root window ID of the default screen using xname.
Verify that the two window IDs are the same.
>>CODE
Window	srw, drw;

	drw = XDefaultRootWindow(Dsp);
	srw = XCALL;

	if( drw != srw) {
		report("%s() did not return the same root window as the associated display.", TestName);
		report("(0x%lx instead of 0x%lx)", (long) srw, (long) drw);
		FAIL;
	} else
		PASS;
