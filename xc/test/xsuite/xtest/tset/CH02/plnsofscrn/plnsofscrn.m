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
>>TITLE XPlanesOfScreen CH02
int
XPlanesOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the depth of the root window of the screen
.A screen .
>>STRATEGY
Obtain the depth of the root window of the default screen using xname.
Obtain the depth of the root window of the default screen using XGetWindowAttributes.
Verify that the depths are the same.
>>CODE
int			plns;
XWindowAttributes	atts;

	plns = XCALL;
	if(XGetWindowAttributes(Dsp, RootWindowOfScreen(screen), &atts) == 0) {
		delete("XGetWindowAttributes() returned zero.");
		return;
	} else
		CHECK;

	if(atts.depth != plns) {
		report("Depth of the default root window was %d instead of %d.", atts.depth, plns);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
