/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $XConsortium: miexpose.c,v 5.3 89/07/04 16:13:12 rws Exp $ */

#include "X.h"
#define NEED_EVENTS
#include "Xproto.h"
#include "Xprotostr.h"

#include "misc.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmap.h"
#include "input.h"

#include "dixstruct.h"
#include "mi.h"
#include "Xmd.h"

extern WindowPtr *WindowTable;

/*
    machine-independent graphics exposure code.  any device that uses
the region package can call this.
*/

#ifndef RECTLIMIT
#define RECTLIMIT 25		/* pick a number, any number > 8 */
#endif

/* miHandleExposures 
    generate a region for exposures for areas that were copied from obscured or
non-existent areas to non-obscured areas of the destination.  Paint the
background for the region, if the destination is a window.

NOTE:
     this should generally be called, even if graphicsExposures is false,
because this is where bits get recovered from backing store.

NOTE:
     added argument 'plane' is used to indicate how exposures from backing
store should be accomplished. If plane is 0 (i.e. no bit plane), CopyArea
should be used, else a CopyPlane of the indicated plane will be used. The
exposing is done by the backing store's GraphicsExpose function, of course.

*/

RegionPtr
miHandleExposures(pSrcDrawable, pDstDrawable,
		  pGC, srcx, srcy, width, height, dstx, dsty, plane)
    register DrawablePtr	pSrcDrawable;
    register DrawablePtr	pDstDrawable;
    GCPtr 			pGC;
    int 			srcx, srcy;
    int 			width, height;
    int 			dstx, dsty;
    unsigned long		plane;
{
    register ScreenPtr pscr = pGC->pScreen;
    RegionPtr prgnSrcClip;	/* drawable-relative source clip */
    RegionPtr prgnDstClip;	/* drawable-relative dest clip */
    BoxRec srcBox;		/* unclipped source */
    RegionPtr prgnExposed;	/* exposed region, calculated source-
				   relative, made dst relative to
				   intersect with visible parts of
				   dest and send events to client, 
				   and then screen relative to paint 
				   the window background
				*/
    WindowPtr pSrcWin;
    BoxRec expBox;
    Bool extents;

    /* avoid work if we can */
    if (!pGC->graphicsExposures &&
	(pDstDrawable->type == DRAWABLE_PIXMAP) &&
	((pSrcDrawable->type == DRAWABLE_PIXMAP) ||
	 (((WindowPtr)pSrcDrawable)->backStorage == (BackingStorePtr)NULL)))
	return NULL;
	
    srcBox.x1 = srcx;
    srcBox.y1 = srcy;
    srcBox.x2 = srcx+width;
    srcBox.y2 = srcy+height;

    if (pSrcDrawable->type == DRAWABLE_WINDOW)
    {
	BoxRec TsrcBox;

	TsrcBox.x1 = srcx + pSrcDrawable->x;
	TsrcBox.y1 = srcy + pSrcDrawable->y;
	TsrcBox.x2 = TsrcBox.x1 + width;
	TsrcBox.y2 = TsrcBox.y1 + height;
	pSrcWin = (WindowPtr) pSrcDrawable;
	if (pGC->subWindowMode == IncludeInferiors)
 	{
	    prgnSrcClip = NotClippedByChildren (pSrcWin);
	    if (((*pscr->RectIn)(prgnSrcClip, &TsrcBox)) == rgnIN)
	    {
		(*pscr->RegionDestroy) (prgnSrcClip);
		return NULL;
	    }
	}
 	else
 	{
	    if (((*pscr->RectIn)(&pSrcWin->clipList, &TsrcBox)) == rgnIN)
		return NULL;
	    prgnSrcClip = (*pscr->RegionCreate)(NullBox, 1);
	    (*pscr->RegionCopy)(prgnSrcClip, &pSrcWin->clipList);
	}
	(*pscr->TranslateRegion)(prgnSrcClip,
				-pSrcDrawable->x, -pSrcDrawable->y);
    }
    else
    {
	BoxRec	box;

	if ((srcBox.x1 >= 0) && (srcBox.y1 >= 0) &&
	    (srcBox.x2 <= pSrcDrawable->width) &&
 	    (srcBox.y2 <= pSrcDrawable->height))
	    return NULL;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pSrcDrawable->width;
	box.y2 = pSrcDrawable->height;
	prgnSrcClip = (*pscr->RegionCreate)(&box, 1);
	pSrcWin = (WindowPtr)NULL;
    }

    if (pDstDrawable == pSrcDrawable)
    {
	prgnDstClip = prgnSrcClip;
    }
    else if (pDstDrawable->type == DRAWABLE_WINDOW)
    {
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    prgnDstClip = NotClippedByChildren((WindowPtr)pDstDrawable);
	}
	else
	{
	    prgnDstClip = (*pscr->RegionCreate)(NullBox, 1);
	    (*pscr->RegionCopy)(prgnDstClip,
				&((WindowPtr)pDstDrawable)->clipList);
	}
	(*pscr->TranslateRegion)(prgnDstClip,
				 -pDstDrawable->x, -pDstDrawable->y);
    }
    else
    {
	BoxRec	box;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pDstDrawable->width;
	box.y2 = pDstDrawable->height;
	prgnDstClip = (*pscr->RegionCreate)(&box, 1);
    }

    /* drawable-relative source region */
    prgnExposed = (*pscr->RegionCreate)(&srcBox, 1);

    /* now get the hidden parts of the source box*/
    (*pscr->Subtract)(prgnExposed, prgnExposed, prgnSrcClip);

    if (pSrcWin && pSrcWin->backStorage)
    {
	/*
	 * Copy any areas from the source backing store. Modifies
	 * prgnExposed.
	 */
	(* pSrcWin->backStorage->funcs->ExposeCopy) (pSrcDrawable,
					      pDstDrawable,
					      pGC,
					      prgnExposed,
					      srcx, srcy,
					      dstx, dsty,
					      plane);
    }
    
    /* move them over the destination */
    (*pscr->TranslateRegion)(prgnExposed, dstx-srcx, dsty-srcy);

    /* intersect with visible areas of dest */
    (*pscr->Intersect)(prgnExposed, prgnExposed, prgnDstClip);

    /*
     * If we have LOTS of rectangles, we decide to take the extents
     * and force an exposure on that.  This should require much less
     * work overall, on both client and server.  This is cheating, but
     * isn't prohibited by the protocol ("spontaneous combustion" :-)
     * for windows.
     */
    extents = pGC->graphicsExposures &&
	      (REGION_NUM_RECTS(prgnExposed) > RECTLIMIT) &&
	      (pDstDrawable->type == DRAWABLE_WINDOW);
