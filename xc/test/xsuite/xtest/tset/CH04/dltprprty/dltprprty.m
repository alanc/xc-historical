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
>>TITLE XDeleteProperty CH04
void

Display *display = Dsp;
Window w = defwin(display);
Atom property = XA_COPYRIGHT;
>>EXTERN
#include "Xatom.h"
>>ASSERTION Good A
When the specified
.A property
exists on the specified window
.A w ,
then
a call to xname deletes the
.A property
and a
.S PropertyNotify
event is generated on the window
.A w .
>>STRATEGY
Create a window with a property and  PropertyChangeMask events selected. 
Call xname to delete the property.
Verify that a good PropertyNotify event occurred.
Verify that the window property was deleted.
>>CODE
char *data="a tested property";
int num = 0;
XEvent ev;

/* Create a window with a property and  PropertyChangeMask events selected.  */
	XChangeProperty(display, w, property, XA_STRING, 8,
		PropModeReplace,(unsigned char *)data, strlen(data));
	XSync(display, True);
        XSelectInput(display, w, PropertyChangeMask);

/* Call xname to delete the property. */
	XCALL;

/* Verify that a good PropertyNotify event occurred. */

	num = getevent(display, &ev);
	if (num != 1) {
		FAIL;
		report("%s caused %d events", num);
		trace("Expecting a single PropertyNotify event");
	} else {
		XEvent good;
		
		good.type = PropertyNotify;
		good.xproperty.type = PropertyNotify;
		good.xproperty.display= display;
		good.xproperty.serial = 0;
		good.xproperty.send_event = False;
		good.xproperty.window = w;
		good.xproperty.atom = property;
		good.xproperty.time = 0;
		good.xproperty.state = PropertyDelete;

#ifdef TESTING
	good.xproperty.atom--;
#endif

		if (checkevent(&good, &ev)) {
			FAIL;
		} else
			CHECK;
	}

/* Verify that the window property was deleted. */
	(void)XListProperties(display, w, &num);
	if (num != 0) {
		FAIL;
		report("%s did not delete a window property", TestName);
		trace("Expected: 0 properties");
		trace("Returned: %d propert%s", num, (num==1?"y":"ies"));
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
When the specified
.A property
does not exist on the specified window
.A w ,
then a call to xname deletes no property of the window
.A w
and no
.S PropertyNotify
event is generated.
>>STRATEGY
Create a window with a property and PropertyChangeMask events selected. 
Call xname to delete a non-existant property.
Verify that no PropertyNotify events occurred.
Verify that the window property was not deleted.
>>CODE
char *data="a tested property";
int num = 0;
XEvent ev;

/* Create a window with a property and PropertyChangeMask events selected.  */
	XChangeProperty(display, w, XA_NOTICE, XA_STRING, 8,
		PropModeReplace,(unsigned char *)data, strlen(data));
	XSync(display, True);
        XSelectInput(display, w, PropertyChangeMask);

/* Call xname to delete a non-existant property. */
#ifdef TESTING
	property = XA_NOTICE;
#endif
	XCALL;

/* Verify that no PropertyNotify events occurred. */
	if (getevent(display, &ev) != 0) {
		FAIL;
		report("%s caused unexpected event(s)", TestName);
		do {
			trace("Event: %s", eventname(ev.type));
		} while(getevent(display, &ev));
	} else
		CHECK;

/* Verify that the window property was not deleted. */
	(void)XListProperties(display, w, &num);
	if (num != 1) {
		FAIL;
		report("%s unexpectedly changed the window properties",
			TestName);
		trace("Expected: 1 property");
		trace("Returned: %d properties", num);
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad A
.ER BadAtom
>>ASSERTION Bad A
.ER BadWindow
