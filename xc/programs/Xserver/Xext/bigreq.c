/* $XConsortium: bigreq.c,v 1.2 92/09/08 15:25:16 rws Exp $ */
/*

Copyright 1992 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "bigreqstr.h"

static unsigned char XBigReqCode;
static int ProcBigReqDispatch();
static void BigReqResetProc();

void
BigReqExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();

    if (extEntry = AddExtension(XBigReqExtensionName, 0, 0,
				 ProcBigReqDispatch, ProcBigReqDispatch,
				 BigReqResetProc, StandardMinorOpcode))
	XBigReqCode = (unsigned char)extEntry->base;
}

/*ARGSUSED*/
static void
BigReqResetProc (extEntry)
    ExtensionEntry	*extEntry;
{
}

static int
ProcBigReqDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xBigReqEnableReq);
    xBigReqEnableReply rep;
    register int n;

    if (client->swapped) {
	swaps(&stuff->length, n);
    }
    if (stuff->brReqType != X_BigReqEnable)
	return BadRequest;
    REQUEST_SIZE_MATCH(xBigReqEnableReq);
    client->big_requests = TRUE;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.max_request_size = MAX_BIG_REQUEST_SIZE;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swapl(&rep.max_request_size, n);
    }
    WriteToClient(client, sizeof(xBigReqEnableReply), (char *)&rep);
    return(client->noClientException);
}
