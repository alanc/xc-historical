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
>>TITLE BadRequest XPROTO
>>SET startup protostartup
>>SET cleanup protocleanup
>>EXTERN
/* Tests for the BadRequest protocol error */
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
xReq req;
xError *err;

static
void
bad_request()
{
	Create_Client(CLIENT);

	req.reqType = Xst_BadType;
	req.length = 1;

	Send_Req(CLIENT, (xReq *) &req);
	Log_Trace("client %d sent request with bad type\n", CLIENT);

        if ((err = Expect_Error(CLIENT, BadRequest)) == NULL) {
	        Log_Err("client %d failed to receive Request error\n", CLIENT);
		Exit();
	}  else  {
		Log_Trace("client %d received Request error\n", CLIENT);
		if (err->majorCode == Xst_BadType && err->minorCode == 0)
			Log_Trace("Op codes OK\n");
		else
			Log_Err("Unexpected Op codes (%d,%d)\n",
				err->majorCode, err->minorCode);
		Free_Error(err);
	}

	(void) Expect_Nothing(CLIENT);

	Exit_OK();
}
>>ASSERTION Good A
When a client sends an invalid protocol request to the X server,
in which the major or minor opcode does not specify a valid request,
then the X server sends back a BadRequest error to the client.
>>STRATEGY
Open a connection to the X server using native byte sex.
Send an invalid protocol request to the X server 
  with major opcode 254 and minor opcode 0.
Verify that the X server sends back a BadRequest error.
Open a connection to the X server using reversed byte sex.
Send an invalid protocol request to the X server 
  with major opcode 254 and minor opcode 0.
Verify that the X server sends back a BadRequest error.
>>CODE

	/* Call a library function to exercise the test code */
	testfunc(bad_request);
