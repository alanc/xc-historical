/* $XConsortium: xtest.c,v 1.0 91/04/16 15:03:32 rws Exp $ */
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
#include "windowstr.h"
#include "xteststr.h"

static unsigned char XTestReqCode;
static int ProcXTestDispatch(), SProcXTestDispatch();
static void XTestResetProc();

CursorPtr GetSpriteCursor();

void
XTestExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();

    if (extEntry = AddExtension(XTESTNAME, 0, 0,
				 ProcXTestDispatch, SProcXTestDispatch,
				 XTestResetProc, StandardMinorOpcode))
	XTestReqCode = (unsigned char)extEntry->base;
}

/*ARGSUSED*/
static void
XTestResetProc (extEntry)
ExtensionEntry	*extEntry;
{
}

static int
ProcXTestGetVersion(client)
    register ClientPtr client;
{
    REQUEST(xXTestGetVersionReq);
    xXTestGetVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xXTestGetVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.major = XTEST_MAJOR;
    rep.minor = XTEST_MINOR;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swaps(&rep.minor, n);
    }
    WriteToClient(client, sizeof(xXTestGetVersionReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXTestCompareCursor(client)
    register ClientPtr client;
{
    REQUEST(xXTestCompareCursorReq);
    xXTestCompareCursorReply rep;
    WindowPtr pWin;
    CursorPtr pCursor;
    register int n;

    REQUEST_SIZE_MATCH(xXTestCompareCursorReq);
    pWin = (WindowPtr)LookupWindow(stuff->window, client);
    if (!pWin)
        return(BadWindow);
    if (stuff->cursor == XTestCurrentCursor)
	pCursor = GetSpriteCursor();
    else if (stuff->cursor)
	pCursor = (CursorPtr)LookupIDByType(stuff->cursor, RT_CURSOR);
    else if (pWin->parent)
	pCursor = wCursor(pWin->parent);
    else
	pCursor = rootCursor;
    if (!pCursor) 
    {
	client->errorValue = stuff->cursor;
	return (BadCursor);
    }
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.same = (wCursor(pWin) == pCursor);
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    }
    WriteToClient(client, sizeof(xXTestCompareCursorReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXTestDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XTestGetVersion:
	return ProcXTestGetVersion(client);
    case X_XTestCompareCursor:
	return ProcXTestCompareCursor(client);
    default:
	return BadRequest;
    }
}

static int
SProcXTestGetVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXTestGetVersionReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXTestGetVersionReq);
    swaps(&stuff->minor, n);
    return ProcXTestGetVersion(client);
}

static int
SProcXTestCompareCursor(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXTestCompareCursorReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXTestCompareCursorReq);
    swapl(&stuff->window, n);
    swapl(&stuff->cursor, n);
    return ProcXTestCompareCursor(client);
}

static int
SProcXTestDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XTestGetVersion:
	return SProcXTestGetVersion(client);
    case X_XTestCompareCursor:
	return SProcXTestCompareCursor(client);
    default:
	return BadRequest;
    }
}
