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
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "dixfontstr.h"
#include "fontstruct.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "cfb.h"
#include "mistruct.h"
#include "mibstore.h"

#include "cfbmskbits.h"

static void cfbValidateGC(), cfbChangeGC(), cfbCopyGC(), cfbDestroyGC();
static void cfbChangeClip(), cfbDestroyClip(), cfbCopyClip();
static cfbDestroyOps();

static GCFuncs cfbFuncs = {
    cfbValidateGC,
    cfbChangeGC,
    cfbCopyGC,
    cfbDestroyGC,
    cfbChangeClip,
    cfbDestroyClip,
    cfbCopyClip,
};

extern void mfbPushPixels();

static GCOps	cfbTEOps = {
    cfbSolidFS,
    cfbSetSpans,
    miPutImage,
    cfbCopyArea,
    miCopyPlane,
    miPolyPoint,
    miZeroLine,
    miPolySegment,
    miPolyRectangle,
    miPolyArc,
    miFillPolygon,
    miPolyFillRect,
    miPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    cfbTEGlyphBlt,
    miPolyGlyphBlt,
    mfbPushPixels,
    miMiter,
};

static GCOps	cfbNonTEOps = {
    cfbSolidFS,
    cfbSetSpans,
    miPutImage,
    cfbCopyArea,
    miCopyPlane,
    miPolyPoint,
    miZeroLine,
    miPolySegment,
    miPolyRectangle,
    miPolyArc,
    miFillPolygon,
    miPolyFillRect,
    miPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    miImageGlyphBlt,
    miPolyGlyphBlt,
    mfbPushPixels,
    miMiter,
};

static GCOps *
matchCommon (pGC)
    GCPtr   pGC;
{
    if (pGC->lineWidth != 0)
	return 0;
    if (pGC->lineStyle != LineSolid)
	return 0;
    if (pGC->fillStyle != FillSolid)
	return 0;
    if (pGC->font &&
	(pGC->font->pFI->maxbounds.metrics.rightSideBearing -
         pGC->font->pFI->minbounds.metrics.leftSideBearing) <= 32 &&
	 pGC->font->pFI->terminalFont &&
	 pGC->fgPixel != pGC->bgPixel)
    {
	return &cfbTEOps;
    }
    else
    {
	return &cfbNonTEOps;
    }
}

Bool
cfbCreateGC(pGC)
    register GCPtr pGC;
{
    cfbPrivGC  *pPriv;

    switch (pGC->depth) {
    case 1:
	return (mfbCreateGC(pGC));
    case PSZ:
	break;
    default:
	ErrorF("cfbCreateGC: unsupported depth: %d\n", pGC->depth);
	return FALSE;
    }
    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;

    /*
     * some of the output primitives aren't really necessary, since they
     * will be filled in ValidateGC because of dix/CreateGC() setting all
     * the change bits.  Others are necessary because although they depend
     * on being a color frame buffer, they don't change 
     */

    pGC->ops = &cfbNonTEOps;
    pGC->funcs = &cfbFuncs;

    /* cfb wants to translate before scan conversion */
    pGC->miTranslate = 1;

    pPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    pPriv->rop = pGC->alu;
    pPriv->fExpose = TRUE;
    pPriv->freeCompClip = FALSE;
    return TRUE;
}

/*ARGSUSED*/
static void
cfbChangeGC(pGC, mask)
    GC		    *pGC;
    BITS32	    mask;
{
    return;
}

static void
cfbDestroyGC(pGC)
    GC 			*pGC;
{
    cfbPrivGC *pPriv;

    pPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    if (pPriv->freeCompClip)
	(*pGC->pScreen->RegionDestroy)(pPriv->pCompositeClip);
    cfbDestroyOps (pGC->ops);
}

/*
 * create a private op array for a gc
 */

static GCOps *
cfbCreateOps (prototype)
    GCOps	*prototype;
{
    GCOps	*ret;
    extern Bool	Must_have_memory;

    /* XXX */ Must_have_memory = TRUE;
    ret = (GCOps *) xalloc (sizeof(GCOps));
    /* XXX */ Must_have_memory = FALSE;
    if (!ret)
	return 0;
    *ret = *prototype;
    ret->devPrivate.val = 1;
    return ret;
}

static
cfbDestroyOps (ops)
    GCOps   *ops;
{
    if (ops->devPrivate.val)
	xfree (ops);
}

