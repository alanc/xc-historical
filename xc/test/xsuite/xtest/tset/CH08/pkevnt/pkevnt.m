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
>>TITLE XPeekEvent CH08
void
XPeekEvent(display, event_return)
Display *display = Dsp;
XEvent	*event_return = &_event;
>>EXTERN
static XEvent _event;
/*
 * Can not use "xcall" because it empties the event queue.
 */
#define	_xcall_()	\
		_startcall(display);\
		XPeekEvent(display, event_return);\
		_endcall(display)
>>ASSERTION Good A
A call to xname
returns the first event from the event queue in
.A event_return .
>>STRATEGY
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XPeekEvent.
Verify that XPeekEvent returned the correct event.
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
/* Call XPeekEvent. */
	_xcall_();
/* Verify that XPeekEvent returned the correct event. */
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
A call to xname does not remove
.A event_return
from the event queue.
>>STRATEGY
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XPending to get the current event queue size.
Call XPeekEvent.
Verify that XPeekEvent returned the correct event.
Call XPending to get the current event queue size.
Verify that size of the event queue has not changed.
Call XPeekEvent.
Verify that XPeekEvent returned the same event as last time.
Call XPending to get the current event queue size.
Verify that size of the event queue has not changed.
>>CODE
int	oldqsize;
int	newqsize;
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
/* Call XPending to get the current event queue size. */
	oldqsize = XPending(display);
/* Call XPeekEvent. */
	_xcall_();
/* Verify that XPeekEvent returned the correct event. */
	if (event_return->type != event.type) {
		report("Returned %s, expected %s", eventname(event_return->type), eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Call XPending to get the current event queue size. */
	newqsize = XPending(display);
/* Verify that size of the event queue has not changed. */
	if (oldqsize != newqsize) {
		report("Event queue size %d, expected %d", newqsize, oldqsize);
		FAIL;
	}
	else
		CHECK;
/* Call XPeekEvent. */
	_xcall_();
/* Verify that XPeekEvent returned the same event as last time. */
	if (event_return->type != event.type) {
		report("Returned %s, expected %s", eventname(event_return->type), eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Call XPending to get the current event queue size. */
	newqsize = XPending(display);
/* Verify that size of the event queue has not changed. */
	if (oldqsize != newqsize) {
		report("Event queue size %d, expected %d", newqsize, oldqsize);
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
Create client2.
Discard all events on the event queue.
Create pixmap.
Call XPeekEvent and verify that blocking did occur.
Verify that the output buffer was flushed.
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
/* Call XPeekEvent and verify that blocking did occur. */
	XPeekEvent_Type(info);
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
	/* empty event queue */
	XSync(display, True);

	CHECKPASS(4);
