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
>>TITLE XScreenNumberOfScreen CH02
int
XScreenNumberOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the screen number of the screen
.A screen .
>>STRATEGY
Obtain the screen number of the screen using xname.
Obtain the root window ID of the screen number using XRootWindow.
Obtain the root window ID of the display using XDefaultRootWindow.
Verify that the root window IDs are the same.
>>CODE
Window	sw, dw;
int	sn;

	sn = XCALL;
	sw = XRootWindow(Dsp, sn);
	dw = XDefaultRootWindow(Dsp);

	if(sw != dw) {
		report("%s() returns a screen number whose root window is not the same as that of the associated display.", TestName);
		report("(0x%lx instead of 0x%lx)", (long) sw, (long) dw);
		FAIL;
	} else
		PASS;
