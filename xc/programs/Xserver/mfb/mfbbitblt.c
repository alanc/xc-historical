/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
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
/* $XConsortium: mfbbitblt.c,v 5.1 89/07/09 15:59:33 rws Exp $ */
#include "X.h"
#include "Xprotostr.h"

#include "miscstruct.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "mi.h"

#include "mfb.h"
#include "maskbits.h"


/* CopyArea and CopyPlane for a monchrome frame buffer


    clip the source rectangle to the source's available bits.  (this
avoids copying unnecessary pieces that will just get exposed anyway.)
this becomes the new shape of the destination.
    clip the destination region to the composite clip in the
GC.  this requires translating the destination region to (dstx, dsty).
    build a list of source points, one for each rectangle in the
destination.  this is a simple translation.
    go do the multiple rectangle copies
    do graphics exposures
*/
/*  #ifdef PURDUE!!!!
 ** Optimized for drawing pixmaps into windows, especially when drawing into
 ** unobscured windows.  Calls to the general-purpose region code were
 ** replaced with rectangle-to-rectangle clipping comparisions.  This is
 ** possible, since the pixmap is a single rectangle.  In an unobscured
 ** window, the destination clip is also a single rectangle, and region
 ** code can be avoided entirely.  This is a big savings, since the region
 ** code uses XAlloc() and makes many function calls.
 **
 ** In addition, if source is a pixmap, there is no need to call the
 ** expensive miHandleExposures() routine.  Instead, we simply return NULL.
 **
 ** Previously, drawing a pixmap into an unobscured window executed at least
 ** 8 XAlloc()'s, 30 function calls, and hundreds of lines of code.
 **
 ** Now, the same operation requires no XAlloc()'s, no region function calls,
 ** and much less overhead.  Nice for drawing lots of small pixmaps.
 */
 
RegionPtr
mfbCopyArea(pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty)
register DrawablePtr pSrcDrawable;
register DrawablePtr pDstDrawable;
GC *pGC;
int srcx, srcy;
int width, height;
int dstx, dsty;
{
    RegionPtr prgnSrcClip;	/* may be a new region, or just a copy */
    int realSrcClip = 0;	/* non-0 if we've created a src clip */

    RegionPtr prgnDst, prgnExposed;
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    register BoxPtr pbox;
    int i;
    register int dx;
    register int dy;
    xRectangle origSource;
    DDXPointRec origDest;
    int numRects;
#ifdef PURDUE
    RegionRec fastRegion;	/* special region for clipping to 1 box */
    BoxRec fastBox;
    int fastClip = 0;		/* for fast clipping with pixmap source */
    int fastExpose = 0;		/* for fast exposures with pixmap source */
#endif /* PURDUE */

    origSource.x = srcx;
    origSource.y = srcy;
    origSource.width = width;
    origSource.height = height;
    origDest.x = dstx;
    origDest.y = dsty;

    srcx += pSrcDrawable->x;
    srcy += pSrcDrawable->y;

    /* clip the source */

    if (pSrcDrawable->type == DRAWABLE_PIXMAP)
    {
	if ((pSrcDrawable == pDstDrawable) &&
	    (pGC->clientClipType == CT_NONE))
	{
	    prgnSrcClip = ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;
	}
	else
#ifndef PURDUE
	{
	    BoxRec box;

	    box.x1 = pSrcDrawable->x;
	    box.y1 = pSrcDrawable->y;
	    box.x2 = box.x1 + (int) pSrcDrawable->width;
	    box.y2 = box.y1 + (int) pSrcDrawable->height;

	    prgnSrcClip = (*pGC->pScreen->RegionCreate)(&box, 1);
	    realSrcClip = 1;
	}
#else /* PURDUE */
	{
	    /* Pixmap sources generate simple exposure events */
	    fastExpose = 1;

	    /* Pixmap is just one clipping rectangle so we can avoid
	       allocating a full-blown region. */
	    fastClip = 1;

	    fastBox.x1 = srcx;
	    fastBox.y1 = srcy;
	    fastBox.x2 = srcx + width;
	    fastBox.y2 = srcy + height;
	    
	    /* Left and top are already clipped, so clip right and bottom */
	    if (fastBox.x2 > pSrcDrawable->x + (int) pSrcDrawable->width)
	      fastBox.x2 = pSrcDrawable->x + (int) pSrcDrawable->width;
	    if (fastBox.y2 > pSrcDrawable->y + (int) pSrcDrawable->height)
	      fastBox.y2 = pSrcDrawable->y + (int) pSrcDrawable->height;

	    (*pGC->pScreen->RegionInit)(&fastRegion, &fastBox, 1);
	}
#endif /* PURDUE */
    }
    else
    {
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    if ((pSrcDrawable == pDstDrawable) &&
		(pGC->clientClipType == CT_NONE))
	    {
		prgnSrcClip = ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;
	    }
	    else
	    {
		prgnSrcClip = NotClippedByChildren((WindowPtr)pSrcDrawable);
		realSrcClip = 1;
	    }
	}
	else
	{
	    prgnSrcClip = &((WindowPtr)pSrcDrawable)->clipList;
	}
    }

