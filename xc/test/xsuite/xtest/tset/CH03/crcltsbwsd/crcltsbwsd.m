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
>>TITLE XCirculateSubwindowsDown CH03
void

Display	*display = Dsp;
Window	w;
>>EXTERN

static char	*SimpleTemplate[] = {
	".",
	"zero . (20,10) 40x40",
	"one . (30,20) 40x40",
	"two . (10,30) 40x40",
	"other1 . (75,10) 15x70",
};
static int	NSimpleTemplate = NELEM(SimpleTemplate);

static char	*Expose1Template[] = {
	".",
	"zero . (20,10) 40x40",
	"one . (30,20) 40x40",
	"other1 . (75,10) 15x70",
};
static int	NExpose1Template = NELEM(Expose1Template);

static char	*Expose2Template[] = {
	".",
	"two . (40,30) 40x40",
};
static int	NExpose2Template = NELEM(Expose2Template);

>>ASSERTION Good A
When there is a mapped child that occludes another child,
then a call to xname lowers the highest such mapped child
to the bottom of the stack.
>>STRATEGY
Create a window hierarchy using buildtree.
Call xname upon the parent window.
Verify that the highest occluding mapped child window was lowered.
>>CODE
Window	parent;

/* Create a window hierarchy using buildtree. */
	parent =  defwin(display);
	(void) buildtree(display, parent, SimpleTemplate, NSimpleTemplate);

/* Call xname upon the parent window. */
	w = parent;
	XCALL;

/* Verify that the highest occluding mapped child window was lowered. */
	PIXCHECK(display, parent);

	CHECKPASS(1);

>>ASSERTION Good A
When a call to xname
uncovers part of any window that was formerly obscured, then
either
.S Expose
events are generated or the contents are restored from backing store.
>>STRATEGY
Create a window hierarchy using buildtree.
Call setforexpose on window 'one' to allow Expose event checking.
Select Expose events on window 'one'.
Call xname in order to expose window 'one' by lowering window 'two'.
Use exposecheck to ensure that the window 'one' was restored correctly.
>>CODE
struct	buildtree	*tree1;
Window	parent, one;

/* Create a window hierarchy using buildtree. */
	parent =  defwin(display);
	tree1= buildtree(display, parent, Expose1Template, NExpose1Template);
	one = btntow(tree1, "one");

/* Call setforexpose on window 'one' to allow Expose event checking. */
	setforexpose(display, one);	
	(void) buildtree(display, parent, Expose2Template, NExpose2Template);

/* Select Expose events on window 'one'. */
	XSelectInput(display, one, ExposureMask);

/* Call xname in order to expose window 'one' by lowering window 'two'. */
	w = parent;
	XCALL;

