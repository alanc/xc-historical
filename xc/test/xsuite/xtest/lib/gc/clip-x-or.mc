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

>># This file does both the x and y clip-mask origin components.
>>#
>>ASSERTION Good A
The clip origin coordinates
.M clip-x-origin
and
.M clip-y-origin
are interpreted relative to the
origin of the destination drawable specified in the graphics
operation.
>>STRATEGY
Create Pixmap and set clip-mask with it.
Vary clip origin
Verify nothing is drawn outside the clip_mask based on the origin.
Pixmap verify results inside the cliparea.
>>CODE
XVisualInfo	*vp;
Pixmap	cmopixmap;
struct	area	area;
unsigned int 	cmowidth;
unsigned int 	cmoheight;
int 	divsize;
int 	i, j;

	divsize = 3;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
#ifdef A_DRAWABLE2
		winpair(A_DISPLAY, vp, &A_DRAWABLE, &A_DRAWABLE2);
#if T_XCopyPlane
		dset(A_DISPLAY, A_DRAWABLE, ~0L);
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

		/*
		 * Get size of the pixmap.  It is divsize smaller on all sides
		 * than the window.
		 */
		getsize(A_DISPLAY, A_DRAW, &cmowidth, &cmoheight);
		cmowidth /= divsize;
		cmoheight /= divsize;
		debug(2, "clip-mask height=%d, width=%d", cmowidth, cmoheight);

		/*
		 * Create a pixmap that is about divsize^2 of the area of
		 * the window.
		 */
		cmopixmap = XCreatePixmap(A_DISPLAY, A_DRAW, cmowidth, cmoheight, 1);
		dset(A_DISPLAY, cmopixmap, 1L);

		XSetClipMask(A_DISPLAY, A_GC, cmopixmap);
		XFreePixmap(A_DISPLAY, cmopixmap);

		for (i = 0; i < divsize; i++) {
			for (j = 0; j < divsize; j++) {
				setarea(&area, i*cmowidth, j*cmoheight, cmowidth, cmoheight);
				debug(2, "Origin at (%d,%d)", area.x, area.y);
				XSetClipOrigin(A_DISPLAY, A_GC, area.x, area.y);
				XCALL;
				if (checkarea(A_DISPLAY,A_DRAW,&area,W_BG,W_BG,CHECK_OUT))
					CHECK;
				else {
					report("Drawing occurred outside clip_mask");
					FAIL;
				}
				PIXCHECK(A_DISPLAY, A_DRAW);
				dclear(A_DISPLAY, A_DRAW);
			}
		}
	}

	CHECKPASS(2*divsize*divsize*nvinf());

