/*
 * cfb copy area
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

Author: Keith Packard

*/
/* $XConsortium: cfbbitblt.c,v 5.15 89/10/04 16:23:36 keith Exp $ */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"

cfbDoBitblt(pSrc, pDst, alu, prgnDst, pptSrc, planemask)
    DrawablePtr	    pSrc, pDst;
    int		    alu;
    RegionPtr	    prgnDst;
    DDXPointPtr	    pptSrc;
    unsigned long   planemask;
{
    unsigned int *psrcBase, *pdstBase;	
				/* start of src and dst bitmaps */
    int widthSrc, widthDst;	/* add to get to same position in next line */

    BoxPtr pbox;
    int nbox;

    BoxPtr pboxTmp, pboxNext, pboxBase, pboxNew1, pboxNew2;
				/* temporaries for shuffling rectangles */
    DDXPointPtr pptTmp, pptNew1, pptNew2;
				/* shuffling boxes entails shuffling the
				   source points too */
    int w, h;
    int xdir;			/* 1 = left right, -1 = right left/ */
    int ydir;			/* 1 = top down, -1 = bottom up */

    unsigned int *psrcLine, *pdstLine;	
				/* pointers to line with current src and dst */
    register unsigned int *psrc;/* pointer to current src longword */
    register unsigned int *pdst;/* pointer to current dst longword */

				/* following used for looping through a line */
    unsigned int startmask, endmask;	/* masks for writing ends of dst */
    int nlMiddle;		/* whole longwords in dst */
    register int nl;		/* temp copy of nlMiddle */
    register unsigned int tmp, bits;
#ifdef FAST_CONSTANT_OFFSET_MODE
    register unsigned int bits1;
#endif
				/* place to store full source word */
    int xoffSrc, xoffDst;
    register int leftShift, rightShift;

    int nstart;			/* number of ragged bits at start of dst */
    int nend;			/* number of ragged bits at end of dst */
    int srcStartOver;		/* pulling nstart bits from src
				   overflows into the next word? */
    int careful;
    int tmpSrc;

    if (pSrc->type == DRAWABLE_WINDOW)
    {
	psrcBase = (unsigned int *)
		(((PixmapPtr)(pSrc->pScreen->devPrivate))->devPrivate.ptr);
	widthSrc = (int)
		   ((PixmapPtr)(pSrc->pScreen->devPrivate))->devKind
		    >> 2;
    }
    else
    {
	psrcBase = (unsigned int *)(((PixmapPtr)pSrc)->devPrivate.ptr);
	widthSrc = (int)(((PixmapPtr)pSrc)->devKind) >> 2;
    }

    if (pDst->type == DRAWABLE_WINDOW)
    {
	pdstBase = (unsigned int *)
		(((PixmapPtr)(pDst->pScreen->devPrivate))->devPrivate.ptr);
	widthDst = (int)
		   ((PixmapPtr)(pDst->pScreen->devPrivate))->devKind
		    >> 2;
    }
    else
    {
	pdstBase = (unsigned int *)(((PixmapPtr)pDst)->devPrivate.ptr);
	widthDst = (int)(((PixmapPtr)pDst)->devKind) >> 2;
    }

    /* XXX we have to err on the side of safety when both are windows,
     * because we don't know if IncludeInferiors is being used.
     */
    careful = ((pSrc == pDst) ||
	       ((pSrc->type == DRAWABLE_WINDOW) &&
		(pDst->type == DRAWABLE_WINDOW)));

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);

    pboxNew1 = NULL;
    pptNew1 = NULL;
    pboxNew2 = NULL;
    pptNew2 = NULL;
    if (careful && (pptSrc->y < pbox->y1))
    {
        /* walk source botttom to top */
	ydir = -1;
	widthSrc = -widthSrc;
	widthDst = -widthDst;

	if (nbox > 1)
	{
	    /* keep ordering in each band, reverse order of bands */
	    pboxNew1 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    if(!pboxNew1)
		return;
	    pptNew1 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
	    if(!pptNew1)
	    {
	        DEALLOCATE_LOCAL(pboxNew1);
	        return;
	    }
	    pboxBase = pboxNext = pbox+nbox-1;
	    while (pboxBase >= pbox)
	    {
	        while ((pboxNext >= pbox) &&
		       (pboxBase->y1 == pboxNext->y1))
		    pboxNext--;
	        pboxTmp = pboxNext+1;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp <= pboxBase)
	        {
		    *pboxNew1++ = *pboxTmp++;
		    *pptNew1++ = *pptTmp++;
	        }
	        pboxBase = pboxNext;
	    }
	    pboxNew1 -= nbox;
	    pbox = pboxNew1;
	    pptNew1 -= nbox;
	    pptSrc = pptNew1;
        }
    }
    else
    {
	/* walk source top to bottom */
	ydir = 1;
    }

    if (careful && (pptSrc->x < pbox->x1))
    {
	/* walk source right to left */
        xdir = -1;

	if (nbox > 1)
	{
	    /* reverse order of rects in each band */
	    pboxNew2 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    pptNew2 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
	    if(!pboxNew2 || !pptNew2)
	    {
		if (pptNew2) DEALLOCATE_LOCAL(pptNew2);
		if (pboxNew2) DEALLOCATE_LOCAL(pboxNew2);
		if (pboxNew1)
		{
		    DEALLOCATE_LOCAL(pptNew1);
		    DEALLOCATE_LOCAL(pboxNew1);
		}
	        return;
	    }
	    pboxBase = pboxNext = pbox;
	    while (pboxBase < pbox+nbox)
	    {
	        while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		    pboxNext++;
	        pboxTmp = pboxNext;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp != pboxBase)
	        {
		    *pboxNew2++ = *--pboxTmp;
		    *pptNew2++ = *--pptTmp;
	        }
	        pboxBase = pboxNext;
	    }
	    pboxNew2 -= nbox;
	    pbox = pboxNew2;
	    pptNew2 -= nbox;
	    pptSrc = pptNew2;
	}
    }
    else
    {
	/* walk source left to right */
        xdir = 1;
    }

    /* special case copy */
    if (alu == GXcopy && (planemask & PMSK) == PMSK)
    {
	while(nbox--)
	{
	    w = pbox->x2 - pbox->x1;
	    h = pbox->y2 - pbox->y1;

	    if (ydir == -1) /* start at last scanline of rectangle */
	    {
	        psrcLine = psrcBase + ((pptSrc->y+h-1) * -widthSrc);
	        pdstLine = pdstBase + ((pbox->y2-1) * -widthDst);
	    }
	    else /* start at first scanline */
	    {
	        psrcLine = psrcBase + (pptSrc->y * widthSrc);
	        pdstLine = pdstBase + (pbox->y1 * widthDst);
	    }
	    if ((pbox->x1 & PIM) + w <= PPW)
	    {
		pdst = pdstLine + (pbox->x1 >> PWSH);
		psrc = psrcLine + (pptSrc->x >> PWSH);
		xoffSrc = pptSrc->x & PIM;
		xoffDst = pbox->x1 & PIM;
		while (h--)
		{
		    getbits (psrc, xoffSrc, w, bits)
		    putbits (bits, xoffDst, w, pdst, ~0)
		    psrc += widthSrc;
		    pdst += widthDst;
		}
	    }
	    else
	    {
	    	maskbits(pbox->x1, w, startmask, endmask, nlMiddle);
	    	if (xdir == 1)
	    	{
	    	    xoffSrc = pptSrc->x & PIM;
	    	    xoffDst = pbox->x1 & PIM;
		    pdstLine += (pbox->x1 >> PWSH);
		    psrcLine += (pptSrc->x >> PWSH);
		    if (xoffSrc == xoffDst)
		    {
	    	    	while (h--)
	    	    	{
		    	    psrc = psrcLine;
		    	    pdst = pdstLine;
		    	    pdstLine += widthDst;
		    	    psrcLine += widthSrc;
			    if (startmask)
			    {
			    	*pdst = (*pdst & ~startmask) | (*psrc++ & startmask);
			    	pdst++;
			    }
			    nl = nlMiddle;
#ifdef FAST_CONSTANT_OFFSET_MODE
			    psrc += nl & (UNROLL-1);
			    pdst += nl & (UNROLL-1);

#define BodyOdd(n) pdst[-n] = psrc[-n];
#define BodyEven(n) pdst[-n] = psrc[-n];

#define LoopReset \
    pdst += UNROLL; \
    psrc += UNROLL;

			    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
			    DuffL(nl, label1, *pdst++ = *psrc++;)
#endif

			    if (endmask)
			    	*pdst = (*pdst & ~endmask) | (*psrc++ & endmask);
		    	}
		    }
		    else
		    {
		    	if (xoffSrc > xoffDst)
			{
			    leftShift = (xoffSrc - xoffDst) << (5 - PWSH);
			    rightShift = 32 - leftShift;
			}
		    	else
			{
			    rightShift = (xoffDst - xoffSrc) << (5 - PWSH);
			    leftShift = 32 - rightShift;
			}
		    	while (h--)
		    	{
			    psrc = psrcLine;
			    pdst = pdstLine;
			    pdstLine += widthDst;
			    psrcLine += widthSrc;
			    bits = 0;
			    if (xoffSrc > xoffDst)
			    	bits = *psrc++;
			    if (startmask)
			    {
			    	tmp = BitLeft(bits,leftShift);
			    	bits = *psrc++;
			    	tmp |= BitRight(bits,rightShift);
			    	*pdst = (*pdst & ~startmask) |
				    	(tmp & startmask);
			    	pdst++;
			    }
			    nl = nlMiddle;
#ifdef FAST_CONSTANT_OFFSET_MODE
			    bits1 = bits;
			    psrc += nl & (UNROLL-1);
			    pdst += nl & (UNROLL-1);

#define BodyOdd(n) \
    bits = psrc[-n]; \
    pdst[-n] = BitLeft(bits1, leftShift) | BitRight(bits, rightShift);

#define BodyEven(n) \
    bits1 = psrc[-n]; \
    pdst[-n] = BitLeft(bits, leftShift) | BitRight(bits1, rightShift);

#define LoopReset \
    pdst += UNROLL; \
    psrc += UNROLL;

			    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
			    DuffL (nl,label2,
				tmp = BitLeft(bits, leftShift);
				bits = *psrc++;
				*pdst++ = tmp | BitRight(bits, rightShift);
			    )
#endif
    
			    if (endmask)
			    {
			    	tmp = BitLeft(bits, leftShift);
			    	if (BitLeft(endmask, rightShift))
			    	{
				    bits = *psrc++;
				    tmp |= BitRight(bits, rightShift);
			    	}
			    	*pdst = (*pdst & ~endmask) |
				    	(tmp & endmask);
			    }
		    	}
		    }
	    	}
	    	else	/* xdir == -1 */
	    	{
	    	    xoffSrc = (pptSrc->x + w - 1) & PIM;
	    	    xoffDst = (pbox->x2 - 1) & PIM;
		    pdstLine += ((pbox->x2-1) >> PWSH) + 1;
		    psrcLine += ((pptSrc->x+w - 1) >> PWSH) + 1;
		    if (xoffSrc == xoffDst)
		    {
	    	    	while (h--)
	    	    	{
		    	    psrc = psrcLine;
		    	    pdst = pdstLine;
		    	    pdstLine += widthDst;
		    	    psrcLine += widthSrc;
			    if (endmask)
			    {
			    	pdst--;
			    	*pdst = (*pdst & ~endmask) | (*--psrc & endmask);
			    }
			    nl = nlMiddle;

#ifdef FAST_CONSTANT_OFFSET_MODE
			    psrc -= nl & (UNROLL - 1);
			    pdst -= nl & (UNROLL - 1);

#define BodyOdd(n) pdst[n-1] = psrc[n-1];

#define BodyEven(n) pdst[n-1] = psrc[n-1];

#define LoopReset \
    pdst -= UNROLL;\
    psrc -= UNROLL;

			    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
			    DuffL(nl,label3, *--pdst = *--psrc;)
#endif

			    if (startmask)
			    {
			    	--pdst;
			    	*pdst = (*pdst & ~startmask) | (*--psrc & startmask);
			    }
		    	}
		    }
		    else
		    {
			if (xoffDst > xoffSrc)
			{
			    rightShift = (xoffDst - xoffSrc) << (5 - PWSH);
			    leftShift = 32 - rightShift;
			}
			else
			{
		    	    leftShift = (xoffSrc - xoffDst) << (5 - PWSH);
		    	    rightShift = 32 - leftShift;
			}
	    	    	while (h--)
	    	    	{
		    	    psrc = psrcLine;
		    	    pdst = pdstLine;
		    	    pdstLine += widthDst;
		    	    psrcLine += widthSrc;
			    bits = 0;
			    if (xoffDst > xoffSrc)
				bits = *--psrc;
			    if (endmask)
			    {
			    	tmp = BitRight(bits, rightShift);
			    	bits = *--psrc;
			    	tmp |= BitLeft(bits, leftShift);
			    	pdst--;
			    	*pdst = (*pdst & ~endmask) |
				    	(tmp & endmask);
			    }
			    nl = nlMiddle;

#ifdef FAST_CONSTANT_OFFSET_MODE
			    bits1 = bits;
			    psrc -= nl & (UNROLL - 1);
			    pdst -= nl & (UNROLL - 1);

#define BodyOdd(n) \
    bits = psrc[n-1]; \
    pdst[n-1] = BitRight(bits1, rightShift) | BitLeft(bits, leftShift);

#define BodyEven(n) \
    bits1 = psrc[n-1]; \
    pdst[n-1] = BitRight(bits, rightShift) | BitLeft(bits1, leftShift);

#define LoopReset \
    pdst -= UNROLL; \
    psrc -= UNROLL;

			    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
			    DuffL (nl, label4,
				tmp = BitRight(bits, rightShift);
				bits = *--psrc;
				*--pdst = tmp | BitLeft(bits, leftShift);
			    )
#endif

			    if (startmask)
			    {
			    	tmp = BitRight(bits, rightShift);
			    	if (BitRight (startmask, leftShift))
			    	{
				    bits = *--psrc;
				    tmp |= BitLeft(bits, leftShift);
			    	}
			    	--pdst;
			    	*pdst = (*pdst & ~startmask) |
				    	(tmp & startmask);
			    }
		    	}
		    }
		}
	    }
	    pbox++;
	    pptSrc++;
	}
    } else {
    	while (nbox--)
    	{
    	    w = pbox->x2 - pbox->x1;
    	    h = pbox->y2 - pbox->y1;
    	    if (ydir == -1)
    	    {
            	psrcLine = psrcBase + ((pptSrc->y+h-1) * -widthSrc);
            	pdstLine = pdstBase + ((pbox->y2-1) * -widthDst);
    	    }
    	    else
    	    {
            	psrcLine = psrcBase + (pptSrc->y * widthSrc);
            	pdstLine = pdstBase + (pbox->y1 * widthDst);
    	    }
    	    if (w <= PPW)
    	    {
	    	int tmpSrc;
            	int srcBit, dstBit;
            	pdstLine += (pbox->x1 >> PWSH);
            	psrcLine += (pptSrc->x >> PWSH);
            	psrc = psrcLine;
            	pdst = pdstLine;
            	srcBit = pptSrc->x & PIM;
            	dstBit = pbox->x1 & PIM;
            	while(h--)
            	{
	    	    getbits(psrc, srcBit, w, tmpSrc)
    		    putbitsrop(tmpSrc, dstBit, w, pdst, planemask, alu)
	    	    pdst += widthDst;
	    	    psrc += widthSrc;
            	}
    	    }
    	    else
    	    {
            	maskbits(pbox->x1, w, startmask, endmask, nlMiddle)
            	if (startmask)
	    	    nstart = PPW - (pbox->x1 & PIM);
            	else
	    	    nstart = 0;
            	if (endmask)
            	    nend = pbox->x2 & PIM;
            	else
	    	    nend = 0;
            	xoffSrc = ((pptSrc->x & PIM) + nstart) & PIM;
            	srcStartOver = ((pptSrc->x & PIM) + nstart) > PLST;
            	if (xdir == 1)
            	{
            	    pdstLine += (pbox->x1 >> PWSH);
            	    psrcLine += (pptSrc->x >> PWSH);
	    	    while (h--)
	    	    {
	            	psrc = psrcLine;
	            	pdst = pdstLine;
	            	if (startmask)
	            	{
		    	    getbits(psrc, (pptSrc->x & PIM), nstart, tmpSrc)
    			    putbitsrop(tmpSrc, (pbox->x1 & PIM), nstart, pdst, planemask, alu)
		    	    pdst++;
		    	    if (srcStartOver)
		            	psrc++;
	            	}
	            	nl = nlMiddle;
	            	while (nl--)
	            	{
		    	    getbits(psrc, xoffSrc, PPW, tmpSrc)
			    putbitsrop (tmpSrc, 0, PPW, pdst, planemask, alu)
		    	    pdst++;
		    	    psrc++;
	            	}
	            	if (endmask)
	            	{
		    	    getbits(psrc, xoffSrc, nend, tmpSrc)
    			    putbitsrop(tmpSrc, 0, nend, pdst, planemask, alu)
	            	}
	            	pdstLine += widthDst;
	            	psrcLine += widthSrc;
	    	    }
            	}
            	else
            	{
            	    pdstLine += (pbox->x2 >> PWSH);
            	    psrcLine += (pptSrc->x+w >> PWSH);
	    	    if (xoffSrc + nend >= PPW)
	            	--psrcLine;
	    	    while (h--)
	    	    {
	            	psrc = psrcLine;
	            	pdst = pdstLine;
	            	if (endmask)
	            	{
		    	    getbits(psrc, xoffSrc, nend, tmpSrc)
    			    putbitsrop(tmpSrc, 0, nend, pdst, planemask, alu)
	            	}
	            	nl = nlMiddle;
	            	while (nl--)
	            	{
		    	    --psrc;
		    	    getbits(psrc, xoffSrc, PPW, tmpSrc)
		    	    --pdst;
			    putbitsrop(tmpSrc, 0, PPW, pdst, planemask, alu);
	            	}
	            	if (startmask)
	            	{
		    	    if (srcStartOver)
		            	--psrc;
		    	    --pdst;
		    	    getbits(psrc, (pptSrc->x & PIM), nstart, tmpSrc)
    			    putbitsrop(tmpSrc, (pbox->x1 & PIM), nstart, pdst, planemask, alu)
	            	}
	            	pdstLine += widthDst;
	            	psrcLine += widthSrc;
	    	    }
            	}
    	    }
    	    pbox++;
    	    pptSrc++;
    	}
    }
}

