/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

                    All Rights Reserved

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
no- tice appear in all copies and that both that copyright
no- tice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

MIT DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL MIT BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: shape.c,v 1.3 89/03/23 20:18:36 keith Exp $ */
#define NEED_REPLIES
#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#define _SHAPE_SERVER_	/* don't want Xlib structures */
#include "shapestr.h"
#include "regionstr.h"
#include "gcstruct.h"

extern void SwapShorts();

static unsigned char ShapeReqCode = 0;
static int ShapeEventBase = 0;
static unsigned short ShapeResourceClass; /* resource class for event masks */

/*
 * each window has a list of clients requesting
 * ShapeNotify events.  Each client has a resource
 * for each window it selects ShapeNotify input for,
 * this resource is used to delete the ShapeNotifyRec
 * entry from the per-window queue.
 */

typedef struct _ShapeEvent *ShapeEventPtr;

typedef struct _ShapeEvent {
    ShapeEventPtr   next;
    ClientPtr	    client;
    WindowPtr	    window;
    XID		    clientResource;
} ShapeEventRec;

/****************
 * ShapeExtensionInit
 *
 * Called from InitExtensions in main() or from QueryExtension() if the
 * extension is dynamically loaded.
 *
 ****************/

void
ShapeExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();
    int ProcShapeDispatch(), SProcShapeDispatch();
    void  ShapeResetProc();
    Atom MakeAtom();

    if (!MakeAtom(SHAPENAME, 13, TRUE))
	return;
    extEntry = AddExtension(SHAPENAME, ShapeNumberEvents, 0, ProcShapeDispatch,
			    SProcShapeDispatch, ShapeResetProc);
    if (extEntry)
    {
	ShapeReqCode = (unsigned char)extEntry->base;
	ShapeEventBase = extEntry->eventBase;
	ShapeResourceClass = CreateNewResourceClass ();
    }
}

/*ARGSUSED*/
void
ShapeResetProc (extEntry)
ExtensionEntry	*extEntry;
{
}

static
RegionOperate (pWin, destRgnp, srcRgn, op, xoff, yoff, create)
    WindowPtr	pWin;
    RegionPtr	*destRgnp, srcRgn;
    int		op;
    int		xoff, yoff;
    RegionPtr	(*create)();	/* creates a reasonable *destRgnp */
{
    int	ret = Success;
    ScreenPtr	pScreen = pWin->drawable.pScreen;

    if (xoff || yoff)
	(*pScreen->TranslateRegion) (srcRgn, xoff, yoff);
    switch (op) {
    case ShapeSet:
	if (*destRgnp)
	    (*pScreen->RegionDestroy) (*destRgnp);
	*destRgnp = srcRgn;
	srcRgn = 0;
	break;
    case ShapeUnion:
	if (*destRgnp)
	    (*pScreen->Union) (*destRgnp, *destRgnp, srcRgn);
	break;
    case ShapeIntersect:
	if (*destRgnp)
	    (*pScreen->Intersect) (*destRgnp, *destRgnp, srcRgn);
	else {
	    *destRgnp = srcRgn;
	    srcRgn = 0;
	}
	break;
    case ShapeSubtract:
	if (!*destRgnp)
	    *destRgnp = (*create)(pWin);
	(*pScreen->Subtract) (*destRgnp, *destRgnp, srcRgn);
	break;
    case ShapeInvert:
	if (!*destRgnp)
	    *destRgnp = (*pScreen->RegionCreate) ((BoxPtr) 0, 0);
	else
	    (*pScreen->Subtract) (*destRgnp, srcRgn, *destRgnp);
	break;
    default:
	ret = BadValue;
    }
    if (srcRgn)
	(*pScreen->RegionDestroy) (srcRgn);
    return ret;
}

static RegionPtr
RectsToRegion (pScreen, nrects, prect)
    ScreenPtr	pScreen;
    int		nrects;
    xRectangle	*prect;
{
    RegionPtr	result, thisrect;
    BoxRec	box;

    result = (*pScreen->RegionCreate) ((BoxPtr) NULL, 0);
    thisrect = (*pScreen->RegionCreate) ((BoxPtr) NULL, 0);
    while (nrects--) {
	box.x1 = prect->x;
	box.y1 = prect->y;
	box.x2 = box.x1 + prect->width;
	box.y2 = box.y1 + prect->height;
	if (box.x2 < box.x1)
	    box.x2 = box.x1;
	if (box.y2 < box.y1)
	    box.y2 = box.y1;
	(*pScreen->RegionReset) (thisrect, &box);
	(*pScreen->Union) (result, result, thisrect);
	++prect;
    }
    (*pScreen->RegionDestroy) (thisrect);
    return result;
}

