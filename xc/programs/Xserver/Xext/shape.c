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
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: Exp $ */
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
#include "shape.h"
#include "regionstr.h"
#include "gcstruct.h"

static int ShapeReqCode = 0;
static short ShapeClass = 0;

extern void (*ReplySwapVector[256]) ();
extern void CopySwap16Write ();

/****************
 * ShapeExtensionInit
 *
 * Called from InitExtensions in main() or from QueryExtension() if the
 * extension is dynamically loaded.
 *
 * SHAPE has no events or errors (other than the core errors)
 ****************/

void
ShapeExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();
    int ProcShapeDispatch(), SProcShapeDispatch();
    void  ShapeResetProc();

    extEntry = AddExtension(SHAPENAME, 0, 0, ProcShapeDispatch,
		   SProcShapeDispatch, ShapeResetProc);
    if (extEntry)
    {
	ShapeReqCode = extEntry->base;
	(void)MakeAtom(SHAPENAME, 13, TRUE);
	ShapeClass = CreateNewResourceClass();

    } else {
	FatalError("ShapeExtensionInit: AddExtensions failed\n");
    }
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

static int
RegionToRectangles (pScreen, pRegion, pprect)
    ScreenPtr	pScreen;
    RegionPtr	pRegion;
    xRectangle	**pprect;
{
    int		nrects, n;
    BoxPtr	box;
    xRectangle	*prect;

    /* XXX this knows the structure of mi regions. */
    nrects = pRegion->numRects;
    if (!nrects)
	return 0;
    prect = (xRectangle *) xalloc (nrects * sizeof (xRectangle));
    if (!prect)
	return 0;
    *pprect = prect;
    box = (BoxPtr) pRegion->rects;
    for (n = 0; n < nrects; n++) {
	prect->x = box->x1;
	prect->y = box->y1;
	prect->width = box->x2 - box->x1;
	prect->height = box->y2 - box->y1;
	++box;
	++prect;
    }
    return nrects;
}

static RegionPtr
BitmapToRegion (pPixmap)
    PixmapPtr	pPixmap;
{
    extern RegionPtr	mfbPixmapToRegion ();
    
    /* XXX this assumes that mfb is being used for 1-bit pixmaps */
    return mfbPixmapToRegion (pPixmap);
}

static void
RegionToBitmap (pRegion, pPixmap, xOff, yOff)
    RegionPtr	pRegion;
    PixmapPtr	pPixmap;
{
    GCPtr	pGC;
    ScreenPtr	pScreen = pPixmap->drawable.pScreen;
    BoxRec	extents;
    xRectangle	rect;
    RegionPtr	pCopy;

    pGC = GetScratchGC (pPixmap->drawable.depth, pScreen);
    pCopy = (*pScreen->RegionCreate) ((BoxPtr) 0, 1);
    (*pScreen->RegionCopy) (pCopy, pRegion);
    if (xOff || yOff)
	(*pScreen->TranslateRegion) (pCopy, xOff, yOff);
    extents = *(*pScreen->RegionExtents) (pCopy);
    /*
     * erase the pixmap
     */
    miClearDrawable (pPixmap, pGC);
    /*
     * fill a rectangle clipped to the region
     */
    (*pGC->ChangeClip) (pGC, CT_REGION, (pointer) pCopy, 0);
    ValidateGC (pGC, pPixmap);
    rect.x = extents.x1;
    rect.y = extents.y1;
    rect.width = extents.x2 - extents.x1;
    rect.height = extents.y2 - extents.y1;
    (*pGC->PolyFillRect) (pPixmap, pGC, 1, &rect);
}

/*****************
 * ProcSetWindowShapeRectangles
 *
 *****************/

static int
ProcSetWindowShapeRectangles (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xSetWindowShapeRectanglesReq);
    xRectangle		*prects;
    int		        nrects;
    ScreenPtr		pScreen;

    REQUEST_AT_LEAST_SIZE (xSetWindowShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    pScreen = pWin->drawable.pScreen;
    nrects = ((stuff->length  << 2) - sizeof(xSetWindowShapeRectanglesReq)) >> 3;
    prects = (xRectangle *) &stuff[1];
    if (pWin->windowShape) {
	(*pScreen->RegionDestroy) (pWin->windowShape);
	pWin->windowShape = 0;
    }
    if (nrects > 0)
	pWin->windowShape = RectsToRegion (pScreen, nrects, prects);
    SetShape (pWin);
    return Success;
}

