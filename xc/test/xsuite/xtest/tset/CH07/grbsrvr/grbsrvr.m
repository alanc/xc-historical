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
>>TITLE XGrabServer CH07
void

Display	*display = Dsp;
>>EXTERN

#include	"Xatom.h"

>>ASSERTION Good A
A call to xname
disables processing of requests on all
connections apart from the one issuing this request.
>>STRATEGY
Create first connection client1.
Create second connection client2.
Call xname on client1.
Verify that requests are still processed on grabbing connection.
Create another process.
  In created process:
  Perform a XChangeProperty request for client2.
Wait sufficient time to allow client2 request to be processed.
Verify that no PropertyChange event has been produced.
Ungrab server.
Wait sufficient time to allow client2 request to be processed.
Check that event has now been received.
>>EXTERN

static Display	*client1;
static Display	*client2;

static Window	win;

>>CODE

	client1 = opendisplay();
	client2 = opendisplay();

	win = defwin(Dsp);
	XSelectInput(client1, win, PropertyChangeMask);
	XSelectInput(client2, win, PropertyChangeMask);

	if (isdeleted())
		return;

	display = client1;
	XCALL;

	/*
	 * The return value only tells us that the test has been deleted
	 * and reported as such.  There is nothing further that we can
	 * do with it.
	 */
	(void) tet_fork(cproc, pproc, 1, ~0);
	/* Test now continues in pproc */
>>EXTERN
static void
pproc()
{
int 	waittime;
XEvent	ev;
int 	pass = 0, fail = 0;

	/* Allow time for change prop to occur */
	waittime = 2*config.speedfactor + 5;
	sleep(waittime);

	if (XCheckWindowEvent(display, win, PropertyChangeMask, &ev)) {
		report("A request was processed for other than the grabbing client");
		FAIL;
	} else
		CHECK;

	/*
	 * Release the grab and wait a bit to allow the requests on the other
	 * connection to go ahead.
	 * NOTE: If the ungrab does not work then the child will time out
	 * and the tet_fork function will delete the test.  This isn't really
	 * what we want, but is not unreasonable in this case because we are
	 * testing Grab and not Ungrab.
	 */
	XUngrabServer(display);
	XSync(display, False);

	/* Allow time for change prop to occur */
	sleep(waittime);
	if (XCheckWindowEvent(display, win, PropertyChangeMask, &ev))
		CHECK;
	else {
		delete("Requests were not processed after server grab was released.");
		FAIL;
	}

	CHECKPASS(2);
}
>>EXTERN
/*
 * Perform operation on client2 while client1 has the server grabbed.
 */
static void
cproc()
{
long 	val;
Atom	name;

	val = 5;
	name = XInternAtom(client2, "name", False);
	XChangeProperty(client2, win, name, XA_INTEGER, 32, PropModeReplace,
		(unsigned char *)&val, 1);
	XFlush(client2);
	exit(0);
}
