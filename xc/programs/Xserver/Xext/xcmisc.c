/* $XConsortium: xcmisc.c,v 1.2 92/09/08 15:25:16 rws Exp $ */
/*

Copyright 1993 by the Massachusetts Institute of Technology

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
#include "xcmiscstr.h"

static unsigned char XCMiscCode;
static int ProcXCMiscDispatch(), SProcXCMiscDispatch();
static void XCMiscResetProc();

void
XCMiscExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();

    if (extEntry = AddExtension(XCMiscExtensionName, 0, 0,
				ProcXCMiscDispatch, SProcXCMiscDispatch,
				XCMiscResetProc, StandardMinorOpcode))
	XCMiscCode = (unsigned char)extEntry->base;
}

/*ARGSUSED*/
static void
XCMiscResetProc (extEntry)
    ExtensionEntry	*extEntry;
{
}

static int
ProcXCMiscGetVersion(client)
    register ClientPtr client;
{
    REQUEST(xXCMiscGetVersionReq);
    xXCMiscGetVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xXCMiscGetVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XCMiscMajorVersion;
    rep.minorVersion = XCMiscMinorVersion;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swaps(&rep.majorVersion, n);
	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xXCMiscGetVersionReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXCMiscGetXIDRange(client)
    register ClientPtr client;
{
    REQUEST(xXCMiscGetXIDRangeReq);
    xXCMiscGetXIDRangeReply rep;
    register int n;
    XID min_id, max_id;

    REQUEST_SIZE_MATCH(xXCMiscGetXIDRangeReq);
    GetXIDRange(client->index, FALSE, &min_id, &max_id);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.min_id = min_id;
    rep.max_id = max_id;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swapl(&rep.min_id, n);
	swapl(&rep.max_id, n);
    }
    WriteToClient(client, sizeof(xXCMiscGetXIDRangeReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXCMiscDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XCMiscGetVersion:
	return ProcXCMiscGetVersion(client);
    case X_XCMiscGetXIDRange:
	return ProcXCMiscGetXIDRange(client);
    default:
	return BadRequest;
    }
}

static int
SProcXCMiscGetVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXCMiscGetVersionReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXCMiscGetVersionReq);
    swaps(&stuff->majorVersion, n);
    swaps(&stuff->minorVersion, n);
    return ProcXCMiscGetVersion(client);
}

static int
SProcXCMiscGetXIDRange(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xReq);

    swaps(&stuff->length, n);
    return ProcXCMiscGetXIDRange(client);
}

static int
SProcXCMiscDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XCMiscGetVersion:
	return SProcXCMiscGetVersion(client);
    case X_XCMiscGetXIDRange:
	return SProcXCMiscGetXIDRange(client);
    default:
	return BadRequest;
    }
}
