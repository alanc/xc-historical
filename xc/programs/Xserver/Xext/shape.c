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

/* $XConsortium: shape.c,v 1.7 89/04/09 17:24:59 rws Exp $ */
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
CreateBoundingShape (pWin)
    WindowPtr	pWin;
{
    BoxRec	extents;

    extents.x1 = -pWin->borderWidth;
    extents.y1 = -pWin->borderWidth;
    extents.x2 = pWin->clientWinSize.width + pWin->borderWidth;
    extents.y2 = pWin->clientWinSize.height + pWin->borderWidth;
    return (*pWin->drawable.pScreen->RegionCreate) (&extents, 1);
}

static RegionPtr
CreateClipShape (pWin)
    WindowPtr	pWin;
{
    BoxRec	extents;

    extents.x1 = 0;
    extents.y1 = 0;
    extents.x2 = pWin->clientWinSize.width;
    extents.y2 = pWin->clientWinSize.height;
    return (*pWin->drawable.pScreen->RegionCreate) (&extents, 1);
}


ProcShapeQueryVersion (client)
    register ClientPtr	client;
{
    REQUEST(xShapeQueryVersionReq);
    xShapeQueryVersionReply	rep;
    register int		n;

    REQUEST_SIZE_MATCH (xShapeQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = SHAPE_MAJOR_VERSION;
    rep.minorVersion = SHAPE_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.majorVersion, n);
	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof (xShapeQueryVersionReply), (char *)&rep);
    return (client->noClientException);
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
    case ShapeBounding:
	destRgn = &pWin->boundingShape;
	createDefault = CreateBoundingShape;
	break;
    case ShapeClip:
	destRgn = &pWin->clipShape;
	createDefault = CreateClipShape;
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
	SendShapeNotify (pWin, (int)stuff->destKind);
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
    case ShapeBounding:
	destRgn = &pWin->boundingShape;
	createDefault = CreateBoundingShape;
	break;
    case ShapeClip:
	destRgn = &pWin->clipShape;
	createDefault = CreateClipShape;
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
	SendShapeNotify (pWin, (int)stuff->destKind);
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
    case ShapeBounding:
	destRgn = &pDestWin->boundingShape;
	createDefault = CreateBoundingShape;
	break;
    case ShapeClip:
	destRgn = &pDestWin->clipShape;
	createDefault = CreateClipShape;
	break;
    default:
	return BadValue;
    }
    pScreen = pDestWin->drawable.pScreen;

    pSrcWin = LookupWindow (stuff->src, client);
    if (!pSrcWin)
	return BadWindow;
    switch (stuff->srcKind) {
    case ShapeBounding:
	srcRgn = pSrcWin->boundingShape;
	createSrc = CreateBoundingShape;
	break;
    case ShapeClip:
	srcRgn = pSrcWin->clipShape;
	createSrc = CreateClipShape;
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
	SendShapeNotify (pDestWin, (int)stuff->destKind);
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
    case ShapeBounding:
	srcRgn = pWin->boundingShape;
	break;
    case ShapeClip:
	srcRgn = pWin->clipShape;
	break;
    default:
	return BadValue;
    }
    pScreen = pWin->drawable.pScreen;
    (*pScreen->TranslateRegion) (srcRgn, stuff->xOff, stuff->yOff);
    SetShape (pWin);
    SendShapeNotify (pWin, (int)stuff->destKind);
    return Success;
}

static int
ProcShapeQueryExtents (client)
    register ClientPtr	client;
{
    REQUEST(xShapeQueryExtentsReq);
    WindowPtr		pWin;
    xShapeQueryExtentsReply	rep;
    BoxRec		extents;
    register int	n;

    REQUEST_SIZE_MATCH (xShapeQueryExtentsReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.boundingShaped = (pWin->boundingShape != 0);
    rep.clipShaped = (pWin->clipShape != 0);
    if (pWin->boundingShape) {
	extents = *(pWin->drawable.pScreen->RegionExtents) (pWin->boundingShape);
    } else {
	extents.x1 = -pWin->borderWidth;
	extents.y1 = -pWin->borderWidth;
	extents.x2 = pWin->clientWinSize.width + pWin->borderWidth;
	extents.y2 = pWin->clientWinSize.height + pWin->borderWidth;
    }
    rep.xBoundingShape = extents.x1;
    rep.yBoundingShape = extents.y1;
    rep.widthBoundingShape = extents.x2 - extents.x1;
    rep.heightBoundingShape = extents.y2 - extents.y1;
    if (pWin->clipShape) {
	extents = *(pWin->drawable.pScreen->RegionExtents) (pWin->clipShape);
    } else {
	extents.x1 = 0;
	extents.y1 = 0;
	extents.x2 = pWin->clientWinSize.width;
	extents.y2 = pWin->clientWinSize.height;
    }
    rep.xClipShape = extents.x1;
    rep.yClipShape = extents.y1;
    rep.widthClipShape = extents.x2 - extents.x1;
    rep.heightClipShape = extents.y2 - extents.y1;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.xBoundingShape, n);
	swaps(&rep.yBoundingShape, n);
	swaps(&rep.widthBoundingShape, n);
	swaps(&rep.heightBoundingShape, n);
	swaps(&rep.xClipShape, n);
	swaps(&rep.yClipShape, n);
	swaps(&rep.widthClipShape, n);
	swaps(&rep.heightClipShape, n);
    }
    WriteToClient(client, sizeof (xShapeQueryExtentsReply), (char *)&rep);
    return (client->noClientException);
}

