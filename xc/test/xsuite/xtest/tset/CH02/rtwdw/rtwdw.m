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
>>TITLE XRootWindow CH02
Window
XRootWindow(display, screen_number)
Display	*display = Dsp;
int	screen_number = DefaultScreen(Dsp);
>>ASSERTION Good A
A call to xname returns the root window of the screen specified by the
.A screen_number
argument.
>>STRATEGY
Obtain the attributes of the root window of the default screen using XGetWindowAttributes.
Obtain the root window attribute of the default screen using xname.
Verify that the window IDs are the same.
>>CODE
XWindowAttributes	atts;
Window			rootw;

	if(XGetWindowAttributes(display, DefaultRootWindow(display), &atts) == 0) {
		delete("XGetWindowAttributes() returned 0.");
		return;
	} else
		CHECK;

	rootw = XCALL;	

	if( atts.root != rootw) {
		report("%s() returned ID %lx instead of %lx.", TestName, (long) rootw, (long) atts.root);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
