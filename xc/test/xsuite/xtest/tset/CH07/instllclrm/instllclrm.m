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
>>TITLE XInstallColormap CH07
void
xname
Display	*display = Dsp;
Colormap	colormap;
>>EXTERN

static	Colormap	*savcm;
static	int 	nsavcm;

/*
 * savcm and restorecm attempt to restore the colormaps that were
 * installed originally.  However it is possible that different
 * colour maps will end up on the required list than before.
 */
savecm()
{
	savcm = XListInstalledColormaps(display, DRW(display), &nsavcm);
}

restorecm()
{
int 	i;

	for (i = 0; i < nsavcm; i++) {
		debug(1, "Re-installing colour map 0x%x", savcm[i]);
		XInstallColormap(display, savcm[i]);
	}
	if (nsavcm && savcm)
		XFree((char*)savcm);
}

>>ASSERTION Good A
A call to xname installs the specified colourmap for its associated screen.
>>STRATEGY
For each visual type.
  Create colourmap.
  Install colourmap.
  Verify that new colourmap has been installed by listing the installed
  colourmaps.
>>CODE
XVisualInfo	*vp;
Colormap	*cmp;
int 	mode;
int 	n;
int 	i;

	savecm();

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		switch (vp->class) {
		case StaticGray:
		case StaticColor:
		case TrueColor:
			mode = AllocNone;
			break;
		default:
			mode = AllocAll;
			break;
		}
		colormap = makecolmap(display, vp->visual, mode);

		XCALL;

		cmp = XListInstalledColormaps(display, DRW(display), &n);
		for (i = 0; i < n; i++) {
			if (cmp[i] == colormap) {
				CHECK;
				break;
			}
		}
		if (i == n) {
			report("Colourmap was not installed");
			FAIL;
		}

		XFree((char*)cmp);
	}

	CHECKPASS(nvinf());

	restorecm();
>>ASSERTION Good B 3
All windows associated with the specified colourmap immediately display with
true colours.
>>ASSERTION Good B 1
A call to xname adds the
specified colourmap to the head of the required list of colourmaps for its
associated screen, and the required list is truncated to the minimum
number of installed colourmaps for the screen.
>>ASSERTION Good A
When the specified colourmap is not already an installed colourmap, then a
.S ColormapNotify
event is generated on each window that has that colourmap. 
>>STRATEGY
For each visual.
  Create windows.
  Create colormap for those windows.
  Set the window colour maps.
  Create another window without using the new colour map.
  Install the colour map.
  Verify that a colour map notify event is received on the windows that have
  that colour map.
  Verify that the event is not received on the other window.
>>CODE
XVisualInfo	*vp;
Window	base;
Window	w1, w2;
Window	w3nocm;
XEvent	ev;
XColormapEvent	good;
XColormapEvent	*cmp;
int 	got;
#define	GOT1	1
#define GOT2	2

	defsetevent(good, display, ColormapNotify);
	good.new = False;
	good.state = ColormapInstalled;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {

		base = makewin(display, vp);
		w1 = crechild(display, base, (struct area *)0);
		w2 = crechild(display, base, (struct area *)0);
		w3nocm = crechild(display, base, (struct area *)0);

		colormap = makecolmap(display, vp->visual, AllocNone);
		XSetWindowColormap(display, w1, colormap);
		XSelectInput(display, w1, ColormapChangeMask);
		XSetWindowColormap(display, w2, colormap);
		XSelectInput(display, w2, ColormapChangeMask);
		XSelectInput(display, w3nocm, ColormapChangeMask);

		XCALL;

		while (getevent(display, &ev)) {
			cmp = (XColormapEvent*)&ev;
			/*
			 * Server is allowed to install or uninstalled implementation
			 * defined colormaps implicitly, so we have to ignore
			 * all the ones that we don't know about.
			 */
			if (cmp->colormap != colormap)
				continue;
			if (cmp->window == w3nocm) {
				report("ColormapNotify event received for window that did not have that colourmap");
				FAIL;
			} else
				CHECK;

			if (cmp->window == w1)
				got |= GOT1;
			else if (cmp->window == w2)
				got |= GOT2;
			else {
				report("ColormapNotify received on unexpected window");
				FAIL;
			}

			good.window = cmp->window;
			good.colormap = colormap;
			if (checkevent((XEvent*)&good, &ev))
				FAIL;
			else
				CHECK;
		}
	}
	if (got & GOT1)
		CHECK;
	else {
		report("Event not received on window 'w1'");
		FAIL;
	}
	if (got & GOT2)
		CHECK;
	else {
		report("Event not received on window 'w2'");
		FAIL;
	}

	CHECKPASS(nvinf()*4+2);

>>ASSERTION Good B 1
When another colourmap is installed or uninstalled as 
a side effect of a call to xname, then a
.S ColormapNotify 
event is generated on each window that has that colourmap.
>>ASSERTION Bad A
.ER BadColor
