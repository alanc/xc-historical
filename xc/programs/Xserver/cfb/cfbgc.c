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
#ifdef CFBROTPIX
extern cfbXRotatePixmap();
extern cfbYRotatePixmap();
#endif

static void cfbValidateGC(), cfbChangeGC(), cfbCopyGC(), cfbDestroyGC();
static void cfbChangeClip(), cfbDestroyClip(), cfbCopyClip();

static GCFuncs cfbFuncs = {
    cfbValidateGC,
    cfbChangeGC,
    cfbCopyGC,
    cfbDestroyGC,
    cfbChangeClip,
    cfbDestroyClip,
    cfbCopyClip,
};

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

    /* cfb wants to translate before scan convesion */
    pGC->miTranslate = 1;

    pPriv = (cfbPrivGC *) xalloc(sizeof(cfbPrivGC));
    if (!pPriv)
	return FALSE;
    else {
	pPriv->rop = pGC->alu;
	pPriv->fExpose = TRUE;
	pGC->devPrivates[cfbGCPrivateIndex].ptr = (pointer) pPriv;
#ifdef CFBROTPIX
	pPriv->pRotatedTile = NullPixmap;
	pPriv->pRotatedStipple = NullPixmap;
#endif
	pPriv->pAbsClientRegion = (*pGC->pScreen->RegionCreate) (NULL, 1);
	/* since freeCompClip isn't FREE_CC, we don't need to create
	   a null region -- no one will try to free the field.
	*/
	pPriv->freeCompClip = REPLACE_CC;
    }
    return TRUE;
}

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
#ifdef CFBROTPIX
    if (pPriv->pRotatedTile)
	cfbDestroyPixmap(pPriv->pRotatedTile);
    if (pPriv->pRotatedStipple)
	cfbDestroyPixmap(pPriv->pRotatedStipple);
#endif
    if (pPriv->freeCompClip == FREE_CC)
	(*pGC->pScreen->RegionDestroy)(pPriv->pCompositeClip);
    if(pPriv->pAbsClientRegion)
	(*pGC->pScreen->RegionDestroy)(pPriv->pAbsClientRegion);
    xfree(pGC->devPrivates[cfbGCPrivateIndex].ptr);
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
    ret = (GCOps *) xalloc (sizeof *ret);
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
#ifdef CFBROTPIX
    int         xrot, yrot;	/* rotations for tile and stipple pattern */
    Bool        fRotate = FALSE;/* True if rotated pixmaps are needed */
#endif
    int         new_line, new_text, new_fillspans;
    /* flags for changing the proc vector */
    cfbPrivGCPtr devPriv;
    DDXPointRec	oldOrg;		/* origin of thing GC was last used with */

    oldOrg = pGC->lastWinOrg;
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
	/*
	 * if there is a client clip (always a region, for us) AND it has
	 * moved or is different OR the window has moved we need to
	 * (re)translate it. 
	 */
	if ((pGC->clientClipType == CT_REGION) &&
	    ((changes & (GCClipXOrigin | GCClipYOrigin | GCClipMask)) ||
	     (oldOrg.x != pGC->lastWinOrg.x) || (oldOrg.y != pGC->lastWinOrg.y)
	     )
	    )
	{
	    /* retranslate client clip */
	    (*pGC->pScreen->RegionCopy) (devPriv->pAbsClientRegion,
					 pGC->clientClip);

	    (*pGC->pScreen->TranslateRegion) (
				devPriv->pAbsClientRegion,
				pGC->lastWinOrg.x + pGC->clipOrg.x,
				pGC->lastWinOrg.y + pGC->clipOrg.y);
	}

	if (pWin) {
	    RegionPtr   pregWin;
	    int         freeTmpClip, freeCompClip;

	    if (pGC->subWindowMode == IncludeInferiors) {
		pregWin = NotClippedByChildren(pWin);
		freeTmpClip = FREE_CC;
	    }
	    else {
		pregWin = pWin->clipList;
		freeTmpClip = REPLACE_CC;
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
		if (freeCompClip == FREE_CC) {
		    (*pGC->pScreen->RegionDestroy) (devPriv->pCompositeClip);
		}
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

		if ((freeTmpClip == FREE_CC) && (freeCompClip == FREE_CC)) {
		    (*pGC->pScreen->Intersect) (
						devPriv->pCompositeClip,
						pregWin,
					      devPriv->pAbsClientRegion);
		    (*pGC->pScreen->RegionDestroy) (pregWin);
		}
		else if ((freeTmpClip == REPLACE_CC) &&
			 (freeCompClip == FREE_CC)) {
		    (*pGC->pScreen->Intersect) (
						devPriv->pCompositeClip,
						pregWin,
					      devPriv->pAbsClientRegion);
		}
		else if ((freeTmpClip == FREE_CC) &&
			 (freeCompClip == REPLACE_CC)) {
		    (*pGC->pScreen->Intersect) (
						pregWin,
						pregWin,
					      devPriv->pAbsClientRegion);
		    devPriv->pCompositeClip = pregWin;
		}
		else if ((freeTmpClip == REPLACE_CC) &&
			 (freeCompClip == REPLACE_CC)) {
		    devPriv->pCompositeClip =
			(*pGC->pScreen->RegionCreate) (NULL, 1);
		    (*pGC->pScreen->Intersect) (
						devPriv->pCompositeClip,
						pregWin,
					      devPriv->pAbsClientRegion);
		}
		devPriv->freeCompClip = FREE_CC;
	    }
	}			/* end of composite clip for a window */
	else {
	    BoxRec      pixbounds;

	    pixbounds.x1 = 0;
	    pixbounds.y1 = 0;
	    pixbounds.x2 = pDrawable->width;
	    pixbounds.y2 = pDrawable->height;

	    if (devPriv->freeCompClip == FREE_CC)
		(*pGC->pScreen->RegionReset) (
				    devPriv->pCompositeClip, &pixbounds);
	    else {
		devPriv->freeCompClip = FREE_CC;
		devPriv->pCompositeClip =
		    (*pGC->pScreen->RegionCreate) (&pixbounds, 1);
	    }

	    if (pGC->clientClipType == CT_REGION)
		(*pGC->pScreen->Intersect) (
					    devPriv->pCompositeClip,
					    devPriv->pCompositeClip,
					    devPriv->pAbsClientRegion);
	}			/* end of composute clip for pixmap */
    }

