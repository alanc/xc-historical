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
 * $XConsortium: TestSup.c,v 1.2 92/06/11 15:52:53 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

#include <stdio.h>
#include "Xstlib.h"

void
Map_Window(client,win)	/* use this routine only when exposure events are */
	int client;	/* being accepted */
	Window win;
{
	xResourceReq *mwr;
	xEvent *ev;

	mwr = (xResourceReq *) Make_Req(client, X_MapWindow);
	mwr->id = win;
	Send_Req(client, (xReq *) mwr);
	Log_Trace("sent default MapWindow\n");

	if ((ev = (xEvent *) Expect_Event(client,Expose)) == NULL) {
		Log_Err("failed to receive an Expose event\n");
		Exit();
	}  else  {
		Log_Trace("received an Expose event\n");
		/* do any event checking here */
		Free_Event(ev);
	}
        Expect_Nothing(client);
	Free_Req(mwr);
}


void
Map_a_Window(client, win)	/* use when no expose events are expected */
	int client;
	Window win;
{
	xResourceReq *req;

        req = (xResourceReq *) Make_Req(client, X_MapWindow);
        req->id = win;
	Send_Req(client, (xReq *) req);
        Log_Trace("client %d MapWindow request on window %d\n", client, win);
	(void) Expect_Nothing(client);
        Free_Req(req);
}

void
Unmap_Window(client, win)
	int client;
	Window win;
{
	xResourceReq *req;
	xEvent *ev;

        req = (xResourceReq *) Make_Req(client, X_UnmapWindow);
        req->id = win;
	Send_Req(client, (xReq *) req);
        Log_Trace("client %d UnmapWindow request on window %d\n", client, win);

	if ((ev = (xEvent *) Expect_Event(client,Expose)) == NULL) {
		Log_Err("failed to receive an Expose event\n");
		Exit();
	}  else  {
		Log_Trace("received an Expose event\n");
		/* do any event checking here */
		Free_Event(ev);
	}
	(void) Expect_Nothing(client);
        Free_Req(req);
}

void
Unmap_a_Window(client, win)
	int client;
	Window win;
{
	xResourceReq *req;

        req = (xResourceReq *) Make_Req(client, X_UnmapWindow);
        req->id = win;
	Send_Req(client, (xReq *) req);
        Log_Trace("client %d UnmapWindow request on window %d\n", client, win);
	(void) Expect_Nothing(client);
        Free_Req(req);
}

void
Set_Event_Mask(client, win, mask)
	int client;
	Window win;
	unsigned long mask;
{
	xChangeWindowAttributesReq *cwar;

        cwar = (xChangeWindowAttributesReq *) Make_Req(client, X_ChangeWindowAttributes);
        cwar = (xChangeWindowAttributesReq *) Clear_Masked_Value(cwar);
        cwar = (xChangeWindowAttributesReq *) Add_Masked_Value(cwar, CWEventMask, mask);
        cwar->window = win;
        Send_Req(client, (xReq *) cwar);
        Log_Trace("client %d sent ChangeWindowAttributes setting event mask for\n", client);
	Log_Trace("	window %d to 0x%x\n", win, mask);

        Expect_Nothing(client);
	Free_Req(cwar);
}

void
Destroy_Window(client, win)
	int client;
	Window win;
{
	xResourceReq *req;

        req = (xResourceReq *) Make_Req(client, X_DestroyWindow);
        req->id = win;
	Send_Req(client, (xReq *) req);
        Log_Trace("client %d DestroyWindow request on window %d\n", client, win);
	(void) Expect_Nothing(client);
        Free_Req(req);
}
