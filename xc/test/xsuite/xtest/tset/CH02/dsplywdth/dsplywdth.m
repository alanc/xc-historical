/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of th * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */
>>SET   macro
>>TITLE XDisplayWidth CH02
int
XDisplayWidth(display, screen_number)
Display	*display = Dsp;
int	screen_number = DefaultScreen(Dsp);
>>ASSERTION Good A
A call to xname returns the width in pixels of the screen
.A screen_number .
>>STRATEGY
Obtain the width of the root window using XGetWindowAttributes.
Obtain the pixel width of the screen using xname.
Verify that the two numbers are the same.
>>CODE
XWindowAttributes	atts;
int			width;

	if(XGetWindowAttributes(display, RootWindow(display, screen_number), &atts) == 0) {
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