/*
    if (pWin) {

	*
	 * rotate tile patterns so that pattern can be combined in word by
	 * word, but the pattern seems to begin aligned with the window 
	 *
	xrot = pWin->absCorner.x;
	yrot = pWin->absCorner.y;
    }
    else {
*/
#ifdef CFBROTPIX
	yrot = 0;
	xrot = 0;
#endif
/*
    }
*/


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
	    if (pGC->tile)
	    {
		int width = pGC->tile->drawable.width * PSZ;
		PixmapPtr ntile;

		if ((width <= 32) && !(width & (width - 1)) &&
		    (ntile = cfbCopyPixmap(pGC->tile)))
		{
		    (void)cfbPadPixmap(ntile);
		    cfbDestroyPixmap(pGC->tile);
		    pGC->tile = ntile;
		}
	    }
#ifdef CFBROTPIX
	    fRotate = TRUE;
#endif
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
#ifdef CFBROTPIX
	    fRotate = TRUE;
#endif
	    new_fillspans = TRUE;
	    break;

#ifdef CFBROTPIX
	case GCTileStipXOrigin:
	    fRotate = TRUE;
	    break;

	case GCTileStipYOrigin:
	    fRotate = TRUE;
	    break;
#endif

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

#ifdef CFBROTPIX
    if (xrot || yrot || fRotate) {
	/*
	 * First destroy any previously-rotated tile/stipple
	 */
	if (devPriv->pRotatedTile) {
	    cfbDestroyPixmap(devPriv->pRotatedTile);
	    devPriv->pRotatedTile = (PixmapPtr)NULL;
	}
	if (devPriv->pRotatedStipple) {
	    cfbDestroyPixmap(devPriv->pRotatedStipple);
	    devPriv->pRotatedStipple = (PixmapPtr)NULL;
	}
	if (pGC->tile)
	    devPriv->pRotatedTile = cfbCopyPixmap(pGC->tile);
	if (pGC->stipple)
	    devPriv->pRotatedStipple = cfbCopyPixmap(pGC->stipple);
	/*
	 * If we've gotten here, we're probably going to rotate the tile
	 * and/or stipple, so we have to add the pattern origin into
	 * the rotation factor, even if it hasn't changed.
	 */
	xrot += pGC->patOrg.x;
	yrot += pGC->patOrg.y;
	if (xrot) {
	    if (pGC->tile && devPriv->pRotatedTile)
		cfbXRotatePixmap(devPriv->pRotatedTile, xrot);
	    if (pGC->stipple && devPriv->pRotatedStipple)
		cfbXRotatePixmap(devPriv->pRotatedStipple, xrot);
	}
	if (yrot) {
	    if (pGC->tile && devPriv->pRotatedTile)
		cfbYRotatePixmap(devPriv->pRotatedTile, yrot);
	    if (pGC->stipple && devPriv->pRotatedStipple)
		cfbYRotatePixmap(devPriv->pRotatedStipple, yrot);
	}
    }
#endif
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
	pGC->clientClip = (pointer) mfbPixmapToRegion((PixmapPtr)pvalue);
	(*pGC->pScreen->DestroyPixmap)(pvalue);
    }
    else if (type == CT_REGION) {
	/* stuff the region in the GC */
	pGC->clientClip = pvalue;
    }
    else if (type != CT_NONE)
    {
	pGC->clientClip = (pointer) miRectsToRegion(pGC, nrects,
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
