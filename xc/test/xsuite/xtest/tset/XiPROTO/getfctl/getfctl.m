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
 * $XConsortium: getfctl.m,v 1.3 94/01/29 15:29:04 rws Exp $
 */
>>TITLE GetFeedbackControl XIPROTO
>>SET startup protostartup
>>SET cleanup protocleanup
>>EXTERN

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
static TestType test_type = SETUP;
xGetFeedbackControlReq *req;
xGetFeedbackControlReply *rep;

static
void
tester()
{
	Create_Client(CLIENT);

	Set_Test_Type(CLIENT, test_type);
	req = (xGetFeedbackControlReq *) Make_XInput_Req(CLIENT, X_GetFeedbackControl);

	Send_XInput_Req(CLIENT, (xReq *) req);
	Set_Test_Type(CLIENT, GOOD);
	switch(test_type) {
	case GOOD:
		Log_Trace("client %d sent default GetFeedbackControl request\n", CLIENT);
		if ((rep = (xGetFeedbackControlReply *) Expect_Ext_Reply(CLIENT, X_GetFeedbackControl, XInputMajorOpcode)) == NULL) {
			Log_Err("client %d failed to receive GetFeedbackControl reply\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received GetFeedbackControl reply\n", CLIENT);
			/* do any reply checking here */
			Free_Reply(rep);
		}
		(void) Expect_Nothing(CLIENT);
		break;
	case BAD_LENGTH:
		Log_Trace("client %d sent GetFeedbackControl request with bad length (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	case TOO_LONG:
	case JUST_TOO_LONG:
		Log_Trace("client %d sent overlong GetFeedbackControl request (%d)\n", CLIENT, req->length);
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
then the X server sends back a reply to the client
with the minimum required length.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back a reply.
Open a connection to the X server using reversed byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back a reply.
>>CODE
int status=-1;

	if (SetFeedbackInfo (KFeedMask,0))
	    {
	    test_type = GOOD;
	    /* Call a library function to exercise the test code */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (PFeedMask,0))
	    {
	    test_type = GOOD;
	    /* Call a library function to exercise the test code */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (IFeedMask,0))
	    {
	    test_type = GOOD;
	    /* Call a library function to exercise the test code */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (SFeedMask,0))
	    {
	    test_type = GOOD;
	    /* Call a library function to exercise the test code */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (BFeedMask,0))
	    {
	    test_type = GOOD;
	    /* Call a library function to exercise the test code */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (LFeedMask,0))
	    {
	    test_type = GOOD;
	    /* Call a library function to exercise the test code */
	    testfunc(tester);
	    status=0;
	    }
	if (status==0)
	    PASS;
	else
	    UNTESTED;

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
int status=-1;

	if (SetFeedbackInfo (KFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (PFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (IFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (SFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (BFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (LFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }
	if (status==0)
	    PASS;
	else
	    UNTESTED;

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
int status=-1;

	if (SetFeedbackInfo (KFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (PFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (IFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (SFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (BFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }

	if (SetFeedbackInfo (LFeedMask,0))
	    {
	    test_type = BAD_LENGTH; /* < minimum */
	    /* Call a library function to exercise the test code */
	    testfunc(tester);

	    test_type = JUST_TOO_LONG; /* > minimum */
	    testfunc(tester);
	    status=0;
	    }
	if (status==0)
	    PASS;
	else
	    UNTESTED;
