/* $XConsortium: xcmisc.c,v 1.2 93/10/23 12:04:41 rws Exp $ */
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
extern void Swap32Write(); /* XXX should be in header file */

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
    rep.start_id = min_id;
    rep.count = max_id - min_id + 1;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swapl(&rep.start_id, n);
	swapl(&rep.count, n);
    }
    WriteToClient(client, sizeof(xXCMiscGetXIDRangeReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXCMiscGetXIDList(client)
    register ClientPtr client;
{
    REQUEST(xXCMiscGetXIDListReq);
    xXCMiscGetXIDListReply rep;
    register int n;
    XID *pids;
    unsigned int count;

    REQUEST_SIZE_MATCH(xXCMiscGetXIDListReq);

    pids = (XID *)ALLOCATE_LOCAL(stuff->count * sizeof(XID));
    if (!pids)
    {
	return BadAlloc;
    }
    count = GetXIDList(client, stuff->count, pids);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = count;
    rep.count = count;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.count, n);
    }
    WriteToClient(client, sizeof(xXCMiscGetXIDListReply), (char *)&rep);
    if (count)
    {
    	client->pSwapReplyFunc = Swap32Write;
	WriteSwappedDataToClient(client, count * sizeof(XID), pids);
    }
    DEALLOCATE_LOCAL(pids);
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
    case X_XCMiscGetXIDList:
	return ProcXCMiscGetXIDList(client);
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
SProcXCMiscGetXIDList(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXCMiscGetXIDListReq);

    swaps(&stuff->length, n);
    swapl(&stuff->count, n);
    return ProcXCMiscGetXIDList(client);
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
    case X_XCMiscGetXIDList:
	return SProcXCMiscGetXIDList(client);
    default:
	return BadRequest;
    }
}
