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
 * $XConsortium: crcltwdw.m,v 1.5 92/06/11 17:47:41 rws Exp $
 */
>>TITLE CirculateWindow XPROTO
>>SET startup protostartup
>>SET cleanup protocleanup
>>EXTERN
/* Touch test for CirculateWindow request */

/****************************************************************************
 * Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                          *
 *                                                                          *
 *                         All Rights Reserved                              *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation for any purpose and without fee is hereby granted,         *
 * provided that the above copyright notice appears in all copies and that  *
 * both that copyright notice and this permission notice appear in          *
 * supporting documentation, and that the name of Sequent not be used       *
 * in advertising or publicity pertaining to distribution or use of the     *
 * software without specific, written prior permission.                     *
 *                                                                          *
 * SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS; IN NO EVENT SHALL *
 * SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 * SOFTWARE.                                                                *
 ****************************************************************************/

#include "Xstlib.h"


#define CLIENT 0
static TestType test_type = SETUP;
Window Default_Window;
xCreateWindowReq *cwr;
xResourceReq *mwr;
xCirculateWindowReq *req;
xEvent *ev;
Bool haveSecondEvent;

static
void
tester()
{
	Create_Client(CLIENT);

	Default_Window = Create_Default_Window(CLIENT);

	cwr = (xCreateWindowReq *) Make_Req(CLIENT, X_CreateWindow);
	cwr->parent = Default_Window;
	Send_Req(CLIENT, (xReq *) cwr);
	Log_Trace("client %d sent additional CreateWindow request\n", CLIENT);
        (void) Expect_Nothing(CLIENT);
	Free_Req(cwr);

	cwr = (xCreateWindowReq *) Make_Req(CLIENT, X_CreateWindow);
	cwr->parent = Default_Window;
	Send_Req(CLIENT, (xReq *) cwr);
	Log_Trace("client %d sent additional CreateWindow request\n", CLIENT);
        (void) Expect_Nothing(CLIENT);
	Free_Req(cwr);

	mwr = (xResourceReq *) Make_Req(CLIENT, X_MapWindow);
	Send_Req(CLIENT, (xReq *) mwr);
	Log_Trace("client %d sent default MapWindow\n", CLIENT);

	if ((ev = (xEvent *) Expect_Event(CLIENT, Expose)) == NULL) {
		Log_Err("client %d failed to receive Expose\n", CLIENT);
		Exit();
	}  else  {
		Log_Trace("client %d received Expose\n", CLIENT);
		/* do any event checking here */
		Free_Event(ev);
	}
        (void) Expect_Nothing(CLIENT);
	Free_Req(mwr);

	mwr = (xResourceReq *) Make_Req(CLIENT, X_MapSubwindows);
	Send_Req(CLIENT, (xReq *) mwr);
	Log_Trace("client %d sent default MapSubwindows\n", CLIENT);

	if ((ev = (xEvent *) Expect_Event(CLIENT, Expose)) == NULL) {
		Log_Err("client %d failed to receive Expose\n", CLIENT);
		Exit();
	}  else  {
		Log_Trace("client %d received Expose\n", CLIENT);
		/* do any event checking here */
		Free_Event(ev);
	}
	if ((ev = (xEvent *) Expect_01Event(CLIENT, Expose)) == NULL) {
		haveSecondEvent = False;
	}  else  {
		haveSecondEvent = True;
		Log_Trace("client %d received second Expose\n", CLIENT);
		/* do any event checking here */
		Free_Event(ev);
	        (void) Expect_Nothing(CLIENT);
	}
	Free_Req(mwr);

	Set_Test_Type(CLIENT, test_type);
	req = (xCirculateWindowReq *) Make_Req(CLIENT, X_CirculateWindow);
	Send_Req(CLIENT, (xReq *) req);
	Set_Test_Type(CLIENT, GOOD);
	switch(test_type) {
	case GOOD:
		Log_Trace("client %d sent default CirculateWindow request\n", CLIENT);
		if (!haveSecondEvent) {
			if ((ev = (xEvent *) Expect_Event(CLIENT, Expose)) == NULL) {
				Log_Err("client %d failed to receive Expose\n", CLIENT);
				Exit();
			}  else  {
				Log_Trace("client %d received Expose\n", CLIENT);
				/* do any event checking here */
				Free_Event(ev);
			}
		}
		(void) Expect_Nothing(CLIENT);
		break;
	case BAD_LENGTH:
		Log_Trace("client %d sent CirculateWindow request with bad length (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	case TOO_LONG:
	case JUST_TOO_LONG:
		Log_Trace("client %d sent overlong CirculateWindow request (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	default:
		Log_Err("INTERNAL ERROR: test_type %d not one of GOOD(%d), BAD_LENGTH(%d), TOO_LONG(%d) or JUST_TOO_LONG(%d)\n",
			test_type, GOOD, BAD_LENGTH, TOO_LONG, JUST_TOO_LONG);
		Abort();
		/*NOTREACHED*/
		break;
	}
	Free_Req(req);
	Exit_OK();
}
>>ASSERTION Good A
When a client sends a valid xname protocol request to the X server,
and regions of formerly obscured windows are exposed,
then the X server sends back an 
.S Expose
event to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back an Expose event.
Open a connection to the X server using reversed byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back an Expose event.
>>CODE

	test_type = GOOD;

	/* Call a library function to exercise the test code */
	testfunc(tester);

>>ASSERTION Bad A
When a client sends an invalid xname protocol request to the X server,
in which the length field of the request is not the minimum length required to 
contain the request,
then the X server sends back a BadLength error to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one less than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one less than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.

Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.
>>CODE

	test_type = BAD_LENGTH; /* < minimum */

	/* Call a library function to exercise the test code */
	testfunc(tester);

	test_type = JUST_TOO_LONG; /* > minimum */

	/* Call a library function to exercise the test code */
	testfunc(tester);

>>ASSERTION Bad B 1
When a client sends an invalid xname protocol request to the X server,
in which the length field of the request exceeds the maximum length accepted
by the X server,
then the X server sends back a BadLength error to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the maximum length accepted by the server.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the maximum length accepted by the server.
Verify that the X server sends back a BadLength error.
>>CODE

	test_type = TOO_LONG;

	/* Call a library function to exercise the test code */
	testfunc(tester);
