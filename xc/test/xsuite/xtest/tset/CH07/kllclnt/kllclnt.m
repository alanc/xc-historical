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
>>TITLE XKillClient CH07
void

Display	*display = Dsp;
XID 	resource;
>>ASSERTION Good A
A call to xname forces a close-down of the client
that created the specified
.A resource .
>>STRATEGY
Create a client client1.
Create several resources using client1.
Call xname with one of these resources.
Verify that all client1's resources are destroyed and infer that client1 was
destroyed.
>>CODE
Display	*client1;
Window	win;
Pixmap	pix;
Colormap	cm;
unsigned int 	width, height;

/* We must disable resource registration here to prevent attempted
   deallocation of closed connections and deleted resources. */
	regdisable(); 
	client1 = opendisplay();
	if (client1 == 0) {
		delete("Could not open display");
		regenable();
		return;
	}

	win = defwin(client1);
	pix = XCreatePixmap(client1, DRW(display), 2, 3,
		DefaultDepth(client1, DefaultScreen(client1)));
	cm = makecolmap(client1, DefaultVisual(client1, DefaultScreen(client1)), AllocNone);

	regenable();

	XSync(client1, False);
	if (isdeleted())
		return;

	resource = (XID) cm;
	XCALL;

	CATCH_ERROR(Dsp);
	getsize(Dsp, win, &width, &height);
	if (GET_ERROR(Dsp) == BadDrawable)
		CHECK;
	else {
		report("Window was not destroyed");
		FAIL;
	}

	reseterr();
	getsize(Dsp, pix, &width, &height);
	if (GET_ERROR(Dsp) == BadDrawable)
		CHECK;
	else {
		report("Pixmap was not destroyed");
		FAIL;
	}
	RESTORE_ERROR(Dsp);

	CHECKPASS(2);
>>ASSERTION Good A
When the client that created the resource has already terminated in
either
.S RetainPermanent
or
.S RetainTemporary
mode, then all the resources of that client
are destroyed.
>>STRATEGY
Create a connection, client1.
Create resources for client1.
Set close-down mode to RetainPermanent.
Close client1 connection with XCloseDisplay.
Check client1 resources still exist.
Call xname with one of the client1 resources.
Verify that all client1 resources are destroyed.
>>EXTERN

#define	N_RES	4	

>>CODE
Display	*client1;
Window	w[N_RES];
XWindowAttributes	atts;
int 	i;

/* We must disable resource registration here to prevent attempted
   deallocation of closed connections and deleted resources. */
	regdisable(); 

	client1 = opendisplay();
	if (client1 == 0) {
		delete("Could not open display");
		regenable();
		return;
	}

	w[0] = defwin(client1);
	w[1] = defwin(client1);
	w[2] = defwin(client1);
	w[3] = defwin(client1);
	regenable();

	XSetCloseDownMode(client1, RetainPermanent);
	XCloseDisplay(client1);

	CATCH_ERROR(Dsp);
	for (i = 0; i < N_RES; i++) {
		if (XGetWindowAttributes(Dsp, w[i], &atts) == False) {
			delete("RetainPermanent windows were destroyed");
			return;
		}
	}
	RESTORE_ERROR(Dsp);

	resource = w[2];
	XCALL;

	CATCH_ERROR(Dsp);
	for (i = 0; i < N_RES; i++) {
		if (XGetWindowAttributes(Dsp, w[i], &atts) == True) {
			report("RetainPermanent window was not destroyed");
			FAIL;
		} else
			CHECK;
	}
	RESTORE_ERROR(Dsp);

	CHECKPASS(N_RES);

>>ASSERTION Good A
When a
.A resource
of
.S AllTemporary
is specified, then the resources of all clients that have terminated in
.S RetainTemporary
mode are destroyed.
>>STRATEGY
Open connection client1.
Open connextion client2.
Make resources for both clients.
Set close-down mode of RetainTemporary for both.
Close both clients with XCloseDisplay.
Call xname with ID AllTemporary.
Verify that all clients 1 and 2 resources are destroyed.
>>CODE
XWindowAttributes	atts;
Display	*client1;
Display	*client2;
Window	w1;
Window	w2;

/* We must disable resource registration here to prevent attempted
   deallocation of closed connections and deleted resources. */
	regdisable(); 

	if ((client1 = opendisplay()) == NULL) {
		delete("Could not open display");
		regenable();
		return;
	}
	if ((client2 = opendisplay()) == NULL) {
		delete("Could not open display");
		regenable();
		return;
	}

	w1 = defwin(client1);
	w2 = defwin(client2);

	regenable();

	XSetCloseDownMode(client1, RetainTemporary);
	XSetCloseDownMode(client2, RetainTemporary);

	XCloseDisplay(client1);
	XCloseDisplay(client2);

	resource = AllTemporary;
	XCALL;

	CATCH_ERROR(Dsp);
	if (XGetWindowAttributes(Dsp, w1, &atts) == True) {
		report("RetainTemporary window was not destroyed");
		FAIL;
	} else
		CHECK;
	if (XGetWindowAttributes(Dsp, w2, &atts) == True) {
		report("RetainTemporary window was not destroyed");
		FAIL;
	} else
		CHECK;
	RESTORE_ERROR(Dsp);

	CHECKPASS(2);
>>ASSERTION Bad A
When the specified
.A resource
is not valid, then a
.S BadValue
error occurs.
>>STRATEGY
Get bad id using badwin().
Call xname with this value.
Verify that a BadValue error occurs.
>>CODE BadValue

	resource = badwin(Dsp);
	XCALL;

	if (geterr() == BadValue)
		PASS;
