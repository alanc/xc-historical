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
>>TITLE XGetTransientForHint CH09
Status
XGetTransientForHint(display, w, prop_window_return)
Display	*display = Dsp;
Window	w = DRW(Dsp);
Window	*prop_window_return = &propwindow;
>>EXTERN
#include "Xatom.h"
Window	propwindow;
>>ASSERTION Good A
When the WM_TRANSIENT_FOR property is set for the window
.A w ,
and is format 32, length 1 element and of type
.S WINDOW ,
then a call to xname returns the property value in the
.A prop_window_return
argument and returns non-zero.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_TRANSIENT_FOR property using XSetTransientForHint.
Obtain the value of the WM_TRANSIENT_FOR property using XGetTransientForHint.
Verify that the call did not return False.
Verify that the value was correct.
>>CODE
Status		status;
Window		win, twin;
Window		rwin = (Window) -1;
XVisualInfo	*vp;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);
	twin = makewin(display, vp);

	XSetTransientForHint(display, win, twin);

	w = win;
	prop_window_return = &rwin;
	status = XCALL;

	if(status == False) {
		report("XGetTransientForHint() returned False.");
		FAIL;
	} else
		CHECK;

	if(rwin != twin) {
		report("The value of the WM_TRANSIENT_FOR property was %lx instead of %lx.", (long) rwin, (long) twin);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
When the WM_TRANSIENT_FOR property is not set for the window
.A w ,
or is format other than 32, length < 1 element or of type other than
.S WINDOW ,
then a call to xname returns zero. 
>>STRATEGY
Create a window with XCreateWindow.
Obtain the value of the unset WM_TRANSIENT_FOR property with XGetWMHints.
Verify that the call returned False.

Create a window with XCreateWindow.
Set the WM_TRANSIENT_FOR property with format 16 and type WINDOW and size 1 using XChangeProperty.
Obtain the value of the WM_TRANSIENT_FOR property with XGetWMHints.
Verify that the call returned False.

Create a window with XCreateWindow.
Set the WM_TRANSIENT_FOR property with format 32 type STRING and size 1 using XChangeProperty.
Obtain the value of the WM_TRANSIENT_FOR property with XGetWMHints.
Verify that the call returned False.

Create a window with XCreateWindow.
Set the WM_TRANSIENT_FOR property with format 32 type WINDOW and size 0 using XChangeProperty.
Obtain the value of the WM_TRANSIENT_FOR property with XGetWMHints.
Verify that the call returned False.
>>CODE
Status		status;
XVisualInfo	*vp;
Window		pwin;

	resetvinf(VI_WIN);
	nextvinf(&vp);

	prop_window_return = &pwin;
	pwin = makewin(display, vp);	
	w = makewin(display, vp);

/* Property unset */

	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the WM_TRANSIENT_FOR property was not set.", TestName);
		FAIL;
	} else
		CHECK;

	w = makewin(display, vp);

/* Format 16 */
	XChangeProperty(display, w, XA_WM_TRANSIENT_FOR, XA_WINDOW, 16, PropModeReplace, (unsigned char *) &pwin, 1);

	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the WM_TRANSIENT_FOR property format was 16.", TestName);
		FAIL;
	} else
		CHECK;

	w = makewin(display, vp);

/*  Type STRING */
	XChangeProperty(display, w, XA_WM_TRANSIENT_FOR, XA_STRING, 32, PropModeReplace, (unsigned char *) &pwin, 1);

	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the WM_TRANSIENT_FOR property type was STRING.", TestName);
		FAIL;
	} else
		CHECK;

	w = makewin(display, vp);

/* No elements = 0 */
	XChangeProperty(display, w, XA_WM_TRANSIENT_FOR, XA_WINDOW, 32, PropModeReplace, (unsigned char *) &pwin, 0);

	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the WM_TRANSIENT_FOR property size  was 0.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(4);


>>ASSERTION Bad A
.ER BadWindow
>># Kieron	Completed	Review
