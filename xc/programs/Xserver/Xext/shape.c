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

/* $XConsortium: shape.c,v 1.1 89/02/06 17:45:20 keith Exp $ */
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

    REQUEST_AT_LEAST_SIZE (xSetWindowShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    nrects = ((stuff->length  << 2) - sizeof(xSetWindowShapeRectanglesReq)) >> 3;
    prects = (xRectangle *) &stuff[1];
    return SetRects (pWin, &pWin->windowShape, nrects, prects);
}

static int
ProcUnionWindowShapeRectangles (client)
    register ClientPtr	client;
{
    register WindowPtr	pWin;
    REQUEST(xUnionWindowShapeRectanglesReq);
    xRectangle		*prects;
    int		        nrects;

    REQUEST_AT_LEAST_SIZE (xUnionWindowShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    nrects = ((stuff->length  << 2) - sizeof(xUnionWindowShapeRectanglesReq)) >> 3;
    prects = (xRectangle *) &stuff[1];
    return UnionRects (pWin, &pWin->windowShape, nrects, prects);
}

static int
ProcIntersectWindowShapeRectangles (client)
    register ClientPtr	client;
{
    register WindowPtr	pWin;
    REQUEST(xIntersectWindowShapeRectanglesReq);
    xRectangle		*prects;
    int		        nrects;

    REQUEST_AT_LEAST_SIZE (xIntersectWindowShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    nrects = ((stuff->length  << 2) - sizeof(xIntersectWindowShapeRectanglesReq)) >> 3;
    prects = (xRectangle *) &stuff[1];
    IntersectRects (pWin, &pWin->windowShape, nrects, prects);
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

    REQUEST_SIZE_MATCH(xGetWindowShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    return GetRects (client, pWin, pWin->windowShape);
}

/*****************
 * ProcSetWindowShapeMask
 *
 *****************/

static int
ProcSetWindowShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xSetWindowShapeMaskReq);

    REQUEST_SIZE_MATCH (xSetWindowShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    return SetMask (pWin, &pWin->windowShape, stuff->mask, stuff->xOff, stuff->yOff);
}

static int
ProcIntersectWindowShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xIntersectWindowShapeMaskReq);

    REQUEST_SIZE_MATCH (xIntersectWindowShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    return IntersectMask (pWin, &pWin->windowShape, stuff->mask, stuff->xOff, stuff->yOff);
}

static int
ProcUnionWindowShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xUnionWindowShapeMaskReq);

    REQUEST_SIZE_MATCH (xUnionWindowShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    return UnionMask (pWin, &pWin->windowShape, stuff->mask, stuff->xOff, stuff->yOff);
}

/*****************
 * ProcGetWindowShapeMask
 *****************/

static int
ProcGetWindowShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xGetWindowShapeMaskReq);

    REQUEST_SIZE_MATCH (xGetWindowShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    return GetMask (stuff->mask, pWin->windowShape, stuff->xOff, stuff->yOff);
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

    REQUEST_AT_LEAST_SIZE (xSetBorderShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    nrects = ((stuff->length  << 2) - sizeof(xSetBorderShapeRectanglesReq)) >> 3;
    prects = (xRectangle *) &stuff[1];
    return SetRects (pWin, &pWin->borderShape, nrects, prects);
}

static int
ProcUnionBorderShapeRectangles (client)
    register ClientPtr	client;
{
    register WindowPtr	pWin;
    REQUEST(xUnionBorderShapeRectanglesReq);
    xRectangle		*prects;
    int		        nrects;

    REQUEST_AT_LEAST_SIZE (xUnionBorderShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    nrects = ((stuff->length  << 2) - sizeof(xUnionBorderShapeRectanglesReq)) >> 3;
    prects = (xRectangle *) &stuff[1];
    return UnionRects (pWin, &pWin->borderShape, nrects, prects);
}

static int
ProcIntersectBorderShapeRectangles (client)
    register ClientPtr	client;
{
    register WindowPtr	pWin;
    REQUEST(xIntersectBorderShapeRectanglesReq);
    xRectangle		*prects;
    int		        nrects;

    REQUEST_AT_LEAST_SIZE (xIntersectBorderShapeRectanglesReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    nrects = ((stuff->length  << 2) - sizeof(xIntersectBorderShapeRectanglesReq)) >> 3;
    prects = (xRectangle *) &stuff[1];
    IntersectRects (pWin, &pWin->borderShape, nrects, prects);
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
    return GetRects (client, pWin, pWin->borderShape);
}

/*****************
 * ProcSetBorderShapeMask
 *
 *****************/

static int
ProcSetBorderShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xSetBorderShapeMaskReq);

    REQUEST_SIZE_MATCH (xSetBorderShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    return SetMask (pWin, &pWin->borderShape, stuff->mask, stuff->xOff, stuff->yOff);
}

static int
ProcIntersectBorderShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xIntersectBorderShapeMaskReq);

    REQUEST_SIZE_MATCH (xIntersectBorderShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    return IntersectMask (pWin, &pWin->borderShape, stuff->mask, stuff->xOff, stuff->yOff);
}

static int
ProcUnionBorderShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xUnionBorderShapeMaskReq);

    REQUEST_SIZE_MATCH (xUnionBorderShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    return UnionMask (pWin, &pWin->borderShape, stuff->mask, stuff->xOff, stuff->yOff);
}

