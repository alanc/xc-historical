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
>>TITLE XDefaultVisual CH02
Visual *
XDefaultVisual(display, screen_number)
Display	*display = Dsp;
int	screen_number = DefaultScreen(Dsp);
>>ASSERTION Good A
A call to xname returns the 
>># comments subsequent to the review period removed the following words ...
>>#default 
visual for 
>># comments subsequent to the review period resulted in the extra words ...
the root window of
the screen specified by the
.A screen_number
argument.
>>STRATEGY
Obtain the attributes of the default root window using XGetWindowAttributes.
Obtain the default visual using xname.
Verify that the visual IDs are the same using XVisualIDFromVisual.
>>CODE
XWindowAttributes	atts;
Visual			*dv;
VisualID		dvid, rvid;

	if(XGetWindowAttributes(display, RootWindow(display, screen_number), &atts) == 0) {
		delete("XGetWindowAttributes() returned zero.");
		return;
	} else
		CHECK;

	dv = XCALL;	

	dvid = XVisualIDFromVisual(dv);
	rvid = XVisualIDFromVisual(atts.visual);

	if(dvid != rvid) {
		report("%s() returned a visual of ID %lx instead of %lx.", TestName, (long) dvid, (long) rvid);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
