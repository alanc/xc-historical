/* $XConsortium: xtest.c,v 1.15 93/02/05 17:33:35 rws Exp $ */
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
#define _XTEST_SERVER_
#include "XTest.h"
#include "xteststr.h"
#ifdef XINPUT
#include "XIproto.h"
#define EXTENSION_EVENT_BASE	64
#endif /* XINPUT */

static unsigned char XTestReqCode;
static int ProcXTestDispatch(), SProcXTestDispatch();
static void XTestResetProc();
static int XTestSwapFakeInput();
CursorPtr GetSpriteCursor();
WindowPtr GetCurrentRootWindow();
#ifdef XINPUT
extern int DeviceValuator;
DeviceIntPtr LookupDeviceIntRec();
#endif /* XINPUT */

void
XTestExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();

    if (extEntry = AddExtension(XTestExtensionName, 0, 0,
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
    rep.majorVersion = XTestMajorVersion;
    rep.minorVersion = XTestMinorVersion;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swaps(&rep.minorVersion, n);
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
    REQUEST(xXTestFakeInputReq);
    int nev;
    int	n;
    xEvent *ev;
    DeviceIntPtr dev;
    WindowPtr root;
    int type;
#ifdef XINPUT
    Bool extension = FALSE;
#endif /* XINPUT */

    nev = (stuff->length << 2) - sizeof(xReq);
    if ((nev % sizeof(xEvent)) || !nev)
	return BadLength;
    nev /= sizeof(xEvent);
    UpdateCurrentTime();
    ev = (xEvent *)&((xReq *)stuff)[1];
    type = ev->u.u.type & 0177;
#ifdef XINPUT
    if (type >= EXTENSION_EVENT_BASE)
    {
	if (nev > 2)
	    return BadLength;
	type -= DeviceValuator;
	switch (type) {
	case XI_DeviceKeyPress:
	case XI_DeviceKeyRelease:
	case XI_DeviceButtonPress:
	case XI_DeviceButtonRelease:
	case XI_DeviceMotionNotify:
	case XI_ProximityIn:
	case XI_ProximityOut:
	    break;
	default:
	    client->errorValue = ev->u.u.type;
	    return BadValue;
	}
	if (nev > 1)
	{
	    if (ev[1].u.u.type != DeviceValuator)
	    {
		client->errorValue = ev[1].u.u.type;
		return BadValue;
	    }
	    if (((deviceValuator *)(ev+1))->first_valuator)
	    {
		client->errorValue =
		    ((deviceValuator *)(ev+1))->first_valuator;
		return BadValue;
	    }
	}
	else if (type == XI_DeviceMotionNotify)
	    return BadLength;
	type = type - XI_DeviceKeyPress + KeyPress;
	extension = TRUE;
    }
    else
#endif /* XINPUT */
    {
	if (nev != 1)
	    return BadLength;
	switch (type)
	{
	case KeyPress:
	case KeyRelease:
	case MotionNotify:
	case ButtonPress:
	case ButtonRelease:
	    break;
	default:
	    client->errorValue = ev->u.u.type;
	    return BadValue;
	}
    }
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
	/* swap the request back so we can simply re-execute it */
	if (client->swapped)
	{
    	    (void) XTestSwapFakeInput(client, (xReq *)stuff);
	    swaps(&stuff->length, n);
	}
	ResetCurrentRequest (client);
	client->sequence--;
	if (!ClientSleepUntil(client, &activateTime, NULL, NULL))
	{
	    /* 
	     * flush this request - must be in this order because
	     * ResetCurrentRequest adds the client back to 
	     * clientsWithInput which will cause the client to
	     * keep going, instead of waiting for the timeout.
	     */
	    (void) ReadRequestFromClient (client);
	    client->sequence++;
	    return BadAlloc;
	}
	return Success;
    }
#ifdef XINPUT
    if (extension)
    {
	dev = LookupDeviceIntRec(stuff->deviceid & 0177);
	if (!dev)
	{
	    client->errorValue = stuff->deviceid & 0177;
	    return BadValue;
	}
	if ((nev > 1 && (!dev->valuator || !dev->valuator->numAxes)) ||
	    (nev == 1 && dev->valuator && dev->valuator->numAxes))
	    return BadLength;
	if (nev > 1 && (((deviceValuator *)(ev+1))->num_valuators !=
			dev->valuator->numAxes))
	{
	    client->errorValue = ((deviceValuator *)(ev+1))->num_valuators;
	    return BadValue;
	}
    }
#endif /* XINPUT */
    switch (type)
    {
    case KeyPress:
    case KeyRelease:
#ifdef XINPUT
	if (!extension)
#endif /* XINPUT */
	    dev = (DeviceIntPtr)LookupKeyboardDevice();
	if (ev->u.u.detail < dev->key->curKeySyms.minKeyCode ||
	    ev->u.u.detail > dev->key->curKeySyms.maxKeyCode)
	{
	    client->errorValue = ev->u.u.detail;
	    return BadValue;
	}
	break;
    case MotionNotify:
#ifdef XINPUT
	if (extension)
	    break;
#endif /* XINPUT */
	dev = (DeviceIntPtr)LookupPointerDevice();
	if (ev->u.keyButtonPointer.root == None)
	    root = GetCurrentRootWindow();
	else
	{
	    root = LookupWindow(ev->u.keyButtonPointer.root, client);
	    if (!root)
		return BadWindow;
	    if (root->parent)
	    {
		client->errorValue = ev->u.keyButtonPointer.root;
		return BadValue;
	    }
	}
	if (ev->u.u.detail == xTrue)
	{
	    int x, y;
	    GetSpritePosition(&x, &y);
	    ev->u.keyButtonPointer.rootX += x;
	    ev->u.keyButtonPointer.rootY += y;
	}
	else if (ev->u.u.detail != xFalse)
	{
	    client->errorValue = ev->u.u.detail;
	    return BadValue;
	}
	if (ev->u.keyButtonPointer.rootX < 0)
	    ev->u.keyButtonPointer.rootX = 0;
	else if (ev->u.keyButtonPointer.rootX >= root->drawable.width)
	    ev->u.keyButtonPointer.rootX = root->drawable.width - 1;
	if (ev->u.keyButtonPointer.rootY < 0)
	    ev->u.keyButtonPointer.rootY = 0;
	else if (ev->u.keyButtonPointer.rootY >= root->drawable.height)
	    ev->u.keyButtonPointer.rootY = root->drawable.height - 1;
	if (root != GetCurrentRootWindow())
	{
	    NewCurrentScreen(root->drawable.pScreen,
			     ev->u.keyButtonPointer.rootX,
			     ev->u.keyButtonPointer.rootY);
	    return client->noClientException;
	}
	(*root->drawable.pScreen->SetCursorPosition)
	    (root->drawable.pScreen,
	     ev->u.keyButtonPointer.rootX,
	     ev->u.keyButtonPointer.rootY, FALSE);
	break;
    case ButtonPress:
    case ButtonRelease:
#ifdef XINPUT
	if (!extension)
#endif /* XINPUT */
	    dev = (DeviceIntPtr)LookupPointerDevice();
	if (!ev->u.u.detail || ev->u.u.detail > dev->button->numButtons)
	{
	    client->errorValue = ev->u.u.detail;
	    return BadValue;
	}
	break;
    }
    if (screenIsSaved == SCREEN_SAVER_ON)
	SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);
    ev->u.keyButtonPointer.time = currentTime.milliseconds;
    (*dev->public.processInputProc)(ev, (DevicePtr)dev, nev);
    return client->noClientException;
}

