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
>>TITLE XSetIconName CH09
int
XSetIconName(display, w, icon_name)
Display	*display = Dsp;
Window	w = DRW(Dsp);
char	*icon_name = "XTestUninit";
>>EXTERN
#include	"Xatom.h"
>>ASSERTION Good A
A call to xname sets the
.S WM_ICON_NAME
property for the window
.A w
to be of
.M type
.S STRING ,
.M format
8 and to have
.M value
set to the null terminated string specified by the
.A icon_name
argument.
>>STRATEGY
Create a window with XCreateWindow
Set the icon name for the window with XSetIconName
Get the WM_ICON_NAME property for the window with XGetTextProperty.
Verify that type and format are STRING and 8 respectively.
Verify that the value was set correctly.
>>CODE
Window	win;
int	count;
char	*wname = "XTestICNName";
XTextProperty	tp;
char	**wnameret = NULL;
XVisualInfo	*vp;
Status	status;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	w = win;
	icon_name = wname;
	XCALL;

	status = XGetTextProperty(display, win, &tp, XA_WM_ICON_NAME);

	if (status == False || tp.value == NULL) {
		status = False;
		tp.value = NULL;
		report("Failed to get WM_ICON_NAME.");
		FAIL;
	} else 
		CHECK;

	if (status && tp.encoding != XA_STRING) {
		status = False;
		report("Encoding is \"%s\", not STRING.", XGetAtomName(display, tp.encoding) );
		FAIL;
	} else
		CHECK;

	if (status && tp.format != 8) {
		status = False;
		report("Format is %d instead of 8.", tp.format);
		FAIL;
	} else {
		CHECK;

		status = XTextPropertyToStringList(&tp, &wnameret, &count);

		if (status == False) {
			wnameret = NULL;
			delete("XTextPropertyToStringList returned False.");
			return;
		} else
			CHECK;

		if (count != 1) {
			status = False;
			report("WM_ICON_NAME not single string.");
			FAIL;
		} else
			CHECK;
	}

	if(status && strcmp(wnameret[0], wname) != 0) {
		report("Icon name was \"%s\" instead of \"%s\".", wnameret[0], wname);
		FAIL;
	} else
		CHECK;

	if (wnameret != NULL)
		XFree((char*)wnameret);
	if (tp.value != NULL)
		XFree((char*)tp.value);
	CHECKPASS(6);

>>ASSERTION Bad B 1
.ER BadAlloc
>>ASSERTION Good A
.ER BadWindow
>># Kieron	Completed	Review
