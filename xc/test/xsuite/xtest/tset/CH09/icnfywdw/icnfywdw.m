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
>>TITLE XIconifyWindow CH09
Status
XIconifyWindow(display, w, screen_number)
Display	*display = Dsp;
Window	w = DRW(Dsp);
int	screen_number = DefaultScreen(Dsp);
>>ASSERTION Good A
A call to xname sends a WM_CHANGE_STATE 
.S ClientMessage 
event with a
.M window
of
.A w ,
a
.M format
of 32 and a first
.M data
element of 
.S IconicState 
to the root window of the screen specified by the
.A screen_number
argument using an event mask of
.S SubstructureRedirectMask |
.S SubstructureNotifyMask
and returns non-zero.
>>STRATEGY
Create a window using XCreateWindow.
Obtain the atom for the string "WM_CHANGE_STATE" using XInternAtom.
Select SubstructureNotify events on the root window with XSelectInput.
Generate an event on the root window using XIconifyWindow.
Verify that the call returned non-zero.
Verify that an event was generated with XNextEvent.
Verify that the event type was ClientMessage.
Verify that the event window was correct.
Verify that the event message_type was WM_CHANGE_STATE.
Verify that the event format was 32.
Verify that the first data element of the event structure was IconicState.

Select SubstructureRedirect events on the root window with XSelectInput.
Generate an event on the root window using XIconifyWindow.
Verify that the call returned non-zero.
Verify that an event was generated with XNextEvent.
Verify that the event type was ClientMessage.
Verify that the event window was correct.
Verify that the event message_type was WM_CHANGE_STATE.
Verify that the event format was 32.
Verify that the first data element of the event structure was IconicState.
>>CODE
Status			status;
Atom			wm_change_state;
XEvent			ev, rev;
int			i;
int			nevents = 0;
XVisualInfo		*vp;
XWindowAttributes	watts;
unsigned long		event_mask[2];

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);
	screen_number = DefaultScreen(display);

	event_mask[0] = SubstructureNotifyMask;
	event_mask[1] = SubstructureRedirectMask;

	if( (wm_change_state = XInternAtom(display, "WM_CHANGE_STATE", False)) == None) {
		delete("The string \"WM_CHANGE_STATE\" could not be interned.");
		return;
	} else
		CHECK;

	ev.type = ClientMessage;
	ev.xany.display = display;


	ev.xclient.window = w;
	ev.xclient.message_type = wm_change_state;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = IconicState;
	ev.xclient.data.l[1] = 0;
	ev.xclient.data.l[2] = 0;
	ev.xclient.data.l[3] = 0;
	ev.xclient.data.l[4] = 0;

	for(i=0; i < 2; i++) {

		startcall(display);
		XSelectInput(display, DRW(display), event_mask[i]);
		endcall(display);
	
		if(geterr() != Success) {
			delete("XSelectInput() failed with an event mask of 0x%lx.", event_mask[i]);
			return;
		} else
			CHECK;
	
		status = XCALL;
	
		if(status == 0) {
			delete("%s() returned zero.", TestName);
			return;
		} else
			CHECK;
	
		rev.type = -1;
		if( (nevents = getevent(display, &rev)) == 0 ) {
			report("No event was generated.");
			FAIL;
		} else {
			CHECK;
			if(nevents != 1) {
				delete("There were %d events generated instead of 1.", nevents);
				return;
			} else	{
				CHECK;
	
				rev.xclient.data.l[1] = 0;
				rev.xclient.data.l[2] = 0;
				rev.xclient.data.l[3] = 0;
				rev.xclient.data.l[4] = 0;
	
				if( checkevent(&ev, &rev) != 0 ) {
					FAIL;
				} else
					CHECK;
			}
		}
	}

	CHECKPASS(11);

>>ASSERTION Bad B 1
>># Untestable, and not worth the effort of adding XTest extension facilities
>># to provoke the error.
When the atom name \(lqWM_CHANGE_STATE\(rq cannot be interned, then
a call to xname
does not send a message and returns zero.
>>ASSERTION Bad B 1
>># Untestable, and not worth the effort of adding XTest extension facilities
>># to provoke the error.
When the
.S ClientMessage
event cannot be sent, then
a call to xname
returns zero.
>># Assertion removed at request of MIT (Bug Report 67) because
>># XIconifyWindow cannot generate this error (Spec bug)
>># >>ASSERTION Bad A
>># .ER Window