/*****************
 * ProcGetBorderShapeMask
 *****************/

static int
ProcGetBorderShapeMask (client)
    register ClientPtr client;
{
    register WindowPtr	pWin;
    REQUEST(xGetBorderShapeMaskReq);

    REQUEST_SIZE_MATCH (xGetBorderShapeMaskReq);
    pWin = (WindowPtr)LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    return GetMask (stuff->mask, pWin->borderShape, stuff->xOff, stuff->yOff);
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
    case X_IntersectWindowShapeRectangles:
	return(ProcIntersectWindowShapeRectangles(client));
    case X_UnionWindowShapeRectangles:
	return(ProcUnionWindowShapeRectangles(client));
    case X_SetWindowShapeMask:
	return(ProcSetWindowShapeMask(client));
    case X_IntersectWindowShapeMask:
	return(ProcIntersectWindowShapeMask(client));
    case X_UnionWindowShapeMask:
	return(ProcUnionWindowShapeMask(client));
    case X_GetWindowShapeRectangles:
	return(ProcGetWindowShapeRectangles(client));
    case X_GetWindowShapeMask:
	return(ProcGetWindowShapeMask(client));
    case X_SetBorderShapeRectangles:
	return(ProcSetBorderShapeRectangles(client));
    case X_IntersectBorderShapeRectangles:
	return(ProcIntersectBorderShapeRectangles(client));
    case X_UnionBorderShapeRectangles:
	return(ProcUnionBorderShapeRectangles(client));
    case X_SetBorderShapeMask:
	return(ProcSetBorderShapeMask(client));
    case X_IntersectBorderShapeMask:
	return(ProcIntersectBorderShapeMask(client));
    case X_UnionBorderShapeMask:
	return(ProcUnionBorderShapeMask(client));
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
    case X_IntersectWindowShapeRectangles:
	return(SProcIntersectWindowShapeRectangles(client));
    case X_UnionWindowShapeRectangles:
	return(SProcUnionWindowShapeRectangles(client));
    case X_SetWindowShapeMask:
	return(SProcSetWindowShapeMask(client));
    case X_IntersectWindowShapeMask:
	return(SProcIntersectWindowShapeMask(client));
    case X_UnionWindowShapeMask:
	return(SProcUnionWindowShapeMask(client));
    case X_GetWindowShapeRectangles:
	return(SProcGetWindowShapeRectangles(client));
    case X_GetWindowShapeMask:
	return(SProcGetWindowShapeMask(client));
    case X_SetBorderShapeRectangles:
	return(SProcSetBorderShapeRectangles(client));
    case X_IntersectBorderShapeRectangles:
	return(SProcIntersectBorderShapeRectangles(client));
    case X_UnionBorderShapeRectangles:
	return(SProcUnionBorderShapeRectangles(client));
    case X_SetBorderShapeMask:
	return(SProcSetBorderShapeMask(client));
    case X_IntersectBorderShapeMask:
	return(SProcIntersectBorderShapeMask(client));
    case X_UnionBorderShapeMask:
	return(SProcUnionBorderShapeMask(client));
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

static int
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

static int
SProcIntersectWindowShapeRectangles(client)
     register ClientPtr client;
{
     register char n;

     REQUEST(xIntersectWindowShapeRectanglesReq);
     swaps(&stuff->length, n);
     swapl(&stuff->window, n);
     SwapRestS(stuff);
     return (ProcIntersectWindowShapeRectangles(client));
}

static int
SProcUnionWindowShapeRectangles(client)
     register ClientPtr client;
{
     register char n;

     REQUEST(xUnionWindowShapeRectanglesReq);
     swaps(&stuff->length, n);
     swapl(&stuff->window, n);
     SwapRestS(stuff);
     return (ProcUnionWindowShapeRectangles(client));
}

static int
SProcGetWindowShapeRectangles(client)
     register ClientPtr client;
{
     register char n;

     REQUEST(xGetWindowShapeRectanglesReq);
     swaps(&stuff->length, n);
     swapl(&stuff->window, n);
     return (ProcGetWindowShapeRectangles(client));
}

static int
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

static int
SProcIntersectWindowShapeMask(client)
    register ClientPtr	client;
{
    register char   n;

    REQUEST(xIntersectWindowShapeMaskReq);
    swaps(&stuff->length, n);
    swaps(&stuff->xOff, n);
    swaps(&stuff->yOff, n);
    swapl(&stuff->window, n);
    swapl(&stuff->mask, n);
    return (ProcIntersectWindowShapeMask(client));
}

static int
SProcUnionWindowShapeMask(client)
    register ClientPtr	client;
{
    register char   n;

    REQUEST(xUnionWindowShapeMaskReq);
    swaps(&stuff->length, n);
    swaps(&stuff->xOff, n);
    swaps(&stuff->yOff, n);
    swapl(&stuff->window, n);
    swapl(&stuff->mask, n);
    return (ProcUnionWindowShapeMask(client));
}

static int
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

static int
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

static int
SProcIntersectBorderShapeRectangles(client)
     register ClientPtr client;
{
     register char n;

     REQUEST(xIntersectBorderShapeRectanglesReq);
     swaps(&stuff->length, n);
     swapl(&stuff->window, n);
     SwapRestS(stuff);
     return (ProcIntersectBorderShapeRectangles(client));
}

static int
SProcUnionBorderShapeRectangles(client)
     register ClientPtr client;
{
     register char n;

     REQUEST(xUnionBorderShapeRectanglesReq);
     swaps(&stuff->length, n);
     swapl(&stuff->window, n);
     SwapRestS(stuff);
     return (ProcUnionBorderShapeRectangles(client));
}

static int
SProcGetBorderShapeRectangles(client)
     register ClientPtr client;
{
     register char n;

     REQUEST(xGetBorderShapeRectanglesReq);
     swaps(&stuff->length, n);
     swapl(&stuff->window, n);
     return (ProcGetBorderShapeRectangles(client));
}

static int
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

static int
SProcIntersectBorderShapeMask(client)
    register ClientPtr	client;
{
    register char   n;

    REQUEST(xIntersectBorderShapeMaskReq);
    swaps(&stuff->length, n);
    swaps(&stuff->xOff, n);
    swaps(&stuff->yOff, n);
    swapl(&stuff->window, n);
    swapl(&stuff->mask, n);
    return (ProcIntersectBorderShapeMask(client));
}

static int
SProcUnionBorderShapeMask(client)
    register ClientPtr	client;
{
    register char   n;

    REQUEST(xUnionBorderShapeMaskReq);
    swaps(&stuff->length, n);
    swaps(&stuff->xOff, n);
    swaps(&stuff->yOff, n);
    swapl(&stuff->window, n);
    swapl(&stuff->mask, n);
    return (ProcUnionBorderShapeMask(client));
}

static int
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

/* Various utility routines for mangling regions around */

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
    (*pGC->ChangeClip) (pGC, CT_NONE, (pointer) 0, 0);
}

