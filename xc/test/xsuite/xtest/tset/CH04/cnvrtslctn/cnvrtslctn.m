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
 * $XConsortium: cnvrtslctn.m,v 1.5 92/06/11 16:12:31 rws Exp $
 */
>>TITLE XConvertSelection CH04
void

Display *display = Dsp;
Atom selection = XA_COPYRIGHT;
Atom target = XA_NOTICE;
Atom property = XA_INTEGER;
Window requestor = defwin(display);
Time  thetime = CurrentTime;
>>EXTERN
#include "Xatom.h"
>>ASSERTION Good A
When the specified
.A selection
has an owner, then a call to xname generates a
.S SelectionRequest 
event to the selection owner, with
.A selection ,
.A target ,
.A property ,
.A requestor ,
and
.A time
arguments passed unchanged as event structure members.
>>STRATEGY
Create a new client.
Create a window with a selection which it owns on client2.
Call xname to convert the selection.
Verify that the correct SelectionNotify event was delivered to client2
Verify no events were delivered to client1.
>>CODE
Display *client1, *client2;
Window owner;
int num_ev;
XEvent ev;

/* Create a new client. */
	client1 = display;
	client2 = opendisplay();
	if (client2 == (Display *)NULL) {
		delete("Could not create client2");
		return;
	} else
		CHECK;

/* Create a window with a selection which it owns on client2. */
	owner = defwin(client2);
	XSetSelectionOwner(client2, selection, owner, CurrentTime);
	XSync(client2, True);

/* Call xname to convert the selection. */
	XCALL;
	XSync(client1, False);
	XSync(client2, False);

/* Verify that the correct SelectionNotify event was delivered to client2 */
	num_ev = getevent(client2, &ev);
	if (num_ev != 1) {
		FAIL;
		report("%s did not cause a single SelectionNotify event",
			TestName);
		trace("Received %d events", num_ev);
		while (num_ev > 0) {
			trace("Event %s", eventname(ev.type));
			num_ev = getevent(client2, &ev);
		}
	} else {
		XEvent good;

		good.type = SelectionRequest;
		good.xselectionrequest.type = SelectionRequest;
		good.xselectionrequest.display = client2;
		good.xselectionrequest.owner = owner;
		good.xselectionrequest.requestor = requestor;
		good.xselectionrequest.selection = selection;
		good.xselectionrequest.target = target;
		good.xselectionrequest.property = property; 
		good.xselectionrequest.time	= -1;

		if (checkevent(&good, &ev)) {
			FAIL;
			report("SelectionNotify event was incorrect");
		} else
			CHECK;
	}

/* Verify no events were delivered to client1. */
	num_ev = getevent(client1, &ev);
	if (num_ev != 0) {
		FAIL;
		report("%s generated unexpected events on client1",
			TestName);
		trace("Expected 0 events");
		trace("Received %d events", num_ev);
		do {
			trace("Event: %s", eventname(ev.type));
		} while(getevent(client1, &ev));
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good A
When the specified
.A selection
has no owner, then a call to xname generates a
.S SelectionNotify 
event to the
.A requestor 
window with
.A selection ,
.A target 
and
.A time
arguments passed unchanged as event structure members, and with
.A property
set to
.S None . 
>>STRATEGY
Call xname to convert the selection.
Verify that the correct SelectionNotify event was delivered to display.
>>CODE
int num_ev;
XEvent ev;

/* Call xname to convert the selection. */
	XCALL;
	XSync(display, False);

/* Verify that the correct SelectionNotify event was delivered to display. */
	num_ev = getevent(display, &ev);
	if (num_ev != 1) {
		FAIL;
		report("%s did not cause a single SelectionNotify event",
			TestName);
		trace("Received %d events", num_ev);
		while (num_ev > 0) {
			trace("Event %s", eventname(ev.type));
			num_ev = getevent(display, &ev);
		}
	} else {
		XEvent good;

		good.type = SelectionNotify;
		good.xselection.type = SelectionNotify;
		good.xselection.display = display;
		good.xselection.requestor = requestor;
		good.xselection.selection = selection;
		good.xselection.target = target;
		good.xselection.property = None; /* is passed with no owner */
		good.xselection.time	= -1;

		if (checkevent(&good, &ev)) {
			FAIL;
			report("SelectionNotify event was incorrect");
		} else
			CHECK;
	}

	CHECKPASS(1);

>>ASSERTION Good A
The atoms
.S PRIMARY
and
.S SECONDARY
are predefined selection atoms.
>>STRATEGY
Obtain the server representation for the PRIMARY and SECONDARY atoms.
Verify that the atoms were defined.
>>CODE
Atom primary, secondary;

/* Obtain the server representation for the PRIMARY and SECONDARY atoms. */
	primary = XInternAtom(display, "PRIMARY", True);
	secondary = XInternAtom(display, "SECONDARY", True);

/* Verify that the atoms were defined. */
	if (primary == None) {
		FAIL;
		report("PRIMARY was not defined");
	} else
		CHECK;

	if (secondary == None) {
		FAIL;
		report("SECONDARY was not defined");
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad A
.ER BadWindow
>>ASSERTION Bad A
.ER BadAtom
