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
 * $XConsortium: getfocus.m,v 1.5 94/01/30 12:09:43 rws Exp $
 */
>>TITLE XGetDeviceFocus XINPUT
void

Display	*display = Dsp;
XDevice *device;
Window	*focus = &fwin;
int	*revert_to = &revert;
Time	*time = &focus_time;
>>EXTERN
Window fwin;
int revert;
Time focus_time;
XID baddevice;
extern ExtDeviceInfo Devs;

>>ASSERTION Good B 3
A call to GetDeviceFocus returns the focus window, PointerRoot,
FollowKeyboard, or None to focus_return, the current focus
revert state to revert_to_return, and the last_focus_time to
time_return.
>>STRATEGY
Touch test.
>>CODE
Window w;
XID devicestatenotify;
XEventClass devicestatenotifyclass;
XWindowAttributes attr;

	if (!Setup_Extension_DeviceInfo(FocusMask))
	    {
	    untested("%s: Required input extension devices not found.\n", TestName);
	    return;
	    }
	device = Devs.Focus;

	XCALL;
	if (fwin!=None && fwin!=PointerRoot && fwin!=FollowKeyboard)
	    if (XGetWindowAttributes(display, fwin, &attr) != Success)
		{
		report("Bad window returned by %s\n",TestName);
		FAIL;
		}
	    else
		CHECK;
	else
	    CHECK;
	if (revert!=RevertToNone && revert!=RevertToPointerRoot && 
	    revert!=RevertToFollowKeyboard && revert!=RevertToParent)
	    {
	    report("Bad revert_to returned by %s\n",TestName);
	    FAIL;
	    }
	else
	    CHECK;


	XSetDeviceFocus(display, device, PointerRoot, RevertToPointerRoot, CurrentTime);
	XCALL;
	if (fwin!=PointerRoot)
	    {
	    report("Bad window returned by %s\n",TestName);
	    FAIL;
	    }
	else
	    CHECK;
	if (revert!=RevertToPointerRoot)
	    {
	    report("Bad revert_to returned by %s\n",TestName);
	    FAIL;
	    }
	else
	    CHECK;


	XSetDeviceFocus(display, device, FollowKeyboard, RevertToFollowKeyboard, CurrentTime);
	XCALL;
	if (fwin!=FollowKeyboard)
	    {
	    report("Bad window returned by %s\n",TestName);
	    FAIL;
	    }
	else
	    CHECK;
	if (revert!=RevertToFollowKeyboard)
	    {
	    report("Bad revert_to returned by %s\n",TestName);
	    FAIL;
	    }
	else
	    CHECK;

	w = defwin(display);
	DeviceStateNotify(device, devicestatenotify, devicestatenotifyclass);
	XSelectExtensionEvent(display, w, &devicestatenotifyclass, 1);
	XSync(display,0);

	XSetDeviceFocus(display, device, None, RevertToNone, CurrentTime);
	XCALL;
	if (fwin!=None)
	    {
	    report("Bad window returned by %s\n",TestName);
	    FAIL;
	    }
	else
	    CHECK;
	if (revert!=RevertToNone)
	    {
	    report("Bad revert_to returned by %s\n",TestName);
	    FAIL;
	    }
	else
	    CHECK;
	
	XSetDeviceFocus(display, device, w, RevertToPointerRoot, CurrentTime);
	XCALL;
	if (fwin!=w)
	    {
	    report("Bad window returned by %s\n",TestName);
	    FAIL;
	    }
	else
	    CHECK;
	if (revert!=RevertToPointerRoot)
	    {
	    report("Bad revert_to returned by %s\n",TestName);
	    FAIL;
	    }
	else
	    CHECK;

	CHECKPASS(10);


>>ASSERTION Bad B 3
A call to xname specifying an invalid device results in a BadDevice error.
>>STRATEGY
Make the call with an invalid device.
>>CODE baddevice
XDevice nodevice;
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
		CHECK;
	else
		FAIL;

	CHECKPASS(1);
