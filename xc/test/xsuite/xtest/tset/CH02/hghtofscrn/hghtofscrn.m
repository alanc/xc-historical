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
>>TITLE XHeightOfScreen CH02
int
XHeightOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the height in pixels of the screen
.A screen .
>>STRATEGY
Obtain the height of the root window using XGetWindowAttributes.
Obtain the pixel height of the screen using xname.
Verify that heights are the same.
>>CODE
XWindowAttributes	atts;
int			height;

	if(XGetWindowAttributes(Dsp, DefaultRootWindow(Dsp), &atts) == 0) {
		report("XGetWindowAttributes() returned zero.");
		FAIL;
	} else
		CHECK;

	height = XCALL;

	if(height != atts.height) {
		report("%s() returned %d instead of %d.", TestName, height, atts.height);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
