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

/* $XConsortium: shape.c,v 1.2 89/02/14 14:34:42 keith Exp $ */
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
#include "shapestr.h"
#include "regionstr.h"
#include "gcstruct.h"

static unsigned char ShapeReqCode = 0;

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
    extEntry = AddExtension(SHAPENAME, 0, 0, ProcShapeDispatch,
			    SProcShapeDispatch, ShapeResetProc);
    if (extEntry)
	ShapeReqCode = (unsigned char)extEntry->base;
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
    if (ret == Success)
	SetShape (pWin);
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
    if (ret == Success)
	SetShape (pWin);
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
    if (ret == Success)
	SetShape (pDestWin);
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

extern void SwapShorts();

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
    default:
	SendErrorToClient (client, ShapeReqCode, stuff->data, (XID)0,
			   BadRequest);
	return BadRequest;
    }
}