/*****************
 * ProcGetWindowShapeRectangles
 *
 *****************/

static int
ProcGetWindowShapeRectangles(client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xGetWindowShapeRectanglesReq);          /* xGetWindowShapeRectanglesReq *stuff; */
    xGenericReply	reply;
    xRectangle		*prect;
    int			nrect;

    REQUEST_SIZE_MATCH(xGetWindowShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    nrect = 0;
    prect = 0;
    if (pWin->windowShape)
        nrect = RegionToRectangles
	    (pWin->drawable.pScreen, pWin->windowShape, &prect);
    reply.type = X_Reply;
    reply.length = (nrect * sizeof (xRectangle)) / 4;
    reply.sequenceNumber = client->sequence;
    WriteReplyToClient(client, sizeof(xGenericReply), &reply);
    if (prect && nrect) {
	client->pSwapReplyFunc = CopySwap16Write;
	WriteSwappedDataToClient (client, nrect * sizeof (xRectangle), prect);
	xfree (prect);
    }
    return Success;
}

/*****************
 * ProcSetWindowShapeBitmap
 *
 *****************/

static int
ProcSetWindowShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    register PixmapPtr	pPixmap;
    REQUEST(xSetWindowShapeMaskReq);
    BoxPtr		extents;
    CARD16		width, height;
    ScreenPtr		pScreen;

    REQUEST_SIZE_MATCH (xSetWindowShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin) {
	return BadWindow;
    }
    pPixmap = 0;
    if (stuff->mask != None) {
        pPixmap = (PixmapPtr)LookupID (stuff->mask, RT_PIXMAP, RC_CORE);
        if (!pPixmap) {
	    return BadPixmap;
        }
	if (pPixmap->drawable.depth != 1)
	    return BadMatch;
    }
    pScreen = pWin->drawable.pScreen;
    if (pWin->windowShape) {
	(*pScreen->RegionDestroy) (pWin->windowShape);
	pWin->windowShape = 0;
    }
    if (pPixmap) {
        pWin->windowShape = BitmapToRegion (pPixmap);
	(*pScreen->TranslateRegion) (pWin->windowShape, -stuff->xOff, -stuff->yOff);
    }
    SetShape (pWin);
    return Success;
}

/*****************
 * ProcGetWindowShapeMask
 *****************/

static int
ProcGetWindowShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    register PixmapPtr	pPixmap;
    REQUEST(xGetWindowShapeMaskReq);
    BoxPtr		extents;
    CARD16		width, height;
    ScreenPtr		pScreen;
    INT16		xOff, yOff;

    REQUEST_SIZE_MATCH (xGetWindowShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin) {
	return BadWindow;
    }
    pPixmap = 0;
    if (stuff->mask != None) {
        pPixmap = (PixmapPtr)LookupID (stuff->mask, RT_PIXMAP, RC_CORE);
        if (!pPixmap) {
	    return BadPixmap;
        }
	if (pPixmap->drawable.depth != 1)
	    return BadMatch;
    }
    xOff = stuff->xOff;
    yOff = stuff->yOff;
    pScreen = pWin->drawable.pScreen;
    if (pPixmap && pWin->windowShape)
	RegionToBitmap (pWin->windowShape, pPixmap, xOff, yOff);
    return Success;
}

/*****************
 * ProcSetBorderShapeRectangles
 *
 *****************/

static int
ProcSetBorderShapeRectangles (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xSetBorderShapeRectanglesReq);
    xRectangle		*prects;
    int		        nrects;
    ScreenPtr		pScreen;

    REQUEST_AT_LEAST_SIZE (xSetBorderShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    pScreen = pWin->drawable.pScreen;
    nrects = ((stuff->length  << 2) - sizeof(xSetBorderShapeRectanglesReq)) >> 3;
    prects = (xRectangle *) &stuff[1];
    if (pWin->borderShape) {
	(*pScreen->RegionDestroy) (pWin->borderShape);
	pWin->borderShape = 0;
    }
    if (nrects > 0)
	pWin->borderShape = RectsToRegion (pScreen, nrects, prects);
    SetShape (pWin);
    return Success;
}