static int
ProcXTestGrabControl(client)
    register ClientPtr client;
{
    REQUEST(xXTestGrabControlReq);

    REQUEST_SIZE_MATCH(xXTestGrabControlReq);
    if ((stuff->impervious != xTrue) && (stuff->impervious != xFalse))
    {
	client->errorValue = stuff->impervious;
        return(BadValue);
    }
    if (stuff->impervious)
	MakeClientGrabImpervious(client);
    else
	MakeClientGrabPervious(client);
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
    case X_XTestGrabControl:
	return ProcXTestGrabControl(client);
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
    swaps(&stuff->minorVersion, n);
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
XTestSwapFakeInput(client, req)
    register ClientPtr	client;
    xReq *req;
{
    register int nev;
    register xEvent *ev;
    xEvent sev;
    void (*proc)(), NotImplemented();

    nev = ((req->length << 2) - sizeof(xReq)) / sizeof(xEvent);
    for (ev = (xEvent *)&req[1]; --nev >= 0; ev++)
    {
    	/* Swap event */
    	proc = EventSwapVector[ev->u.u.type & 0177];
	/* no swapping proc; invalid event type? */
    	if (!proc || (int (*)()) proc == (int (*)()) NotImplemented) {
	    client->errorValue = ev->u.u.type;
	    return BadValue;
	}
    	(*proc)(ev, &sev);
	*ev = sev;
    }
    return Success;
}

static int
SProcXTestFakeInput(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xReq);

    swaps(&stuff->length, n);
    n = XTestSwapFakeInput(client, stuff);
    if (n != Success)
	return n;
    return ProcXTestFakeInput(client);
}

static int
SProcXTestGrabControl(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXTestGrabControlReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXTestGrabControlReq);
    return ProcXTestGrabControl(client);
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
    case X_XTestGrabControl:
	return SProcXTestGrabControl(client);
    default:
	return BadRequest;
    }
}
