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
>>TITLE XDefaultColormap CH02
int
XDefaultColormap(display, screen_number)
Display	*display = Dsp;
int	screen_number;
>>ASSERTION Good A
A call to xname returns the ID of the default colourmap for
the screen
.A screen_number .
>>STRATEGY
Obtain the colourmap associated with the root window of the default screen using XGetWindowAttributes.
Verify that the returned colourmap is the default colourmap using xname.
>>CODE
Colormap		rtcmap;
XWindowAttributes	atts;

	screen_number = DefaultScreen(display);
	rtcmap = XCALL;

	if(XGetWindowAttributes(display, RootWindow(display, screen_number), &atts) == 0) {
		delete("XGetWindowAttributes() returned zero.");
		return;
	} else
		CHECK;

	if(rtcmap != atts.colormap) {
		report("%s returned ID %lx instead of %lx for the default colourmap", TestName, (long) rtcmap, (long) atts.colormap);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