static RegionPtr
BitmapToRegion (pScreen, pPixmap)
    ScreenPtr	pScreen;
    PixmapPtr	pPixmap;
{
    extern RegionPtr	mfbPixmapToRegion ();
    
    /* XXX this assumes that mfb is being used for 1-bit pixmaps */
    return mfbPixmapToRegion (pPixmap);
}

static RegionPtr
CreateWindowShape (pWin)
    WindowPtr	pWin;
{
    BoxRec	extents;

    extents.x1 = 0;
    extents.y1 = 0;
    extents.x2 = pWin->clientWinSize.width;
    extents.y2 = pWin->clientWinSize.height;
    return (*pWin->drawable.pScreen->RegionCreate) (&extents, 1);
}

static RegionPtr
CreateBorderShape (pWin)
    WindowPtr	pWin;
{
    BoxRec	extents;

    extents.x1 = -pWin->borderWidth;
    extents.y1 = -pWin->borderWidth;
    extents.x2 = pWin->clientWinSize.width + pWin->borderWidth;
    extents.y2 = pWin->clientWinSize.height + pWin->borderWidth;
    return (*pWin->drawable.pScreen->RegionCreate) (&extents, 1);
}

/*****************
 * ProcShapeRectangles
 *
 *****************/

static int
ProcShapeRectangles (client)
    register ClientPtr client;
{
    WindowPtr		pWin;
    ScreenPtr		pScreen;
    REQUEST(xShapeRectanglesReq);
    xRectangle		*prects;
    int		        nrects;
    RegionPtr		srcRgn;
    RegionPtr		*destRgn;
    RegionPtr		(*createDefault)();
    int			ret;

    REQUEST_AT_LEAST_SIZE (xShapeRectanglesReq);
    pWin = LookupWindow (stuff->dest, client);
    if (!pWin)
	return BadWindow;
    switch (stuff->destKind) {
    case ShapeWindow:
	destRgn = &pWin->windowShape;
	createDefault = CreateWindowShape;
	break;
    case ShapeBorder:
	if (pWin->class == InputOnly)
	    return BadMatch;
	destRgn = &pWin->borderShape;
	createDefault = CreateBorderShape;
	break;
    default:
	return BadValue;
    }
    pScreen = pWin->drawable.pScreen;
    nrects = ((stuff->length  << 2) - sizeof(xShapeRectanglesReq));
    if (nrects & 4)
	return(BadLength);
    nrects >>= 3;
    prects = (xRectangle *) &stuff[1];
    srcRgn = RectsToRegion (pScreen, nrects, prects);
    ret = RegionOperate (pWin, destRgn, srcRgn, (int)stuff->op,
			 stuff->xOff, stuff->yOff, createDefault);
    if (ret == Success) {
	SetShape (pWin);
	SendShapeNotify (pWin, stuff->destKind);
    }
    return ret;
}

/**************
 * ProcShapeMask
 **************/

ProcShapeMask (client)
    register ClientPtr client;
{
    WindowPtr		pWin;
    ScreenPtr		pScreen;
    REQUEST(xShapeMaskReq);
    RegionPtr		srcRgn;
    RegionPtr		*destRgn;
    PixmapPtr		pPixmap;
    RegionPtr		(*createDefault)();
    int			ret;

    REQUEST_SIZE_MATCH (xShapeMaskReq);
    pWin = LookupWindow (stuff->dest, client);
    if (!pWin)
	return BadWindow;
    switch (stuff->destKind) {
    case ShapeWindow:
	destRgn = &pWin->windowShape;
	createDefault = CreateWindowShape;
	break;
    case ShapeBorder:
	if (pWin->class == InputOnly)
	    return BadMatch;
	destRgn = &pWin->borderShape;
	createDefault = CreateBorderShape;
	break;
    default:
	return BadValue;
    }
    pScreen = pWin->drawable.pScreen;
    if (stuff->src == None)
	srcRgn = 0;
    else {
        pPixmap = (PixmapPtr) LookupID(stuff->src, RT_PIXMAP, RC_CORE);
        if (!pPixmap)
	    return BadPixmap;
	if (pPixmap->drawable.pScreen != pScreen)
	    return BadMatch;
	srcRgn = BitmapToRegion (pScreen, pPixmap);
    }
    ret = RegionOperate (pWin, destRgn, srcRgn, (int)stuff->op,
			 stuff->xOff, stuff->yOff, createDefault);
    if (ret == Success) {
	SetShape (pWin);
	SendShapeNotify (pWin, stuff->destKind);
    }
    return ret;
}