/* Use exposecheck to ensure that the window 'one' was restored correctly. */
	if (!exposecheck(display, one)) {
		report("Neither Expose events or backing store processing");
		report("could correctly restore the window contents.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
When some other client has selected 
.S SubstructureRedirectMask 
on the window, then a 
.S CirculateRequest 
event is generated, and no further processing is performed.
>>STRATEGY
Create client1 and client2 with a window on client1
Create a window hierarchy for client1.
Save parent window image as reference image.
Select all events other than SubstructureRedirectMask events on
	all the windows for client1, to catch rogue events.
Select SubstructureRedirectMask on the parent window for client2.
Call xname on client1 in order to lower window two.
Verify that no events were delivered to client1 using getevent.
Verify that a correct CirculateRequest event was delievered to client2 using getevent and checkevent.
Verify that no further processing occurred by comparing the window and our reference image.
>>CODE
Display	*client1,*client2;
struct	buildtree	*c1tree;
XImage	*image;
Window	parent, zero, one, two;
XEvent	ev;
int	numevent;

/* Create client1 and client2 with a window on client1 */
	client1 = opendisplay();
	if (client1 == NULL) {
		delete("could not create client1");
		return;
	}
	else
		CHECK;
	client2 = opendisplay();
	if (client2 == NULL) {
		delete("could not create client2");
		return;
	}
	else
		CHECK;

/* Create a window hierarchy for client1. */
	parent = defwin(client1);
	c1tree = buildtree(client1, parent, SimpleTemplate, NSimpleTemplate);
	zero = btntow(c1tree, "zero");
	one  = btntow(c1tree, "one");
	two  = btntow(c1tree, "two");

/* Save parent window image as reference image. */
	image= savimage(client1, parent);

/* Select all events other than SubstructureRedirectMask events on */
/* 	all the windows for client1, to catch rogue events. */
	XSelectInput(client1, parent, ALLEVENTS & ~(SubstructureRedirectMask));
	XSelectInput(client1, zero, ALLEVENTS & ~(SubstructureRedirectMask));
	XSelectInput(client1, one, ALLEVENTS & ~(SubstructureRedirectMask));
	XSelectInput(client1, two, ALLEVENTS & ~(SubstructureRedirectMask));

/* Select SubstructureRedirectMask on the parent window for client2. */
	XSelectInput(client2, parent, SubstructureRedirectMask);
	XSync(client2, True);

/* Call xname on client1 in order to lower window two. */
	display = client1;
	w = parent;
	XCALL;
	XSync(client2, False);

/* Verify that no events were delivered to client1 using getevent. */
	numevent = getevent(client1, &ev);
	if( numevent != 0 ) {
		FAIL;
		report("%d unexpected %s delivered to client1",
			numevent, (numevent==1)?"event was":"events were");
		report("%sevent was %s", (numevent!=1)?"first ":"",
			eventname(ev.type));
		while(getevent(client1, &ev) != 0)
			report("next event was %s", eventname(ev.type));
	} else
		CHECK;

/* Verify that a correct CirculateRequest event was delievered to client2 using getevent and checkevent. */
	numevent = getevent(client2, &ev);
	if( numevent != 1 ) {
		FAIL;
		report("Expecting a single CirculateRequest event");
		report("Received %d events", numevent);
		if(numevent != 0) {
			report("First event was %s", eventname(ev.type));
			while(getevent(client2, &ev) != 0)
				report("next event was %s", eventname(ev.type));
		}
	} else {
		XCirculateRequestEvent	good;

		good.type = CirculateRequest;
		good.serial = 0;
		good.send_event = False;
		good.display = client2;
		good.parent = parent;
		good.window = two;
		good.place = PlaceOnBottom;
		if ( checkevent((XEvent *)&good, &ev) )
			FAIL;
		else
			CHECK;
	}

/* Verify that no further processing occurred by comparing the window and our reference image. */
	if(!compsavimage(client1, parent, image)) {
		FAIL;
	} else {
		CHECK;
	}
	CHECKPASS(5);
>>ASSERTION Good A
When a child is actually restacked, then a
.S CirculateNotify 
event is generated.
>>STRATEGY
Create client1, client2, and client3, with a window on client1
Create a window hierarchy using buildtree.
Select StructureNotifyMask events on the window zero on client2.
Select SubstructureNotifyMask events on the parent window on client3.
Call xname in order to lower window zero.
Verify that a correct CirculateNotify event was delivered to 
	window two on client2 using getevent and checkevent.
Verify that a correct CirculateNotify event was delivered to the
 	parent window on client3 using getevent and checkevent.
>>CODE
Display	*client1, *client2, *client3;
struct	buildtree	*tree;
Window	parent, zero, one, two;
XEvent	ev;
int	numevent;

/* Create client1, client2, and client3, with a window on client1 */
	client1 = opendisplay();
	if (client1 == NULL) {
		delete("could not create client1");
		return;
	}
	else
		CHECK;
	client2 = opendisplay();
	if (client2 == NULL) {
		delete("could not create client2");
		return;
	}
	else
		CHECK;
	client3 = opendisplay();
	if (client3 == NULL) {
		delete("could not create client3");
		return;
	}
	else
		CHECK;
	parent = defwin(client1);

/* Create a window hierarchy using buildtree. */
	tree = buildtree(client1, parent, SimpleTemplate, NSimpleTemplate);
	zero = btntow(tree, "zero"); trace("window zero is %0x", zero);
	one  = btntow(tree, "one");  trace("window one is %0x", one);
	two  = btntow(tree, "two");  trace("window two is %0x", two);

/* Select StructureNotifyMask events on the window zero on client2. */
	XSelectInput(client2, two, StructureNotifyMask);
	XSync(client2, True);

/* Select SubstructureNotifyMask events on the parent window on client3. */
	XSelectInput(client3, parent, SubstructureNotifyMask);
	XSync(client3 , True);

/* Call xname in order to lower window zero. */
	display = client1;
	w = parent;
	XCALL;
	XSync(client2, False);
	XSync(client3, False);

/* Verify that a correct CirculateNotify event was delivered to  */
/* 	window two on client2 using getevent and checkevent. */
	numevent = getevent(client2, &ev);
	trace("%d events were queued on client2", numevent);
	if (numevent == 0) {
		FAIL;
		report("No event was delivered to client2 when the child windows");
		report("were restacked. Expecting a CirculateNotify event");
	} else {
		XCirculateEvent	good;

		trace("Checking that:");
		trace("display==client2, event==two, window==two,");
		trace("place==PlaceOnBottom");
		good.type = CirculateNotify;
		good.serial = 0;
		good.send_event = False;
		good.display = client2;
		good.event = two;
		good.window = two;
		good.place = PlaceOnBottom;
		if ( checkevent((XEvent *)&good, &ev) )
			FAIL;
		else
			CHECK;
	}

/* Verify that a correct CirculateNotify event was delivered to the */
/* 	parent window on client3 using getevent and checkevent. */
	numevent = getevent(client3, &ev);
	trace("%d events were queued on client3", numevent);
	if (numevent == 0) {
		FAIL;
		report("No event was delivered to client3 when the child windows");
		report("were restacked. Expecting a CirculateNotify event");
	} else {
		XCirculateEvent	good;

		trace("Checking that:");
		trace("display==client3, event==parent, window==two,");
		trace("place==PlaceOnBottom");
		good.type = CirculateNotify;
		good.serial = 0;
		good.send_event = False;
		good.display = client3;
		good.event = parent;
		good.window = two;
		good.place = PlaceOnBottom;
		if ( checkevent((XEvent *)&good, &ev) )
			FAIL;
		else
			CHECK;
	}

	CHECKPASS(5);

>>ASSERTION Bad A
.ER BadWindow 
