/* $XConsortium: xtest.c,v 1.2 92/01/27 11:41:39 rws Exp $ */
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
#define NEED_EVENTS
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "windowstr.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "xteststr.h"

static unsigned char XTestReqCode;
static int ProcXTestDispatch(), SProcXTestDispatch();
static void XTestResetProc();
static void XTestSwapFakeInput();
CursorPtr GetSpriteCursor();
WindowPtr GetCurrentRootWindow();

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
    if (stuff->cursor == None)
	pCursor = NullCursor;
    else if (stuff->cursor == XTestCurrentCursor)
	pCursor = GetSpriteCursor();
    else {
	pCursor = (CursorPtr)LookupIDByType(stuff->cursor, RT_CURSOR);
	if (!pCursor) 
	{
	    client->errorValue = stuff->cursor;
	    return (BadCursor);
	}
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
ProcXTestFakeInput(client)
    register ClientPtr client;
{
    REQUEST(xReq);
    int nev;
    xEvent *ev;
    DeviceIntPtr dev;
    WindowPtr root;

    nev = (stuff->length << 2) - sizeof(xReq);
    if (nev % sizeof(xEvent))
	return BadLength;
    nev /= sizeof(xEvent);
    if (nev != 1)
	return BadLength; /* for now */
    ev = (xEvent *)&stuff[1];
    switch (ev->u.u.type)
    {
    case KeyPress:
    case KeyRelease:
	dev = (DeviceIntPtr)LookupKeyboardDevice();
	if (ev->u.u.detail < dev->key->curKeySyms.minKeyCode ||
	    ev->u.u.detail > dev->key->curKeySyms.maxKeyCode)
	{
	    client->errorValue = ev->u.u.detail;
	    return BadValue;
	}
	break;
    case MotionNotify:
	dev = (DeviceIntPtr)LookupPointerDevice();
	root = LookupWindow(ev->u.keyButtonPointer.root, client);
	if (!root)
	    return BadWindow;
	if (root->parent)
	{
	    client->errorValue = ev->u.keyButtonPointer.root;
	    return BadValue;
	}
	if (ev->u.keyButtonPointer.rootX < 0 ||
	    ev->u.keyButtonPointer.rootX > root->drawable.width)
	{
	    client->errorValue = ev->u.keyButtonPointer.rootX;
	    return BadValue;
	}
	if (ev->u.keyButtonPointer.rootY < 0 ||
	    ev->u.keyButtonPointer.rootY > root->drawable.height)
	{
	    client->errorValue = ev->u.keyButtonPointer.rootY;
	    return BadValue;
	}
	if (root != GetCurrentRootWindow())
	    NewCurrentScreen(root->drawable.pScreen,
			     ev->u.keyButtonPointer.rootX,
			     ev->u.keyButtonPointer.rootY);
	else
	    (*root->drawable.pScreen->SetCursorPosition)
		(root->drawable.pScreen,
		 ev->u.keyButtonPointer.rootX,
		 ev->u.keyButtonPointer.rootY, FALSE);
	break;
    case ButtonPress:
    case ButtonRelease:
	dev = (DeviceIntPtr)LookupPointerDevice();
	if (ev->u.u.detail == 0 || ev->u.u.detail > dev->button->numButtons)
	{
	    client->errorValue = ev->u.u.detail;
	    return BadValue;
	}
	break;
    default:
	client->errorValue = ev->u.u.type;
	return BadValue;
    }
    UpdateCurrentTime();
    if (ev->u.keyButtonPointer.time)
    {
	TimeStamp activateTime;
	CARD32 ms;

	activateTime = currentTime;
	ms = activateTime.milliseconds + ev->u.keyButtonPointer.time;
	if (ms < activateTime.milliseconds)
	    activateTime.months++;
	activateTime.milliseconds = ms;
	ev->u.keyButtonPointer.time = 0;
#ifdef notyet
	ClientSleepUntil(client, &activateTime, SwapXTestFakeInput);
#endif
    }
    ev->u.keyButtonPointer.time = currentTime.milliseconds;
    (*dev->public.processInputProc)(ev, (DevicePtr)dev, 1); 
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
    case X_XTestFakeInput:
	return ProcXTestFakeInput(client);
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

static void
XTestSwapFakeInput(req)
    xReq *req;
{
    register int n;
    register int nev;
    register xEvent *ev;

    nev = ((req->length << 2) - sizeof(xReq)) / sizeof(xEvent);
    for (ev = (xEvent *)&req[1]; --nev >= 0; ev++)
    {
	swapl(&ev->u.keyButtonPointer.time, n);
	swapl(&ev->u.keyButtonPointer.root, n);
	swaps(&ev->u.keyButtonPointer.rootX, n);
    }
}

static int
SProcXTestFakeInput(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xReq);

    swaps(&stuff->length, n);
    XTestSwapFakeInput(stuff);
    return ProcXTestFakeInput(client);
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
    case X_XTestFakeInput:
	return SProcXTestFakeInput(client);
    default:
	return BadRequest;
    }
}