#ifdef PURDUE
    /* Don't create a source region if we are doing a fast clip */
    if (!fastClip)
#endif /* PURDUE */
    {
	BoxRec srcBox;

	srcBox.x1 = srcx;
	srcBox.y1 = srcy;
	srcBox.x2 = srcx + width;
	srcBox.y2 = srcy + height;
	
	prgnDst = (*pGC->pScreen->RegionCreate)(&srcBox, 1);
	(*pGC->pScreen->Intersect)(prgnDst, prgnDst, prgnSrcClip);
    }
    
    dstx += pDstDrawable->x;
    dsty += pDstDrawable->y;

    if (pDstDrawable->type == DRAWABLE_WINDOW)
    {
	if (!((WindowPtr)pDstDrawable)->realized)
	{
#ifdef PURDUE
	    if (!fastClip)
#endif
		(*pGC->pScreen->RegionDestroy)(prgnDst);
	    if (realSrcClip)
		(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
	    return NULL;
	}
    }

    dx = srcx - dstx;
    dy = srcy - dsty;

#ifndef PURDUE
    /* clip the shape of the dst to the destination composite clip */
    (*pGC->pScreen->TranslateRegion)(prgnDst, -dx, -dy);
    (*pGC->pScreen->Intersect)(prgnDst,
			       prgnDst,
			       ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);

#else /* PURDUE */
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
	cclip = ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;
        if (REGION_NUM_RECTS(cclip) == 1)
        {
	    BoxPtr pBox = REGION_RECTS(cclip);
	  
	    if (fastBox.x1 < pBox->x1) fastBox.x1 = pBox->x1;
	    if (fastBox.x2 > pBox->x2) fastBox.x2 = pBox->x2;
	    if (fastBox.y1 < pBox->y1) fastBox.y1 = pBox->y1;
	    if (fastBox.y2 > pBox->y2) fastBox.y2 = pBox->y2;

	    /* Check to see if the region is empty */
	    if (fastBox.x1 >= fastBox.x2 || fastBox.y1 >= fastBox.y2)
		(*pGC->pScreen->RegionEmpty)(&fastRegion);

	    /* Use the fast region for all future computation.
	       The following code insures that RegionDestroy is not
	       called on it. */
	    prgnDst = &fastRegion;
	}
        else
	{
	    /* We must turn off fastClip now, since we must create
	       a full blown region.  It is intersected with the
	       composite clip below. */
	    fastClip = 0;
	    prgnDst = (*pGC->pScreen->RegionCreate)(&fastBox,1);
	}
    }
    else
    {
        (*pGC->pScreen->TranslateRegion)(prgnDst, -dx, -dy);
    }

    if (!fastClip)
    {
	(*pGC->pScreen->Intersect)(prgnDst,
				   prgnDst,
				 ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);
    }