/************
 * ProcShapeCombine
 ************/

static
ProcShapeCombine (client)
    register ClientPtr client;
{
    WindowPtr		pSrcWin, pDestWin;
    ScreenPtr		pScreen;
    REQUEST(xShapeCombineReq);
    RegionPtr		srcRgn;
    RegionPtr		*destRgn;
    RegionPtr		(*createDefault)();
    RegionPtr		(*createSrc)();
    RegionPtr		tmp;
    int			ret;

    REQUEST_SIZE_MATCH (xShapeCombineReq);
    pDestWin = LookupWindow (stuff->dest, client);
    if (!pDestWin)
	return BadWindow;
    switch (stuff->destKind) {
    case ShapeWindow:
	destRgn = &pDestWin->windowShape;
	createDefault = CreateWindowShape;
	break;
    case ShapeBorder:
	if (pDestWin->class == InputOnly)
	    return BadMatch;
	destRgn = &pDestWin->borderShape;
	createDefault = CreateBorderShape;
	break;
    default:
	return BadValue;
    }
    pScreen = pDestWin->drawable.pScreen;

    pSrcWin = LookupWindow (stuff->src, client);
    if (!pSrcWin)
	return BadWindow;
    switch (stuff->srcKind) {
    case ShapeWindow:
	srcRgn = pSrcWin->windowShape;
	createSrc = CreateWindowShape;
	break;
    case ShapeBorder:
	if (pSrcWin->class == InputOnly)
	    return BadMatch;
	srcRgn = pSrcWin->borderShape;
	createSrc = CreateBorderShape;
	break;
    default:
	return BadValue;
    }
    if (pSrcWin->drawable.pScreen != pScreen)
	return BadMatch;

    if (srcRgn) {
        tmp = (*pScreen->RegionCreate) ((BoxPtr) 0, 0);
        (*pScreen->RegionCopy) (tmp, srcRgn);
        srcRgn = tmp;
    } else
	srcRgn = (*createSrc) (pSrcWin);

    ret = RegionOperate (pDestWin, destRgn, srcRgn, (int)stuff->op,
			 stuff->xOff, stuff->yOff, createDefault);
    if (ret == Success) {
	SetShape (pDestWin);
	SendShapeNotify (pDestWin, stuff->destKind);
    }
    return ret;
}

/*************
 * ProcShapeOffset
 *************/

static
ProcShapeOffset (client)
    register ClientPtr client;
{
    WindowPtr		pWin;
    ScreenPtr		pScreen;
    REQUEST(xShapeOffsetReq);
    RegionPtr		srcRgn;

    REQUEST_SIZE_MATCH (xShapeOffsetReq);
    pWin = LookupWindow (stuff->dest, client);
    if (!pWin)
	return BadWindow;
    switch (stuff->destKind) {
    case ShapeWindow:
	srcRgn = pWin->windowShape;
	break;
    case ShapeBorder:
	if (pWin->class == InputOnly)
	    return BadMatch;
	srcRgn = pWin->borderShape;
	break;
    default:
	return BadValue;
    }
    pScreen = pWin->drawable.pScreen;
    (*pScreen->TranslateRegion) (srcRgn);
    SetShape (pWin);
    SendShapeNotify (pWin, stuff->destKind);
    return Success;
}

