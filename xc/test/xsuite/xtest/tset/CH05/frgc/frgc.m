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
>>TITLE XFreeGC CH05
void
XFreeGC(display, gc)
Display *display = Dsp;
GC gc;
>>ASSERTION Good A
A call to xname destroys the GC specified by the
.A gc
argument, and all associated storage.
>>STRATEGY
Create a GC with XCreateGC.
Free the GC with XFreeGC.
Set the clip mask of the gc with XSetClipMask.
Verify that a BadGC error occurred.
>>CODE

	gc = XCreateGC(display, DRW(display), 0L, (XGCValues*)0);
	XCALL;

	startcall(Dsp);

	XSetClipMask(display, gc, None);
	
	endcall(Dsp);

	if(geterr() != BadGC) {
		report("After a successful call to XFreeGC, a call to XSetClipMask");
		report("did not give a BadGC error but instead gave %s", errorname(geterr()));
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Good A
.ER BadGC
>>#HISTORY	Cal 	Completed	Written in new format and style 7/12/90.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writing code.