#ifdef SHAPE
    if (pSrcWin)
    {
	RegionPtr	region;
    	if (!(region = wClipShape (pSrcWin)))
    	    region = wBoundingShape (pSrcWin);
    	/*
     	 * If you try to CopyArea the extents of a shaped window, compacting the
     	 * exposed region will undo all our work!
     	 */
    	if (extents && pSrcWin && region &&
    	    ((*pscr->RectIn)(region, &srcBox) != rgnIN))
	    	extents = FALSE;
    }
#endif
    if (extents)
    {
	WindowPtr pWin = (WindowPtr)pDstDrawable;

	expBox = *(*pscr->RegionExtents)(prgnExposed);
	(*pscr->RegionReset)(prgnExposed, &expBox);
	/* need to clear out new areas of backing store */
	if (pWin->backStorage)
	    (* pWin->backStorage->funcs->ClearToBackground)(
					 pWin,
					 expBox.x1,
					 expBox.y1,
					 expBox.x2 - expBox.x1,
					 expBox.y2 - expBox.y1,
					 FALSE);
    }
    if ((pDstDrawable->type == DRAWABLE_WINDOW) &&
	(((WindowPtr)pDstDrawable)->backgroundState != None))
    {
	WindowPtr pWin = (WindowPtr)pDstDrawable;

	/* make the exposed area screen-relative */
	(*pscr->TranslateRegion)(prgnExposed, 
				 pDstDrawable->x, pDstDrawable->y);

	if (extents)
	{
	    /* PaintWindowBackground doesn't clip, so we have to */
	    (*pscr->Intersect)(prgnExposed, prgnExposed, &pWin->clipList);
	}
	(*pWin->funcs->PaintWindowBackground)(pDstDrawable, prgnExposed, 
				       PW_BACKGROUND);

	if (extents)
	    (*pscr->RegionReset)(prgnExposed, &expBox);
	else
	    (*pscr->TranslateRegion)(prgnExposed,
				     -pDstDrawable->x, -pDstDrawable->y);
    }
    if (prgnDstClip != prgnSrcClip)
	(*pscr->RegionDestroy)(prgnDstClip);
    (*pscr->RegionDestroy)(prgnSrcClip);
    if (pGC->graphicsExposures)
	return prgnExposed;
    else
    {
	(*pscr->RegionDestroy) (prgnExposed);
	return NULL;
    }
}

