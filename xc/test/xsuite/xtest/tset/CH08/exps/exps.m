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
>>TITLE Expose CH08
>>EXTERN
#define	EVENT		Expose
#define	MASK		ExposureMask
>>ASSERTION Good A
The server does not generate xname events
on windows whose class is specified as
.S InputOnly .
>>STRATEGY
Create InputOnly window.
Select for Expose and MapNotify events on window.
Map window.
Verify that a MapNotify event was received.
Verify that no Expose events were received.
>>CODE
Display	*display = Dsp;
Window	w;
XEvent	event_return;
int	events;

#ifdef	OTHERMASK
#undef	OTHERMASK
#endif
#define	OTHERMASK	StructureNotifyMask
#ifdef	OTHEREVENT
#undef	OTHEREVENT
#endif
#define	OTHEREVENT	MapNotify

/* Create InputOnly window. */
	w = iponlywin(display);
/* Select for Expose and MapNotify events on window. */
	XSelectInput(display, w, MASK|OTHERMASK);
/* Map window. */
	XSync(display, True);
	XMapWindow(display, w);
	XSync(display, False);
/* Verify that a MapNotify event was received. */
	if (!XCheckTypedWindowEvent(display, w, OTHEREVENT, &event_return)) {
		delete("Expected %s event, got none", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
/* Verify that no Expose events were received. */
	if (XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
		delete("Got %s event, expected none", eventname(EVENT));
		return;
	}
	else
		CHECK;
	events = XPending(display);
	if (events > 0) {
		delete("Received %d more events than expected", events);
		return;
	}
	else
		CHECK;

	CHECKPASS(3);
>>ASSERTION def
>>#NOTE	Tested below in the "...all clients having set..." assertion.
When no valid contents are available for regions of a window
and the regions are visible,
then an xname event is generated.
>>ASSERTION Good B 1
>>#NOTE	Can't predict when/how the server will perform backing-store.
When no valid contents are available for regions of a window
and the regions are viewable but not visible
and the server is maintaining backing store on the window,
then an xname event is generated.
>>ASSERTION Good B 1
>>#NOTE	Can't predict when/how the server will perform backing-store.
When no valid contents are available for regions of a window
and the window is not viewable but the server is honoring
the window's backing-store attribute of
.S Always
or
.S WhenMapped ,
then an xname event is generated.
>>ASSERTION Good A
When an xname events are generated,
then they are contiguously delivered by the server.
>>STRATEGY
Build and create window hierarchy.
Choose event window.
Select for Expose events on event window.
Generate Expose events on event window.
Verify that events are contiguously delivered.
>>CODE
Display	*display = Dsp;
Winh	*eventw;
Winh	*child;
Winhe	*winhe;
int	expected;
int	lastcount;

/* Build and create window hierarchy. */
	if (winh(display, 2, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Choose event window. */
	eventw = guardian->firstchild;
/* Select for Expose events on event window. */
	if (winh_selectinput(display, eventw, MASK))
		return;
	else
		CHECK;
/* Generate Expose events on event window. */
	XSync(display, True);
	for (child = eventw->firstchild; child != (Winh *) NULL; child = child->nextsibling) {
		static	int	first = 1;

		if (first) {
			first = 0;
			CHECK;
		}
		XUnmapWindow(display, child->window);
	}
	XSync(display, False);
/* Verify that events are contiguously delivered. */
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
	if (winh_qdel == (Winhe *) NULL) {
		delete("No events delivered.");
		return;
	}
	else
		CHECK;
	expected = -1;
	for (winhe = winh_qdel; winhe != (Winhe *) NULL; winhe = winhe->next) {
		if (expected == -1)
			CHECK;
		if (winhe->event->type != EVENT) {
			report("Received %s event while only expecting %s types",
				eventname(winhe->event->type), eventname(EVENT));
			delete("Unexpected event received.");
			return;
		}
		lastcount = winhe->event->xexpose.count;
		if (expected == -1) {
			expected = lastcount;
		}
		else {
			if (lastcount > expected) {
				report("Count (%d) greater than expected (%d)",
					lastcount, expected);
				FAIL;
				break;
			}
			else if (lastcount < expected)
				expected--;
		}
	}
	if (lastcount != 0) {
		report("Last %s had count set to %d, not zero",
			eventname(EVENT), lastcount);
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(7);
>>ASSERTION Good A
When an xname event is generated,
then
all clients having set
.S ExposureMask
event mask bits on the event window are delivered
an xname event.
>>STRATEGY
Create clients client2 and client3.
Build and create window hierarchy.
Choose event window.
Select for Expose events on event window.
Select for Expose events on event window with client2.
Select for no events on event window with client3.
Generate Expose events on event window.
Verify that events are delivered to selecting clients.
Verify that no Expose events are received for client3.
>>CODE
Display	*display = Dsp;
Display	*client2, *client3;
Winh	*eventw;
int	events;
Window	w;
XEvent	event_return;

/* Create clients client2 and client3. */
	if ((client2 = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client2.");
		return;
	}
	else
		CHECK;
	if ((client3 = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client3.");
		return;
	}
	else
		CHECK;
/* Build and create window hierarchy. */
	if (winh(display, 2, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Choose event window. */
	eventw = guardian->firstchild;
	w = eventw->window;
/* Select for Expose events on event window. */
	XSelectInput(display, w, MASK);
/* Select for Expose events on event window with client2. */
	XSelectInput(client2, w, MASK);
/* Select for no events on event window with client3. */
	XSelectInput(client3, w, NoEventMask);
/* Generate Expose events on event window. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XUnmapWindow(display, eventw->firstchild->window);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that events are delivered to selecting clients. */
	if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
		delete("Expected %s event, got none", eventname(EVENT));
		return;
	}
	else
		CHECK;
	if (!XCheckTypedWindowEvent(client2, w, EVENT, &event_return)) {
		delete("Expected %s event, got none with client2", eventname(EVENT));
		return;
	}
	else
		CHECK;
/* Verify that no Expose events are received for client3. */
	if (XCheckTypedWindowEvent(client3, w, EVENT, &event_return)) {
		delete("Got %s event, expected none", eventname(EVENT));
		return;
	}
	else
		CHECK;

	CHECKPASS(6);
>>ASSERTION def
>>#NOTE True for most events (except MappingNotify and selection stuff).
When an xname event is generated,
then
clients not having set
.S ExposureMask
event mask bits on the event window are not delivered
an xname event.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M type
>>#NOTEs is set to
>>#NOTEs xname.
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE The method of expansion is not clear.
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M serial
>>#NOTEs is set
>>#NOTEs from the serial number reported in the protocol
>>#NOTEs but expanded from the 16-bit least-significant bits
>>#NOTEs to a full 32-bit value.
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname event is delivered
>>#NOTEm and the event came from a
>>#NOTEm .S SendEvent
>>#NOTEm protocol request,
>>#NOTEm then
>>#NOTEm .M send_event
>>#NOTEm is set to
>>#NOTEm .S True .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the event was not generated by a
>>#NOTEs .S SendEvent
>>#NOTEs protocol request,
>>#NOTEs then
>>#NOTEs .M send_event
>>#NOTEs is set to
>>#NOTEs .S False .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M display
>>#NOTEs is set to
>>#NOTEs a pointer to the display on which the event was read.
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE Global except for MappingNotify and KeymapNotify.
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M window
>>#NOTEs is set to
>>#NOTEs the
>>#NOTEs ifdef(`WINDOWTYPE', WINDOWTYPE, event)
>>#NOTEs window.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M x
>>#NOTEs and
>>#NOTEs .M y
>>#NOTEs are set to
>>#NOTEs the coordinates relative to the window's origin
>>#NOTEs and indicate the upper-left corner of the rectangle defining the exposed region.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M width
>>#NOTEs and
>>#NOTEs .M height
>>#NOTEs are set to
>>#NOTEs the size (extent) of the rectangle.
>>#NOTEs >>ASSERTION
>>#NOTEs When
>>#NOTEs .M count
>>#NOTEs is set to zero,
>>#NOTEs then no further xname events are to follow for the exposed DRAWABLE.
>>#NOTEs >>ASSERTION
>>#NOTEs When
>>#NOTEs .M count
>>#NOTEs is greater than zero,
>>#NOTEs then at least
>>#NOTEs .M count
>>#NOTEs xname events
>>#NOTEs are to follow for the exposed DRAWABLE.