/*****************
 * ProcGetBorderShapeRectangles
 *
 *****************/

static int
ProcGetBorderShapeRectangles(client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xGetBorderShapeRectanglesReq);          /* xGetBorderShapeRectanglesReq *stuff; */
    xGenericReply	reply;
    xRectangle		*prect;
    int			nrect;

    REQUEST_SIZE_MATCH(xGetBorderShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    nrect = 0;
    prect = 0;
    if (pWin->borderShape)
        nrect = RegionToRectangles
	    (pWin->drawable.pScreen, pWin->borderShape, &prect);
    reply.type = X_Reply;
    reply.length = (nrect * sizeof (xRectangle)) / 4;
    reply.sequenceNumber = client->sequence;
    WriteReplyToClient(client, sizeof(xGenericReply), &reply);
    if (prect && nrect) {
	client->pSwapReplyFunc = CopySwap16Write;
	WriteSwappedDataToClient (client, nrect * sizeof (xRectangle), prect);
	xfree (prect);
    }
    return Success;
}

/*****************
 * ProcSetBorderShapeBitmap
 *
 *****************/

static int
ProcSetBorderShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    register PixmapPtr	pPixmap;
    REQUEST(xSetBorderShapeMaskReq);
    BoxPtr		extents;
    CARD16		width, height;
    ScreenPtr		pScreen;

    REQUEST_SIZE_MATCH (xSetBorderShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin) {
	return BadWindow;
    }
    pPixmap = 0;
    if (stuff->mask != None) {
        pPixmap = (PixmapPtr)LookupID (stuff->mask, RT_PIXMAP, RC_CORE);
        if (!pPixmap) {
	    return BadPixmap;
        }
	if (pPixmap->drawable.depth != 1)
	    return BadMatch;
    }
    pScreen = pWin->drawable.pScreen;
    if (pWin->borderShape) {
	(*pScreen->RegionDestroy) (pWin->borderShape);
	pWin->borderShape = 0;
    }
    if (pPixmap) {
        pWin->borderShape = BitmapToRegion (pPixmap);
	(*pScreen->TranslateRegion) (pWin->borderShape, -stuff->xOff, -stuff->yOff);
    }
    SetShape (pWin);
    return Success;
}

/*****************
 * ProcGetBorderShapeMask
 *****************/

static int
ProcGetBorderShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    register PixmapPtr	pPixmap;
    REQUEST(xGetBorderShapeMaskReq);
    BoxPtr		extents;
    CARD16		width, height;
    ScreenPtr		pScreen;
    INT16		xOff, yOff;

    REQUEST_SIZE_MATCH (xGetBorderShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin) {
	return BadWindow;
    }
    pPixmap = 0;
    if (stuff->mask != None) {
        pPixmap = (PixmapPtr)LookupID (stuff->mask, RT_PIXMAP, RC_CORE);
        if (!pPixmap) {
	    return BadPixmap;
        }
	if (pPixmap->drawable.depth != 1)
	    return BadMatch;
    }
    xOff = stuff->xOff;
    yOff = stuff->yOff;
    pScreen = pWin->drawable.pScreen;
    if (pPixmap && pWin->borderShape)
	RegionToBitmap (pWin->borderShape, pPixmap, xOff, yOff);
    return Success;
}

/*****************
 * ProcShapeDispatch
 *****************/

int
ProcShapeDispatch(client)
    register ClientPtr client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_SetWindowShapeRectangles:
	return(ProcSetWindowShapeRectangles(client));
    case X_SetWindowShapeMask:
	return(ProcSetWindowShapeMask(client));
    case X_GetWindowShapeRectangles:
	return(ProcGetWindowShapeRectangles(client));
    case X_GetWindowShapeMask:
	return(ProcGetWindowShapeMask(client));
    case X_SetBorderShapeRectangles:
	return(ProcSetBorderShapeRectangles(client));
    case X_SetBorderShapeMask:
	return(ProcSetBorderShapeMask(client));
    case X_GetBorderShapeRectangles:
	return(ProcGetBorderShapeRectangles(client));
    case X_GetBorderShapeMask:
	return(ProcGetBorderShapeMask(client));
    default:
	SendErrorToClient(client, ShapeReqCode, stuff->data, 0, BadRequest);
	return(BadRequest);
    }
}