/* send GraphicsExpose events, or a NoExpose event, based on the region */

void
miSendGraphicsExpose (client, pRgn, drawable, major, minor)
    ClientPtr	client;
    RegionPtr	pRgn;
    XID		drawable;
    int	major;
    int	minor;
{
    if (pRgn && !REGION_NIL(pRgn))
    {
        xEvent *pEvent;
	register xEvent *pe;
	register BoxPtr pBox;
	register int i;
	int numRects;

	numRects = REGION_NUM_RECTS(pRgn);
	pBox = REGION_RECTS(pRgn);
	if(!(pEvent = (xEvent *)ALLOCATE_LOCAL(numRects * sizeof(xEvent))))
		return;
	pe = pEvent;

	for (i=1; i<=numRects; i++, pe++, pBox++)
	{
	    pe->u.u.type = GraphicsExpose;
	    pe->u.graphicsExposure.drawable = drawable;
	    pe->u.graphicsExposure.x = pBox->x1;
	    pe->u.graphicsExposure.y = pBox->y1;
	    pe->u.graphicsExposure.width = pBox->x2 - pBox->x1;
	    pe->u.graphicsExposure.height = pBox->y2 - pBox->y1;
	    pe->u.graphicsExposure.count = numRects - i;
	    pe->u.graphicsExposure.majorEvent = major;
	    pe->u.graphicsExposure.minorEvent = minor;
	}
	TryClientEvents(client, pEvent, numRects,
			    (Mask)0, NoEventMask, NullGrab);
	DEALLOCATE_LOCAL(pEvent);
    }
    else
    {
        xEvent event;
	event.u.u.type = NoExpose;
	event.u.noExposure.drawable = drawable;
	event.u.noExposure.majorEvent = major;
	event.u.noExposure.minorEvent = minor;
	TryClientEvents(client, &event, 1,
	    (Mask)0, NoEventMask, NullGrab);
    }
}

#ifdef notdef
void
miSendNoExpose(pGC)
    GCPtr pGC;
{
    if (pGC->graphicsExposures)
    {
        xEvent event;
	event.u.u.type = NoExpose;
	event.u.noExposure.drawable = 
		    requestingClient->lastDrawableID;
        TryClientEvents(requestingClient, &event, 1,
	        0, NoEventMask, NullGrab);
    }
}
#endif

