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
>>TITLE XGetWMColormapWindows CH09
Status
XGetWMColormapWindows(display, w, colormap_windows_return, count_return)
Display	*display = Dsp;
Window	w = DRW(display);
Window	**colormap_windows_return = &window;
int	*count_return = &count;
>>EXTERN
#include	"Xatom.h"
Window	*window;
int	count;
>>ASSERTION Good A
When the WM_COLORMAP_WINDOWS property is set on the window
.A w ,
the property is of
.M type
WINDOW, and is of
.M format
32, 
then a call to xname returns the list of window identifiers stored 
in the property, which can be freed with XFree, in the
.A colormap_windows_return
argument and the number of windows in the
.A count_return
argument and returns non-zero.
>>STRATEGY
Create a window using XCreateWindow.
Set the WM_COLORMAP_WINDOWS property using XSetWMColormapWindows.
Obtain the WM_COLORMAP_WINDOWS property using XGetWMColormapWindows.
Verify that the call did not return False.
Verify that the value of the property is correct.
Release the allocated memory using XFree.
>>CODE
Status		status;
Window		rwin;
XVisualInfo	*vp;
int		nwins = 7;
Window		pwins[7];
Window		*rwins = (Window *) NULL;
Window		*wp;
int		rcnt = 0;
int		i;

	for(i=0; i<nwins; i++)
		pwins[i] = (Window) i;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);
	
	XSetWMColormapWindows(display, w, pwins, nwins);

	colormap_windows_return = &rwins;
	count_return = &rcnt;
	status = XCALL;

	if(status == False) {
		report("%s() returned False.", TestName);
		FAIL;
	} else
		CHECK;

	if(rcnt != nwins) {
		report("The number of elements set for the WM_COLORMAP_WINDOWS property was %d instead of %d", rcnt, nwins);
		FAIL;
	} else
		CHECK;

	if(rwins == (Window *) NULL) {
		report("The WM_COLORMAP_WINDOWS property had no associated value.");
		FAIL;
	} else {

		CHECK;
		for(i = 0, wp = rwins; i < nwins; i++, wp++)
			if( *wp != pwins[i]) {
				report("Element %d of the WM_COLORMAP_WINDOWS value was %lu instead of %lu", i+1, (long) *wp, (long) pwins[i]);
				FAIL;
			} else
				CHECK;

		XFree((char*)rwins);
	}

	CHECKPASS(nwins + 3);

>>ASSERTION Bad B 1
When the atom name \(lqWM_COLORMAP_WINDOWS\(rq cannot be interned,
then a call to xname does not set the
.A colormap_windows_return
or
.A count_return
arguments and returns zero.
>>ASSERTION Good A
When the WM_COLORMAP_WINDOWS property is not set on the window
.A w ,
or is not of
.M type
WINDOW, or is not of
.M format
32, 
then a call to xname does not set the
.A colormap_windows_return
or
.A count_return
arguments and returns zero.
>>STRATEGY
Create a window with XCreateWindow.
Insure that the name "WM_COLOMAP_WINDOWS" is interned using XSetWMColormapWindows.
Obtain the WM_COLORMAP_WINDOWS atom using XInternAtom.

Create a window with XCreateWindow.
Initialise the colormap_windows_return and count_return arguments.
Obtain the value of the WM_COLORMAP_WINDOWS property with XGetWMColormapWindows.
Verify that the call returned False.
Verify that colormap_windows_return and count_return arguments were unchanged.

Create a window with XCreateWindow.
Set the WM_COLORMAP_WINDOWS property with format 8 and type WINDOW using XChangeProperty.
Initialise the colormap_windows_return and count_return arguments.
Obtain the value of the WM_COLORMAP_WINDOWS property with XGetWMColormapWindows.
Verify that the call returned False.
Verify that colormap_windows_return and count_return arguments were unchanged.

Create a window with XCreateWindow.
Initialise the colormap_windows_return and count_return arguments.
Set the WM_COLORMAP_WINDOWS property with format 32 type ATOM using XChangeProperty.
Obtain the value of the WM_COLORMAP_WINDOWS property with XGetWMColormapWindows.
Verify that the call returned False.
Verify that colormap_windows_return and count_return arguments were unchanged.

>>CODE
Status		status;
Atom		xa_wm_colormap_windows;
Atom		*rat;
int		rcnt;
XVisualInfo	*vp;
Window		wdw;

	resetvinf(VI_WIN);
	nextvinf(&vp);

	colormap_windows_return = &rat;
	count_return = &rcnt;
	w = makewin(display, vp);
	
	if(XSetWMColormapWindows(display, w, &wdw, 1) == False) {
		delete("XSetWMColormapWindows() returned False.");
		return;
	} else
		CHECK;

	if( (xa_wm_colormap_windows = XInternAtom(display, "WM_COLORMAP_WINDOWS", True)) == None) {
		delete("The \"WM_COLORMAP_WINDOWS\" string was not interned.");
		return;
	} else
		CHECK;

	w = makewin(display, vp);

/* property unset */

	rat = (Atom *) -1;
	rcnt = -1;
	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the WM_COLORMAP_WINDOWS property was not set.", TestName);
		FAIL;
	} else
		CHECK;

	if( rat != (Atom *) -1) {
		report("Atom list pointer variable was updated when the WM_COLORMAP_WINDOWS property was not set.");
		FAIL;
	} else
		CHECK;

	if( rcnt != -1) {
		report("Atom count variable was updated when the WM_COLORMAP_WINDOWS property was not set.");
		FAIL;
	} else
		CHECK;

	w = makewin(display, vp);

/* format 8 */
 	XChangeProperty(display, w, xa_wm_colormap_windows, XA_WINDOW, 8, PropModeReplace, (unsigned char *) &wdw, 1);

	rat = (Atom *) -1;
	rcnt = -1;
	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the WM_COLORMAP_WINDOWS property had format 8.", TestName);
		FAIL;
	} else
		CHECK;

	if( rat != (Atom *) -1) {
		report("Atom list pointer variable was updated when the WM_COLORMAP_WINDOWS property had format 8.");
		FAIL;
	} else
		CHECK;

	if( rcnt != -1) {
		report("Atom count variable was updated when the WM_COLORMAP_WINDOWS property had format 8.");
		FAIL;
	} else
		CHECK;

	w = makewin(display, vp);

/* type  ATOM */
 	XChangeProperty(display, w, xa_wm_colormap_windows, XA_ATOM, 32, PropModeReplace, (unsigned char *) &wdw, 1);

	rat = (Atom *) -1;
	rcnt = -1;
	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the WM_COLORMAP_WINDOWS property had type ATOM.", TestName);
		FAIL;
	} else
		CHECK;

	if( rat != (Atom *) -1) {
		report("Atom list pointer variable was updated when the WM_COLORMAP_WINDOWS property had type ATOM.");
		FAIL;
	} else
		CHECK;

	if( rcnt != -1) {
		report("Atom count variable was updated when the WM_COLORMAP_WINDOWS property had type ATOM.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(11);

>>ASSERTION Bad A
.ER BadWindow 
>># Kieron	Action	Review
