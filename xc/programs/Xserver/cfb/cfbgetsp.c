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
#include "servermd.h"

#include "misc.h"
#include "region.h"
#include "gc.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "cfb.h"
#include "cfbmskbits.h"

extern void miBSGetSpans();

/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
unsigned int	*
cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
{
    register unsigned int	*pdst;		/* where to put the bits */
    register unsigned int	*psrc;		/* where to get the bits */
    register unsigned int	tmpSrc;		/* scratch buffer for bits */
    unsigned int		*psrcBase;	/* start of src bitmap */
    int			widthSrc;	/* width of pixmap in bytes */
    register DDXPointPtr pptLast;	/* one past last point to get */
    int         	xEnd;		/* last pixel to copy from */
    register int	nstart; 
    int	 		nend; 
    int	 		srcStartOver; 
    int	 		startmask, endmask, nlMiddle, nl, srcBit;
    int			w;
    unsigned int	*pdstStart;
    unsigned int	*pdstNext;
#ifdef NOTDEF
    DDXPointPtr	  	pptInit;
    int	    	  	*pwidthInit;
#endif
    int	    	  	*pwidthPadded;
    int	    	  	i;

    switch (pDrawable->depth) {
	case 1:
	    return (mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans));
	case 8:
	    break;
	default:
	    FatalError("cfbGetSpans: invalid depth\n");
    }
    pptLast = ppt + nspans;
#ifdef NOTDEF
    pptInit = ppt;
    pwidthInit = pwidth;
#endif
    pwidthPadded = (int *)ALLOCATE_LOCAL(nspans * sizeof(int));
    if (!pwidthPadded)
	return (unsigned int *)NULL;

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	psrcBase = (unsigned int *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	widthSrc = (int)
		   ((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind;
    }
    else
    {
	psrcBase = (unsigned int *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	widthSrc = (int)(((PixmapPtr)pDrawable)->devKind);
    }
    pdstStart = (unsigned int *)xalloc(nspans * PixmapBytePad(wMax, PSZ));
    if (!pdstStart)
    {
	DEALLOCATE_LOCAL(pwidthPadded);
	return (unsigned int*)NULL;
    }
    pdst = pdstStart;

    i = 0;
    while(ppt < pptLast)
    {
	xEnd = min(ppt->x + *pwidth, widthSrc << (PWSH-2) );
	psrc = psrcBase + (ppt->y * (widthSrc >> 2)) + (ppt->x >> PWSH); 
	w = xEnd - ppt->x;
	srcBit = ppt->x & PIM;
	/* This shouldn't be needed */
	pdstNext = pdst + PixmapWidthInPadUnits(w, PSZ);
	pwidthPadded[i] = PixmapWidthInPadUnits(w, PSZ) * PPW;
	i++;

	if (srcBit + w <= PPW) 
	{ 
	    getbits(psrc, srcBit, w, tmpSrc);
/*XXX*/	    putbits(tmpSrc, 0, w, pdst, -1); 
	    pdst++;
	} 
	else 
	{ 

	    maskbits(ppt->x, w, startmask, endmask, nlMiddle);
	    if (startmask) 
		nstart = PPW - srcBit; 
	    else 
		nstart = 0; 
	    if (endmask) 
		nend = xEnd & PIM; 
	    srcStartOver = srcBit + nstart > PLST;
	    if (startmask) 
	    { 
		getbits(psrc, srcBit, nstart, tmpSrc);
/*XXX*/		putbits(tmpSrc, 0, nstart, pdst, -1);
		if(srcStartOver)
		    psrc++;
	    } 
	    nl = nlMiddle; 
	    while (nl--) 
	    { 
		tmpSrc = *psrc;
/*XXX*/		putbits(tmpSrc, nstart, PPW, pdst, -1);
		psrc++;
		pdst++;
	    } 
	    if (endmask) 
	    { 
		getbits(psrc, 0, nend, tmpSrc);
/*XXX*/		putbits(tmpSrc, nstart, nend, pdst, -1);
		if(nstart + nend >= PPW)
		    pdst++;
	    } 
#ifdef	notdef
	    pdst++; 
	    while(pdst < pdstNext)
	    {
		*pdst++ = 0;
	    }
#else
	    pdst = pdstNext;
#endif notdef
	} 
        ppt++;
	pwidth++;
    }
#ifdef NOTDEF
    /*
     * If the drawable is a window with some form of backing-store, consult
     * the backing-store module to fetch any invalid spans from the window's
     * backing-store. The pixmap is made into one long scanline and the
     * backing-store module takes care of the rest. We do, however, have
     * to tell the backing-store module exactly how wide each span is, padded
     * to the correct boundary, so we allocate pwidthPadded and set those
     * widths into it.
     */
    if ((pDrawable->type == DRAWABLE_WINDOW) &&
	(((WindowPtr)pDrawable)->backingStore != NotUseful))
    {
	PixmapRec pixmap;

	pixmap.drawable.type = DRAWABLE_PIXMAP;
	pixmap.drawable.pScreen = pDrawable->pScreen;
	pixmap.drawable.depth = pDrawable->depth;
	pixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
	pixmap.devKind = PixmapBytePad(wMax, PSZ) * nspans;
	pixmap.width = (pixmap.devKind >> 2) * PPW;
	pixmap.height = 1;
	pixmap.refcnt = 1;
	pixmap.devPrivate = (pointer)pdstStart;
	miBSGetSpans(pDrawable, &pixmap, wMax, pptInit, pwidthInit,
		     pwidthPadded, nspans);
    }
#endif
    DEALLOCATE_LOCAL(pwidthPadded);
    return(pdstStart);
}