static int
ProcShapeQuery (client)
    register ClientPtr	client;
{
    REQUEST(xShapeQueryReq);
    WindowPtr		pWin;
    xShapeQueryReply	rep;
    BoxRec		extents;
    register int	n;

    REQUEST_SIZE_MATCH (xShapeQueryReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.windowShaped = (pWin->windowShape != 0);
    rep.borderShaped = (pWin->borderShape != 0);
    if (pWin->windowShape) {
	extents = *(pWin->drawable.pScreen->RegionExtents) (pWin->windowShape);
    } else {
	extents.x1 = 0;
	extents.y1 = 0;
	extents.x2 = pWin->clientWinSize.width;
	extents.y2 = pWin->clientWinSize.height;
    }
    rep.xWindowShape = extents.x1;
    rep.yWindowShape = extents.y1;
    rep.widthWindowShape = extents.x2 - extents.x1;
    rep.heightWindowShape = extents.y2 - extents.y1;
    if (pWin->borderShape) {
	extents = *(pWin->drawable.pScreen->RegionExtents) (pWin->borderShape);
    } else {
	extents.x1 = -pWin->borderWidth;
	extents.y1 = -pWin->borderWidth;
	extents.x2 = pWin->clientWinSize.width + pWin->borderWidth;
	extents.y2 = pWin->clientWinSize.height + pWin->borderWidth;
    }
    rep.xBorderShape = extents.x1;
    rep.yBorderShape = extents.y1;
    rep.widthBorderShape = extents.x2 - extents.x1;
    rep.heightBorderShape = extents.y2 - extents.y1;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.xWindowShape, n);
	swaps(&rep.yWindowShape, n);
	swaps(&rep.widthWindowShape, n);
	swaps(&rep.heightWindowShape, n);
	swaps(&rep.xBorderShape, n);
	swaps(&rep.yBorderShape, n);
	swaps(&rep.widthBorderShape, n);
	swaps(&rep.heightBorderShape, n);
    }
    WriteToClient(client, sizeof (xShapeQueryReply), (char *)&rep);
    return (client->noClientException);
}

static int
ShapeFreeClient (data, id)
    pointer	    data;
    XID		    id;
{
    ShapeEventPtr   pShapeEvent;
    WindowPtr	    pWin;
    ShapeEventPtr   *pHead, pCur, pPrev;

    pShapeEvent = (ShapeEventPtr) data;
    pWin = pShapeEvent->window;
    pHead = (ShapeEventPtr *) LookupID
			(pWin->wid, RT_WINDOW, ShapeResourceClass);
    if (pHead) {
	pPrev = 0;
	for (pCur = *pHead; pCur && pCur != pShapeEvent; pCur=pCur->next)
	    pPrev = pCur;
	if (pPrev)
	    pPrev->next = pShapeEvent->next;
	else
	    *pHead = pShapeEvent->next;
    }
    xfree ((pointer) pShapeEvent);
}

/*ARGSUSED*/
static int
ShapeFreeEvents (data, id)
    pointer	    data;
    XID		    id;
{
    ShapeEventPtr   *pHead, pCur, pNext;

    pHead = (ShapeEventPtr *) data;
    for (pCur = *pHead; pCur; pCur = pNext) {
	pNext = pCur->next;
	FreeResource (pCur->clientResource, ShapeResourceClass);
	xfree ((pointer) pCur);
    }
    xfree ((pointer) pHead);
}

