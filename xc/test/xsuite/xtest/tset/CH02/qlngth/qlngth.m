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
>>SET   macro
>>TITLE XQLength CH02
int
XQLength(display)
Display	*display = Dsp;
>>ASSERTION Good A
A call to xname returns the length of the event queue for connection
specified by the
.A display
argument.
>>STRATEGY
Discard all events on the event queue using XSync.
Put 3 events on the event queue using XPutBackEvent.
Verify that xname returned 3 events.
Empty the event queue using XSync.
>>CODE
int	eventsput;
int	event_count;
XEvent	event;

	XSync(display, True);
	event.type = MapNotify;
	eventsput = 0;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;

	_startcall(display);
	event_count = XQLength(display);
	_endcall(display);

	if (event_count != eventsput) {
		report("Returned %d, expected %d", event_count, eventsput);
		FAIL;
	} else
		PASS;

	XSync(display, True);
