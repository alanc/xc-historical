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
>>TITLE XGetClassHint CH09
Status
XGetClassHint(display, w, class_hints_return)
Display		*display = Dsp;
Window		w = DRW(Dsp);
XClassHint	*class_hints_return = &chints;
>>EXTERN
#include	"Xatom.h"
XClassHint	chints;
XClassHint	chints_ret = { "<Unset String>", "<UnsetString>" };
>>ASSERTION Good A
When the WM_CLASS property is set for the window
.A w ,
and has format 8 and type
.S STRING ,
then a call to xname returns the class hint, which can be freed
with XFree, in the
.S XClassHint
structure named by the
.A class_hints_return
argument and returns non-zero. 
>>STRATEGY
Create a window using XCreateWindow.
Set the WM_CLASS property using XSetClassHint.
Obtain the WM_CLASS property value with XGetWindowProperty.
Verify that the returned values are correct.
Release the allocated hints using XFree.
>>CODE
char		*s;
Window		win;
Status		status;
XVisualInfo	*vp;
XClassHint	chints;


	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	chints.res_name = "Xtest_res_name";
	chints.res_class = "Xtest_res_class";

	XSetClassHint(display, win, &chints);

	w = win;
	class_hints_return = &chints_ret;
	status = XCALL;

	if(status == False) {
		report("XGetClassHint() returned False.");
		FAIL;
	} else
		CHECK;

	if( strcmp((s=chints_ret.res_name), chints.res_name) != 0) {
		report("The res_name component of the XClassHint structure was \"%s\" instead of %s.",
			s == (char *)NULL ? "NULL" : s,
			chints.res_name);
		FAIL;
	} else
		CHECK;

	if(strcmp((s=chints_ret.res_class), chints.res_class) != 0) {
		report("The res_class component of the XClassHint structure was \"%s\" instead of %s.",
			s == (char *)NULL ? "NULL" : s,
			chints.res_class);
		FAIL;
	} else
		CHECK;


	XFree(chints_ret.res_name);
	XFree(chints_ret.res_class);

	CHECKPASS(3);

>>ASSERTION Good A
When the WM_CLASS property is not set for the window
.A w ,
or has a format other than 8 or a type other than
.S STRING ,
then a call to xname returns zero.
>>STRATEGY
Create a window with XCreateWindow.
Obtain the value of the WM_CLASS property with XGetClassHint.
Verify that the call returned zero.

Create a window with XCreateWindow.
Set the WM_CLASS property with format 16 and type STRING using XChangeProperty.
Obtain the value of the WM_CLASS property with XGetClassHint.
Verify that the call returned zero.

Create a window with XCreateWindow.
Set the WM_CLASS property with format 8 type ATOM using XChangeProperty.
Obtain the value of the WM_CLASS property with XGetClassHint.
Verify that the call returned zero.

>>CODE
Status		status;
XVisualInfo	*vp;
char		*s1 = "XTestResName";
char		*s2 = "XTestResClass";
unsigned 	lenname, lenclass;
char		*hints, *hp;
XClassHint	chints_ret;


	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);

/* property not set */
	if( (char *) (hints = malloc( (unsigned int) ((unsigned)2 + (lenname = strlen(s1)) + (lenclass = strlen(s2)))) ) == (char *) 0) {
		delete("malloc() failed.");
		return;	
	} else 
		CHECK;

	hp = hints;
	strcpy( hp, s1);
	hp += lenname + 1;
	strcpy( hp, s2);

	class_hints_return = &chints_ret;
	status = XCALL;

	if(status != False) {
		report("XGetClassHint() did not return False with the WM_CLASS property unset.");
		FAIL;
	} else
		CHECK;

/* format not 8 */

	w = makewin(display, vp);
 	
	XChangeProperty(display, w, XA_WM_CLASS, XA_STRING, 16,
			PropModeReplace, (unsigned char *) &hints, 2 + lenclass +lenname);

	class_hints_return = &chints_ret;
	status = XCALL;

	if(status != False) {
		report("XGetClassHint() did not return False with the WM_CLASS property format set to 16.");
		FAIL;
	} else
		CHECK;


/* type wrong */

	w = makewin(display, vp);
 	
	XChangeProperty(display, w, XA_WM_CLASS, XA_ATOM, 8,
			PropModeReplace, (unsigned char *) &hints, 2 + lenclass + lenname);

	class_hints_return = &chints_ret;
	status = XCALL;

	if(status != False) {
		report("XGetClassHint() did not return False with the WM_CLASS property type set to Atom.");
		FAIL;
	} else
		CHECK;

	free(hints);

	CHECKPASS(4);

>>ASSERTION Bad A
.ER BadWindow
>># Kieron	Action	Review
