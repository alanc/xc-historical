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
>>### Match gc-drawable-screen
When the graphics context and the drawable were not created for the same
root, then a
.S BadMatch
error occurs.
>>STRATEGY
If multiple screens are supported
  Create pixmap of depth 1.
  Create gc on alternate screen.
  Call test function with this pixmap and gc.
  Verify that a BadMatch error occurs.
else
  report UNSUPPORTED
>>CODE BadMatch
XVisualInfo	vi;
Pixmap	errpm;
int 	scr_num;

        if (config.alt_screen == -1) {
                unsupported("No alternate root supported");
                return;
        }

	scr_num = config.alt_screen;
	if (scr_num == DefaultScreen(A_DISPLAY)) {
		delete("The alternate root was the same as the one under test");
		return;
	}
	if (scr_num >= ScreenCount(A_DISPLAY)) {
		delete("The alternate root could not be accessed");
		return;
	}

	vi.visual = NULL;
	vi.screen = DefaultScreen(A_DISPLAY);	/* XXX */
	vi.depth = 1;
	A_DRAWABLE = makepixm(A_DISPLAY, &vi);
#ifdef A_DRAWABLE2
	A_DRAWABLE2 = makepixm(A_DISPLAY, &vi);
#endif
#ifdef A_IMAGE
	A_IMAGE = makeimg(A_DISPLAY, &vi, ZPixmap);
	dsetimg(A_IMAGE, W_FG);
#endif

	/*
	 * Create a 1x1 depth 1 pixmap on other screen
	 * and use it to create a gc
	 */
	errpm = XCreatePixmap(A_DISPLAY, RootWindow(A_DISPLAY, scr_num), 1, 1, 1);
	A_GC = makegc(A_DISPLAY, errpm);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

	XFreePixmap(A_DISPLAY, errpm);