static int (*doBitBlt)() = cfbDoBitblt;

RegionPtr
cfbCopyArea(pSrcDrawable, pDstDrawable,
            pGC, srcx, srcy, width, height, dstx, dsty)
    register DrawablePtr pSrcDrawable;
    register DrawablePtr pDstDrawable;
    GC *pGC;
    int srcx, srcy;
    int width, height;
    int dstx, dsty;
{
    RegionPtr prgnSrcClip;	/* may be a new region, or just a copy */
    RegionRec rgnSrcRec;
    Bool freeSrcClip = FALSE;

    RegionPtr prgnExposed;
    RegionRec rgnDst;
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    register BoxPtr pbox;
    int i;
    register int dx;
    register int dy;
    xRectangle origSource;
    DDXPointRec origDest;
    int numRects;
    BoxRec fastBox;
    int fastClip = 0;		/* for fast clipping with pixmap source */
    int fastExpose = 0;		/* for fast exposures with pixmap source */

    origSource.x = srcx;
    origSource.y = srcy;
    origSource.width = width;
    origSource.height = height;
    origDest.x = dstx;
    origDest.y = dsty;

    if ((pSrcDrawable != pDstDrawable) &&
	pSrcDrawable->pScreen->SourceValidate)
	(*pSrcDrawable->pScreen->SourceValidate) (pSrcDrawable, srcx, srcy, width, height);

    srcx += pSrcDrawable->x;
    srcy += pSrcDrawable->y;

    /* clip the source */

    if (pSrcDrawable->type == DRAWABLE_PIXMAP)
    {
	if ((pSrcDrawable == pDstDrawable) &&
	    (pGC->clientClipType == CT_NONE))
	{
	    prgnSrcClip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
	}
	else
	{
	    fastClip = 1;
	}
    }
    else
    {
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    if (!((WindowPtr) pSrcDrawable)->parent)
	    {
		/*
		 * special case bitblt from root window in
		 * IncludeInferiors mode; just like from a pixmap
		 */
		fastClip = 1;
	    }
	    else if ((pSrcDrawable == pDstDrawable) &&
		(pGC->clientClipType == CT_NONE))
	    {
		prgnSrcClip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
	    }
	    else
	    {
		prgnSrcClip = NotClippedByChildren((WindowPtr)pSrcDrawable);
		freeSrcClip = TRUE;
	    }
	}
	else
	{
	    prgnSrcClip = &((WindowPtr)pSrcDrawable)->clipList;
	}
    }

    fastBox.x1 = srcx;
    fastBox.y1 = srcy;
    fastBox.x2 = srcx + width;
    fastBox.y2 = srcy + height;

    /* Don't create a source region if we are doing a fast clip */
    if (fastClip)
    {
	fastExpose = 1;
	/*
	 * clip the source; if regions extend beyond the source size,
 	 * make sure exposure events get sent
	 */
	if (fastBox.x1 < pSrcDrawable->x)
	{
	    fastBox.x1 = pSrcDrawable->x;
	    fastExpose = 0;
	}
	if (fastBox.y1 < pSrcDrawable->y)
	{
	    fastBox.y1 = pSrcDrawable->y;
	    fastExpose = 0;
	}
	if (fastBox.x2 > pSrcDrawable->x + (int) pSrcDrawable->width)
	{
	    fastBox.x2 = pSrcDrawable->x + (int) pSrcDrawable->width;
	    fastExpose = 0;
	}
	if (fastBox.y2 > pSrcDrawable->y + (int) pSrcDrawable->height)
	{
	    fastBox.y2 = pSrcDrawable->y + (int) pSrcDrawable->height;
	    fastExpose = 0;
	}
    }
    else
    {
	(*pGC->pScreen->RegionInit)(&rgnDst, &fastBox, 1);
	(*pGC->pScreen->Intersect)(&rgnDst, &rgnDst, prgnSrcClip);
    }

    dstx += pDstDrawable->x;
    dsty += pDstDrawable->y;

    if (pDstDrawable->type == DRAWABLE_WINDOW)
    {
	if (!((WindowPtr)pDstDrawable)->realized)
	{
	    if (!fastClip)
		(*pGC->pScreen->RegionUninit)(&rgnDst);
	    if (prgnSrcClip == &rgnSrcRec)
		(*pGC->pScreen->RegionUninit)(prgnSrcClip);
	    else if (freeSrcClip)
		(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
	    return NULL;
	}
    }

    dx = srcx - dstx;
    dy = srcy - dsty;

    /* Translate and clip the dst to the destination composite clip */
    if (fastClip)
    {
	RegionPtr cclip;

        /* Translate the region directly */
        fastBox.x1 -= dx;
        fastBox.x2 -= dx;
        fastBox.y1 -= dy;
        fastBox.y2 -= dy;

	/* If the destination composite clip is one rectangle we can
	   do the clip directly.  Otherwise we have to create a full
	   blown region and call intersect */
	cclip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
        if (REGION_NUM_RECTS(cclip) == 1)
        {
	    BoxPtr pBox = REGION_RECTS(cclip);

	    if (fastBox.x1 < pBox->x1) fastBox.x1 = pBox->x1;
	    if (fastBox.x2 > pBox->x2) fastBox.x2 = pBox->x2;
	    if (fastBox.y1 < pBox->y1) fastBox.y1 = pBox->y1;
	    if (fastBox.y2 > pBox->y2) fastBox.y2 = pBox->y2;

	    /* Check to see if the region is empty */
	    if (fastBox.x1 >= fastBox.x2 || fastBox.y1 >= fastBox.y2)
		(*pGC->pScreen->RegionInit)(&rgnDst, NullBox, 0);
	    else
		(*pGC->pScreen->RegionInit)(&rgnDst, &fastBox, 1);
	}
        else
	{
	    /* We must turn off fastClip now, since we must create
	       a full blown region.  It is intersected with the
	       composite clip below. */
	    fastClip = 0;
	    (*pGC->pScreen->RegionInit)(&rgnDst, &fastBox,1);
	}
    }
    else
    {
        (*pGC->pScreen->TranslateRegion)(&rgnDst, -dx, -dy);
    }

    if (!fastClip)
    {
	(*pGC->pScreen->Intersect)(&rgnDst,
				   &rgnDst,
				 ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip);
    }

    /* Do bit blitting */
    numRects = REGION_NUM_RECTS(&rgnDst);
    if (numRects)
    {
	if(!(pptSrc = (DDXPointPtr)ALLOCATE_LOCAL(numRects *
						  sizeof(DDXPointRec))))
	{
	    (*pGC->pScreen->RegionUninit)(&rgnDst);
	    if (prgnSrcClip == &rgnSrcRec)
		(*pGC->pScreen->RegionUninit)(prgnSrcClip);
	    else if (freeSrcClip)
		(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
	    return NULL;
	}
	pbox = REGION_RECTS(&rgnDst);
	ppt = pptSrc;
	for (i = numRects; --i >= 0; pbox++, ppt++)
	{
	    ppt->x = pbox->x1 + dx;
	    ppt->y = pbox->y1 + dy;
	}

	(*doBitBlt) (pSrcDrawable, pDstDrawable, pGC->alu, &rgnDst, pptSrc, pGC->planemask);
	DEALLOCATE_LOCAL(pptSrc);
    }

    prgnExposed = NULL;
    if (((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->fExpose)
    {
	extern RegionPtr    miHandleExposures();

        /* Pixmap sources generate a NoExposed (we return NULL to do this) */
        if (!fastExpose)
	    prgnExposed =
		miHandleExposures(pSrcDrawable, pDstDrawable, pGC,
				  origSource.x, origSource.y,
				  (int)origSource.width,
				  (int)origSource.height,
				  origDest.x, origDest.y, (unsigned long)0);
    }
    (*pGC->pScreen->RegionUninit)(&rgnDst);
    if (prgnSrcClip == &rgnSrcRec)
	(*pGC->pScreen->RegionUninit)(prgnSrcClip);
    else if (freeSrcClip)
	(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
    return prgnExposed;
}

#if (PPW == 4)

cfbCopyPlane1to8 (pSrcDrawable, pDstDrawable, rop, prgnDst, pptSrc, planemask)
    DrawablePtr pSrcDrawable;
    DrawablePtr pDstDrawable;
    int	rop;
    unsigned long planemask;
    RegionPtr prgnDst;
    DDXPointPtr pptSrc;
{
    int	srcx, srcy, dstx, dsty, width, height;
    unsigned int bits, tmp;
    int xoffSrc, xoffDst;
    int leftShift, rightShift;
    unsigned int *psrcBase, *pdstBase;
    int	widthSrc, widthDst;
    unsigned int *psrcLine, *pdstLine;
    register unsigned int *psrc, *pdst;
    unsigned int startmask, endmask;
    register int nlMiddle, nl;
    int firstoff, secondoff;
    int nbox;
    BoxPtr  pbox;

    if (pSrcDrawable->type == DRAWABLE_WINDOW)
    {
	psrcBase = (unsigned int *)
		(((PixmapPtr)(pSrcDrawable->pScreen->devPrivate))->devPrivate.ptr);
	widthSrc = (int)
		   ((PixmapPtr)(pSrcDrawable->pScreen->devPrivate))->devKind
		    >> 2;
    }
    else
    {
	psrcBase = (unsigned int *)(((PixmapPtr)pSrcDrawable)->devPrivate.ptr);
	widthSrc = (int)(((PixmapPtr)pSrcDrawable)->devKind) >> 2;
    }

    if (pDstDrawable->type == DRAWABLE_WINDOW)
    {
	pdstBase = (unsigned int *)
		(((PixmapPtr)(pDstDrawable->pScreen->devPrivate))->devPrivate.ptr);
	widthDst = (int)
		   ((PixmapPtr)(pDstDrawable->pScreen->devPrivate))->devKind
		    >> 2;
    }
    else
    {
	pdstBase = (unsigned int *)(((PixmapPtr)pDstDrawable)->devPrivate.ptr);
	widthDst = (int)(((PixmapPtr)pDstDrawable)->devKind) >> 2;
    }

    nbox = REGION_NUM_RECTS(prgnDst);
    pbox = REGION_RECTS(prgnDst);
    while (nbox--)
    {
	dstx = pbox->x1;
	dsty = pbox->y1;
	srcx = pptSrc->x;
	srcy = pptSrc->y;
	width = pbox->x2 - pbox->x1;
	height = pbox->y2 - pbox->y1;
	pbox++;
	pptSrc++;
	psrcLine = psrcBase + srcy * widthSrc + (srcx >> 5);
	pdstLine = pdstBase + dsty * widthDst + (dstx >> 2);
	xoffSrc = srcx & 0x1f;
	xoffDst = dstx & 0x3;
	if (xoffDst + width < 4)
	{
	    maskpartialbits(dstx, width, startmask);
	    endmask = 0;
	    nlMiddle = 0;
	}
	else
	{
	    maskbits(dstx, width, startmask, endmask, nlMiddle);
	}
	/*
	 * compute constants for the first four bits to be
	 * copied.  This avoids troubles with partial first
	 * writes, and difficult shift computation
	 */
	if (startmask)
	{
	    firstoff = xoffSrc - xoffDst;
	    if (firstoff > 28)
		secondoff = 32 - firstoff;
	    if (xoffDst)
	    {
	    	srcx += (4-xoffDst);
	    	dstx += (4-xoffDst);
	    	xoffSrc = srcx & 0x1f;
	    }
	}
	leftShift = xoffSrc;
	rightShift = 32 - leftShift;
	if (rop == GXcopy && (planemask & PMSK) == PMSK)
	{
	    while (height--)
	    {
	    	psrc = psrcLine;
	    	pdst = pdstLine;
	    	psrcLine += widthSrc;
	    	pdstLine += widthDst;
	    	bits = *psrc++;
	    	if (startmask)
	    	{
		    if (firstoff < 0)
		    	tmp = BitRight (bits, -firstoff);
		    else
		    {
		    	tmp = BitLeft (bits, firstoff);
			/*
			 * need a more cautious test for partialmask
			 * case...
			 */
		    	if (firstoff > 28)
		    	{
			    bits = *psrc++;
			    tmp |= BitRight (bits, secondoff);
		    	}
		    }
		    *pdst = *pdst & ~startmask | GetFourPixels(tmp) & startmask;
		    pdst++;
	    	}
	    	nl = nlMiddle;
	    	while (nl >= 8)
	    	{
		    nl -= 8;
		    tmp = BitLeft(bits, leftShift);
		    bits = *psrc++;
		    if (rightShift != 32)
		    	tmp |= BitRight(bits, rightShift);
#ifdef FAST_CONSTANT_OFFSET_MODE
		    pdst[0] = GetFourPixels(tmp);
		    NextFourBits(tmp);
		    pdst[1] = GetFourPixels(tmp);
		    NextFourBits(tmp);
		    pdst[2] = GetFourPixels(tmp);
		    NextFourBits(tmp);
		    pdst[3] = GetFourPixels(tmp);
		    NextFourBits(tmp);
		    pdst[4] = GetFourPixels(tmp);
		    NextFourBits(tmp);
		    pdst[5] = GetFourPixels(tmp);
		    NextFourBits(tmp);
		    pdst[6] = GetFourPixels(tmp);
		    NextFourBits(tmp);
		    pdst[7] = GetFourPixels(tmp);
		    pdst += 8;
#else
	    	    *pdst++ = GetFourPixels(tmp);
	    	    NextFourBits(tmp);
	    	    *pdst++ = GetFourPixels(tmp);
	    	    NextFourBits(tmp);
	    	    *pdst++ = GetFourPixels(tmp);
	    	    NextFourBits(tmp);
	    	    *pdst++ = GetFourPixels(tmp);
	    	    NextFourBits(tmp);
	    	    *pdst++ = GetFourPixels(tmp);
	    	    NextFourBits(tmp);
	    	    *pdst++ = GetFourPixels(tmp);
	    	    NextFourBits(tmp);
	    	    *pdst++ = GetFourPixels(tmp);
	    	    NextFourBits(tmp);
	    	    *pdst++ = GetFourPixels(tmp);
#endif
	    	}
	    	if (nl || endmask)
	    	{
		    tmp = BitLeft(bits, leftShift);
		    /*
		     * better condition needed -- mustn't run
		     * off the end of the source...
		     */
		    if (rightShift != 32)
		    {
		    	bits = *psrc++;
		    	tmp |= BitRight (bits, rightShift);
		    }
#ifdef FAST_CONSTANT_OFFSET_MODE
		    pdst += nl;
		    switch (nl)
		    {
		    case 7:
		    	pdst[-7] = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 6:
		    	pdst[-6] = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 5:
		    	pdst[-5] = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 4:
		    	pdst[-4] = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 3:
		    	pdst[-3] = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 2:
		    	pdst[-2] = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 1:
		    	pdst[-1] = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    }
#else
		    switch (nl)
		    {
		    case 7:
		    	*pdst++ = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 6:
		    	*pdst++ = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 5:
		    	*pdst++ = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 4:
		    	*pdst++ = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 3:
		    	*pdst++ = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 2:
		    	*pdst++ = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    case 1:
		    	*pdst++ = GetFourPixels(tmp);
		    	NextFourBits(tmp);
		    }
#endif
		    if (endmask)
		    	*pdst = *pdst & ~endmask | GetFourPixels(tmp) & endmask;
	    	}
	    }
	}
	else
	{
	    register unsigned int   src;

	    planemask = PFILL(planemask);
	    while (height--)
	    {
	    	psrc = psrcLine;
	    	pdst = pdstLine;
	    	psrcLine += widthSrc;
	    	pdstLine += widthDst;
	    	bits = *psrc++;
	    	if (startmask)
	    	{
		    if (firstoff < 0)
		    	tmp = BitRight (bits, -firstoff);
		    else
		    {
		    	tmp = BitLeft (bits, firstoff);
		    	if (firstoff > 28)
		    	{
			    bits = *psrc++;
			    tmp |= BitRight (bits, secondoff);
		    	}
		    }
		    src = GetFourPixels (tmp);
		    *pdst = *pdst & ~(startmask & planemask) |
			    DoRop (rop, src, *pdst) &
 			    (startmask & planemask);
		    pdst++;
	    	}
	    	nl = nlMiddle;
	    	while (nl >= 8)
	    	{
		    int	i;
		    nl -= 8;
		    tmp = BitLeft(bits, leftShift);
		    bits = *psrc++;
		    if (rightShift != 32)
		    	tmp |= BitRight(bits, rightShift);
		    i = 8;
		    while (i--)
		    {
			src = GetFourPixels (tmp);
		    	*pdst = *pdst & ~planemask |
			    	DoRop(rop, src, *pdst) & planemask;
		    	pdst++;
		    	NextFourBits(tmp);
		    }
	    	}
	    	if (nl || endmask)
	    	{
		    tmp = BitLeft(bits, leftShift);
		    /*
		     * better condition needed -- mustn't run
		     * off the end of the source...
		     */
		    if (rightShift != 32)
		    {
		    	bits = *psrc++;
		    	tmp |= BitRight (bits, rightShift);
		    }
		    while (nl--)
		    {
			src = GetFourPixels (tmp);
		    	*pdst = *pdst & ~planemask |
			    	DoRop(rop, src, *pdst) & planemask;
		    	pdst++;
			NextFourBits(tmp);
		    }
		    if (endmask)
		    {
			src = GetFourPixels (tmp);
			*pdst = *pdst & ~(endmask & planemask) |
				DoRop (rop, src, *pdst) &
				(endmask & planemask);
		    }
	    	}
	    }
	}
    }
}
#endif

RegionPtr cfbCopyPlane(pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty, bitPlane)
    DrawablePtr 	pSrcDrawable;
    DrawablePtr		pDstDrawable;
    GCPtr		pGC;
    int 		srcx, srcy;
    int 		width, height;
    int 		dstx, dsty;
    unsigned long	bitPlane;
{
    RegionPtr	ret;
    extern RegionPtr    miHandleExposures();

#if (PPW == 4)
    if (pSrcDrawable->depth == 1 && pDstDrawable->depth == 8)
    {
    	if (bitPlane == 1)
	{
    	    doBitBlt = cfbCopyPlane1to8;
	    if (!cfb8CheckPixels(pGC->fgPixel, pGC->bgPixel))
		cfb8SetPixels (pGC->fgPixel, pGC->bgPixel);
    	    ret = cfbCopyArea (pSrcDrawable, pDstDrawable,
	    	    pGC, srcx, srcy, width, height, dstx, dsty);
    	    doBitBlt = cfbDoBitblt;
	}
	else
	    ret = miHandleExposures (pSrcDrawable, pDstDrawable,
	    	pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    }
    else
#endif
    {
	ret = miCopyPlane (pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    }
    return ret;
}
