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
 * $XConsortium: getfctl.m,v 1.4 94/01/29 15:19:08 rws Exp $
 */
>>TITLE XGetFeedbackControl XINPUT
XFeedbackState *
xname()
Display	*display = Dsp;
XDevice	*device;
int	*num_feedbacks_return = &Nfeed;
>>EXTERN
extern ExtDeviceInfo Devs;
int Nfeed;

>>ASSERTION Good B 3
A call to xname
returns in Nfeed the number of feedbacks supported by the device.
>>STRATEGY
Call xname to get the feedbacks supported by this device.
UNTESTED touch test only.
>>CODE
int i, j, ndevices;
XDeviceInfoPtr list;
XInputClassInfo *ip;
XFeedbackState *state;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	list = XListInputDevices (display, &ndevices);
	for (i=0; i<ndevices; i++,list++)
	    if (list->use == IsXExtensionDevice)
		{
		device = XOpenDevice (display, list->id);
		for (j=0, ip=device->classes; j<device->num_classes; j++,ip++)
		    if (ip->input_class == FeedbackClass)
			{
			state = XCALL;
			trace("Number of feedbacks reported as %d", Nfeed);
			}
		}

	report("There is no reliable test method, but a touch test was performed");

	UNTESTED;
>>ASSERTION Bad B 3
A call to xname
returns a BadDevice error if an invalid device is specified.
>>CODE baddevice
XID 	baddevice;
XDevice bogus;
XFeedbackState *state;
int major, first, err;

	if (!XQueryExtension (display, INAME, &major, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	BadDevice(display,baddevice);
	bogus.device_id = -1;
	device = &bogus;
	state = XCALL;

	if (geterr() == baddevice)
		CHECK;
	else {
		report("No BadDevice for invalid device");
		FAIL;
		}
	CHECKPASS(1);
>>ASSERTION Bad B 3
A call to xname
returns a BadMatch error if the device has no feedbacks.
>>CODE BadMatch
XFeedbackState *state;

	if (!Setup_Extension_DeviceInfo(NFeedMask))
	    {
	    untested("%s: No input extension device without feedbacks.\n", TestName);
	    return;
	    }
	device = Devs.NoFeedback;
	state = XCALL;

	if (geterr() == BadMatch)
		PASS;
	else {
		report("No BadMatch error for device with no feedbacks");
		FAIL;
		}
