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
>>EXTERN
#define	PLANEFG	0x55555555
static	int 	planelist[] = {
	0, 0x1, 0x2, 0x23, 0xf8, 0x765, 0x3987, 0x129078, 0x23567193};
>>ASSERTION Good A
The value for
.M plane_mask
is truncated to the depth of the GC.
>>STRATEGY
Set foreground to pattern of 1's and 0's
Set plane-mask to values larger than the depth.
Verify that result is what would be expected if the extra bits
 were masked off.
>>CODE
XVisualInfo	*vp;
int 	depthmask;
int 	*ip;
unsigned long	pix;
unsigned long	expected;
int 	pmx = -1, pmy;
int 	ntested;

	ntested = 0;
	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
#ifdef A_DRAWABLE2
		winpair(A_DISPLAY, vp, &A_DRAWABLE, &A_DRAWABLE2);
#if T_XCopyPlane
		dset(A_DISPLAY, A_DRAWABLE, ~0);
#else
		dset(A_DISPLAY, A_DRAWABLE, W_FG);
#endif
#else
		A_DRAW = makewin(A_DISPLAY, vp);
#endif
		A_GC = makegc(A_DISPLAY, A_DRAW);
#ifdef A_IMAGE
		A_IMAGE = makeimg(A_DISPLAY, vp, ZPixmap);
		dsetimg(A_IMAGE, W_FG);
#endif

		if (pmx == -1) {
			/*
			 * Get a point that is set in the drawable.
			 */
			XCALL;
			setfuncpixel(A_DISPLAY, A_DRAW, &pmx, &pmy);
			dclear(A_DISPLAY, A_DRAW);
		}

		depthmask = DEPTHMASK(vp->depth);

#ifdef A_IMAGE
		/* ZPixmap image so fg not used */
		dsetimg(A_IMAGE, PLANEFG);
#else
#if T_XCopyArea
		dset(A_DISPLAY, A_DRAWABLE, PLANEFG);
#else
		XSetForeground(A_DISPLAY, A_GC, PLANEFG);
#endif
#endif

		for (ip = planelist; ip < &planelist[NELEM(planelist)]; ip++) {
			if (*ip <= depthmask)
				continue;

			trace("plane-mask 0x%x", *ip);
			XSetPlaneMask(A_DISPLAY, A_GC, *ip);
			ntested++;

			dclear(A_DISPLAY, A_DRAW);
			XCALL;

			expected = (*ip & PLANEFG) & depthmask;
			pix = getpixel(A_DISPLAY, A_DRAW, pmx, pmy);

			if (pix == expected)
				CHECK;
			else {
				report("got pixel 0x%x, expecting 0x%x", pix, expected);
				FAIL;
			}
		}

	}

	CHECKPASS(ntested);

>>ASSERTION Good A
The
.M plane_mask
specifies which planes of the
destination are to be modified, one bit per plane, with bits being
assigned to planes from the least significant bit of the word
to the most significant bit.
>>STRATEGY
Set foreground to pattern of 1's and 0's
Select a variety of plane masks.
Verify that the expected pixels are drawn.
>>CODE
XVisualInfo	*vp;
int 	depthmask;
int 	*ip;
unsigned long	pix;
unsigned long	expected;
int 	pmx = -1, pmy;
int 	ntested = 0;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
#ifdef A_DRAWABLE2
		winpair(A_DISPLAY, vp, &A_DRAWABLE, &A_DRAWABLE2);
#if T_XCopyPlane
		dset(A_DISPLAY, A_DRAWABLE, ~0);
#else
		dset(A_DISPLAY, A_DRAWABLE, W_FG);
#endif
#else
		A_DRAW = makewin(A_DISPLAY, vp);
#endif
		A_GC = makegc(A_DISPLAY, A_DRAW);
#ifdef A_IMAGE
		A_IMAGE = makeimg(A_DISPLAY, vp, ZPixmap);
		dsetimg(A_IMAGE, W_FG);
#endif

		if (pmx == -1) {
			/*
			 * Get a point that is set in the drawable.
			 */
			XCALL;
			setfuncpixel(A_DISPLAY, A_DRAW, &pmx, &pmy);
			dclear(A_DISPLAY, A_DRAW);
		}

		depthmask = DEPTHMASK(vp->depth);

#ifdef A_IMAGE
		/* ZPixmap image so fg not used */
		dsetimg(A_IMAGE, PLANEFG);
#else
#if T_XCopyArea
		dset(A_DISPLAY, A_DRAWABLE, PLANEFG);
#else
		XSetForeground(A_DISPLAY, A_GC, PLANEFG);
#endif
#endif
		for (ip = planelist; ip < &planelist[NELEM(planelist)]; ip++) {
			if (*ip > depthmask)
				break;

			trace("plane-mask 0x%x", *ip);
			XSetPlaneMask(A_DISPLAY, A_GC, *ip);
			ntested++;

			dclear(A_DISPLAY, A_DRAW);
			XCALL;

			expected = *ip & PLANEFG;
			pix = getpixel(A_DISPLAY, A_DRAW, pmx, pmy);

			if (pix == expected)
				CHECK;
			else {
				report("got pixel 0x%x, expecting 0x%x", pix, expected);
				FAIL;
			}
		}

	}

	CHECKPASS(ntested);

