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
#ifdef A_WINDOW2
#include "window2 in subwindow-mode"
>>#CHANGED	peterc
>>#>>ASSERTION Good A
>>#When
>>#.M subwindow_mode
>>#is
>>#.S ClipByChildren , 
>>#then both source and destination windows are
>>#clipped by all viewable 
>>#.S InputOutput
>>#children.
>>ASSERTION Good A
When
.M subwindow_mode
is
.S ClipByChildren , 
then the destination window and source
(if it is a window)
are clipped by all viewable 
.S InputOutput
children.
>>STRATEGY
Set subwindow-mode to be ClipByChildren
Cover window with many strip sub-windows.
Do drawing operation
Verify that all subwindows are clear.
>>CODE
XVisualInfo	*vp;
Window	subwins[10];
unsigned int 	swmwidth, swmheight;
struct	area	area;
int 	i;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
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

		XSetSubwindowMode(A_DISPLAY, A_GC, ClipByChildren);

		/*
		 * Place subwindows in strips across the window
		 * leaving spaces between them.
		 */
		getsize(A_DISPLAY, A_DRAW, &swmwidth, &swmheight);
		swmwidth /= 2*NELEM(subwins);

		for (i = 0; i < NELEM(subwins); i++) {
			setarea(&area, 2*i*swmwidth, 0, swmwidth, 0);
			subwins[i] = crechild(A_DISPLAY, A_DRAW, &area);
		}

		XCALL;

		for (i = 0; i < NELEM(subwins); i++) {
			if (checkclear(A_DISPLAY, subwins[i]))
				CHECK;
			else {
				report("Area of subwindow was not clear");
				FAIL;
			}
		}
	}

	CHECKPASS(NELEM(subwins)*nvinf());

>>#CHANGED	peterc
>>#>>ASSERTION Good A
>>#When
>>#.M subwindow_mode
>>#is
>>#.S IncludeInferiors ,
>>#then neither source nor destination window is clipped by inferiors.
>>ASSERTION Good A
When
.M subwindow_mode
is
.S IncludeInferiors ,
then neither the destination window nor source
(if it is a window) is clipped by inferiors.
>>#(this will result in including subwindow contents in the source
>>#and drawing through subwindow boundaries of the destination).
>>STRATEGY
Place subwindows over parts of the main window
Create subwindows over these windows as well.
Do graphics operation.
Verify that the operation was not affected.
>>CODE
XVisualInfo	*vp;
Window	subwins[5];
XImage	*swmsav;
unsigned int 	swmwidth, swmheight;
struct	area	area;
int 	i, j;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
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

		XCALL;
		swmsav = savimage(A_DISPLAY, A_DRAW);
		dclear(A_DISPLAY, A_DRAW);

		XSetSubwindowMode(A_DISPLAY, A_GC, IncludeInferiors);

		/*
		 * Place subwindows in strips across the window
		 * leaving spaces between them.
		 */
		getsize(A_DISPLAY, A_DRAW, &swmwidth, &swmheight);
		swmwidth /= 2*NELEM(subwins);

		for (i = 0; i < NELEM(subwins); i++) {
			setarea(&area, 2*i*swmwidth, 0, swmwidth, 0);
			subwins[i] = crechild(A_DISPLAY, A_DRAW, &area);
			for (j = 0; j < swmheight; j += 10) {
				setarea(&area, 0, j, swmwidth, 6);
				(void) crechild(A_DISPLAY, subwins[i], &area);
			}
		}

		XCALL;

		if (compsavimage(A_DISPLAY, A_DRAW, swmsav))
			CHECK;
		else {
			report("Inferiors affected result with IncludeInferiors");
			FAIL;
		}

		/*
		 * As a special test due to its importance, test that drawing
		 * on to the root window has the same effect.  Move the test
		 * window to the root window origin.
		 */
		{
		Drawable savdraw;

			dclear(A_DISPLAY, A_DRAW);
			XSetWindowBorderWidth(A_DISPLAY, A_DRAW, 0);
			XMoveWindow(A_DISPLAY, A_DRAW, 0, 0);

			savdraw = A_DRAW;
			A_DRAW = DRW(A_DISPLAY);

			XCALL;

			A_DRAW = savdraw;

			if (compsavimage(A_DISPLAY, A_DRAW, swmsav))
				CHECK;
			else {
				report("Drawing on root window with IncludeInferiors gave incorrect results");
				FAIL;
			}
		}
	}

	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When
.M subwindow_mode
is
.S IncludeInferiors 
on a window of one depth with mapped
inferiors of differing depth, then no errors occur.
>>STRATEGY
If only one depth window supported
  Result is UNSUPPORTED

Find two visuals with different depths.
Use one of these visuals to create a window.
Use the other to create a subwindow.
Verify that no errors occur.
For information purposes only see if graphics operation is rendered properly.
>>CODE
XVisualInfo	*vp1, *vp2;
XImage	*swmsav;
int 	found;

	vp2 = NULL;
	found = 0;

	for (resetvinf(VI_WIN); nextvinf(&vp1); ) {
		if (vp2 == NULL)
			vp2 = vp1;
		if (vp1->depth != vp2->depth) {
			found++;
			break;
		}
	}

	if (!found) {
		report("Only one depth visual supported");
		tet_result(TET_UNSUPPORTED);
		return;
	}

	/*
	 * The remainder has not been fully tested - 
	 * it requires support for visuals at more than one depth.
	 */

#ifdef A_DRAWABLE2
	winpair(A_DISPLAY, vp1, &A_DRAWABLE, &A_DRAWABLE2);
#if T_XCopyPlane
	dset(A_DISPLAY, A_DRAWABLE, ~0L);
#else
	dset(A_DISPLAY, A_DRAWABLE, W_FG);
#endif
#else
	A_DRAW = makewin(A_DISPLAY, vp1);
#endif
	A_GC = makegc(A_DISPLAY, A_DRAW);
#ifdef A_IMAGE
	A_IMAGE = makeimg(A_DISPLAY, vp1, ZPixmap);
	dsetimg(A_IMAGE, W_FG);
#endif

	XCALL;
	swmsav = savimage(A_DISPLAY, A_DRAW);
	dclear(A_DISPLAY, A_DRAW);

	XCreateWindow(A_DISPLAY
		, A_DRAW
		, 0
		, 0
		, 300
		, 300
		, 0
		, vp2->depth
		, InputOutput
		, vp2->visual
		, 0
		, (XSetWindowAttributes*)0
		);
	XSetWindowBackground(A_DISPLAY, A_DRAW, W_BG);
	XMapWindow(A_DISPLAY, A_DRAW);

	/* Wait for Expose? */
	/* No.... we're not a top level window so no wmgr interference */
		
	XSetSubwindowMode(A_DISPLAY, A_GC, IncludeInferiors);

	XCALL;

	if (compsavimage(A_DISPLAY, A_DRAW, swmsav))
		trace("Drawing through to different depth window produces same results");
	else
		trace("Drawing through to different depth window produces different results");
		
	if (geterr() == Success)
		PASS;
	else
		FAIL;
>>#HISTORY Completed	peterc Incorporated RTCB3.