static int
ProcShapeSelectInput (client)
    register ClientPtr	client;
{
    REQUEST(xShapeSelectInputReq);
    WindowPtr		pWin;
    ShapeEventPtr	pShapeEvent, pNewShapeEvent, *pHead;
    XID			clientResource;

    REQUEST_SIZE_MATCH (xShapeSelectInputReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    pNewShapeEvent = (ShapeEventPtr) xalloc (sizeof (ShapeEventRec));
    if (!pNewShapeEvent)
	return BadAlloc;
    pNewShapeEvent->next = 0;
    pNewShapeEvent->client = client;
    pNewShapeEvent->window = pWin;
    /* add a resource that will be deleted when
     * the client goes away
     */
    clientResource = FakeClientID (client->index);
    pNewShapeEvent->clientResource = clientResource;
    if (!AddResource (clientResource, RT_FAKE, (pointer)pNewShapeEvent,
		      (int (*)())ShapeFreeClient, ShapeResourceClass))
    {
	xfree (pNewShapeEvent);
	return BadAlloc;
    }
    pHead = (ShapeEventPtr *) LookupID 
			    (pWin->wid, RT_WINDOW, ShapeResourceClass);
    /*
     * create a resource to contain a pointer to the list
     * of clients selecting input.  This must be indirect as
     * the list may be arbitrarily rearranged which cannot be
     * done through the resource database.
     */
    if (!pHead)
    {
	pHead = (ShapeEventPtr *) xalloc (sizeof (ShapeEventPtr));
	if (!pHead ||
	    !AddResource (pWin->wid, RT_WINDOW, (pointer)pHead,
			  (int(*)())ShapeFreeEvents, ShapeResourceClass))
	{
	    FreeResource (clientResource, ShapeResourceClass);
	    xfree (pHead);
	    xfree (pNewShapeEvent);
	    return BadAlloc;
	}
	*pHead = 0;
    }
    pNewShapeEvent->next = *pHead;
    *pHead = pNewShapeEvent;
    return Success;
}

SendShapeNotify (pWin, which)
    WindowPtr	pWin;
    int		which;
{
    ShapeEventPtr	*pHead, pShapeEvent;
    ClientPtr		client;
    xShapeNotifyEvent	se;
    BoxRec		extents;
    RegionPtr		region;
    register int	n;

    pHead = (ShapeEventPtr *) LookupID
		    (pWin->wid, RT_WINDOW, ShapeResourceClass);
    if (!pHead)
	return;
    if (which == ShapeWindow) {
	region = pWin->windowShape;
	if (region)
	    extents = *(pWin->drawable.pScreen->RegionExtents) (region);
	else {
	    extents.x1 = 0;
	    extents.y1 = 0;
	    extents.x2 = pWin->clientWinSize.width;
	    extents.y2 = pWin->clientWinSize.height;
	}
    } else {
	region = pWin->borderShape;
	if (region)
	    extents = *(pWin->drawable.pScreen->RegionExtents) (region);
	else {
	    extents.x1 = -pWin->borderWidth;
	    extents.y1 = -pWin->borderWidth;
	    extents.x2 = pWin->clientWinSize.width + pWin->borderWidth;
	    extents.y2 = pWin->clientWinSize.height + pWin->borderWidth;
	}
    }
    for (pShapeEvent = *pHead; pShapeEvent; pShapeEvent = pShapeEvent->next) {
	client = pShapeEvent->client;
	se.type = ShapeNotify + ShapeEventBase;
	se.kind = which;
	se.window = pWin->wid;
	se.sequenceNumber = client->sequence;
	se.x = extents.x1;
	se.y = extents.y1;
	se.width = extents.x2 - extents.x1;
	se.height = extents.y2 - extents.y1;
	se.time = currentTime.milliseconds;
	if (client->swapped) {
	    swapl (&se.window, n);
	    swapl (&se.sequenceNumber, n);
	    swaps (&se.x, n);
	    swaps (&se.y, n);
	    swaps (&se.width, n);
	    swaps (&se.height, n);
	    swapl (&se.time, n);
	}
	if (client != serverClient && !client->clientGone)
	    WriteEventsToClient (client, 1, &se);
    }
}

static int
ProcShapeGetRectangles (client)
    register ClientPtr	client;
{
    REQUEST(xShapeGetRectanglesReq);
    WindowPtr			pWin;
    xShapeGetRectanglesReply	rep;
    xRectangle			*rects;
    int				nrects, i;
    RegionPtr			region;
    register int		n;

    REQUEST_SIZE_MATCH(xShapeGetRectanglesReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    switch (stuff->kind) {
    case ShapeWindow:
	region = pWin->windowShape;
	break;
    case ShapeBorder:
	region = pWin->borderShape;
	break;
    default:
	return BadValue;
    }
    if (!region) {
	nrects = 1;
	rects = (xRectangle *) ALLOCATE_LOCAL (sizeof (xRectangle));
	if (!rects)
	    return BadAlloc;
	switch (stuff->kind) {
	case ShapeWindow:
	    rects->x = 0;
	    rects->y = 0;
	    rects->width = pWin->clientWinSize.width;
	    rects->height = pWin->clientWinSize.height;
	    break;
	case ShapeBorder:
	    rects->x = - (int) pWin->borderWidth;
	    rects->y = - (int) pWin->borderWidth;
	    rects->width = pWin->clientWinSize.width + pWin->borderWidth;
	    rects->height = pWin->clientWinSize.height + pWin->borderWidth;
	    break;
	}
    } else {
	nrects = region->numRects;
	rects = (xRectangle *) ALLOCATE_LOCAL (nrects * sizeof (xRectangle));
	if (!rects)
	    return BadAlloc;
	for (i = 0; i < nrects; i++) {
	    rects[i].x = region->rects[i].x1;
	    rects[i].y = region->rects[i].y1;
	    rects[i].width = region->rects[i].x2 - region->rects[i].x1;
	    rects[i].height = region->rects[i].y2 - region->rects[i].y1;
	}
	DEALLOCATE_LOCAL ((pointer) rects);
    }
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = (nrects * sizeof (xRectangle)) >> 2;
    rep.nrects = nrects;
    if (client->swapped) {
	swaps (&rep.sequenceNumber, n);
	swapl (&rep.length, n);
	swapl (&rep.nrects, n);
	SwapShorts (rects, nrects * 4);
    }
    WriteToClient (client, (char *) &rep, sizeof (rep));
    WriteToClient (client, (char *) rects, nrects * sizeof (xRectangle));
    return client->noClientException;
}

int
ProcShapeDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_ShapeRectangles:
	return ProcShapeRectangles (client);
    case X_ShapeMask:
	return ProcShapeMask (client);
    case X_ShapeCombine:
	return ProcShapeCombine (client);
    case X_ShapeOffset:
	return ProcShapeOffset (client);
    case X_ShapeQuery:
	return ProcShapeQuery (client);
    case X_ShapeSelectInput:
	return ProcShapeSelectInput (client);
    case X_ShapeGetRectangles:
	return ProcShapeGetRectangles (client);
    default:
	SendErrorToClient (client, ShapeReqCode, stuff->data, (XID)0,
			   BadRequest);
	return BadRequest;
    }
}

