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
 * $XConsortium: gtslctevnt.m,v 1.5 94/01/29 15:20:14 rws Exp $
 */

>>TITLE XGetSelectedExtensionEvents XINPUT
XEventClass *

Display	*display = Dsp;
Window	win;
int	*this_client_count = &tcount;
XEventClass **this_client_list= &tlist;
int	*all_clients_count = &acount;
XEventClass **all_clients_list= &alist;
>>EXTERN
int acount, tcount;
XEventClass *tlist, *alist;
>>ASSERTION Good B 3
A successful call to xname returns the selected input extension events for the 
specified window.
>>STRATEGY
Call xname.
>>CODE
Window	w;
Display	*client1, *client2;
int 	ret;

/* Create client1. */
	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
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
	CHECKPASS(3);

	SuppressExtensionEvents(display,w);
	XSync (display, True);
	ret = XCALL;
	if (geterr() == Success)
		CHECK;
	else
		FAIL;
	CHECKPASS(4);

	SelectExtensionEvents(display,w);
	XSync (display, True);
	ret = XCALL;
	if (geterr() == Success)
		CHECK;
	else
		FAIL;
	CHECKPASS(5);

	SelectExtensionEvents(client1,w);
	XSync (client1, True);
	ret = XCALL;
	if (geterr() == Success)
		CHECK;
	else
		FAIL;
	CHECKPASS(6);

	SelectExtensionEvents(client2,w);
	XSync (client2, True);
	ret = XCALL;
	if (geterr() == Success)
		CHECK;
	else
		FAIL;
	CHECKPASS(7);

>>ASSERTION Bad B 3
A call to xname with an invalid window returns a BadWindow error.
>>STRATEGY
Call xname with an invalid window.
>>CODE BadWindow
Window	w;
Display	*client1;
int 	ret;

/* Create client1. */
	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
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

