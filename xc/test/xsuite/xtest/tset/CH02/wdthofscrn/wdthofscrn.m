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
>>TITLE XWidthOfScreen CH02
int
XWidthOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the width in pixels of the screen
.A screen .
>>STRATEGY
Obtain the Width of the root window using XGetWindowAttributes.
Obtain the pixel width of the screen using xname.
Verify that the two numbers are the same.
>>CODE
XWindowAttributes	atts;
int			width;

	if(XGetWindowAttributes(Dsp, DefaultRootWindow(Dsp), &atts) == 0) {
		report("XGetWindowAttributes() returned zero.");
		FAIL;
	} else
		CHECK;

	width = XCALL;

	if(width != atts.width) {
		report("%s() returned %d instead of %d.", TestName, width, atts.width);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
