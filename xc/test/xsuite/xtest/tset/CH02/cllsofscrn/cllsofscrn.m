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
>>TITLE XCellsOfScreen CH02
int
XCellsOfScreen(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>ASSERTION Good A
A call to xname returns the number of colourmap cells in the default
colourmap of the screen
.A screen .
>>STRATEGY
Obtain the default visual using XDefaultVisual.
Obtain the XVisualInfo structure corresponding to the DefaultVisual using XGetVisualInfo.
Obtain the number of colourmap entries in the default visual from the structure.
Obtain the number of colourmap entries in the default colourmap using xname.
Verify that the number of cells is the same.
>>CODE
Visual		*dv;
int		nv;
int		dnc;
XVisualInfo	vit;
XVisualInfo	*vil;

	dv = XDefaultVisual(Dsp, DefaultScreen(Dsp));
	vit.visualid = XVisualIDFromVisual(dv);

	if((vil = XGetVisualInfo(Dsp, VisualIDMask, &vit, &nv)) == (XVisualInfo *) NULL) {
		delete("XGetVisualInfo() returned NULL.");
		return;
	} else
		CHECK;

	dnc = XCALL;

	if(dnc != vil->colormap_size) {
		report("%s() returned %d colourmap cells instead of %d.", TestName, dnc, vil->colormap_size);
		FAIL;
	} else
		CHECK;

	XFree((char *) vil);
	CHECKPASS(2);
