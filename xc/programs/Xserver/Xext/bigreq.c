/* $XConsortium: bigreq.c,v 1.12 92/04/20 13:15:13 rws Exp $ */
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
static int ProcBigReqDispatch(), SProcBigReqDispatch();
static void BigReqResetProc();

void
BigReqExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();

    if (extEntry = AddExtension(XBigReqExtensionName, 0, 0,
				 ProcBigReqDispatch, SProcBigReqDispatch,
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
ProcBigReqGetVersion(client)
    register ClientPtr client;
{
    REQUEST(xBigReqGetVersionReq);
    xBigReqGetVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xBigReqGetVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XBigReqMajorVersion;
    rep.minorVersion = XBigReqMinorVersion;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xBigReqGetVersionReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcBigReqControl(client)
    register ClientPtr client;
{
    REQUEST(xBigReqControlReq);

    REQUEST_SIZE_MATCH(xBigReqControlReq);
    if (stuff->enable != xTrue && stuff->enable != xFalse)
    {
	client->errorValue = stuff->enable;
        return(BadValue);
    }
    client->big_requests = stuff->enable;
    return(client->noClientException);
}

static int
ProcBigReqQueryState(client)
    register ClientPtr client;
{
    REQUEST(xBigReqQueryStateReq);
    xBigReqQueryStateReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xBigReqQueryStateReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.enabled = client->big_requests;
    rep.max_request_size = MAX_BIG_REQUEST_SIZE;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swapl(&rep.max_request_size, n);
    }
    WriteToClient(client, sizeof(xBigReqQueryStateReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcBigReqDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_BigReqGetVersion:
	return ProcBigReqGetVersion(client);
    case X_BigReqControl:
	return ProcBigReqControl(client);
    case X_BigReqQueryState:
	return ProcBigReqQueryState(client);
    default:
	return BadRequest;
    }
}

static int
SProcBigReqGetVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xBigReqGetVersionReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xBigReqGetVersionReq);
    swaps(&stuff->minorVersion, n);
    return ProcBigReqGetVersion(client);
}

static int
SProcBigReqControl(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xReq);

    swaps(&stuff->length, n);
    return ProcBigReqControl(client);
}

static int
SProcBigReqQueryState(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xReq);

    swaps(&stuff->length, n);
    return ProcBigReqQueryState(client);
}

static int
SProcBigReqDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_BigReqGetVersion:
	return SProcBigReqGetVersion(client);
    case X_BigReqControl:
	return SProcBigReqControl(client);
    case X_BigReqQueryState:
	return SProcBigReqQueryState(client);
    default:
	return BadRequest;
    }
}
