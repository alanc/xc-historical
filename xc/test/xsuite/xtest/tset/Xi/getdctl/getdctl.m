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
 * $XConsortium: getdctl.m,v 1.4 94/01/29 15:22:32 rws Exp $
 */
>>TITLE XGetDeviceControl XINPUT
XGetDeviceControl *

Display	*display = Dsp;
XDevice *device;
int control = DEVICE_RESOLUTION;
>>EXTERN
extern ExtDeviceInfo Devs;

>>ASSERTION Good B 3
A successful call to xname returns the device state information
for the specified device.
>>STRATEGY
Touch test.
>>CODE

	if (!Setup_Extension_DeviceInfo(DCtlMask))
	    {
	    untested("%s: No input extension test device.\n", TestName);
	    return;
	    }
	device = Devs.DvCtl;
	control = DEVICE_RESOLUTION;

	XCALL;

	if (geterr() == Success)
		PASS;
	else
		FAIL;

>>ASSERTION Bad B 3
A call to xname will fail with a BadValue error if an invalid device
control is specified.
>>STRATEGY
Make the call with an invalid device control.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(DCtlMask))
	    {
	    untested("%s: No input extension test device.\n", TestName);
	    return;
	    }
	device = Devs.DvCtl;
	control = -1;

	XCALL;

	if (geterr() == BadValue)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
A call to xname will fail with a BadMatch error if a valid device
with no valuators is specified.
>>STRATEGY
Make the call with a valid device that has no valuators.
>>CODE BadMatch

	if (!Setup_Extension_DeviceInfo(NValsMask))
	    {
	    untested("%s: No input extension device without valuators.\n", TestName);
	    return;
	    }
	device = Devs.NoValuators;

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
A call to xname will fail with a BadDevice error if an invalid device
is specified.
>>STRATEGY
Make the call with an invalid device.
>>CODE baddevice
XDevice nodevice;
XID baddevice;
int major, first, err;

	if (!XQueryExtension (display, INAME, &major, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	BadDevice (display, baddevice);
	nodevice.device_id = -1;
	device = &nodevice;

	XCALL;

	if (geterr() == baddevice)
		PASS;
	else
		FAIL;