static int
SetRgn (pWin, ppRgn, pRgn)
    WindowPtr	pWin;
    RegionPtr	*ppRgn, pRgn;
{
    if (*ppRgn)
	(*pWin->drawable.pScreen->RegionDestroy) (*ppRgn);
    *ppRgn = pRgn;
    SetShape (pWin);
    return Success;
}

static int
SetRects (pWin, ppRgn, nrects, prects)
    WindowPtr	pWin;
    RegionPtr	*ppRgn;
    int		nrects;
    xRectangle	*prects;
{
    RegionPtr	pRgn;

    if (nrects == 0)
	pRgn = 0;
    else
	pRgn = RectsToRegion (pWin->drawable.pScreen, nrects, prects);
    return SetRgn (pWin, ppRgn, pRgn);
}

static int
SetMask (pWin, ppRgn, mask, xOff, yOff)
    WindowPtr	pWin;
    RegionPtr	*ppRgn;
    XID		mask;
{
    RegionPtr	pRgn;
    PixmapPtr	pPixmap;

    if (mask == None)
	pRgn = 0;
    else {
	pPixmap = (PixmapPtr)LookupID (mask, RT_PIXMAP, RC_CORE);
	if (!pPixmap)
	    return BadPixmap;
	if (pPixmap->drawable.depth != 1)
	    return BadMatch;
        pRgn = BitmapToRegion (pPixmap);
        if (xOff || yOff)
	    (*pWin->drawable.pScreen->TranslateRegion) (pRgn, -xOff, -yOff);
    }
    return SetRgn (pWin, ppRgn, pRgn);
}