#endif /* PURDUE */

    /* Do bit blitting */
    numRects = REGION_NUM_RECTS(prgnDst);
    if (numRects)
    {
	if(!(pptSrc = (DDXPointPtr)ALLOCATE_LOCAL(numRects *
						  sizeof(DDXPointRec))))
	{
#ifdef PURDUE
	    if (!fastClip)
#endif /* PURDUE */
		(*pGC->pScreen->RegionDestroy)(prgnDst);
	    if (realSrcClip)
		(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
	    return NULL;
	}
	pbox = REGION_RECTS(prgnDst);
	ppt = pptSrc;
	for (i = numRects; --i >= 0; pbox++, ppt++)
	{
	    ppt->x = pbox->x1 + dx;
	    ppt->y = pbox->y1 + dy;
	}
    
	if (pGC->planemask & 1)
	    mfbDoBitblt(pSrcDrawable, pDstDrawable, pGC->alu, prgnDst, pptSrc);
	DEALLOCATE_LOCAL(pptSrc);
    }

    prgnExposed = NULL;
    if (((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->fExpose) {
#ifdef PURDUE
        /* Pixmap sources generate a NoExposed (we return NULL to do this) */
        if (!fastExpose)
#endif /* PURDUE */
	    prgnExposed =
		miHandleExposures(pSrcDrawable, pDstDrawable, pGC,
				  origSource.x, origSource.y,
				  (int)origSource.width,
				  (int)origSource.height,
				  origDest.x, origDest.y, (unsigned long)0);
	}
#ifdef PURDUE
    
    /* Destroy any created regions */
    if (!fastClip)
#endif /* PURDUE */
	(*pGC->pScreen->RegionDestroy)(prgnDst);
    if (realSrcClip)
	(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
    return prgnExposed;
}

/* DoBitblt() does multiple rectangle moves into the rectangles
   DISCLAIMER:
   this code can be made much faster; this implementation is
designed to be independent of byte/bit order, processor
instruction set, and the like.  it could probably be done
in a similarly device independent way using mask tables instead
of the getbits/putbits macros.  the narrow case (w<32) can be
subdivided into a case that crosses word boundaries and one that
doesn't.

   we have to cope with the dircetion on a per band basis,
rather than a per rectangle basis.  moving bottom to top
means we have to invert the order of the bands; moving right
to left requires reversing the order of the rectangles in
each band.

   if src or dst is a window, the points have already been
translated.
*/

/*
 * magic macro for copying longword aligned regions
 */

#define longcopy(from,to,count)    \
{ \
    switch (count & 7) { \
	  case 0:   *to++ = *from++; \
	  case 7:   *to++ = *from++; \
	  case 6:   *to++ = *from++; \
	  case 5:   *to++ = *from++; \
	  case 4:   *to++ = *from++; \
	  case 3:   *to++ = *from++; \
	  case 2:   *to++ = *from++; \
	  case 1:   *to++ = *from++; \
    } \
    while ((count -= 8) > 0) { \
	  *to++ = *from++; \
	  *to++ = *from++; \
	  *to++ = *from++; \
	  *to++ = *from++; \
	  *to++ = *from++; \
	  *to++ = *from++; \
	  *to++ = *from++; \
	  *to++ = *from++; \
    } \
}

#ifndef PURDUE

#define longRop(alu,from,to,count)    \
{ \
    switch (count & 7) { \
	  case 0:   *to = DoRop (alu, *from++, *to); to++; \
	  case 7:   *to = DoRop (alu, *from++, *to); to++; \
	  case 6:   *to = DoRop (alu, *from++, *to); to++; \
	  case 5:   *to = DoRop (alu, *from++, *to); to++; \
	  case 4:   *to = DoRop (alu, *from++, *to); to++; \
	  case 3:   *to = DoRop (alu, *from++, *to); to++; \
	  case 2:   *to = DoRop (alu, *from++, *to); to++; \
	  case 1:   *to = DoRop (alu, *from++, *to); to++; \
    } \
    while ((count -= 8) > 0) { \
	  *to = DoRop (alu, *from++, *to); to++; \
	  *to = DoRop (alu, *from++, *to); to++; \
	  *to = DoRop (alu, *from++, *to); to++; \
	  *to = DoRop (alu, *from++, *to); to++; \
	  *to = DoRop (alu, *from++, *to); to++; \
	  *to = DoRop (alu, *from++, *to); to++; \
	  *to = DoRop (alu, *from++, *to); to++; \
	  *to = DoRop (alu, *from++, *to); to++; \
    } \
}

#define getunalignedword(psrc, x, dst) \
{ \
    int m; \
    m = 32-(x); \
    (dst) = (SCRLEFT(*(psrc), (x)) & endtab[m]) | \
        (SCRRIGHT(*((psrc)+1), m) & starttab[m]); \
}

#else  /* PURDUE */
#define longRop(alu,from,to,count) \
    while (count--) \
	{ \
	    DoRop(*to, alu, *from++, *to); to++; \
	}

#ifdef FASTGETBITS
#define getunalignedword(psrc, x, dst) { \
	register int _tmp; \
	FASTGETBITS(psrc, x, 32, _tmp); \
	dst = _tmp; \
}
#else
#define getunalignedword(psrc, x, dst) \
{ \
    dst = (SCRLEFT((unsigned) *(psrc), (x))) | \
	  (SCRRIGHT((unsigned) *((psrc)+1), 32-(x))); \
}
#endif  /* FASTGETBITS */

#endif  /* PURDUE */

mfbDoBitblt(pSrcDrawable, pDstDrawable, alu, prgnDst, pptSrc)
DrawablePtr pSrcDrawable;
DrawablePtr pDstDrawable;
unsigned char alu;
RegionPtr prgnDst;
DDXPointPtr pptSrc;
{
    unsigned int *psrcBase, *pdstBase;	
				/* start of src and dst bitmaps */
    int widthSrc, widthDst;	/* add to get to same position in next line */

    register BoxPtr pbox;
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
    register unsigned int tmpSrc;
				/* place to store full source word */
    register int xoffSrc;	/* offset (>= 0, < 32) from which to
			           fetch whole longwords fetched 
				   in src */
    int nstart;			/* number of ragged bits at start of dst */
    int nend;			/* number of ragged bits at end of dst */
    int srcStartOver;		/* pulling nstart bits from src
				   overflows into the next word? */
    int careful;


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

    /* XXX we have to err on the side of safety when both are windows,
     * because we don't know if IncludeInferiors is being used.
     */
    careful = ((pSrcDrawable == pDstDrawable) ||
	       ((pSrcDrawable->type == DRAWABLE_WINDOW) &&
		(pDstDrawable->type == DRAWABLE_WINDOW)));

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
    if (alu == GXcopy)
    {
        while (nbox--)
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

	    /* x direction doesn't matter for < 1 longword */
	    if (w <= 32)
	    {
	        int srcBit, dstBit;	/* bit offset of src and dst */

	        pdstLine += (pbox->x1 >> 5);
	        psrcLine += (pptSrc->x >> 5);
	        psrc = psrcLine;
	        pdst = pdstLine;

	        srcBit = pptSrc->x & 0x1f;
	        dstBit = pbox->x1 & 0x1f;

	        while(h--)
	        {
#ifndef PURDUE
		    getbits(psrc, srcBit, w, tmpSrc)
		    putbits(tmpSrc, dstBit, w, pdst)
#else
		    getandputbits(psrc, srcBit, dstBit, w, pdst)
#endif /* PURDUE */
		    pdst += widthDst;
		    psrc += widthSrc;
	        }
	    }
	    else
	    {
	        maskbits(pbox->x1, w, startmask, endmask, nlMiddle)
	        if (startmask)
		    nstart = 32 - (pbox->x1 & 0x1f);
	        else
		    nstart = 0;
	        if (endmask)
	            nend = pbox->x2 & 0x1f;
	        else
		    nend = 0;

	        xoffSrc = ((pptSrc->x & 0x1f) + nstart) & 0x1f;
	        srcStartOver = ((pptSrc->x & 0x1f) + nstart) > 31;

	        if (xdir == 1) /* move left to right */
	        {
	            pdstLine += (pbox->x1 >> 5);
	            psrcLine += (pptSrc->x >> 5);

		    while (h--)
		    {
		        psrc = psrcLine;
		        pdst = pdstLine;

		        if (startmask)
		        {
#ifndef PURDUE
			    getbits(psrc, (pptSrc->x & 0x1f), nstart, tmpSrc)
			    putbits(tmpSrc, (pbox->x1 & 0x1f), nstart, pdst)
#else
			    getandputbits(psrc, (pptSrc->x & 0x1f),
					  (pbox->x1 & 0x1f), nstart, pdst)
#endif /* PURDUE */
			    pdst++;
			    if (srcStartOver)
			        psrc++;
		        }

			/* special case for aligned copies (scrolling) */
			if (xoffSrc == 0)
			{
			    
			    if ((nl = nlMiddle) != 0)
			    {
				longcopy (psrc, pdst, nl)
			    }
			}
 			else
			{
			    nl = nlMiddle + 1;
			    while (--nl)
		            {
				getunalignedword (psrc, xoffSrc, *pdst++);
				psrc++;
			    }
			}

		        if (endmask)
		        {
#ifndef PURDUE
			    getbits(psrc, xoffSrc, nend, tmpSrc)
			    putbits(tmpSrc, 0, nend, pdst)
#else
			    getandputbits0(psrc, xoffSrc, nend, pdst);
#endif  /* PURDUE */
		        }

		        pdstLine += widthDst;
		        psrcLine += widthSrc;
		    }
	        }
	        else /* move right to left */
	        {
	            pdstLine += (pbox->x2 >> 5);
	            psrcLine += (pptSrc->x+w >> 5);
		    /* if fetch of last partial bits from source crosses
		       a longword boundary, start at the previous longword
		    */
		    if (xoffSrc + nend >= 32)
		        --psrcLine;

		    while (h--)
		    {
		        psrc = psrcLine;
		        pdst = pdstLine;

		        if (endmask)
		        {
#ifndef PURDUE
			    getbits(psrc, xoffSrc, nend, tmpSrc)
			    putbits(tmpSrc, 0, nend, pdst)
#else
			    getandputbits0(psrc, xoffSrc, nend, pdst);
#endif
		        }

#ifndef PURDUE
		        nl = nlMiddle + 1;
		        while (--nl)
#else
			nl = nlMiddle;
			while (nl--)
#endif  /* PURDUE */
		        {
			    --psrc;
			    getunalignedword (psrc, xoffSrc, *--pdst)
		        }

		        if (startmask)
		        {
			    if (srcStartOver)
			        --psrc;
			    --pdst;
#ifndef PURDUE
			    getbits(psrc, (pptSrc->x & 0x1f), nstart, tmpSrc)
			    putbits(tmpSrc, (pbox->x1 & 0x1f), nstart, pdst)
#else
			    getandputbits(psrc, (pptSrc->x & 0x1f), (pbox->x1 & 0x1f), nstart, pdst)
#endif /* PURDUE */
		        }

		        pdstLine += widthDst;
		        psrcLine += widthSrc;
		    }
	        } /* move right to left */
	    }
	    pbox++;
	    pptSrc++;
        } /* while (nbox--) */
    }
    else /* do some rop */
    {
        while (nbox--)
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

	    /* x direction doesn't matter for < 1 longword */
	    if (w <= 32)
	    {
	        int srcBit, dstBit;	/* bit offset of src and dst */

	        pdstLine += (pbox->x1 >> 5);
	        psrcLine += (pptSrc->x >> 5);
	        psrc = psrcLine;
	        pdst = pdstLine;

	        srcBit = pptSrc->x & 0x1f;
	        dstBit = pbox->x1 & 0x1f;

	        while(h--)
	        {
#ifndef PURDUE
		    getbits(psrc, srcBit, w, tmpSrc)
		    putbitsrop(tmpSrc, dstBit, w, pdst, alu)
#else
		    getandputrop(psrc, srcBit, dstBit, w, pdst, alu)
#endif  /* PURDUE */
		    pdst += widthDst;
		    psrc += widthSrc;
	        }
	    }
	    else
	    {
	        maskbits(pbox->x1, w, startmask, endmask, nlMiddle)
	        if (startmask)
		    nstart = 32 - (pbox->x1 & 0x1f);
	        else
		    nstart = 0;
	        if (endmask)
	            nend = pbox->x2 & 0x1f;
	        else
		    nend = 0;

	        xoffSrc = ((pptSrc->x & 0x1f) + nstart) & 0x1f;
	        srcStartOver = ((pptSrc->x & 0x1f) + nstart) > 31;

	        if (xdir == 1) /* move left to right */
	        {
	            pdstLine += (pbox->x1 >> 5);
	            psrcLine += (pptSrc->x >> 5);

		    while (h--)
		    {
		        psrc = psrcLine;
		        pdst = pdstLine;

		        if (startmask)
		        {
#ifndef PURDUE
			    getbits(psrc, (pptSrc->x & 0x1f), nstart, tmpSrc)
			    putbitsrop(tmpSrc, (pbox->x1 & 0x1f), nstart, pdst,
				       alu)
#else
			    getandputrop(psrc, (pptSrc->x & 0x1f), 
					 (pbox->x1 & 0x1f), nstart, pdst, alu)
#endif /* PURDUE */
			    pdst++;
			    if (srcStartOver)
			        psrc++;
		        }

			/* special case for aligned operations */
			if (xoffSrc == 0)
			{
			    if ((nl = nlMiddle) != 0)
			    {
				longRop (alu, psrc, pdst, nl)
			    }
			}
 			else
			{
			    nl = nlMiddle + 1;
			    while (--nl)
		            {
				getunalignedword (psrc, xoffSrc, tmpSrc)
#ifndef PURDUE
				*pdst = DoRop (alu, tmpSrc, *pdst);
#else
				DoRop (*pdst, alu, tmpSrc, *pdst);
#endif
				pdst++;
				psrc++;
			    }
			}

		        if (endmask)
		        {
#ifndef PURDUE
			    getbits(psrc, xoffSrc, nend, tmpSrc)
			    putbitsrop(tmpSrc, 0, nend, pdst, alu)
#else
			    getandputrop0(psrc, xoffSrc, nend, pdst, alu);
#endif  /* PURDUE */
		        }

		        pdstLine += widthDst;
		        psrcLine += widthSrc;
		    }
	        }
	        else /* move right to left */
	        {
	            pdstLine += (pbox->x2 >> 5);
	            psrcLine += (pptSrc->x+w >> 5);
		    /* if fetch of last partial bits from source crosses
		       a longword boundary, start at the previous longword
		    */
		    if (xoffSrc + nend >= 32)
		        --psrcLine;

		    while (h--)
		    {
		        psrc = psrcLine;
		        pdst = pdstLine;

		        if (endmask)
		        {
#ifndef PURDUE
			    getbits(psrc, xoffSrc, nend, tmpSrc)
			    putbitsrop(tmpSrc, 0, nend, pdst, alu)
#else
			    getandputrop0(psrc, xoffSrc, nend, pdst, alu);
#endif  /* PURDUE */
		        }

		        nl = nlMiddle + 1;
		        while (--nl)
		        {
			    --psrc;
			    --pdst;
			    getunalignedword(psrc, xoffSrc, tmpSrc)
#ifndef PURDUE
			    *pdst = DoRop(alu, tmpSrc, *pdst);
#else
			    DoRop(*pdst, alu, tmpSrc, *pdst);
#endif  /* PURDUE */
		        }

		        if (startmask)
		        {
			    if (srcStartOver)
			        --psrc;
			    --pdst;
#ifndef PURDUE
			    getbits(psrc, (pptSrc->x & 0x1f), nstart, tmpSrc)
			    putbitsrop(tmpSrc, (pbox->x1 & 0x1f), nstart, pdst,
				       alu)
#else
			    getandputrop(psrc, (pptSrc->x & 0x1f), 
					 (pbox->x1 & 0x1f), nstart, pdst, alu)
#endif /* PURDUE */
		        }

		        pdstLine += widthDst;
		        psrcLine += widthSrc;
		    }
	        } /* move right to left */
	    }
	    pbox++;
	    pptSrc++;
        } /* while (nbox--) */
    }

    /* free up stuff */
    if (pboxNew2)
    {
	DEALLOCATE_LOCAL(pptNew2);
	DEALLOCATE_LOCAL(pboxNew2);
    }
    if (pboxNew1)
    {
	DEALLOCATE_LOCAL(pptNew1);
	DEALLOCATE_LOCAL(pboxNew1);
    }
}