/* Macros needed for byte-swapping, copied from swapreq.c.  Really
   should be in a header file somewhere. */

#define LengthRestS(stuff) \
    ((stuff->length << 1)  - (sizeof(*stuff) >> 1))

#define SwapRestS(stuff) \
    SwapShorts((short *)(stuff + 1), (unsigned long)LengthRestS(stuff))

static int
SProcShapeRectangles (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeRectanglesReq);

    swaps (&stuff->length, n);
    swapl (&stuff->dest, n);
    swaps (&stuff->xOff, n);
    swaps (&stuff->yOff, n);
    SwapRestS(stuff);
    return ProcShapeRectangles (client);
}

static int
SProcShapeMask (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeMaskReq);

    swaps (&stuff->length, n);
    swapl (&stuff->dest, n);
    swaps (&stuff->xOff, n);
    swaps (&stuff->yOff, n);
    swapl (&stuff->src, n);
    return ProcShapeMask (client);
}

static int
SProcShapeCombine (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeCombineReq);

    swaps (&stuff->length, n);
    swapl (&stuff->dest, n);
    swaps (&stuff->xOff, n);
    swaps (&stuff->yOff, n);
    swapl (&stuff->src, n);
    return ProcShapeCombine (client);
}

static int
SProcShapeOffset (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeOffsetReq);

    swaps (&stuff->length, n);
    swapl (&stuff->dest, n);
    swaps (&stuff->xOff, n);
    swaps (&stuff->yOff, n);
    return ProcShapeOffset (client);
}

static int
SProcShapeQuery (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeQueryReq);

    swapl (&stuff->window, n);
    return ProcShapeQuery (client);
}

static int
SProcShapeSelectInput (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeSelectInputReq);

    swapl (&stuff->window, n);
    return ProcShapeSelectInput (client);
}

static int
SProcShapeGetRectangles (client)
    register ClientPtr	client;
{
    REQUEST(xShapeGetRectanglesReq);
    register char   n;

    swapl (&stuff->window, n);
    return ProcShapeGetRectangles (client);
}

int
SProcShapeDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_ShapeRectangles:
	return SProcShapeRectangles (client);
    case X_ShapeMask:
	return SProcShapeMask (client);
    case X_ShapeCombine:
	return SProcShapeCombine (client);
    case X_ShapeOffset:
	return SProcShapeOffset (client);
    case X_ShapeQuery:
	return SProcShapeQuery (client);
    case X_ShapeSelectInput:
	return SProcShapeSelectInput (client);
    case X_ShapeGetRectangles:
	return SProcShapeGetRectangles (client);
    default:
	SendErrorToClient (client, ShapeReqCode, stuff->data, (XID)0,
			   BadRequest);
	return BadRequest;
    }
}

