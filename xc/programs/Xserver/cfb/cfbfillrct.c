/*
 * Fill rectangles.
 */

/*
Copyright 1989 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission.  M.I.T. makes no
representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
*/

/* $XConsortium: cfbfillrct.c,v 5.9 89/11/19 15:42:14 rws Exp $ */

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"

#include "cfb.h"
#include "cfbmskbits.h"

#if PPW == 4
extern void cfb8FillRectOpaqueStippled32();
extern void cfb8FillRectTransparentStippled32();
extern void cfb8FillRectStippledUnnatural();
#endif

void
cfbFillRectSolid (pDrawable, pGC, nBox, pBox)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nBox;
    BoxPtr	    pBox;
{
    unsigned long   *pdstBase, *pdstRect;
    int		    widthDst;
    register int    h;
    register unsigned long   xor;
    register unsigned long   *pdst;
    register unsigned long   leftMask, rightMask;
    int		    nmiddle;
    register int    m;
    int		    w;
    unsigned long   and;
    cfbPrivGCPtr    devPriv;
    int		    rrop;

    devPriv = ((cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr));
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pdstBase = (unsigned long *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	widthDst = (int)
		  (((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	pdstBase = (unsigned long *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	widthDst = (int)(((PixmapPtr)pDrawable)->devKind) >> 2;
    }

    xor = devPriv->xor;
    and = devPriv->and;
    rrop = devPriv->rop;
    for (; nBox; nBox--, pBox++)
    {
    	pdstRect = pdstBase + pBox->y1 * widthDst;
    	h = pBox->y2 - pBox->y1;
	w = pBox->x2 - pBox->x1;
#if PPW == 4
	if (w == 1)
	{
	    register char    *pdstb = ((char *) pdstRect) + pBox->x1;
	    int	    incr = widthDst << 2;

	    switch (rrop) {
	    case GXcopy:
		while (h--)
		{
		    *pdstb = xor;
		    pdstb += incr;
		}
		break;
	    case GXxor:
		while (h--)
		{
		    *pdstb ^= xor;
		    pdstb += incr;
		}
		break;
	    default:
		while (h--)
		{
		    *pdstb = DoRRop (*pdstb, and, xor);
		    pdstb += incr;
		}
		break;
	    }
	}
	else
	{
#endif
	pdstRect += (pBox->x1 >> PWSH);
	if ((pBox->x1 & PIM) + w <= PPW)
	{
	    register unsigned long  lxor, land;

	    maskpartialbits(pBox->x1, w, leftMask);
	    pdst = pdstRect;
	    land = and | ~leftMask;
	    lxor = xor & leftMask;
	    while (h--) {
		*pdst = DoRRop (*pdst, land, lxor);
		pdst += widthDst;
	    }
	}
	else
	{
	    maskbits (pBox->x1, w, leftMask, rightMask, nmiddle);
	    if (leftMask)
	    {
		unsigned long	land, lxor;

		land = and | ~leftMask;
		lxor = xor & leftMask;
		if (rightMask)	/* left mask and right mask */
		{
		    unsigned long   rand, rxor;

		    rand = and | ~rightMask;
		    rxor = xor & rightMask;
		    switch (rrop) {
		    case GXcopy:
			while (h--) {
			    pdst = pdstRect;
			    *pdst = DoRRop (*pdst, land, lxor);
			    pdst++;
			    m = nmiddle;
			    while (m--)
				*pdst++ = xor;
			    *pdst = DoRRop (*pdst, rand, rxor);
			    pdstRect += widthDst;
			}
			break;
		    case GXxor:
			while (h--) {
			    pdst = pdstRect;
			    *pdst = DoRRop (*pdst, land, lxor);
			    pdst++;
			    m = nmiddle;
			    while (m--)
				*pdst++ ^= xor;
			    *pdst = DoRRop (*pdst, rand, rxor);
			    pdstRect += widthDst;
			}
			break;
		    default:
			while (h--) {
			    pdst = pdstRect;
			    *pdst = DoRRop (*pdst, land, lxor);
			    pdst++;
			    m = nmiddle;
			    while (m--)
			    {
				*pdst = DoRRop (*pdst, and, xor);
				pdst++;
			    }
			    *pdst = DoRRop (*pdst, rand, rxor);
			    pdstRect += widthDst;
			}
			break;
		    }
		}
		else	/* left mask and no right mask */
		{
		    switch (rrop) {
		    case GXcopy:
			while (h--) {
			    pdst = pdstRect;
			    *pdst = DoRRop (*pdst, land, lxor);
			    pdst++;
			    m = nmiddle;
			    while (m--)
				*pdst++ = xor;
			    pdstRect += widthDst;
			}
			break;
		    case GXxor:
			while (h--) {
			    pdst = pdstRect;
			    *pdst = DoRRop (*pdst, land, lxor);
			    pdst++;
			    m = nmiddle;
			    while (m--)
				*pdst++ ^= xor;
			    pdstRect += widthDst;
			}
			break;
		    default:
			while (h--) {
			    pdst = pdstRect;
			    *pdst = DoRRop (*pdst, land, lxor);
			    pdst++;
			    m = nmiddle;
			    while (m--)
			    {
				*pdst = DoRRop (*pdst, and, xor);
				pdst++;
			    }
			    pdstRect += widthDst;
			}
			break;
		    }
		}
	    }
	    else
	    {
		if (rightMask)	/* no left mask and right mask */
		{
		    unsigned long   rand, rxor;

		    rand = and | ~rightMask;
		    rxor = xor & rightMask;
		    switch (rrop) {
		    case GXcopy:
			while (h--) {
			    pdst = pdstRect;
			    m = nmiddle;
			    while (m--)
				*pdst++ = xor;
			    *pdst = DoRRop (*pdst, rand, rxor);
			    pdstRect += widthDst;
			}
			break;
		    case GXxor:
			while (h--) {
			    pdst = pdstRect;
			    m = nmiddle;
			    while (m--)
				*pdst++ ^= xor;
			    *pdst = DoRRop (*pdst, rand, rxor);
			    pdstRect += widthDst;
			}
			break;
		    default:
			while (h--) {
			    pdst = pdstRect;
			    m = nmiddle;
			    while (m--)
			    {
				*pdst = DoRRop (*pdst, and, xor);
				pdst++;
			    }
			    *pdst = DoRRop (*pdst, rand, rxor);
			    pdstRect += widthDst;
			}
			break;
		    }
		}
		else	/* no left mask and no right mask */
		{
		    switch (rrop) {
		    case GXcopy:
			while (h--) {
			    pdst = pdstRect;
			    m = nmiddle;
			    while (m--)
				*pdst++ = xor;
			    pdstRect += widthDst;
			}
			break;
		    case GXxor:
			while (h--) {
			    pdst = pdstRect;
			    m = nmiddle;
			    while (m--)
				*pdst++ ^= xor;
			    pdstRect += widthDst;
			}
			break;
		    default:
			while (h--) {
			    pdst = pdstRect;
			    m = nmiddle;
			    while (m--)
			    {
				*pdst = DoRRop (*pdst, and, xor);
				pdst++;
			    }
			    pdstRect += widthDst;
			}
			break;
		    }
		}
	    }
	}
#if PPW == 4
	}
#endif
    }
}

void
cfbFillRectTile32 (pDrawable, pGC, nBox, pBox)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nBox;	/* number of boxes to fill */
    BoxPtr 	    pBox;	/* pointer to list of boxes to fill */
{
    register int srcpix;	
    int *psrc;		/* pointer to bits in tile, if needed */
    int tileHeight;	/* height of the tile */

    int nlwDst;		/* width in longwords of the dest pixmap */
    int w;		/* width of current box */
    register int h;	/* height of current box */
    register unsigned long startmask;
    register unsigned long endmask; /* masks for reggedy bits at either end of line */
    int nlwMiddle;	/* number of longwords between sides of boxes */
    int nlwExtra;	/* to get from right of box to left of next span */
    register int nlw;	/* loop version of nlwMiddle */
    register unsigned long *p;	/* pointer to bits we're writing */
    int y;		/* current scan line */
    int srcy;		/* current tile position */

    unsigned long *pbits;/* pointer to start of pixmap */
    PixmapPtr	    tile;	/* rotated, expanded tile */
    Bool	    isCopy;

    tile = ((cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr))->pRotatedPixmap;
    tileHeight = tile->drawable.height;
    psrc = (int *)tile->devPrivate.ptr;
    isCopy = (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pbits = (unsigned long *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	nlwDst = (int)
		  (((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	pbits = (unsigned long *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	nlwDst = (int)(((PixmapPtr)pDrawable)->devKind) >> 2;
    }

    while (nBox--)
    {
	w = pBox->x2 - pBox->x1;
	h = pBox->y2 - pBox->y1;
	y = pBox->y1;
	p = pbits + (pBox->y1 * nlwDst) + (pBox->x1 >> PWSH);
	srcy = y % tileHeight;

	if ( ((pBox->x1 & PIM) + w) < PPW)
	{
	    maskpartialbits(pBox->x1, w, startmask);
	    nlwExtra = nlwDst;
	    if (isCopy)
	    {
	    	while (h--)
	    	{
		    srcpix = psrc[srcy];
		    ++srcy;
		    if (srcy == tileHeight)
		    	srcy = 0;
		    *p = (*p & ~startmask) | (srcpix & startmask);
		    p += nlwExtra;
	    	}
	    }
	    else
	    {
	    	while (h--)
	    	{
		    srcpix = psrc[srcy];
		    ++srcy;
		    if (srcy == tileHeight)
		    	srcy = 0;
		    *p = (*p & ~startmask) |
			 (DoRop (pGC->alu, srcpix, *p) & startmask);
		    p += nlwExtra;
	    	}
	    }
	}
	else
	{
	    maskbits(pBox->x1, w, startmask, endmask, nlwMiddle);
	    nlwExtra = nlwDst - nlwMiddle;

	    if (isCopy)
	    {
	    	if (startmask && endmask)
	    	{
		    nlwExtra -= 1;
		    while (h--)
		    {
		    	srcpix = psrc[srcy];
		    	++srcy;
		    	if (srcy == tileHeight)
		            srcy = 0;
		    	nlw = nlwMiddle;
		    	*p = (*p & ~startmask) | (srcpix & startmask);
		    	p++;
		    	while (nlw--)
			    *p++ = srcpix;
		    	*p = (*p & ~endmask) | (srcpix & endmask);
		    	p += nlwExtra;
		    }
	    	}
	    	else if (startmask && !endmask)
	    	{
		    nlwExtra -= 1;
		    while (h--)
		    {
		    	srcpix = psrc[srcy];
		    	++srcy;
		    	if (srcy == tileHeight)
		            srcy = 0;
		    	nlw = nlwMiddle;
		    	*p = (*p & ~startmask) | (srcpix & startmask);
		    	p++;
		    	while (nlw--)
			    *p++ = srcpix;
		    	p += nlwExtra;
		    }
	    	}
	    	else if (!startmask && endmask)
	    	{
		    while (h--)
		    {
		    	srcpix = psrc[srcy];
		    	++srcy;
		    	if (srcy == tileHeight)
		            srcy = 0;
		    	nlw = nlwMiddle;
		    	while (nlw--)
			    *p++ = srcpix;
		    	*p = (*p & ~endmask) | (srcpix & endmask);
		    	p += nlwExtra;
		    }
	    	}
	    	else /* no ragged bits at either end */
	    	{
		    while (h--)
		    {
		    	srcpix = psrc[srcy];
		    	++srcy;
		    	if (srcy == tileHeight)
		            srcy = 0;
		    	nlw = nlwMiddle;
		    	while (nlw--)
			    *p++ = srcpix;
		    	p += nlwExtra;
		    }
	    	}
	    }
	    else    /* non copy rop */
	    {
		if (startmask)
		    nlwExtra -= 1;
		while (h--)
		{
		    srcpix = psrc[srcy];
		    ++srcy;
		    if (srcy == tileHeight)
			srcy = 0;
		    nlw = nlwMiddle;
		    if (startmask)
		    {
			*p = (*p & ~startmask) |
			     (DoRop (pGC->alu, srcpix, *p) & startmask);
			p++;
		    }
		    while (nlw--)
		    {
			*p = DoRop (pGC->alu, srcpix, *p);
			++p;
		    }
		    if (endmask)
		    {
		    	*p = (*p & ~endmask) |
			     (DoRop (pGC->alu, srcpix, *p) & endmask);
		    }
		    p += nlwExtra;
		}
	    }
	}
        pBox++;
    }
}

void
cfbFillRectTileOdd (pDrawable, pGC, nBox, pBox)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nBox;
    BoxPtr	pBox;
{
    int	xrot, yrot;

    xrot = pDrawable->x + pGC->patOrg.x;
    yrot = pDrawable->y + pGC->patOrg.y;
    cfbFillBoxTileOdd (pDrawable, nBox, pBox, pGC->tile.pixmap, xrot, yrot);
}

#define NUM_STACK_RECTS	1024

void
cfbPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    register GCPtr pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    xRectangle	    *prect;
    RegionPtr	    prgnClip;
    register BoxPtr pbox;
    register BoxPtr pboxClipped;
    BoxPtr	    pboxClippedBase;
    BoxPtr	    pextent;
    BoxRec	    stackRects[NUM_STACK_RECTS];
    cfbPrivGC	    *priv;
    int		    numRects;
    void	    (*BoxFill)();
    int		    n;
    int		    xorg, yorg;

    priv = (cfbPrivGC *) pGC->devPrivates[cfbGCPrivateIndex].ptr;
    prgnClip = priv->pCompositeClip;

    BoxFill = 0;
    switch (pGC->fillStyle)
    {
    case FillSolid:
	BoxFill = cfbFillRectSolid;
	break;
    case FillTiled:
	if (!((cfbPrivGCPtr) pGC->devPrivates[cfbGCPrivateIndex].ptr)->
							pRotatedPixmap)
	    BoxFill = cfbFillRectTileOdd;
	else
	    BoxFill = cfbFillRectTile32;
	break;
#if (PPW == 4)
    case FillStippled:
	if (!((cfbPrivGCPtr) pGC->devPrivates[cfbGCPrivateIndex].ptr)->
							pRotatedPixmap)
	    BoxFill = cfb8FillRectStippledUnnatural;
	else
	    BoxFill = cfb8FillRectTransparentStippled32;
	break;
    case FillOpaqueStippled:
	if (!((cfbPrivGCPtr) pGC->devPrivates[cfbGCPrivateIndex].ptr)->
							pRotatedPixmap)
	    BoxFill = cfb8FillRectStippledUnnatural;
	else
	    BoxFill = cfb8FillRectOpaqueStippled32;
	break;
#endif
    }
    prect = prectInit;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (xorg || yorg)
    {
	prect = prectInit;
	n = nrectFill;
	while(n--)
	{
	    prect->x += xorg;
	    prect->y += yorg;
	    prect++;
	}
    }

    prect = prectInit;

    numRects = REGION_NUM_RECTS(prgnClip) * nrectFill;
    if (numRects > NUM_STACK_RECTS)
    {
	pboxClippedBase = (BoxPtr)ALLOCATE_LOCAL(numRects * sizeof(BoxRec));
	if (!pboxClippedBase)
	    return;
    }
    else
	pboxClippedBase = stackRects;

    pboxClipped = pboxClippedBase;
	
    if (REGION_NUM_RECTS(prgnClip) == 1)
    {
	int x1, y1, x2, y2, bx2, by2;

	pextent = REGION_RECTS(prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
    	while (nrectFill--)
    	{
	    if ((pboxClipped->x1 = prect->x) < x1)
		pboxClipped->x1 = x1;
    
	    if ((pboxClipped->y1 = prect->y) < y1)
		pboxClipped->y1 = y1;
    
	    bx2 = (int) prect->x + (int) prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    pboxClipped->x2 = bx2;
    
	    by2 = (int) prect->y + (int) prect->height;
	    if (by2 > y2)
		by2 = y2;
	    pboxClipped->y2 = by2;

	    prect++;
	    if ((pboxClipped->x1 < pboxClipped->x2) &&
		(pboxClipped->y1 < pboxClipped->y2))
	    {
		pboxClipped++;
	    }
    	}
    }
    else
    {
	int x1, y1, x2, y2, bx2, by2;

	pextent = (*pGC->pScreen->RegionExtents)(prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
    	while (nrectFill--)
    	{
	    BoxRec box;
    
	    if ((box.x1 = prect->x) < x1)
		box.x1 = x1;
    
	    if ((box.y1 = prect->y) < y1)
		box.y1 = y1;
    
	    bx2 = (int) prect->x + (int) prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    box.x2 = bx2;
    
	    by2 = (int) prect->y + (int) prect->height;
	    if (by2 > y2)
		by2 = y2;
	    box.y2 = by2;
    
	    prect++;
    
	    if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    	continue;
    
	    n = REGION_NUM_RECTS (prgnClip);
	    pbox = REGION_RECTS(prgnClip);
    
	    /* clip the rectangle to each box in the clip region
	       this is logically equivalent to calling Intersect()
	    */
	    while(n--)
	    {
		pboxClipped->x1 = max(box.x1, pbox->x1);
		pboxClipped->y1 = max(box.y1, pbox->y1);
		pboxClipped->x2 = min(box.x2, pbox->x2);
		pboxClipped->y2 = min(box.y2, pbox->y2);
		pbox++;

		/* see if clipping left anything */
		if(pboxClipped->x1 < pboxClipped->x2 && 
		   pboxClipped->y1 < pboxClipped->y2)
		{
		    pboxClipped++;
		}
	    }
    	}
    }
    if (pboxClipped != pboxClippedBase)
	(*BoxFill) (pDrawable, pGC,
		    pboxClipped-pboxClippedBase, pboxClippedBase);
    if (pboxClippedBase != stackRects)
    	DEALLOCATE_LOCAL(pboxClippedBase);
}
