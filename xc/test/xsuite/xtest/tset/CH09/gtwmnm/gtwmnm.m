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
>>TITLE XGetWMName CH09
Status
XGetWMName(display, w, text_prop_return)
Display	*display = Dsp;
Window	w = DRW(display);
XTextProperty	*text_prop_return = &textprop;
>>EXTERN
#include	"Xatom.h"
static XTextProperty	textprop = { (unsigned char *) 0, XA_STRING, 8, (unsigned long) 0 };
>>ASSERTION Good A
When the WM_NAME property
exists on the window specified by the
.A w
argument, then a call to xname stores
the data, which can be freed with
.S XFree ,
in the
.M value
field, the type of the data in the
.M encoding
field, the format of the data in the
.M format
field, and the number of items of data in the
.M nitems
field of the 
.S XTextProperty 
structure named by the
.A text_prop_return
argument and returns non-zero.
>>STRATEGY
Create a window with XCreateWindow.
Set the property WM_NAME for the window with XSetWMName.
Obtain the value of the WM_NAME property with XGetWMName.
Verify that the encoding, format, value and nitems fields of the returned structure are correct.
Release the allocated memory using XFree.
>>CODE
Window	win;
char	*str1 = "Xtest test string1";
char	*str2 = "Xtest test string2";
char	*str[2];
Status	status;
char	**list_return;
int	count_return;
XTextProperty	tp, rtp;
XVisualInfo	*vp;

	str[0] = str1;
	str[1] = str2;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	if(XStringListToTextProperty(str, 2, &tp) == False) {
		delete("XStringListToTextProperty() Failed.");
		return;
	} else 
		CHECK;

	XSetWMName(display, win, &tp);

	text_prop_return = &rtp;
	w = win;
	status = XCALL;

	if(status == False) {
		report("XGetTextProperty() returned False.");
		FAIL;
	} else
		CHECK;

	if(tp.encoding != rtp.encoding) {
		report("The encoding component of the XTextProperty was incorrect.");
		FAIL;
	} else
		CHECK;

	if(tp.format != rtp.format) {
		report("The format component of the XTextProperty was %d instead of %d.", rtp.format, tp.format);
		FAIL;
	} else
		CHECK;

	if(tp.nitems != rtp.nitems) {
		report("The nitems component of the XTextProperty was %lu instead of %lu.", rtp.nitems, rtp.nitems);
		FAIL;
	} else
		CHECK;

	if(XTextPropertyToStringList( &rtp, &list_return, &count_return) == False) {
		delete("XTextPropertyToStringList() returned False.");
		return;
	} else
		CHECK;

	if (count_return != 2) {
		delete("XTextPropertyToStringList() count_return was %d instead of 2.", count_return);
		return;
	} else
		CHECK;

	if( (strncmp(str1, list_return[0], strlen(str1)) != 0) || (strncmp(str2, list_return[1], strlen(str2)) != 0) ) {
		report("Value strings were:");
		report("\"%s\" and \"%s\"", list_return[0], list_return[1]);
		report("Instead of:");
		report("\"%s\" and \"%s\"", str1, str2);
		FAIL;
	} else
		CHECK;

	XFree((char*)tp.value);
	XFree((char*)rtp.value);
	XFreeStringList(list_return);

	CHECKPASS(8);

>>ASSERTION Good A
When the WM_NAME property
does not exist on the window
specified by the
.A w
argument, then a call to xname sets the 
.M value
field to NULL, the 
.M encoding
field to None, the
.M format
field to 0 and the 
.M nitems
field to 0 of the
.S XTextProperty
structure named by the
.A text_prop_return
argument and returns zero.
>>STRATEGY
Create a window with XCreateWindow
Get the value of the unset property WM_NAME with XGetWMName
Verify that the call returned False
Verify that in the returned XTextProperty structure the encoding field was none,
  the format component was 0, the nitems component was 0 and the
  value component was NULL.
>>CODE
Window	win;
Status	status;
char	**list_return;
int	count_return;
XTextProperty	tp;
XVisualInfo	*vp;

	tp.value = (unsigned char *) "XTuninitialised\0";
	tp.encoding = XA_STRING;
	tp.format = 13;
	tp.nitems = 1 + strlen(tp.value);

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	text_prop_return = &tp;
	w = win;
	status = XCALL;

	if(status != False) {
		report("XGetTextProperty() did not return False.");
		FAIL;
	} else
		CHECK;

	if(tp.encoding != None) {
		report("The Property was not of type None.");
		FAIL;
	} else
		CHECK;
	
	if(tp.format != 0) {
		report("The format component was %d instead of 0.", tp.format);
		FAIL;
	} else
		CHECK;

	if(tp.nitems != 0) {
		report("The nitems component was %lu instead of 0.", tp.nitems);
		FAIL;
	} else
		CHECK;

	if(tp.value != (unsigned char *) NULL) {
		report("The value component was not NULL."); 
		FAIL;
	} else
		CHECK;

	CHECKPASS(5);

>>ASSERTION Bad A
.ER BadWindow
>># Kieron	Completed	Review
