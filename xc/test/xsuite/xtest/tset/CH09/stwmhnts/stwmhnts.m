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
>>TITLE XSetWMHints CH09

XSetWmHints(display, w, wmhints)
Display		*display = Dsp;
Window		w = DRW(Dsp);
XWMHints	*wmhints = &dummyhints;
>>EXTERN
#include	"Xatom.h"
#define		NumPropWMHintsElements 9
XWMHints	dummyhints = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
>>ASSERTION Good A
A call to xname sets the WM_HINTS property
for the window
.A w
to be of type
.S WM_HINTS ,
format 32 and to have value set
to the hints specified in the
.S XWMHints
structure named by the
.A wmhints
argument.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_HINTS property for the window with XSetWMHints.
Verify type and format are XA_WM_HINTS and 32, respectively.
Verify that the property value was correctly set with XGetWindowProperty.
>>CODE
Window		win;
XVisualInfo	*vp;
XWMHints	hints;
XWMHints	*hints_ret;
unsigned long	leftover, nitems;
int		actual_format;
Atom		actual_type;


	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	hints.flags = AllHints;
	hints.input = True;
	hints.initial_state = IconicState;
	hints.icon_pixmap =  1L;
	hints.icon_window = 1L;
	hints.icon_x = 13;
	hints.icon_y = 7;
	hints.icon_mask = 1L;
	hints.window_group = 1L;

	w = win;
	wmhints = &hints;
	XCALL;

	if (XGetWindowProperty(display, win, XA_WM_HINTS, 0L, (long)NumPropWMHintsElements,
	    False, AnyPropertyType, &actual_type, &actual_format,  &nitems, &leftover,
	    (unsigned char **)&hints_ret) != Success) {
		delete("XGetWindowProperty() did not return Success.");
		return;
	} else
		CHECK;

	if(leftover != 0) {
		report("The leftover elements numbered %lu instead of 0", leftover);
		FAIL;
	} else
		CHECK;

	if(actual_format != 32) {
		report("The format of the WM_HINTS property was %lu instead of 32", actual_format);
		FAIL;
	} else
		CHECK;

	if(actual_type != XA_WM_HINTS) {
		report("The type of the WM_HINTS property was %lu instead of XA_WM_HINTS (%lu)", actual_type, XA_WM_HINTS);
		FAIL;
	} else
		CHECK;

	if(nitems != NumPropWMHintsElements) {
		report("The number of elements comprising the WM_HINTS property was %lu instead of %lu.",
				nitems, (unsigned long) NumPropWMHintsElements);
		FAIL;
	} else
		CHECK;

	if(hints_ret->flags != AllHints) {
		report("The flags component was %lu instead of AllHints.", hints_ret->flags);
		FAIL;
	} else
		CHECK;

	if(hints_ret->input != True) {
		report("The hints_ret component of the XWMHints structure was %d instead of True.", (Bool) hints_ret->input);
		FAIL;
	} else
		CHECK;

	if(hints_ret->initial_state != IconicState) {
		report("The initial_state component of the XWMHints structure was %d instead of IconicState.",
			hints_ret->initial_state);
		FAIL;
	} else
		CHECK;

	if(hints_ret->icon_pixmap !=  1L) {
		report("The icon_pixmap component of the XWMHints structure was %lu instead of 1.", hints_ret->icon_pixmap);
		FAIL;
	} else
		CHECK;

	if(hints_ret->icon_window != 1L) {
		report("The icon_window component of the XWMHints structure was %lu instead of 1.", hints_ret->icon_window);
		FAIL;
	} else
		CHECK;

	if(hints_ret->icon_x != 13) {
		report("The icon_x component of the XWMHints structure was %d instead of 13.", hints_ret->icon_x);
		FAIL;
	} else
		CHECK;

	if(hints_ret->icon_y != 7) {
		report("The icon_y component of the XWMHints structure was %d instead of 7.", hints_ret->icon_y);
		FAIL;
	} else
		CHECK;

	if(hints_ret->icon_mask != 1L) {
		report("The icon_mask component of the XWMHints structure was %lu instead of 1.", hints_ret->icon_mask);
		FAIL;
	} else
		CHECK;

	if(hints_ret->window_group != 1L) {
		report("The window_group component of the XWMHints structure was %lu instead of 1.", hints_ret->window_group);
		FAIL;
	} else
		CHECK;

	XFree((char*)hints_ret);

	CHECKPASS(14);

>>ASSERTION Bad B 1
.ER BadAlloc 
>>ASSERTION Bad A
.ER BadWindow 
>># Completed	Kieron	Review
