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
>>### Drawable
When a drawable argument does not name a valid xerrlist, then a
.S BadDrawable
error occurs.
>>STRATEGY
Create a bad drawable by creating and destroying a window.
Call test function using bad drawable as the drawable argument.
Verify that a BadDrawable error occurs.
>>CODE BadDrawable
#ifdef A_IMAGE
	XVisualInfo	*vp;
#endif

	seterrdef();

#ifdef A_IMAGE
	resetvinf(VI_WIN_PIX); nextvinf(&vp);
	A_IMAGE = makeimg(A_DISPLAY, vp, ZPixmap);
	dsetimg(A_IMAGE, W_FG);
#endif
	A_DRAWABLE = (Drawable)badwin(A_DISPLAY);
#ifdef A_DRAWABLE2
	A_DRAWABLE2 = A_DRAWABLE;
#endif

	XCALL;

	if (geterr() == BadDrawable)
		PASS;
	else
		FAIL;
