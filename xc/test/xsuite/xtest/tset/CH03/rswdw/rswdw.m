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
>>TITLE XRaiseWindow CH03
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
	"two . (10,30) 40x40",
};
static int	NExpose2Template = NELEM(Expose2Template);

>>ASSERTION Good A
A call to xname raises the specified window to the top
of the stack so that no sibling window obscures it.
>>STRATEGY
Create a window hierarchy using buildtree.
Call xname on window 'one' to raise it
Verify that window 'one' became the highest sibling window.
>>CODE
struct	buildtree	*tree;
Window	parent;

/* Create a window hierarchy using buildtree. */
	parent = defwin(display);
	tree   = buildtree(display, parent, SimpleTemplate, NSimpleTemplate);

/* Call xname on window 'one' to raise it */
	w = btntow(tree, "one");
	XCALL;

/* Verify that window 'one' became the highest sibling window. */
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
Call xname on window 'one' in order to expose window 'one'.
Use exposecheck to ensure that the window 'one' was restored correctly.
>>CODE
struct	buildtree	*tree1;
Window	parent, one;

/* Create a window hierarchy using buildtree. */
	parent =  defwin(display);
	tree1= buildtree(display, parent, Expose1Template, NExpose1Template);
	one = btntow(tree1, "zero");

/* Call setforexpose on window 'one' to allow Expose event checking. */
	setforexpose(display, one);	
	(void) buildtree(display, parent, Expose2Template, NExpose2Template);

/* Select Expose events on window 'one'. */
	XSelectInput(display, one, ExposureMask);

/* Call xname on window 'one' in order to expose window 'one'. */
	w = one;
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
When the override-redirect attribute of the window is 
.S False 
and some other client has selected 
.S SubstructureRedirectMask 
on the parent window, then a
.S ConfigureRequest 
event is generated, and no further processing is performed.
>>STRATEGY
Create client1 and client2.
Create a window hierarchy for client1.
Save parent window image as reference image.
Set override-redirect on window one to False.
Select SubstructureRedirectMask events on the parent window for client2.
Call xname on window one for client1.
Verify that no events were delivered to client1.
Verify that a correct ConfigureRequest event was delivered to client2.
Verify that no further processing occurred by comparing the window
	to our reference window.
>>CODE
Display	*client1, *client2;
Window	parent, one; 
struct	buildtree	*c1tree;
XImage	*image;
XSetWindowAttributes	attr;
XEvent	ev;
int	numevent;

/* Create client1 and client2. */
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
	one = btntow(c1tree, "one");

/* Save parent window image as reference image. */
	image = savimage(client1, parent);

/* Set override-redirect on window one to False. */
	attr.override_redirect = False;
	XChangeWindowAttributes(client1, one, CWOverrideRedirect, &attr);

/* Select SubstructureRedirectMask events on the parent window for client2. */
	XSelectInput(client2, parent, SubstructureRedirectMask);
	XSync(client2, True);

/* Call xname on window one for client1. */
	display = client1;
	w = one;
	XCALL;
	XSync(client2, False);

/* Verify that no events were delivered to client1. */
	numevent = getevent(client1, &ev);
	if (numevent != 0) {
		FAIL;
		report("%d unexpected %s delivered to client1",
			numevent, (numevent==1)?"event was":"events were");
		report("%sevent was %s", (numevent!=1)?"first ":"",
			eventname(ev.type));
		while(getevent(client1, &ev) != 0)
			report("next event was %s", eventname(ev.type));
	} else
		CHECK;

/* Verify that a correct ConfigureRequest event was delivered to client2. */
	numevent = getevent(client2, &ev);
	if (numevent != 1) {
		FAIL;
		report("Expecting a single ConfigureRequest event");
		report("Received %d events", numevent);
		if (numevent != 0) {
			report("First event was %s", eventname(ev.type));
			while(getevent(client2, &ev) != 0)
				report("next event was %s", eventname(ev.type));
		}
	} else	{
		XConfigureRequestEvent	good;

		good.type = ConfigureRequest;
		good.serial = 0;
		good.send_event = False;
		good.display = client2;
		good.parent = parent;
		good.window = one;
		good.x	= ev.xconfigure.x;
		good.y  = ev.xconfigure.y;
		good.width = ev.xconfigure.width;
		good.height = ev.xconfigure.height;
		good.border_width = ev.xconfigure.border_width;
		good.above = ev.xconfigure.above;
		good.detail= Above;
		good.value_mask = CWStackMode;

		if ( checkevent((XEvent *)&good, &ev) )
			FAIL;
		else
			CHECK;
	}

/* Verify that no further processing occurred by comparing the window */
/* 	to our reference window. */
	if (!compsavimage(client1, parent, image)) {
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(5);
>>ASSERTION Bad A
.ER BadWindow 
