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
>>TITLE XMaxCmapsOfScreen CH02
int
XMaxCmapsOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the maximum number of installed colourmaps supported by the screen
.A screen .
>>STRATEGY
Obtain the maximum allowed number of installed colourmaps using xname.
Install the maximum+2 colourmaps using XInstallColormap.
Obtain the number of installed colourmaps using XListInstalledColormaps.
Verify that the maximum was not exceeded.
>>CODE
int		maxm;
int		minm;
int	 	i;
int		n;
Colormap	cmap;
Colormap	*clist;

	maxm = XCALL;

	for(i=1; i< maxm+2 ;i++) {
		if(i == maxm)
			CHECK;

		cmap = makecolmap(Dsp, DefaultVisual(Dsp, DefaultScreen(Dsp)), AllocNone);
		XInstallColormap(Dsp, cmap);
	}

	if((clist=XListInstalledColormaps(Dsp, RootWindowOfScreen(screen), &n)) == (Colormap *) NULL) {
		delete("XListInstalledColormaps() returned NULL.");
		return;
	} else
		CHECK;

	XFree((char *) clist);

	if(n != maxm) {
		report("%s() returns %d, but %d colourmaps can be installed.", TestName, maxm, n);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
