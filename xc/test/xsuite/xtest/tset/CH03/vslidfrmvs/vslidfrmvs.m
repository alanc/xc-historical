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
>>TITLE XVisualIDFromVisual CH03
VisualID

Visual	*visual;
>>ASSERTION Good A
A call to xname returns the visual ID for the specified visual.
>>STRATEGY
Call xname
Do simple checks on returned id.
>>CODE
VisualID	vid;
XVisualInfo	*vp;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		visual = vp->visual;

		vid = XCALL;
		trace("Returned visual id was %ld", (long)vid);

		/*
		 * VisualID is a 32 bit value with the top three bits 0
		 */
		if (vid & 0xe0000000) {
			report("Some of top three bits of VisualID were set");
			report("value was 0x%lx", vid);
			FAIL;
		} else {
			CHECK;
		}
	}
	CHECKPASS(nvinf());