/*
    if fg == 1 and bg ==0, we can do an ordinary CopyArea.
    if fg == bg, we can do a CopyArea with alu = mfbReduceRop(alu, fg)
    if fg == 0 and bg == 1, we use the same rasterop, with
	source operand inverted.

    CopyArea deals with all of the graphics exposure events.
    This code depends on knowing that we can change the
alu in the GC without having to call ValidateGC() before calling
CopyArea().

*/

RegionPtr
mfbCopyPlane(pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty, plane)
DrawablePtr pSrcDrawable, pDstDrawable;
register GC *pGC;
int srcx, srcy;
int width, height;
int dstx, dsty;
unsigned long plane;
{
    int alu;
    RegionPtr	prgnExposed;

    /* XXX a deeper screen ought to wrap ValidateGC to get around this */
    if (pSrcDrawable->depth != 1)
	return miCopyPlane(pSrcDrawable, pDstDrawable,
			   pGC, srcx, srcy, width, height, dstx, dsty, plane);
    if (plane != 1)
	return NULL;

    if ((pGC->fgPixel == 1) && (pGC->bgPixel == 0))
    {
	prgnExposed = (*pGC->ops->CopyArea)(pSrcDrawable, pDstDrawable,
			 pGC, srcx, srcy, width, height, dstx, dsty);
    }
    else if (pGC->fgPixel == pGC->bgPixel)
    {
	alu = pGC->alu;
	pGC->alu = mfbReduceRop(pGC->alu, pGC->fgPixel);
	prgnExposed = (*pGC->ops->CopyArea)(pSrcDrawable, pDstDrawable,
			 pGC, srcx, srcy, width, height, dstx, dsty);
	pGC->alu = alu;
    }
    else /* need to invert the src */
    {
	alu = pGC->alu;
	pGC->alu = InverseAlu[alu];
	prgnExposed = (*pGC->ops->CopyArea)(pSrcDrawable, pDstDrawable,
			 pGC, srcx, srcy, width, height, dstx, dsty);
	pGC->alu = alu;
    }
    return prgnExposed;
}

