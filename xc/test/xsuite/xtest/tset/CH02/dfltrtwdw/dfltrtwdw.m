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
>>TITLE XDefaultRootWindow CH02
Window
XDefaultRootWindow(display)
Display	*display = Dsp;
>>ASSERTION Good A
A call to xname returns the root window of the default screen.
>>STRATEGY
Obtain the root window ID of the default screen using xname.
Obtain the root window ID of the default screen using XGetWindowAttributes.
Verify that the two IDs are the same.
>>CODE
Window			dwin;
XWindowAttributes	atts;
int			screen_number;

	screen_number = DefaultScreen(display);

	dwin = XCALL;
	if(XGetWindowAttributes(display, RootWindow(display, screen_number), &atts) == 0) {	
		delete("XGetWindowAttributes() returned 0.");
		return;
	} else
		CHECK;

	if(dwin != atts.root) {
		report("The default root window ID was %lx instead of %lx.", (long) dwin, (long) atts.root);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
