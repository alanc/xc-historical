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
>>TITLE XPutBackEvent CH08
void
XPutBackEvent(display, event)
Display *display = Dsp;
XEvent	*event = &_event;
>>EXTERN
/*
 * Can not use "xcall" because it empties the event queue.
 */
#define	_xcall_()	\
		_startcall(display);\
		XPutBackEvent(display, event);\
		_endcall(display)
static XEvent _event;
>>ASSERTION Good A
A call to xname pushes a copy of
.A event
onto the head of the display's event queue.
>>STRATEGY
Call XSync to empty event queue.
Call XPutBackEvent to push event onto the head of the event queue.
Call XPeekEvent to verify that first event is the event that was pushed.
Call XPutBackEvent to push another event onto the head of the event queue.
Call XPeekEvent to verify that first event is the event that was pushed.
>>CODE
XEvent	event_return;

/* Call XSync to empty event queue. */
	XSync(display, True);
/* Call XPutBackEvent to push event onto the head of the event queue. */
	event->type = ButtonPress;
	_xcall_();
/* Call XPeekEvent to verify that first event is the event that was pushed. */
	XPeekEvent(display, &event_return);
	if (event_return.type != event->type) {
		report("Returned %s, expected %s", eventname(event_return.type), eventname(event->type));
		FAIL;
	}
	else
		CHECK;
/* Call XPutBackEvent to push another event onto the head of the event queue. */
	event->type = KeyPress;
	_xcall_();
/* Call XPeekEvent to verify that first event is the event that was pushed. */
	XPeekEvent(display, &event_return);
	if (event_return.type != event->type) {
		report("Returned %s, expected %s", eventname(event_return.type), eventname(event->type));
		FAIL;
	}
	else
		CHECK;
	XSync(display, True);
	
	CHECKPASS(2);
>>ASSERTION Good B 5
A call to xname
can be made an unlimited number of times in succession.
>>STRATEGY
Call XPutBackEvent 1000 times.
>>CODE
int	i;

/* Call XPutBackEvent 1000 times. */
	event->type = ButtonPress;
	for (i=0; i<1000; i++) {
		_xcall_();
	}
	UNTESTED;
