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
>>TITLE XUninstallColormap CH07
void

Display	*display = Dsp;
Colormap	colormap;
>>ASSERTION Good B 3
A call to xname removes the specified colourmap from the required
list for its associated screen.
>>ASSERTION Good B 1
When the specified colourmap is uninstalled by a call to xname, then a
.S ColormapNotify
event is generated on each window that has that colourmap.
>>STRATEGY
Create colour map.
Install colour map.
Create window with the colour map.
Call xname to uninstall the colourmap.
If there is a ColormapNotify event
  Verify that the fields are correct.
else
  UNTESTED.
>>CODE
XVisualInfo	*vp;
Window	win;
XEvent	ev;
XColormapEvent	good;
XColormapEvent	*cmp;

	defsetevent(good, display, ColormapNotify);
	good.new = False;
	good.state = ColormapUninstalled;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {

		win = makewin(display, vp);
		colormap = makecolmap(display, vp->visual, AllocNone);
		XSetWindowColormap(display, win, colormap);
		XSelectInput(display, win, ColormapChangeMask);
		XInstallColormap(display, colormap);

		XCALL;

		while (XCheckWindowEvent(display, win, ColormapChangeMask, &ev)) {
			cmp = (XColormapEvent*)&ev;
			if (cmp->colormap != colormap)
				continue;

			good.window = win;
			good.colormap = colormap;
			if (checkevent((XEvent*)&good, &ev))
				FAIL;
			else
				CHECK;

		}
	}

	if (fail == 0 && pass == nvinf())
		PASS;
	else if (fail == 0 && pass == 0)
		untested("The assertion could not be tested since no event was sent");
	else if (fail == 0)
		untested("The assertion could only be tested for some visual types");

>>ASSERTION Good B 1
When another colourmap is installed or uninstalled as a
side effect of a call to xname, then a
.S ColormapNotify
event is generated on each window that has that colourmap.
>>ASSERTION Good B 3
No other colourmaps are removed from the required list by a call to xname.
>>ASSERTION Bad A
.ER BadColor
