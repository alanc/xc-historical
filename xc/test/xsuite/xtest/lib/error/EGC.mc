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
>>ASSERTION Bad A
>>### GC
When the GC argument does not name a defined GC, then a
.S BadGC
error occurs.
>>STRATEGY
Create a GC.
Set GC id to -1 using XTestSetGContextOfGC.
Call test function using bad GC as the GC argument.
Verify that a BadGC error occurs.
>>CODE BadGC
#ifdef A_IMAGE
	XVisualInfo	*vp;
#endif

	seterrdef();

#ifdef A_IMAGE
	resetvinf(VI_WIN_PIX); nextvinf(&vp);
	A_IMAGE = makeimg(A_DISPLAY, vp, ZPixmap);
	dsetimg(A_IMAGE, W_FG);
#endif

	A_GC = badgc(A_DISPLAY);

	XCALL;

	if (geterr() == BadGC)
		PASS;
	else
		FAIL;
