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
>>TITLE XPending CH08
int
XPending(display)
Display *display = Dsp;
>>EXTERN

/*
 * Can not use "xcall" because it empties the event queue.
 */
#define	_xcall_(return_value)	\
		_startcall(display);\
		return_value = XPending(display);\
		_endcall(display)
>>ASSERTION Good A
>>#NOTE	These assertions are similar to those in
>>#NOTE XEventsQueued for the QueuedAfterFlush tests.
When the number of events already in the event queue is non-zero,
then a call to xname
returns the number of events in the event queue.
>>STRATEGY
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XPending.
Verify that XPending returned the correct number of events.
>>CODE
int	eventsput;
int	event_count;
XEvent	event;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = MapNotify;
	eventsput = 0;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
/* Call XPending. */
	_xcall_(event_count);
/* Verify that XPending returned the correct number of events. */
	if (event_count != eventsput) {
		report("Returned %d, expected %d", event_count, eventsput);
		FAIL;
	}
	else
		CHECK;
	/* empty event queue */
	XSync(display, True);

	CHECKPASS(1);
>>ASSERTION Good A
When there are no events in the event queue,
then a call to xname
flushes the output buffer.
>>STRATEGY
Create client2.
Discard all events on the event queue.
Create pixmap.
Call XPending.
Empty the buffer.
Ensure the server has dealt with anything flushed to it: do XSync()
Verify that the output buffer was flushed by effect on server.
Verify that XPending returned the correct number of events.
>>CODE
int	event_count;
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
/* Call XPending. */
	_xcall_(event_count);
/* Empty the buffer. */
	(void)XTestDiscard(display);
/* Ensure the server has dealt with anything flushed to it: do XSync() */
	XSync(display, False);
/* Verify that the output buffer was flushed by effect on server. */
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
/* Verify that XPending returned the correct number of events. */
	if (event_count != 0) {
		report("Returned %d, expected none", event_count);
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Good A
When there are no events in the event queue,
then a call to xname
attempts to read more events out of the client's connection
without blocking
and returns the number read.
>>STRATEGY
Discard all events on the event queue.
Call XPending and verify that blocking did not occur.
Discard all events on the event queue.
>>CODE
Block_Info info;
int	block_status;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPending and verify that blocking did not occur. */
	XPending_Type(info);
	block_status = block(display, (XEvent *) NULL, &info);
	if (block_status == -1)
		return;
	else
		CHECK;
	if (block_status == 1) {
		report("Blocking occurred with QueuedAfterFlush when it should not have.");
		FAIL;
	}
	else
		CHECK;
	if (info.int_return != 0) {
		delete("Unexpected number of events in event queue: %d", info.int_return);
		return;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
	CHECKPASS(3);