/*ARGSUSED*/
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
    pHead = (ShapeEventPtr *) LookupID 
		    	(pWin->wid, RT_WINDOW, ShapeResourceClass);
    switch (stuff->enable) {
    case xTrue:
	if (pHead) {

	    /* check for existing entry. */
	    for (pShapeEvent = *pHead;
		 pShapeEvent;
 		 pShapeEvent = pShapeEvent->next)
	    {
		if (pShapeEvent->client == client)
		    return Success;
	    }
	}

	/* build the entry */
    	pNewShapeEvent = (ShapeEventPtr)
			    xalloc (sizeof (ShapeEventRec));
    	if (!pNewShapeEvent)
	    return BadAlloc;
    	pNewShapeEvent->next = 0;
    	pNewShapeEvent->client = client;
    	pNewShapeEvent->window = pWin;
    	/*
 	 * add a resource that will be deleted when
     	 * the client goes away
     	 */
   	clientResource = FakeClientID (client->index);
    	pNewShapeEvent->clientResource = clientResource;
    	if (!AddResource (clientResource,
			  RT_FAKE, (pointer)pNewShapeEvent,
		      	  ShapeFreeClient, ShapeResourceClass))
    	{
	    xfree (pNewShapeEvent);
	    return BadAlloc;
    	}
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
	    	!AddResource (pWin->wid,
			      RT_WINDOW, (pointer)pHead,
			      ShapeFreeEvents, ShapeResourceClass))
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
	break;
    case xFalse:
	/* delete the interest */
	if (pHead) {
	    pNewShapeEvent = 0;
	    for (pShapeEvent = *pHead; pShapeEvent; pShapeEvent = pShapeEvent->next) {
		if (pShapeEvent->client == client)
		    break;
		pNewShapeEvent = 0;
	    }
	    if (pShapeEvent) {
		FreeResource (pShapeEvent->clientResource, ShapeResourceClass);
		if (pNewShapeEvent)
		    pNewShapeEvent->next = pShapeEvent->next;
		else
		    *pHead = pShapeEvent->next;
		Xfree ((pointer) pShapeEvent);
	    }
	}
	break;
    default:
	return BadValue;
    }
    return Success;
}