/* Clipping conventions
	if the drawable is a window
	    CT_REGION ==> pCompositeClip really is the composite
	    CT_other ==> pCompositeClip is the window clip region
	if the drawable is a pixmap
	    CT_REGION ==> pCompositeClip is the translated client region
		clipped to the pixmap boundary
	    CT_other ==> pCompositeClip is the pixmap bounding box
*/

static void
cfbValidateGC(pGC, changes, pDrawable)
    register GCPtr  pGC;
    Mask	    changes;
    DrawablePtr	    pDrawable;
{
    WindowPtr   pWin;
    int         mask;		/* stateChanges */
    int         index;		/* used for stepping through bitfields */
    int         new_line, new_text, new_fillspans;
    /* flags for changing the proc vector */
    cfbPrivGCPtr devPriv;

    pGC->lastWinOrg.x = pDrawable->x;
    pGC->lastWinOrg.y = pDrawable->y;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pWin = (WindowPtr) pDrawable;
    }
    else
    {
	pWin = (WindowPtr) NULL;
    }

    devPriv = ((cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr));

    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */

    if ((changes & (GCClipXOrigin | GCClipYOrigin | GCClipMask)) ||
	(changes & GCSubwindowMode) ||
	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
	)
    {
	ScreenPtr pScreen = pGC->pScreen;

	if (pWin) {
	    RegionPtr   pregWin;
	    Bool        freeTmpClip, freeCompClip;

	    if (pGC->subWindowMode == IncludeInferiors) {
		pregWin = NotClippedByChildren(pWin);
		freeTmpClip = TRUE;
	    }
	    else {
		pregWin = &pWin->clipList;
		freeTmpClip = FALSE;
	    }
	    freeCompClip = devPriv->freeCompClip;

	    /*
	     * if there is no client clip, we can get by with just keeping
	     * the pointer we got, and remembering whether or not should
	     * destroy (or maybe re-use) it later.  this way, we avoid
	     * unnecessary copying of regions.  (this wins especially if
	     * many clients clip by children and have no client clip.) 
	     */
	    if (pGC->clientClipType == CT_NONE) {
		if (freeCompClip)
		    (*pScreen->RegionDestroy) (devPriv->pCompositeClip);
		devPriv->pCompositeClip = pregWin;
		devPriv->freeCompClip = freeTmpClip;
	    }
	    else {
		/*
		 * we need one 'real' region to put into the composite
		 * clip. if pregWin the current composite clip are real,
		 * we can get rid of one. if pregWin is real and the
		 * current composite clip isn't, use pregWin for the
		 * composite clip. if the current composite clip is real
		 * and pregWin isn't, use the current composite clip. if
		 * neither is real, create a new region. 
		 */

		(*pScreen->TranslateRegion)(pGC->clientClip,
					    pDrawable->x + pGC->clipOrg.x,
					    pDrawable->y + pGC->clipOrg.y);
						  
		if (freeCompClip)
		{
		    (*pGC->pScreen->Intersect)(devPriv->pCompositeClip,
					       pregWin, pGC->clientClip);
		    if (freeTmpClip)
			(*pScreen->RegionDestroy)(pregWin);
		}
		else if (freeTmpClip)
		{
		    (*pScreen->Intersect)(pregWin, pregWin, pGC->clientClip);
		    devPriv->pCompositeClip = pregWin;
		}
		else
		{
		    devPriv->pCompositeClip = (*pScreen->RegionCreate)(NullBox,
								       0);
		    (*pScreen->Intersect)(devPriv->pCompositeClip,
					  pregWin, pGC->clientClip);
		}
		devPriv->freeCompClip = TRUE;
		(*pScreen->TranslateRegion)(pGC->clientClip,
					    -(pDrawable->x + pGC->clipOrg.x),
					    -(pDrawable->y + pGC->clipOrg.y));
						  
	    }
	}			/* end of composite clip for a window */
	else {
	    BoxRec      pixbounds;

	    pixbounds.x1 = 0;
	    pixbounds.y1 = 0;
	    pixbounds.x2 = pDrawable->width;
	    pixbounds.y2 = pDrawable->height;

	    if (devPriv->freeCompClip)
		(*pScreen->RegionReset)(devPriv->pCompositeClip, &pixbounds);
	    else {
		devPriv->freeCompClip = TRUE;
		devPriv->pCompositeClip = (*pScreen->RegionCreate)(&pixbounds,
								   1);
	    }

	    if (pGC->clientClipType == CT_REGION)
		(*pScreen->Intersect)(devPriv->pCompositeClip,
				      devPriv->pCompositeClip,
				      pGC->clientClip);
	}			/* end of composute clip for pixmap */
    }

    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;

    mask = changes;
    while (mask) {
	index = lowbit (mask);
	mask &= ~index;

	/*
	 * this switch acculmulates a list of which procedures might have
	 * to change due to changes in the GC.  in some cases (e.g.
	 * changing one 16 bit tile for another) we might not really need
	 * a change, but the code is being paranoid. this sort of batching
	 * wins if, for example, the alu and the font have been changed,
	 * or any other pair of items that both change the same thing. 
	 */
	switch (index) {
	case GCFunction:
	case GCForeground:
	    new_text = TRUE;
	    break;
	case GCPlaneMask:
	    break;
	case GCBackground:
	    new_fillspans = TRUE;
	    break;
	case GCLineStyle:
	case GCLineWidth:
	case GCJoinStyle:
	    new_line = TRUE;
	    break;
	case GCCapStyle:
	    break;
	case GCFillStyle:
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_line = TRUE;
	    break;
	case GCFillRule:
	    break;
	case GCTile:
	    if (!pGC->tileIsPixel)
	    {
		int width = pGC->tile.pixmap->drawable.width * PSZ;
		PixmapPtr ntile;

		if ((width <= 32) && !(width & (width - 1)) &&
		    (ntile = cfbCopyPixmap(pGC->tile.pixmap)))
		{
		    (void)cfbPadPixmap(ntile);
		    cfbDestroyPixmap(pGC->tile.pixmap);
		    pGC->tile.pixmap = ntile;
		}
	    }
	    new_fillspans = TRUE;
	    break;

	case GCStipple:
	    if (pGC->stipple)
	    {
		int width = pGC->stipple->drawable.width * PSZ;
		PixmapPtr nstipple;

		if ((width <= 32) && !(width & (width - 1)) &&
		    (nstipple = cfbCopyPixmap(pGC->stipple)))
		{
		    (void)cfbPadPixmap(nstipple);
		    cfbDestroyPixmap(pGC->stipple);
		    pGC->stipple = nstipple;
		}
	    }
	    new_fillspans = TRUE;
	    break;

	case GCFont:
	    new_text = TRUE;
	    break;
	case GCSubwindowMode:
	    break;
	case GCGraphicsExposures:
	    break;
	case GCClipXOrigin:
	    break;
	case GCClipYOrigin:
	    break;
	case GCClipMask:
	    break;
	case GCDashOffset:
	    break;
	case GCDashList:
	    break;
	case GCArcMode:
	    break;
	default:
	    break;
	}
    }

    /*
     * If the drawable has changed,  check its depth & ensure suitable
     * entries are in the proc vector. 
     */
    if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
	new_fillspans = TRUE;	/* deal with FillSpans later */
    }

    if (new_line || new_fillspans || new_text)
    {
	GCOps	*newops;

	if (newops = matchCommon (pGC))
 	{
	    if (pGC->ops->devPrivate.val)
		cfbDestroyOps (pGC->ops);
	    pGC->ops = newops;
	    new_line = new_fillspans = new_text = 0;
	}
 	else
 	{
	    if (!pGC->ops->devPrivate.val)
	    {
		pGC->ops = cfbCreateOps (pGC->ops);
		pGC->ops->devPrivate.val = 1;
	    }
	}
    }

    /* deal with the changes we've collected */

    if (new_line)
    {
	if (pGC->lineStyle == LineSolid)
	{
	    if(pGC->lineWidth == 0)
		pGC->ops->Polylines = miZeroLine;
	    else
		pGC->ops->Polylines = miWideLine;
	}
	else
	    pGC->ops->Polylines = miWideDash;

	switch(pGC->joinStyle)
	{
	  case JoinMiter:
	    pGC->ops->LineHelper = miMiter;
	    break;
	  case JoinRound:
	  case JoinBevel:
	    pGC->ops->LineHelper = miNotMiter;
	    break;
	}
    }

    if (new_text && (pGC->font))
    {
        if ((pGC->font->pFI->maxbounds.metrics.rightSideBearing -
             pGC->font->pFI->minbounds.metrics.leftSideBearing) > 32)
        {
            pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
        else
        {
            /* special case ImageGlyphBlt for terminal emulator fonts */
            if ((pGC->font->pFI->terminalFont) &&
                (pGC->fgPixel != pGC->bgPixel))
	    {
                pGC->ops->ImageGlyphBlt = cfbTEGlyphBlt;
	    }
            else
                pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
    }    


    if (new_fillspans) {
	switch (pGC->fillStyle) {
	case FillSolid:
	    pGC->ops->FillSpans = cfbSolidFS;
	    break;
	case FillTiled:
	    pGC->ops->FillSpans = cfbUnnaturalTileFS;
	    break;
	case FillStippled:
	    pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	    break;
	case FillOpaqueStippled:
	    if (pGC->fgPixel == pGC->bgPixel)
		pGC->ops->FillSpans = cfbSolidFS;
	    else
		pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	    break;
	default:
	    FatalError("cfbValidateGC: illegal fillStyle\n");
	}
    } /* end of new_fillspans */
}

static void
cfbDestroyClip(pGC)
    GCPtr	pGC;
{
    if(pGC->clientClipType == CT_NONE)
	return;
    else if (pGC->clientClipType == CT_PIXMAP)
    {
	cfbDestroyPixmap((PixmapPtr)(pGC->clientClip));
    }
    else
    {
	/* we know we'll never have a list of rectangles, since
	   ChangeClip immediately turns them into a region 
	*/
        (*pGC->pScreen->RegionDestroy)(pGC->clientClip);
    }
    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;
}

static void
cfbChangeClip(pGC, type, pvalue, nrects)
    GCPtr	pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    cfbDestroyClip(pGC);
    if(type == CT_PIXMAP)
    {
	pGC->clientClip = (pointer) (*pGC->pScreen->BitmapToRegion)((PixmapPtr)pvalue);
	(*pGC->pScreen->DestroyPixmap)(pvalue);
    }
    else if (type == CT_REGION) {
	/* stuff the region in the GC */
	pGC->clientClip = pvalue;
    }
    else if (type != CT_NONE)
    {
	pGC->clientClip = (pointer) (*pGC->pScreen->RectsToRegion)(nrects,
						    (xRectangle *)pvalue,
						    type);
	xfree(pvalue);
    }
    pGC->clientClipType = (type != CT_NONE && pGC->clientClip) ? CT_REGION :
								 CT_NONE;
    pGC->stateChanges |= GCClipMask;
}

