/*
 * Fill rectangles.  Used by both PolyFillRect and PaintWindow.
 * no depth dependencies.
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

/* $XConsortium: Exp $ */

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

cfbFillBox (pDrawable, nBox, pBox, pixel, isCopy)
    DrawablePtr	    pDrawable;
    BoxPtr	    pBox;
    unsigned long   pixel;
    Bool	    isCopy;
{
    unsigned long   *pdstBase, *pdstRect;
    int		    widthDst;
    int		    h;
    unsigned long   fill;
    unsigned long   *pdst;
    unsigned long   leftMask, rightMask;
    int		    xoffleft, xoffright;
    int		    nmiddle;
    int		    m;
    int		    w;

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

    fill = PFILL(pixel);
    for (; nBox; nBox--, pBox++)
    {
    	pdstRect = pdstBase + pBox->y1 * widthDst + (pBox->x1 >> PWSH);
    	h = pBox->y2 - pBox->y1;
	w = pBox->x2 - pBox->x1;
	if ((pBox->x1 & PIM) + w <= PPW)
	{
	    maskpartialbits(pBox->x1, w, leftMask);
	    pdst = pdstRect;
	    if (isCopy)
	    {
	    	while (--h >= 0) {
		    *pdst = (*pdst & ~leftMask) | (fill & leftMask);
		    pdst += widthDst;
	    	}
	    }
	    else
	    {
	    	while (--h >= 0) {
		    *pdst = (*pdst & ~leftMask) | ((fill ^ *pdst) & leftMask);
		    pdst += widthDst;
	    	}
	    }
	}
	else
	{
	    maskbits (pBox->x1, w, leftMask, rightMask, nmiddle);
	    if (isCopy)
	    {
		if (leftMask)
		{
		    if (rightMask)
		    {
    	    	    	while (--h >= 0) {
	    	    	    pdst = pdstRect;
	    	    	    *pdst = (*pdst & ~leftMask) | (fill & leftMask);
		    	    pdst++;
	    	    	    m = nmiddle;
	    	    	    while (--m >= 0)
	    	    	    	*pdst++ = fill;
	    	    	    *pdst = (*pdst & ~rightMask) | (fill & rightMask);
		    	    pdstRect += widthDst;
    	    	    	}
		    }
		    else
		    {
    	    	    	while (--h >= 0) {
	    	    	    pdst = pdstRect;
	    	    	    *pdst = (*pdst & ~leftMask) | (fill & leftMask);
		    	    pdst++;
	    	    	    m = nmiddle;
	    	    	    while (--m >= 0)
	    	    	    	*pdst++ = fill;
		    	    pdstRect += widthDst;
    	    	    	}
		    }
		}
		else
		{
		    if (rightMask)
		    {
    	    	    	while (--h >= 0) {
	    	    	    pdst = pdstRect;
	    	    	    m = nmiddle;
	    	    	    while (--m >= 0)
	    	    	    	*pdst++ = fill;
	    	    	    *pdst = (*pdst & ~rightMask) | (fill & rightMask);
		    	    pdstRect += widthDst;
    	    	    	}
		    }
		    else
		    {
			while (--h >= 0)
			{
			    pdst = pdstRect;
			    m = nmiddle;
			    while (--m >= 0)
				*pdst++ = fill;
			    pdstRect += widthDst;
			}
		    }
		}
	    }
	    else
	    {
    	    	while (--h >= 0) {
	    	    pdst = pdstRect;
	    	    if (leftMask)
		    {
	    	    	*pdst = (*pdst & ~leftMask) | ((fill ^ *pdst) & leftMask);
		    	pdst++;
		    }
	    	    m = nmiddle;
	    	    while (--m >= 0)
	    	    	*pdst++ ^= fill;
	    	    if (rightMask)
	    	    	*pdst = (*pdst & ~rightMask) | ((fill ^ *pdst) & rightMask);
		    pdstRect += widthDst;
    	    	}
	    }
	}
    }
}

void
cfbPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    int xorg, yorg;
    register int n;		/* spare counter */
    xRectangle *prect;		/* temporary */
    RegionPtr prgnClip;
    register BoxPtr pbox;	/* used to clip with */
    register BoxPtr pboxClipped;
    BoxPtr pboxClippedBase;
    BoxPtr pextent;
    cfbPrivGC	*priv;
    int numRects;
    Bool isCopy;
    Pixel pixel;

    priv = (cfbPrivGC *) pGC->devPrivates[cfbGCPrivateIndex].ptr;
    prgnClip = priv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);
    pboxClippedBase = (BoxPtr)ALLOCATE_LOCAL(numRects * sizeof(BoxRec));

    if (!pboxClippedBase)
	return;

    prect = prectInit;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    isCopy = pGC->alu == GXcopy;
    pixel = pGC->fgPixel;
    if (pGC->alu == GXinvert)
	pixel = pGC->planemask;
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

    pextent = (*pGC->pScreen->RegionExtents)(prgnClip);

    while (nrectFill--)
    {
	BoxRec box;
	int	x2, y2;

	/*
	 * clip the box to the extent of the region --
	 * avoids overflowing shorts and minimizes other
	 * computations
	 */

	box.x1 = prect->x;
	if (box.x1 < pextent->x1)
		box.x1 = pextent->x1;

	box.y1 = prect->y;
	if (box.y1 < pextent->y1)
		box.y1 = pextent->y1;

	x2 = (int) prect->x + (int) prect->width;
	if (x2 > pextent->x2)
		x2 = pextent->x2;
	box.x2 = x2;

	y2 = (int) prect->y + (int) prect->height;
	if (y2 > pextent->y2)
		y2 = pextent->y2;
	box.y2 = y2;

	prect++;

	if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    continue;

	switch((*pGC->pScreen->RectIn)(prgnClip, &box))
	{
	  case rgnOUT:
	    break;
	  case rgnIN:
	    cfbFillBox (pDrawable, 1, &box, pixel, isCopy);
	    break;
	  case rgnPART:
	    pboxClipped = pboxClippedBase;
	    pbox = REGION_RECTS(prgnClip);
	    n = numRects;

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
	    cfbFillBox (pDrawable, pboxClipped-pboxClippedBase, 
		   pboxClippedBase, pixel, isCopy);
	    break;
	}
    }
    DEALLOCATE_LOCAL(pboxClippedBase);
}
