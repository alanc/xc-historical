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
>>TITLE ColormapNotify CH08
>>EXTERN
#define	EVENT		ColormapNotify
#define	MASK		ColormapChangeMask
>>ASSERTION Good A
When a xname event is generated,
then
all clients having set
.S ColormapChangeMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Create window.
Select for ColormapNotify events using ColormapChangeMask.
Select for ColormapNotify events using ColormapChangeMask with client2.
Select for no events with client3.
Get visual of window.
Create colormap.
Generate ColormapNotify event.
Verify that a ColormapNotify event was delivered.
Verify that event member fields are correctly set.
Verify that a ColormapNotify event was delivered to client2.
Verify that event member fields are correctly set.
Verify that no events were delivered to client3.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Window	w;
int	count;
XEvent	event_return;
XColormapEvent good;
Colormap	colormap;
XWindowAttributes attrs;

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
/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select for ColormapNotify events using ColormapChangeMask. */
	XSelectInput(display, w, MASK);
/* Select for ColormapNotify events using ColormapChangeMask with client2. */
	XSelectInput(client2, w, MASK);
/* Select for no events with client3. */
	XSelectInput(client3, w, NoEventMask);
/* Get visual of window. */
	XSync(display, True);
	if (!XGetWindowAttributes(display, w, &attrs)) {
		report("Can't get window attributes for window 0x%x", w);
		report("XGetWindowAttributes failed");
		return;
	}
	else
		CHECK;
/* Create colormap. */
	colormap = makecolmap(display, attrs.visual, AllocNone);
/* Generate ColormapNotify event. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XSetWindowColormap(display, w, colormap);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a ColormapNotify event was delivered. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xcolormap;
	good.type = EVENT;
	good.send_event = False;
	good.display = display;
	good.window = w;
	good.colormap = colormap;
	good.new = True;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;
/* Verify that a ColormapNotify event was delivered to client2. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(client2, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xcolormap;
	good.type = EVENT;
	good.send_event = False;
	good.display = client2;
	good.window = w;
	good.colormap = colormap;
	good.new = True;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event for client2");
		FAIL;
	}
	else
		CHECK;
/* Verify that no events were delivered to client3. */
	count = XPending(client3);
	if (count != 0) {
		report("Got %d events, expected %d for client3", count, 0);
		FAIL;
		return;
	}
	else
		CHECK;
	
	CHECKPASS(8);
>>ASSERTION def
>>#NOTE	Tested for in previous assertion.
When a xname event is generated,
then
clients not having set
.S ColormapChangeMask
event mask bits on the event window are not delivered
a xname event.
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
>>#NOTEm >>ASSERTION
>>#NOTEm When a client changes the colormap of a window by calling
>>#NOTEm .F XChangeWindowAttributes ,
>>#NOTEm .F XFreeColormap ,
>>#NOTEm or
>>#NOTEm .F XSetWindowColormap ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a client installs or uninstalls the colormap of a window by calling
>>#NOTEm .F XInstallColormap
>>#NOTEm or
>>#NOTEm .F XUninstallColormap ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M window
>>#NOTEs is set to
>>#NOTEs the window whose associated colormap was
>>#NOTEs changed, installed, or uninstalled.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the colormap associated with
>>#NOTEs .M window
>>#NOTEs was changed by a call to
>>#NOTEs .F XFreeColormap ,
>>#NOTEs then
>>#NOTEs .M colormap
>>#NOTEs is set to
>>#NOTEs .S None .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the colormap associated with
>>#NOTEs .M window
>>#NOTEs was not changed by a call to
>>#NOTEs .F XFreeColormap ,
>>#NOTEs then
>>#NOTEs .M colormap
>>#NOTEs is set to
>>#NOTEs the changed,
>>#NOTEs installed,
>>#NOTEs or
>>#NOTEs uninstalled colormap.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the colormap was changed,
>>#NOTEs then
>>#NOTEs .M new
>>#NOTEs is set to
>>#NOTEs .S True .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the colormap was installed or uninstalled,
>>#NOTEs then
>>#NOTEs .M new
>>#NOTEs is set to
>>#NOTEs .S False .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the colormap was installed,
>>#NOTEs then
>>#NOTEs .M state
>>#NOTEs is set to
>>#NOTEs .S ColormapInstalled .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the colormap was uninstalled,
>>#NOTEs then
>>#NOTEs .M state
>>#NOTEs is set to
>>#NOTEs .S ColormapUninstalled .
