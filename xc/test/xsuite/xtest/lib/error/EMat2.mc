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
>>### Match gc-drawable-depth
When the graphics context and the drawable
do not have the same depth, then a
.S BadMatch
error occurs.
>>STRATEGY
If only one depth supported
  report UNSUPPORTED
Create pixmap of depth 1.
Create gc of different depth.
Call test function with this pixmap and gc.
Verify that a BadMatch error occurs.
>>CODE BadMatch
XVisualInfo	*vp;
Drawable	errpm;
int 	founddepth = 0;

	for (resetvinf(VI_PIX); nextvinf(&vp); ) {
		if (vp->depth != 1) {
			founddepth++;

			errpm = makewin(A_DISPLAY, vp);
			A_GC = makegc(A_DISPLAY, errpm);
#ifdef A_IMAGE
			A_IMAGE = makeimg(A_DISPLAY, vp, ZPixmap);
			dsetimg(A_IMAGE, W_FG);
#endif

			break;
		}
	}

	if (!founddepth) {
		report("Only one depth supported");
		tet_result(TET_UNSUPPORTED);
		return;
	}

	vp->depth = 1;
	A_DRAWABLE = makewin(A_DISPLAY, vp);
#ifdef A_DRAWABLE2
	A_DRAWABLE2 = makewin(A_DISPLAY, vp);
#endif

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
