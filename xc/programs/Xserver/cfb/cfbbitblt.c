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
/* $XConsortium: cfbbitblt.c,v 5.6 89/08/15 00:44:46 keith Exp $ */

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

#undef Duff

#define Duff(counter, block)	    \
    switch (counter & 15) {	    \
    do {			    \
	{ block; }		    \
    case 15:			    \
	{ block; }		    \
    case 14:			    \
	{ block; }		    \
    case 13:			    \
	{ block; }		    \
    case 12:			    \
	{ block; }		    \
    case 11:			    \
	{ block; }		    \
    case 10:			    \
	{ block; }		    \
    case 9:			    \
	{ block; }		    \
    case 8:			    \
	{ block; }		    \
    case 7:			    \
	{ block; }		    \
    case 6:			    \
	{ block; }		    \
    case 5:			    \
	{ block; }		    \
    case 4:			    \
	{ block; }		    \
    case 3:			    \
	{ block; }		    \
    case 2:			    \
	{ block; }		    \
    case 1:			    \
	{ block; }		    \
    case 0:			    \
	;			    \
    } while ((counter -= 16) >= 0);  \
}

cfbDoBitblt(pSrc, pDst, alu, prgnDst, pptSrc, planemask)
    DrawablePtr	    pSrc, pDst;
    int		    alu;
    RegionPtr	    prgnDst;
    DDXPointPtr	    pptSrc;
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
    register unsigned int tmp, bits;
				/* place to store full source word */
    int xoffSrc, xoffDst;
    int	leftShift, rightShift;

    int nstart;			/* number of ragged bits at start of dst */
    int nend;			/* number of ragged bits at end of dst */
    int srcStartOver;		/* pulling nstart bits from src
				   overflows into the next word? */
    int careful;
    int	xoff;

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
    if (alu == GXcopy && (planemask & PIM) == PIM)
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
		    if (xoffSrc > xoffDst)
		    {
		    	leftShift = (xoffSrc - xoffDst) << (5 - PWSH);
		    	rightShift = 32 - leftShift;
	    	    	while (h--)
	    	    	{
		    	    psrc = psrcLine;
		    	    pdst = pdstLine;
		    	    pdstLine += widthDst;
		    	    psrcLine += widthSrc;
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

#define doitup(n)	loop(psrc[n], pdst[n])
			    
#define caseitup(n) case n: doitup(n)

#ifdef NOTDEF
    caseitup(15)    caseitup(14)    caseitup(13)    caseitup(12)    \
    caseitup(11)    caseitup(10)    caseitup( 9)    caseitup( 8)    \
    caseitup( 7)    caseitup( 6)    caseitup( 5)    caseitup( 4)    \
	doitup(8)   doitup(9)	doitup(10)  doitup(11)	\
	doitup(12)  doitup(13)	doitup(14)  doitup(15)	\
	doitup(4)   doitup(5)	doitup(6)   doitup(7)	\

#endif

#define UpStore						\
    switch (nl & 3) {					\
    caseitup( 3)    caseitup( 2)    caseitup( 1)	\
    }							\
    while ((nl -= 4) >= 0) {				\
	doitup(0)   doitup(1)	doitup(2)   doitup(3)	\
	psrc += 4;					\
	pdst += 4;					\
    }

#define doitdn(n)   loop(psrc[-(n+1)], pdst[-(n+1)])

#define caseitdn(n) case n: doitdn(n)

#define DownStore					\
    switch (nl & 3) {					\
    caseitdn( 3)    caseitdn( 2)    caseitdn( 1)	\
    }							\
    while ((nl -= 4) >= 0) {				\
	doitdn(0)   doitdn(1)	doitdn(2)   doitdn(3)	\
	psrc -= 4;					\
	pdst -= 4;					\
    }

#define loop(from,to) \
    tmp = BitLeft(bits, leftShift);	    \
    bits = from;			    \
    to = tmp | BitRight(bits, rightShift);

			    UpStore
#undef loop

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
		    else if (xoffDst > xoffSrc)
		    {
		    	rightShift = (xoffDst - xoffSrc) << (5 - PWSH);
		    	leftShift = 32 - rightShift;
	    	    	while (h--)
	    	    	{
		    	    psrc = psrcLine;
		    	    pdst = pdstLine;
		    	    pdstLine += widthDst;
		    	    psrcLine += widthSrc;
			    tmp = 0;
			    if (startmask)
			    {
			    	bits = *psrc++;
			    	tmp |= BitRight(bits, rightShift);
			    	*pdst = (*pdst & ~startmask) |
				    	(tmp & startmask);
			    	pdst++;
			    	tmp = BitLeft(bits, leftShift);
			    }
			    nl = nlMiddle;

#define loop(from,to) \
    bits = from;			    \
    to = tmp | BitRight(bits, rightShift);  \
    tmp = BitLeft (bits, leftShift);

			    UpStore
#undef loop

			    if (endmask)
			    {
			    	if (BitLeft (endmask, rightShift))
			    	{
			    	    bits = *psrc++;
			    	    tmp |= BitRight (bits, rightShift);
			    	}
			    	*pdst = (*pdst & ~endmask) |
				    	(tmp & endmask);
			    }
		    	}
		    }
		    else
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
#define loop(from,to) \
    to = from;
			    UpStore
#undef loop

			    if (endmask)
			    	*pdst = (*pdst & ~endmask) | (*psrc++ & endmask);
		    	}
	    	    }
	    	}
	    	else	/* xdir == -1 */
	    	{
	    	    xoffSrc = (pptSrc->x + w - 1) & PIM;
	    	    xoffDst = (pbox->x2 - 1) & PIM;
		    pdstLine += ((pbox->x2-1) >> PWSH) + 1;
		    psrcLine += ((pptSrc->x+w - 1) >> PWSH) + 1;
		    if (xoffSrc > xoffDst)
		    {
		    	leftShift = (xoffSrc - xoffDst) << (5 - PWSH);
		    	rightShift = 32 - leftShift;
	    	    	while (h--)
	    	    	{
		    	    psrc = psrcLine;
		    	    pdst = pdstLine;
		    	    pdstLine += widthDst;
		    	    psrcLine += widthSrc;
			    tmp = 0;
			    if (endmask)
			    {
			    	bits = *--psrc;
			    	tmp = BitLeft(bits, leftShift);
			    	pdst--;
			    	*pdst = (*pdst & ~endmask) |
				    	(tmp & endmask);
			    	tmp = BitRight(bits, rightShift);
			    }
			    nl = nlMiddle;
#define loop(from,to) \
    bits = from;			    \
    to = tmp | BitLeft(bits, leftShift);    \
    tmp = BitRight(bits, rightShift);

			    DownStore
#undef loop

			    if (startmask)
			    {
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
		    else if (xoffDst > xoffSrc)
		    {
		    	rightShift = (xoffDst - xoffSrc) << (5 - PWSH);
		    	leftShift = 32 - rightShift;
	    	    	while (h--)
	    	    	{
		    	    psrc = psrcLine;
		    	    pdst = pdstLine;
		    	    pdstLine += widthDst;
		    	    psrcLine += widthSrc;
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

#define loop(from,to) \
    tmp = BitRight(bits, rightShift);		\
    bits = from;				\
    to = tmp | BitLeft (bits, leftShift);

			    DownStore
#undef loop

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
		    else
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
#define loop(from,to)	\
    to = from;
			    DownStore
#undef loop
			    if (startmask)
			    {
			    	--pdst;
			    	*pdst = (*pdst & ~startmask) | (*--psrc & startmask);
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
            	register int xoffSrc;
            	int nstart;
            	int nend;
            	int srcStartOver;
	    	int tmpSrc;
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

	cfbDoBitblt(pSrcDrawable, pDstDrawable, pGC->alu, &rgnDst, pptSrc, pGC->planemask);
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