int
SProcShapeDispatch(client)
    register ClientPtr client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_SetWindowShapeRectangles:
	return(SProcSetWindowShapeRectangles(client));
    case X_SetWindowShapeMask:
	return(SProcSetWindowShapeMask(client));
    case X_GetWindowShapeRectangles:
	return(SProcGetWindowShapeRectangles(client));
    case X_GetWindowShapeMask:
	return(SProcGetWindowShapeMask(client));
    case X_SetBorderShapeRectangles:
	return(SProcSetBorderShapeRectangles(client));
    case X_SetBorderShapeMask:
	return(SProcSetBorderShapeMask(client));
    case X_GetBorderShapeRectangles:
	return(SProcGetBorderShapeRectangles(client));
    case X_GetBorderShapeMask:
	return(SProcGetBorderShapeMask(client));
    default:
	SendErrorToClient(client, ShapeReqCode, stuff->data, 0, BadRequest);
	return(BadRequest);
    }
}

/* Macros needed for byte-swapping, copied from swapreq.c.  Really
   should be in a header file somewhere. */

#define LengthRestS(stuff) \
    ((stuff->length << 1)  - (sizeof(*stuff) >> 1))

#define SwapRestS(stuff) \
    SwapShorts(stuff + 1, LengthRestS(stuff))

int
SProcSetWindowShapeRectangles(client)
     register ClientPtr client;
{
     register char n;

     REQUEST(xSetWindowShapeRectanglesReq);
     swaps(&stuff->length, n);
     swapl(&stuff->window, n);
     SwapRestS(stuff);
     return (ProcSetWindowShapeRectangles(client));
}

int
SProcGetWindowShapeRectangles(client)
     register ClientPtr client;
{
     register char n;

     REQUEST(xGetWindowShapeRectanglesReq);
     swaps(&stuff->length, n);
     swapl(&stuff->window, n);
     return (ProcGetWindowShapeRectangles(client));
}

SProcSetWindowShapeMask(client)
    register ClientPtr	client;
{
    register char   n;

    REQUEST(xSetWindowShapeMaskReq);
    swaps(&stuff->length, n);
    swaps(&stuff->xOff, n);
    swaps(&stuff->yOff, n);
    swapl(&stuff->window, n);
    swapl(&stuff->mask, n);
    return (ProcSetWindowShapeMask(client));
}

SProcGetWindowShapeMask(client)
    register ClientPtr	client;
{
    register char   n;

    REQUEST(xGetWindowShapeMaskReq);
    swaps(&stuff->length, n);
    swaps(&stuff->xOff, n);
    swaps(&stuff->yOff, n);
    swapl(&stuff->window, n);
    swapl(&stuff->mask, n);
    return (ProcGetWindowShapeMask(client));
}

int
SProcSetBorderShapeRectangles(client)
     register ClientPtr client;
{
     register char n;

     REQUEST(xSetBorderShapeRectanglesReq);
     swaps(&stuff->length, n);
     swapl(&stuff->window, n);
     SwapRestS(stuff);
     return (ProcSetBorderShapeRectangles(client));
}

int
SProcGetBorderShapeRectangles(client)
     register ClientPtr client;
{
     register char n;

     REQUEST(xGetBorderShapeRectanglesReq);
     swaps(&stuff->length, n);
     swapl(&stuff->window, n);
     return (ProcGetBorderShapeRectangles(client));
}

SProcSetBorderShapeMask(client)
    register ClientPtr	client;
{
    register char   n;

    REQUEST(xSetBorderShapeMaskReq);
    swaps(&stuff->length, n);
    swaps(&stuff->xOff, n);
    swaps(&stuff->yOff, n);
    swapl(&stuff->window, n);
    swapl(&stuff->mask, n);
    return (ProcSetBorderShapeMask(client));
}

SProcGetBorderShapeMask(client)
    register ClientPtr	client;
{
    register char   n;

    REQUEST(xGetBorderShapeMaskReq);
    swaps(&stuff->length, n);
    swaps(&stuff->xOff, n);
    swaps(&stuff->yOff, n);
    swapl(&stuff->window, n);
    swapl(&stuff->mask, n);
    return (ProcGetBorderShapeMask(client));
}

void
ShapeResetProc()
{
}

void
ShapeFreeState()
{
}

void
ShapeDeleteState()
{
}
