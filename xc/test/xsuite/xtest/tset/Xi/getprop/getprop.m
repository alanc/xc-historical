/*
 * Copyright 1993 by the Hewlett-Packard Company.
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP, MIT, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP, MIT, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: getprop.m,v 1.5 94/01/29 15:19:48 rws Exp $
 */

>>TITLE XGetDeviceDontPropagateList XINPUT
XEventClass *

Display	*display = Dsp;
Window	win;
int	*count_return = &pcount;
>>EXTERN
int pcount;
>>ASSERTION Good B 3
A successful call to xname returns the DontPropagateList for the specified
window.
>>STRATEGY
Call xname.
>>CODE
Window	w;
Display	*client1;
XEventClass 	*ret;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
/* Create client1. */
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Create window with client1. */
	w = mkwin(client1, (XVisualInfo *) NULL, (struct area *) NULL, False);
	win = w;
	XSync (client1, True);
	ret = XCALL;
	if (geterr() == Success)
		CHECK;
	else
		FAIL;
	SelectExtensionEvents(client1,w);
	XSync (client1, True);
	ret = XCALL;
	if (geterr() == Success)
		CHECK;
	else
		FAIL;
	SelectExtensionEvents(display,w);
	SuppressExtensionEvents(display,w);
	ret = XCALL;
	if (geterr() == Success)
		CHECK;
	else
		FAIL;
	CHECKPASS(4);
>>ASSERTION Bad B 3
A call to xname with an invalid window returns a BadWindow error.
>>STRATEGY
Call xname with an invalid window.
>>CODE BadWindow
Window	w;
Display	*client1;
XEventClass 	*ret;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
/* Create client1. */
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Create window with client1. */
	win = 0;
	ret = XCALL;
	if (geterr() == BadWindow)
		CHECK;
	else {
		report("No BadWindow for invalid window test");
		FAIL;
		}
	CHECKPASS(2);
