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
>>TITLE XFlush CH08
void
XFlush(display)
Display *display = Dsp;
>>ASSERTION Good A
A call to xname flushes the output buffer.
>>STRATEGY
Create client2.
Discard all events on the event queue.
Create pixmap.
Empty the buffer.
Ensure the server has dealt with anything flushed to it: do XSync()
Call XFreePixmap from client2.
Call XSync from client2 to insure all potential errors arrive.
Verify that an error did occur.
Discard all events on the event queue.
Create pixmap.
Call XFlush with display.
Empty the buffer.
Ensure the server has dealt with anything flushed to it: do XSync()
Check for unexpected errors after flushing.
Call XFreePixmap from client2.
Call XSync from client2 to insure all potential errors arrive.
Verify that an error did not occur.
>>CODE
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

/* Empty the buffer. */
	(void)XTestDiscard(display);
/* Ensure the server has dealt with anything flushed to it: do XSync() */
	XSync(display, False);
/* Call XFreePixmap from client2. */
	_startcall(client2);
	XFreePixmap(client2, pm);
/* Call XSync from client2 to insure all potential errors arrive. */
	XSync(client2, False);
	_endcall(client2);
/* Verify that an error did occur. */
	if (geterr() == Success) {
		report("Flushing appears to happen automatically");
		UNTESTED;
		/* Without this CHECK we get path count errors. */
		CHECK;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Create pixmap. */
	/* avoid using makepixm() */
	pm = XCreatePixmap(display, DRW(display), 10, 10, 1);
/* Call XFlush with display. */
	_startcall(display);
	XFlush(display);
/* Empty the buffer. */
	(void)XTestDiscard(display);
/* Ensure the server has dealt with anything flushed to it: do XSync() */
	XSync(display, False);
	_endcall(display);
/* Check for unexpected errors after flushing. */
	if (geterr() != Success) {
		delete("Unexpected error '%s' after XFlush.", errorname(geterr()));
		XFreePixmap(display, pm);
		return;
	}
	else
		CHECK;
/* Call XFreePixmap from client2. */
	_startcall(client2);
	XFreePixmap(client2, pm);
/* Call XSync from client2 to insure all potential errors arrive. */
	XSync(client2, False);
	_endcall(client2);
/* Verify that an error did not occur. */
	if (geterr() != Success) {
		report("Flushing did not occur - error '%s'.", errorname(geterr()));
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(4);