static int
IntersectRgn (pWin, ppRgn, pRgn)
    WindowPtr	pWin;
    RegionPtr	*ppRgn;
    RegionPtr	pRgn;
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;

    if (*ppRgn) {
	(*pScreen->Intersect) (*ppRgn, *ppRgn, pRgn);
	(*pScreen->RegionDestroy) (pRgn);
    } else
	*ppRgn = pRgn;
    SetShape (pWin);
    return Success;
}

static int
IntersectRects (pWin, ppRgn, nrects, prects)
    WindowPtr	pWin;
    RegionPtr	*ppRgn;
    int		nrects;
    xRectangle	*prects;
{
    return IntersectRgn (pWin, ppRgn,
		RectsToRegion (pWin->drawable.pScreen, nrects, prects));
}

static int
IntersectMask (pWin, ppRgn, pPixmap)
    WindowPtr	pWin;
    RegionPtr	*ppRgn;
    PixmapPtr	pPixmap;
{
    return IntersectRgn (pWin, ppRgn, BitmapToRegion (pPixmap));
}

static int
UnionRgn (pWin, ppRgn, pRgn)
    WindowPtr	pWin;
    RegionPtr	*ppRgn;
    RegionPtr	pRgn;
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;

    /*
     * if a region is attached, union the new region in.  Else,
     * the desired union is the whole area, so do nothing.
     */
    if (*ppRgn) {
	(*pScreen->Union) (*ppRgn, *ppRgn, pRgn);
	SetShape (pWin);
    }
    (*pScreen->RegionDestroy) (pRgn);
    return Success;
}

static int
UnionRects (pWin, ppRgn, nrects, prects)
    WindowPtr	pWin;
    RegionPtr	*ppRgn;
    int		nrects;
    xRectangle	*prects;
{
    return UnionRgn (pWin, ppRgn,
		RectsToRegion (pWin->drawable.pScreen, nrects, prects));
}

static int
UnionMask (pWin, ppRgn, pPixmap)
    WindowPtr	pWin;
    RegionPtr	*ppRgn;
    PixmapPtr	pPixmap;
{
    return UnionRgn (pWin, ppRgn, BitmapToRegion (pPixmap));
}

GetRects (client, pWin, pRgn)
    ClientPtr	client;
    WindowPtr	pWin;
    RegionPtr	pRgn;
{
    int		nrect = 0;
    xRectangle	*prect;
    xGenericReply   reply;

    if (pRgn) {
        nrect = RegionToRectangles
	    (pWin->drawable.pScreen, pRgn, &prect);
	if (!prect)
	    return BadAlloc;
    }
    reply.type = X_Reply;
    reply.length = (nrect * sizeof (xRectangle)) / 4;
    reply.sequenceNumber = client->sequence;
    WriteReplyToClient(client, sizeof(xGenericReply), &reply);
    if (nrect) {
	client->pSwapReplyFunc = CopySwap16Write;
	WriteSwappedDataToClient (client, nrect * sizeof (xRectangle), prect);
	xfree (prect);
    }
    return Success;
}

GetMask (mask, pRgn, xOff, yOff)
    XID	    mask;
    RegionPtr	pRgn;
    int		xOff, yOff;
{
    PixmapPtr	pPixmap;

    if (mask == None)
	return BadMatch;
    pPixmap = (PixmapPtr)LookupID (mask, RT_PIXMAP, RC_CORE);
    if (!pPixmap)
	return BadPixmap;
    if (pPixmap->drawable.depth != 1)
	return BadMatch;
    RegionToBitmap (pRgn, pPixmap, xOff, yOff);
    return Success;
}

    
