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
>>### Match inputonly
When a drawable argument is an
.S InputOnly
window then a
.S BadMatch
error occurs.
>>STRATEGY
Create an InputOnly window.
Call test function using InputOnly window as the drawable argument.
Verify that a BadMatch error occurs.
>>CODE BadMatch
#ifdef A_IMAGE
	XVisualInfo	*vp;
#endif

	seterrdef();

#ifdef A_IMAGE
	resetvinf(VI_WIN_PIX); nextvinf(&vp);
	A_IMAGE = makeimg(A_DISPLAY, vp, ZPixmap);
	dsetimg(A_IMAGE, W_FG);
#endif

	A_DRAWABLE = (Drawable)iponlywin(A_DISPLAY);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

#ifdef A_DRAWABLE2
	seterrdef();
	A_DRAWABLE2 = (Drawable)iponlywin(A_DISPLAY);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
#endif
