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
>>TITLE XNextEvent CH08
void
XNextEvent(display, event_return)
Display *display = Dsp;
XEvent	*event_return = &_event;
>>EXTERN
static XEvent _event;
/*
 * Can not use "xcall" because it empties the event queue.
 */
#define	_xcall_()	\
		_startcall(display);\
		XNextEvent(display, event_return);\
		_endcall(display)
>>ASSERTION Good A
When the event queue is not empty,
the a call to xname
returns the first event from the event queue in
.A event_return .
>>STRATEGY
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XNextEvent.
Verify that XNextEvent returned the correct event.
>>CODE
XEvent	event;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = KeyPress;
	XPutBackEvent(display, &event);
	event.type = KeyRelease;
	XPutBackEvent(display, &event);
	event.type = ButtonPress;
	XPutBackEvent(display, &event);
/* Call XNextEvent. */
	_xcall_();
/* Verify that XNextEvent returned the correct event. */
	if (event_return->type != event.type) {
		report("Returned %s, expected %s", eventname(event_return->type), eventname(event.type));
		FAIL;
	}
	else
		CHECK;
	/* empty event queue */
	XSync(display, True);

	CHECKPASS(1);
>>ASSERTION Good A
A call to xname removes the returned event from the event queue.
>>STRATEGY
Discard all events on the event queue.
Call XPutBackEvent to put a three events on the event queue.
Call XNextEvent.
Verify that XNextEvent returned the correct event.
Call XNextEvent.
Verify that XNextEvent returned the correct event.
Call XNextEvent.
Verify that XNextEvent returned the correct event.
Verify that the event queue is now empty.
>>CODE
XEvent	event;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put a three events on the event queue. */
	event.type = KeyPress;
	XPutBackEvent(display, &event);
	XPutBackEvent(display, &event);
	XPutBackEvent(display, &event);
/* Call XNextEvent. */
	_xcall_();
/* Verify that XNextEvent returned the correct event. */
	if (event_return->type != event.type) {
		report("Returned %s, expected %s", eventname(event_return->type), eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Call XNextEvent. */
	_xcall_();
/* Verify that XNextEvent returned the correct event. */
	if (event_return->type != event.type) {
		report("Returned %s, expected %s", eventname(event_return->type), eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Call XNextEvent. */
	_xcall_();
/* Verify that XNextEvent returned the correct event. */
	if (event_return->type != event.type) {
		report("Returned %s, expected %s", eventname(event_return->type), eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Verify that the event queue is now empty. */
	if (XPending(display) != 0) {
		report("Events not removed from the event queue.");
		FAIL;
	}
	else
		CHECK;
	/* empty event queue */
	XSync(display, True);

	CHECKPASS(4);
>>ASSERTION Good A
When the event queue is empty,
then a call to xname
flushes the output buffer and
blocks until an event is received and
returns the event in
.A event_return .
>>STRATEGY
Create bad pixmap.
Discard all events on the event queue.
Call XFreePixmap with bad pixmap.
Call XNextEvent and verify that blocking did occur.
Verify that output buffer was flushed.
>>CODE
Block_Info info;
int	block_status;
Pixmap	pm;
Display *client2;

/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Create pixmap. */
	/* avoid using makepixm() */
	pm = XCreatePixmap(display, DRW(display), 10, 10, 1);
/* Call XNextEvent and verify that blocking did occur. */
	XNextEvent_Type(info);
	block_status = block(display, (XEvent *) NULL, &info);
	if (block_status == -1)
		return;
	else
		CHECK;
	if (block_status == 0) {
		report("Blocking did not occur.");
		FAIL;
	}
	else
		CHECK;
/* Verify that the output buffer was flushed. */
	_startcall(client2);
	XFreePixmap(client2, pm);
	XSync(client2, True);
	_endcall(client2);
	if (geterr() != Success) {
		report("The output buffer was not flushed.");
		XFreePixmap(display, pm);
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(4);
