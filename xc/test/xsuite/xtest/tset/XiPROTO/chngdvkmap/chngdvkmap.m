/*
 * Copyright 1993 by the Hewlett-Packard Company.
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP, MIT, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP, MIT, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: chngdvkmap.m,v 1.6 94/01/29 15:27:29 rws Exp $
 */
>>TITLE ChangeDeviceKeyMapping XIPROTO
>>SET startup protostartup
>>SET cleanup protocleanup
>>EXTERN
/* Touch test for ChangeDeviceKeyMapping request */

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
extern int XInputMajorOpcode;
extern ExtDeviceInfo Devs;
static TestType test_type = SETUP;
xGetDeviceKeyMappingReq *gkmr;
xGetDeviceKeyMappingReply *gkmrep;
xChangeDeviceKeyMappingReq *req;
xEvent *ev;
xSelectExtensionEventReq *slctreq;

static
void
tester()
{
int dmn;
XEventClass dmncl;

	Create_Client(CLIENT);
	Create_Default_Window(CLIENT);
	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    Log_Err("Required extension devices are not present\n");
	    UNTESTED;
	    return;
	    }
	DeviceMappingNotify(Devs.Key, dmn, dmncl);
	slctreq = (xSelectExtensionEventReq *) Make_XInput_Req(CLIENT, X_SelectExtensionEvent);
	Send_XInput_Req(CLIENT, (xReq *) slctreq);
	Log_Trace("client %d sent default SelectExtensionEvent request\n", CLIENT);

	/* Get the existing mapping... */

	gkmr = (xGetDeviceKeyMappingReq *) Make_XInput_Req(CLIENT, X_GetDeviceKeyMapping);
	Send_XInput_Req(CLIENT, (xReq *) gkmr);
	Log_Trace("client %d sent default GetDeviceKeyMapping request\n", CLIENT);

	if ((gkmrep = (xGetDeviceKeyMappingReply *) Expect_Ext_Reply(CLIENT, X_GetDeviceKeyMapping, XInputMajorOpcode)) == NULL) {
		Log_Err("client %d failed to receive GetDeviceKeyMapping reply\n", CLIENT);
		Exit();
	}  else  {
		Log_Trace("client %d received GetDeviceKeyMapping reply\n", CLIENT);
		/* do any reply checking here */
	}
	(void) Expect_Nothing(CLIENT);

	/* ... then make a request to duplicate that mapping. */

	Set_Test_Type(CLIENT, test_type);
	req = (xChangeDeviceKeyMappingReq *) Make_XInput_Req(CLIENT, X_ChangeDeviceKeyMapping);
	if (test_type == GOOD)
	{
	req->keyCodes = gkmr->count;
	req->firstKeyCode = gkmr->firstKeyCode;
	req->keySymsPerKeyCode = gkmrep->keySymsPerKeyCode;
	req->length += req->keyCodes * req->keySymsPerKeyCode;
	req = (xChangeDeviceKeyMappingReq *) Xstrealloc((char *) req, req->length<<2);
	{
	  unsigned char *p, *q, *r;

	  p = (unsigned char *) (gkmrep + 1);
	  q = (unsigned char *) (req + 1);
	  r = ((unsigned char *) req) + (int)(req->length<<2); /* endptr */
	  while (q < r)
	    *q++ = *p++;
	}
	}

	Send_XInput_Req(CLIENT, (xReq *) req);
	Set_Test_Type(CLIENT, GOOD);
	switch(test_type) {
	case GOOD:
		Log_Trace("client %d sent default ChangeDeviceKeyMapping request\n", CLIENT);
		if ((ev = (xEvent *) Expect_Event(CLIENT, dmn)) == NULL) {
			Log_Err("client %d failed to receive devicemappingnotify\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received devicemappingnotify\n", CLIENT);
			/* do any event checking here */
			Free_Event(ev);
		}
		(void) Expect_Nothing(CLIENT);
		break;
	case BAD_LENGTH:
		Log_Trace("client %d sent ChangeDeviceKeyMapping request with bad length (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	case TOO_LONG:
	case JUST_TOO_LONG:
		Log_Trace("client %d sent overlong ChangeDeviceKeyMapping request (%d)\n", CLIENT, req->length);
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
then the X server sends back a 
.S DeviceMappingNotify 
event to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back a MappingNotify event.
Open a connection to the X server using reversed byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back a MappingNotify event.
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

>>ASSERTION Bad A
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
