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
 * $XConsortium: listdev.m,v 1.5 94/01/29 15:17:15 rws Exp $
 */

>>TITLE XListInputDevices XINPUT
XDeviceInfo *

Display	*display = Dsp;
int *ndevices_return = &ndevices;
>>EXTERN
int ndevices;

>>ASSERTION Good B 3
A successful call to xname lists all the input devices and returns
a pointer to a list of DeviceInfo structures.
>>STRATEGY
Call xname.
>>CODE
XDeviceInfo *list;
int major, first, err;

	if (!XQueryExtension (display, INAME, &major, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	list = XCALL;
	if (list==NULL)
	    FAIL;
	else
	    PASS;

>>ASSERTION Good B 3
A successful call to ListInputDevices returns the number of
input devices in the ndevices_return parameter.
>>STRATEGY
Call xname.
>>CODE
XDeviceInfo *list;
int major, first, err;

	if (!XQueryExtension (display, INAME, &major, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	list = XCALL;
	if (list==NULL)
	    FAIL;
	else
	    {
	    CHECK;
	    if (ndevices >= 2)
		CHECK;
	    else
		{
		report("%s: failed to get DeviceInfo information for at least the core input devices",TestName);
		FAIL;
		}
	    }
	CHECKPASS(2);