void 
miWindowExposures(pWin, prgn)
    WindowPtr pWin;
    register RegionPtr prgn;
{
    if (!REGION_NIL(prgn))
    {
	xEvent *pEvent;
	register xEvent *pe;
	register BoxPtr pBox;
	register int i;
	RegionPtr exposures = prgn;

	/*
	 * Restore from backing-store FIRST.
	 */
 	if (pWin->backStorage)
	    /*
	     * in some cases, backing store will cause a different
	     * region to be exposed than needs to be repainted
	     * (like when a window is mapped).  RestoreAreas is
	     * allowed to return a region other than prgn,
	     * in which case this routine will free the resultant
	     * region.  If exposures is null, then no events will
	     * be sent to the client; if prgn is empty
	     * no areas will be repainted.
	     */
	    exposures = (*pWin->backStorage->funcs->RestoreAreas)(pWin, prgn);
	if (exposures && (REGION_NUM_RECTS(exposures) > RECTLIMIT))
	{
	    /*
	     * If we have LOTS of rectangles, we decide to take the extents
	     * and force an exposure on that.  This should require much less
	     * work overall, on both client and server.  This is cheating, but
	     * isn't prohibited by the protocol ("spontaneous combustion" :-).
	     */
	    BoxRec box;

	    box = *(* pWin->drawable.pScreen->RegionExtents)(exposures);
	    if (exposures == prgn) {
		exposures = (* pWin->drawable.pScreen->RegionCreate)(&box, 1);
		(* pWin->drawable.pScreen->RegionReset)(prgn, &box);
	    } else {
		(* pWin->drawable.pScreen->RegionReset)(exposures, &box);
		(* pWin->drawable.pScreen->Union)(prgn, prgn, exposures);
	    }
	    /* PaintWindowBackground doesn't clip, so we have to */
	    (* pWin->drawable.pScreen->Intersect)(prgn, prgn, &pWin->clipList);
	    /* need to clear out new areas of backing store, too */
	    if (pWin->backStorage)
		(* pWin->backStorage->funcs->ClearToBackground)(
					     pWin,
					     box.x1 - pWin->drawable.x,
					     box.y1 - pWin->drawable.y,
					     box.x2 - box.x1,
					     box.y2 - box.y1,
					     FALSE);
	}
	if (!REGION_NIL(prgn))
	    (*pWin->funcs->PaintWindowBackground)(pWin, prgn, PW_BACKGROUND);
	if (exposures && !REGION_NIL(exposures))
	{
	    int numRects;

	    (* pWin->drawable.pScreen->TranslateRegion)(exposures,
			-pWin->drawable.x, -pWin->drawable.y);
	    numRects = REGION_NUM_RECTS(exposures);
	    pBox = REGION_RECTS(exposures);

	    pEvent = (xEvent *) ALLOCATE_LOCAL(numRects * sizeof(xEvent));
	    if (pEvent) {
	    	pe = pEvent;
    
	    	for (i=1; i<=numRects; i++, pe++, pBox++)
	    	{
		    pe->u.u.type = Expose;
		    pe->u.expose.window = pWin->drawable.id;
		    pe->u.expose.x = pBox->x1;
		    pe->u.expose.y = pBox->y1;
		    pe->u.expose.width = pBox->x2 - pBox->x1;
		    pe->u.expose.height = pBox->y2 - pBox->y1;
		    pe->u.expose.count = (numRects - i);
	    	}
	    	DeliverEvents(pWin, pEvent, numRects, NullWindow);
	    	DEALLOCATE_LOCAL(pEvent);
	    }
	}
	if (exposures && exposures != prgn)
	    (* pWin->drawable.pScreen->RegionDestroy) (exposures);
	(* pWin->drawable.pScreen->RegionEmpty)(prgn);
    }
}


/*
    this code is highly unlikely.  it is not haile selassie.

    there is some hair here.  we can't just use the window's
clip region as it is, because if we are painting the border,
the border is not in the client area and so we will be excluded
when we validate the GC, and if we are painting a parent-relative
background, the area we want to paint is in some other window.
since we trust the code calling us to tell us to paint only areas
that are really ours, we will temporarily give the window a
clipList the size of the whole screen and an origin at (0,0).
this more or less assumes that ddX code will do translation
based on the window's absolute position, and that ValidateGC will
look at clipList, and that no other fields from the
window will be used.  it's not possible to just draw
in the root because it may be a different depth.

to get the tile to align correctly we set the GC's tile origin to
be the (x,y) of the window's upper left corner, after which we
get the right bits when drawing into the root.

because the clip_mask is being set to None, we may call DoChangeGC with
fPointer set true, thus we no longer need to install the background or
border tile in the resource table.
*/

static GCPtr	screenContext[MAXSCREENS];

/*ARGSUSED*/
static
tossGC (pGC, id)
GCPtr pGC;
GContext id;
{
    int i;

    if (screenContext[i = pGC->pScreen->myNum] == pGC)
	screenContext[i] = (GCPtr)NULL;
    FreeGC (pGC, id);
}


