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
>>TITLE SetModifierMapping XPROTO
>>SET startup protostartup
>>SET cleanup protocleanup
>>EXTERN
/* Touch test for SetModifierMapping request */

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
xReq *gmmr;
xGetModifierMappingReply *gmmrep;
xSetModifierMappingReq *req;
xSetModifierMappingReply *rep;
xEvent *ev;

static
void
tester()
{
	Create_Client(CLIENT);

	/* Get the existing mapping... */

	gmmr = (xReq *) Make_Req(CLIENT, X_GetModifierMapping);
	Send_Req(CLIENT, (xReq *) gmmr);
	Log_Trace("client %d sent default GetModifierMapping request\n", CLIENT);

	if ((gmmrep = (xGetModifierMappingReply *) Expect_Reply(CLIENT, X_GetModifierMapping)) == NULL) {
		Log_Err("client %d failed to receive GetModifierMapping reply\n", CLIENT);
		Exit();
	}  else  {
		Log_Trace("client %d received GetModifierMapping reply\n", CLIENT);
		/* do any reply checking here */
	}
	(void) Expect_Nothing(CLIENT);
	Free_Req(gmmr);

	/* ... then make a request to duplicate that mapping. */

	Set_Test_Type(CLIENT, test_type);
	req = (xSetModifierMappingReq *) Make_Req(CLIENT, X_SetModifierMapping);
	req->numKeyPerModifier = gmmrep->numKeyPerModifier;
	req->length += 2*req->numKeyPerModifier;
	req = (xSetModifierMappingReq *) Xstrealloc((char *) req, req->length<<2);
	{
	  unsigned char *p, *q, *r;

	  p = (unsigned char *) (gmmrep + 1);
	  q = (unsigned char *) (req + 1);
	  r = ((unsigned char *)req) + (int)(req->length<<2);
	  while (q < r)
	    *q++ = *p++;
	}

	Send_Req(CLIENT, (xReq *) req);
	Set_Test_Type(CLIENT, GOOD);
	switch(test_type) {
	case GOOD:
		Log_Trace("client %d sent default SetModifierMapping request\n", CLIENT);
		if ((ev = (xEvent *) Expect_Event(CLIENT, MappingNotify)) == NULL) {
			Log_Err("client %d failed to receive MappingNotify\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received MappingNotify\n", CLIENT);
			/* do any event checking here */
			Free_Event(ev);
		}
		if ((rep = (xSetModifierMappingReply *) Expect_Reply(CLIENT, X_SetModifierMapping)) == NULL) {
			Log_Err("client %d failed to receive SetModifierMapping reply\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received SetModifierMapping reply\n", CLIENT);
			/* do any reply checking here */
			Free_Reply(rep);
		}
		(void) Expect_Nothing(CLIENT);
		break;
	case BAD_LENGTH:
		Log_Trace("client %d sent SetModifierMapping request with bad length (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	case TOO_LONG:
	case JUST_TOO_LONG:
		Log_Trace("client %d sent overlong SetModifierMapping request (%d)\n", CLIENT, req->length);
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
then the X server sends back a reply and a 
.S MappingNotify 
event to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back a reply and a MappingNotify event.
Open a connection to the X server using reversed byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back a reply and a MappingNotify event.
>>CODE

	test_type = GOOD;

	/* Call a library function to exercise the test code */
	testfunc(tester);

>>ASSERTION Bad A
When a client sends an invalid xname protocol request to the X server,
in which the length field of the request is not the minimum length required to 
contain the request
plus zero or more keycode groups,
then the X server sends back a BadLength error to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one less than the minimum length required to contain the request + zero keycode groups.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one less than the minimum length required to contain the request + zero keycode groups.
Verify that the X server sends back a BadLength error.

Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the minimum length required to contain the request + n keycode groups.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the minimum length required to contain the request + n keycode groups.
Verify that the X server sends back a BadLength error.
>>CODE

	test_type = BAD_LENGTH; /* < minimum */

	/* Call a library function to exercise the test code */
	testfunc(tester);

	test_type = JUST_TOO_LONG; /* > minimum + n keycode groups */

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
