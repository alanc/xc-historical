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
>>TITLE XFetchName CH09
Status
XFetchName(display, w, window_return_name)
Display	*display = Dsp;
Window	w = DRW(display);
char	**window_return_name = &winname;
>>EXTERN
#include	"Xatom.h"
char	*winname = "XtestJunkName";
>>ASSERTION Good A
When the WM_NAME property has been set for the
window specified by the
.A w
argument and has type
.S STRING
and format 8, then a call to xname returns in the
.A window_name_return
argument, which can be freed with XFree, the null-terminated
name of the window, and returns non-zero.
>>STRATEGY
Create a window with XCreateWindow
Set the name of the window to XtestWindowName with XStoreName
Obtain the name of the window with XFetchName
Verify that the returned name is the one that was set.
Release the allocated name using XFree.
>>CODE
Status		status;
Window		win;
XVisualInfo	*vp;
char		*wname = "XtestWindowName";
char		*wnameret;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	XStoreName(display, win, wname);

	w = win;
	window_return_name = &wnameret;
	status = XCALL;

	if(status == 0) {
		report("XFetchName() returned 0");
		FAIL;
	} else
		CHECK;

	if(strcmp(wname, (wnameret != NULL) ? wnameret : "NULL") != 0) {
		report("Window name was \"%s\" instead of \"%s\".", wnameret, wname);
		FAIL;
	} else
		CHECK;

	if (wnameret != NULL)
		XFree(wnameret);

	CHECKPASS(2);

>>ASSERTION Good A
When the WM_NAME property has not been set for the window specified by the
.A w
argument, or has format other than 8 or has type other than
.S STRING ,
then a call to xname
sets the 
.A window_name_return
argument to NULL, and returns zero.
>>STRATEGY
Create a window with XCreateWindow.
Obtain the value of the WM_NAME property with XFetchName.
Verify that the call returned zero.
Verify that the returned name was set to NULL.

Create a window with XCreateWindow.
Set the WM_NAME property with format 32 and type STRING using XChangeProperty.
Obtain the value of the WM_NAME property with XFetchName.
Verify that the call returned zero.
Verify that the returned name was set to NULL.

Create a window with XCreateWindow.
Set the WM_NAME property with format 8 type ATOM using XChangeProperty.
Obtain the value of the WM_NAME property with XFetchName.
Verify that the call returned zero.
Verify that the returned name was set to NULL.

>>CODE
Status		status;
Window		win;
XVisualInfo	*vp;
char		*name = "XTWindowName";
char		*wnameret;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

/* property unset */
	w = win;
	window_return_name = &wnameret;
	status = XCALL;


	if(status != 0) {
		report("XFetchName() did not return 0 when the WM_NAME property was not set.");
		FAIL;
	} else
		CHECK;

	if(wnameret != NULL) {
		report("Window name was \"%s\" instead of NULL when the WM_NAME property was not set.", wnameret);
		FAIL;
	} else
		CHECK;

	win = makewin(display, vp);
/* format wrong */
	XChangeProperty(display, win, XA_WM_NAME, XA_STRING, 32, PropModeReplace, (unsigned char *) name, strlen(name));
	w = win;
	window_return_name = &wnameret;
	status = XCALL;

	if(status != 0) {
		report("XFetchName() did not return 0 when the WM_NAME property format was 32.");
		FAIL;
	} else
		CHECK;

	if(wnameret != NULL) {
		report("Window name was \"%s\" instead of NULL when the WM_NAME property format was 32.", wnameret);
		FAIL;
	} else
		CHECK;


	win = makewin(display, vp);
/* type wrong */
	XChangeProperty(display, win, XA_WM_NAME, XA_ATOM, 8, PropModeReplace, (unsigned char *) name, strlen(name));
	w = win;
	window_return_name = &wnameret;
	status = XCALL;

	if(status != 0) {
		report("XFetchName() did not return 0 when the WM_NAME property type was ATOM.");
		FAIL;
	} else
		CHECK;

	if(wnameret != NULL) {
		report("Window name was \"%s\" instead of NULL when the WM_NAME property type was ATOM.", wnameret);
		FAIL;
	} else
		CHECK;

	CHECKPASS(6);

>>ASSERTION Bad A
.ER BadWindow
>># Kieron	Completed	Review.