void
miPaintWindow(pWin, prgn, what)
WindowPtr pWin;
RegionPtr prgn;
int what;
{
    int	status;

    Bool usingScratchGC = FALSE;
    WindowPtr pRoot;
	
#define FUNCTION	0
#define FOREGROUND	1
#define TILE		2
#define FILLSTYLE	3
#define ABSX		4
#define ABSY		5
#define CLIPMASK	6
#define SUBWINDOW	7
#define COUNT_BITS	8

    XID gcval[7];
    XID newValues [COUNT_BITS];

    BITS32 gcmask, index, mask;
    RegionRec prgnWin;
    DDXPointRec oldCorner;
    BoxRec box;
    GCPtr pGC;
    register int i;
    register BoxPtr pbox;
    register ScreenPtr pScreen = pWin->drawable.pScreen;
    register xRectangle *prect;
    int numRects;

    gcmask = 0;

    if (what == PW_BACKGROUND)
    {
	switch (pWin->backgroundState) {
	case None:
	    return;
	case ParentRelative:
	    (*pWin->parent->funcs->PaintWindowBackground)(pWin->parent, prgn, what);
	    return;
	case BackgroundPixel:
	    newValues[FOREGROUND] = pWin->background.pixel;
	    newValues[FILLSTYLE] = FillSolid;
	    gcmask |= GCForeground | GCFillStyle;
	    break;
	case BackgroundPixmap:
	    newValues[TILE] = (XID)pWin->background.pixmap;
	    newValues[FILLSTYLE] = FillTiled;
	    gcmask |= GCTile | GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
	    break;
	}
    }
    else
    {
	if (pWin->borderIsPixel)
	{
	    newValues[FOREGROUND] = pWin->border.pixel;
	    newValues[FILLSTYLE] = FillSolid;
	    gcmask |= GCForeground | GCFillStyle;
	}
	else
	{
	    newValues[TILE] = (XID)pWin->border.pixmap;
	    newValues[FILLSTYLE] = FillTiled;
	    gcmask |= GCTile | GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
	}
    }

    prect = (xRectangle *)ALLOCATE_LOCAL(REGION_NUM_RECTS(prgn) *
					 sizeof(xRectangle));
    if (!prect)
	return;

    newValues[FUNCTION] = GXcopy;
    gcmask |= GCFunction | GCClipMask;

    i = pScreen->myNum;
    pRoot = WindowTable[i];

    if (wVisual (pWin) != wVisual (pRoot))
    {
	usingScratchGC = TRUE;
	pGC = GetScratchGC(pWin->drawable.depth, pWin->drawable.pScreen);
	if (!pGC)
	{
	    DEALLOCATE_LOCAL(prect);
	    return;
	}
	/*
	 * mash the clip list so we can paint the border by
	 * mangling the window in place, pretending it
	 * spans the entire screen
	 */
	if (what == PW_BORDER)
	{
	    prgnWin = pWin->clipList;
	    oldCorner.x = pWin->drawable.x;
	    oldCorner.y = pWin->drawable.y;
	    pWin->drawable.x = pWin->drawable.y = 0;
	    box.x1 = 0;
	    box.y1 = 0;
	    box.x2 = pScreen->width;
	    box.y2 = pScreen->height;
	    (*pScreen->RegionInit)(&pWin->clipList, &box, 1);
	    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	    newValues[ABSX] = pWin->drawable.x;
	    newValues[ABSY] = pWin->drawable.y;
	}
	else
	{
	    newValues[ABSX] = 0;
	    newValues[ABSY] = 0;
	}
    } else {
	/*
	 * draw the background to the root window
	 */
	if (screenContext[i] == (GCPtr)NULL)
	{
	    screenContext[i] = CreateGC((DrawablePtr)pWin, (BITS32) 0,
					(XID *)NULL, &status);
	    if (!screenContext[i])
		return;
	    if (!AddResource(FakeClientID(0), RT_GC, (pointer)screenContext[i],
			     tossGC, RC_CORE))
	        return;
	}
	pGC = screenContext[i];
	newValues[SUBWINDOW] = IncludeInferiors;
	newValues[ABSX] = pWin->drawable.x;
	newValues[ABSY] = pWin->drawable.y;
	gcmask |= GCSubwindowMode;
	pWin = pRoot;
    }
    
    if (pWin->backStorage)
	(*pWin->backStorage->funcs->DrawGuarantee) (pWin, pGC, GuaranteeVisBack);

    mask = gcmask;
    gcmask = 0;
    i = 0;
    while (mask) {
    	index = lowbit (mask);
	mask &= ~index;
	switch (index) {
	case GCFunction:
	    if ((XID) pGC->alu != newValues[FUNCTION]) {
		gcmask |= index;
		gcval[i++] = newValues[FUNCTION];
	    }
	    break;
	case GCTileStipXOrigin:
	    if ((XID) pGC->patOrg.x != newValues[ABSX]) {
		gcmask |= index;
		gcval[i++] = newValues[ABSX];
	    }
	    break;
	case GCTileStipYOrigin:
	    if ((XID) pGC->patOrg.y != newValues[ABSY]) {
		gcmask |= index;
		gcval[i++] = newValues[ABSY];
	    }
	    break;
	case GCClipMask:
	    if ((XID) pGC->clientClipType != CT_NONE) {
		gcmask |= index;
		gcval[i++] = CT_NONE;
	    }
	    break;
	case GCSubwindowMode:
	    if ((XID) pGC->subWindowMode != newValues[SUBWINDOW]) {
		gcmask |= index;
		gcval[i++] = newValues[SUBWINDOW];
	    }
	    break;
	case GCTile:
	    if (pGC->tileIsPixel || (XID) pGC->tile.pixmap != newValues[TILE])
 	    {
		gcmask |= index;
		gcval[i++] = newValues[TILE];
	    }
	    break;
	case GCFillStyle:
	    if ((XID) pGC->fillStyle != newValues[FILLSTYLE]) {
		gcmask |= index;
		gcval[i++] = newValues[FILLSTYLE];
	    }
	    break;
	case GCForeground:
	    if ((XID) pGC->fgPixel != newValues[FOREGROUND]) {
		gcmask |= index;
		gcval[i++] = newValues[FOREGROUND];
	    }
	    break;
	}
    }

    if (gcmask)
        DoChangeGC(pGC, gcmask, gcval, 1);

    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC((DrawablePtr)pWin, pGC);

    numRects = REGION_NUM_RECTS(prgn);
    pbox = REGION_RECTS(prgn);
    for (i= numRects; --i >= 0; pbox++, prect++)
    {
	prect->x = pbox->x1;
	prect->y = pbox->y1;
	prect->width = pbox->x2 - pbox->x1;
	prect->height = pbox->y2 - pbox->y1;
    }
    prect -= numRects;
    (*pGC->ops->PolyFillRect)(pWin, pGC, numRects, prect);
    DEALLOCATE_LOCAL(prect);

    if (pWin->backStorage)
	(*pWin->backStorage->funcs->DrawGuarantee) (pWin, pGC, GuaranteeNothing);

    if (usingScratchGC)
    {
	if (what == PW_BORDER)
	{
	    (*pScreen->RegionUninit)(&pWin->clipList);
	    pWin->clipList = prgnWin;
	    pWin->drawable.x = oldCorner.x;
	    pWin->drawable.y = oldCorner.y;
	    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	}
	FreeScratchGC(pGC);
    }
}


/* MICLEARDRAWABLE -- sets the entire drawable to the background color of
 * the GC.  Useful when we have a scratch drawable and need to initialize 
 * it. */
miClearDrawable(pDraw, pGC)
    DrawablePtr	pDraw;
    GCPtr	pGC;
{
    unsigned long    fg = pGC->fgPixel;
    unsigned long    bg = pGC->bgPixel;
    xRectangle rect;

    rect.x = 0;
    rect.y = 0;
    rect.width = pDraw->width;
    rect.height = pDraw->height;
    DoChangeGC(pGC, GCForeground, &bg, 0);
    ValidateGC(pDraw, pGC);
    (*pGC->ops->PolyFillRect)(pDraw, pGC, 1, &rect);
    DoChangeGC(pGC, GCForeground, &fg, 0);
    ValidateGC(pDraw, pGC);
}
