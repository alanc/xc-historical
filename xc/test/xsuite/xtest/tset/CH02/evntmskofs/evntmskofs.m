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
>>TITLE XEventMaskOfScreen CH02
long
XEventMaskOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the event mask of the root window of the screen
.A screen
at the time the connection was established.
>>STRATEGY
Obtain the all_events_mask of the root window using XGetWindowAttributes.
Obtain the event mask of the root window using xname.
Verify that the masks are the same.
>>CODE
XWindowAttributes	atts;
long			em;

	if( XGetWindowAttributes(Dsp, XRootWindowOfScreen(screen), &atts) == 0) {
		delete("XGetWindowAttributes() returned zero.");
		return;
	} else
		CHECK;

	em = XCALL;	

	if( em != atts.all_event_masks) {
		report("%s() returned 0x%lx instead of 0x%lx.", TestName, em, atts.all_event_masks);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