/*
 * deliver the event
 */

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
    BYTE		shaped;

    pHead = (ShapeEventPtr *) LookupID
		    (pWin->wid, RT_WINDOW, ShapeResourceClass);
    if (!pHead)
	return;
    if (which == ShapeBounding) {
	region = pWin->boundingShape;
	if (region) {
	    extents = *(pWin->drawable.pScreen->RegionExtents) (region);
	    shaped = xTrue;
	} else {
	    extents.x1 = -pWin->borderWidth;
	    extents.y1 = -pWin->borderWidth;
	    extents.x2 = pWin->clientWinSize.width + pWin->borderWidth;
	    extents.y2 = pWin->clientWinSize.height + pWin->borderWidth;
	    shaped = xFalse;
	}
    } else {
	region = pWin->clipShape;
	if (region) {
	    extents = *(pWin->drawable.pScreen->RegionExtents) (region);
	    shaped = xTrue;
	} else {
	    extents.x1 = 0;
	    extents.y1 = 0;
	    extents.x2 = pWin->clientWinSize.width;
	    extents.y2 = pWin->clientWinSize.height;
	    shaped = xFalse;
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
	se.shaped = shaped;
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
ProcShapeInputSelected (client)
    register ClientPtr	client;
{
    REQUEST(xShapeInputSelectedReq);
    WindowPtr		pWin;
    ShapeEventPtr	pShapeEvent, *pHead;
    int			enabled;
    xShapeInputSelectedReply	rep;
    register int		n;

    REQUEST_SIZE_MATCH (xShapeSelectInputReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    pHead = (ShapeEventPtr *) LookupID 
		    	(pWin->wid, RT_WINDOW, ShapeResourceClass);
    enabled = xFalse;
    if (pHead) {
    	for (pShapeEvent = *pHead;
	     pShapeEvent;
	     pShapeEvent = pShapeEvent->next)
    	{
	    if (pShapeEvent->client == client) {
	    	enabled = xTrue;
		break;
	    }
    	}
    }
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.enabled = enabled;
    if (client->swapped) {
	swaps (&rep.sequenceNumber, n);
	swapl (&rep.length, n);
    }
    WriteToClient (client, sizeof (xShapeInputSelectedReply), (char *) &rep);
    return (client->noClientException);
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
    case ShapeBounding:
	region = pWin->boundingShape;
	break;
    case ShapeClip:
	region = pWin->clipShape;
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
	case ShapeBounding:
	    rects->x = - (int) pWin->borderWidth;
	    rects->y = - (int) pWin->borderWidth;
	    rects->width = pWin->clientWinSize.width + pWin->borderWidth;
	    rects->height = pWin->clientWinSize.height + pWin->borderWidth;
	    break;
	case ShapeClip:
	    rects->x = 0;
	    rects->y = 0;
	    rects->width = pWin->clientWinSize.width;
	    rects->height = pWin->clientWinSize.height;
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
	SwapShorts ((short *)rects, (unsigned long)nrects * 4);
    }
    WriteToClient (client, sizeof (rep), (char *) &rep);
    WriteToClient (client, nrects * sizeof (xRectangle), (char *) rects);
    return client->noClientException;
}

int
ProcShapeDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_ShapeQueryVersion:
	return ProcShapeQueryVersion (client);
    case X_ShapeRectangles:
	return ProcShapeRectangles (client);
    case X_ShapeMask:
	return ProcShapeMask (client);
    case X_ShapeCombine:
	return ProcShapeCombine (client);
    case X_ShapeOffset:
	return ProcShapeOffset (client);
    case X_ShapeQueryExtents:
	return ProcShapeQueryExtents (client);
    case X_ShapeSelectInput:
	return ProcShapeSelectInput (client);
    case X_ShapeInputSelected:
	return ProcShapeInputSelected (client);
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
SProcShapeQueryVersion (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xShapeQueryVersionReq);

    swaps (&stuff->length, n);
    return ProcShapeQueryVersion (client);
}

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
SProcShapeQueryExtents (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeQueryExtentsReq);

    swaps (&stuff->length, n);
    swapl (&stuff->window, n);
    return ProcShapeQueryExtents (client);
}

static int
SProcShapeSelectInput (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeSelectInputReq);

    swaps (&stuff->length, n);
    swapl (&stuff->window, n);
    return ProcShapeSelectInput (client);
}

static int
SProcShapeInputSelected (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xShapeInputSelectedReq);

    swaps (&stuff->length, n);
    swapl (&stuff->window, n);
    return ProcShapeInputSelected (client);
}

static int
SProcShapeGetRectangles (client)
    register ClientPtr	client;
{
    REQUEST(xShapeGetRectanglesReq);
    register char   n;

    swaps (&stuff->length, n);
    swapl (&stuff->window, n);
    return ProcShapeGetRectangles (client);
}

int
SProcShapeDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_ShapeQueryVersion:
	return SProcShapeQueryVersion (client);
    case X_ShapeRectangles:
	return SProcShapeRectangles (client);
    case X_ShapeMask:
	return SProcShapeMask (client);
    case X_ShapeCombine:
	return SProcShapeCombine (client);
    case X_ShapeOffset:
	return SProcShapeOffset (client);
    case X_ShapeQueryExtents:
	return SProcShapeQueryExtents (client);
    case X_ShapeSelectInput:
	return SProcShapeSelectInput (client);
    case X_ShapeInputSelected:
	return SProcShapeInputSelected (client);
    case X_ShapeGetRectangles:
	return SProcShapeGetRectangles (client);
    default:
	SendErrorToClient (client, ShapeReqCode, stuff->data, (XID)0,
			   BadRequest);
	return BadRequest;
    }
}