static void
cfbCopyClip (pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
    RegionPtr prgnNew;

    switch(pgcSrc->clientClipType)
    {
      case CT_PIXMAP:
	((PixmapPtr) pgcSrc->clientClip)->refcnt++;
	/* Fall through !! */
      case CT_NONE:
        cfbChangeClip(pgcDst, pgcSrc->clientClipType, pgcSrc->clientClip, 0);
        break;
      case CT_REGION:
        prgnNew = (*pgcSrc->pScreen->RegionCreate)(NULL, 1);
        (*pgcSrc->pScreen->RegionCopy)(prgnNew,
                                       (RegionPtr)(pgcSrc->clientClip));
        cfbChangeClip(pgcDst, CT_REGION, (pointer)prgnNew, 0);
        break;
    }
}

/*ARGSUSED*/
static void
cfbCopyGC (pGCSrc, changes, pGCDst)
    GCPtr	pGCSrc;
    Mask 	changes;
    GCPtr	pGCDst;
{
    RegionPtr		pClip;

    if(changes & GCClipMask)
    {
	if(pGCDst->clientClipType == CT_PIXMAP)
	{
	    ((PixmapPtr)pGCDst->clientClip)->refcnt++;
	}
	else if(pGCDst->clientClipType == CT_REGION)
	{
	    BoxRec pixbounds;

	    pixbounds.x1 = 0;
	    pixbounds.y1 = 0;
	    pixbounds.x2 = 0;
	    pixbounds.y2 = 0;

	    pClip = (RegionPtr) pGCDst->clientClip;
	    pGCDst->clientClip =
	        (pointer)(* pGCDst->pScreen->RegionCreate)(&pixbounds, 1);
	    (* pGCDst->pScreen->RegionCopy)(pGCDst->clientClip, pClip);
	}
    }
}
