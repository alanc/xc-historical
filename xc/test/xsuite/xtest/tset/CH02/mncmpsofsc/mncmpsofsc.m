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
>>SET   macro
>>TITLE XMinCmapsOfScreen CH02
int
XMinCmapsOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good B 1
A call to xname returns the minimum number of installed colourmaps supported by
the screen
.A screen .
>>STRATEGY
Obtain the minimum allowed number of installed colourmaps using xname.
Obtain the list of installed colourmaps on the default screen using XListInstalledColormaps.
Verify that the number of installed colourmaps is greater or equal to the minimum.
>>CODE
int		n;
int		minm;
Colormap	*clist;

	if((clist = XListInstalledColormaps(Dsp, DefaultRootWindow(Dsp), &n)) == (Colormap *) NULL) {
		delete("XListInstalledColormaps() returned NULL.");
		return;
	} else
		CHECK;

	XFree((char *) clist);

	minm = XCALL;

	if(n < minm) {
		report("%s() returns %d, but %d colourmaps are installed.", TestName, minm, n);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(2);
