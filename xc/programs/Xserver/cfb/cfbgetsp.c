/* $XConsortium: cfbgetsp.c,v 5.12 93/09/20 20:09:50 dpw Exp $ */
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

/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
void
cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pchardstStart)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    char		*pchardstStart; /* where to put the bits */
{
    PixelGroup	*pdstStart = (PixelGroup *)pchardstStart;
    register PixelGroup	*pdst;		/* where to put the bits */
    register PixelGroup	*psrc;		/* where to get the bits */
    register PixelGroup	tmpSrc;		/* scratch buffer for bits */
    PixelGroup		*psrcBase;	/* start of src bitmap */
    int			widthSrc;	/* width of pixmap in bytes */
    register DDXPointPtr pptLast;	/* one past last point to get */
    int         	xEnd;		/* last pixel to copy from */
    register int	nstart; 
    int	 		nend; 
    PixelGroup		startmask, endmask;
    int			nlMiddle, nl, srcBit;
    int			w;
    PixelGroup		*pdstNext;

    switch (pDrawable->bitsPerPixel) {
	case 1:
	    mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pchardstStart);
	    return;
	case PSZ:
	    break;
	default:
	    FatalError("cfbGetSpans: invalid depth\n");
    }

    
    cfbGetLongWidthAndPointer (pDrawable, widthSrc, psrcBase)

#ifdef PIXEL_ADDR
    if ((nspans == 1) && (*pwidth == 1))
    {
	tmpSrc = *((PixelType *)(psrcBase + (ppt->y * widthSrc))
		   + ppt->x);
#if BITMAP_BIT_ORDER == MSBFirst
	tmpSrc <<= (sizeof (unsigned long) - sizeof (PixelType)) * 8;
#endif
	*pdstStart = tmpSrc;
	return;
    }
#endif
    pdst = pdstStart;
    pptLast = ppt + nspans;
    while(ppt < pptLast)
    {
	xEnd = min(ppt->x + *pwidth, widthSrc << PWSH);
	psrc = psrcBase + ppt->y * widthSrc + (ppt->x >> PWSH); 
	w = xEnd - ppt->x;
	srcBit = ppt->x & PIM;
    	pdstNext = pdst + ((w + PPW - 1) >> PWSH);

	if (srcBit + w <= PPW) 
	{ 
	    getbits(psrc, srcBit, w, tmpSrc);
	    putbits(tmpSrc, 0, w, pdst, ~((unsigned long)0)); 
	    pdst++;
	} 
	else 
	{ 
	    maskbits(ppt->x, w, startmask, endmask, nlMiddle);
	    nstart = 0; 
	    if (startmask) 
	    { 
		nstart = PPW - srcBit; 
		getbits(psrc, srcBit, nstart, tmpSrc);
		putbits(tmpSrc, 0, nstart, pdst, ~((unsigned long)0));
		if(srcBit + nstart >= PPW)
		    psrc++;
	    } 
	    nl = nlMiddle; 
	    while (nl--) 
	    { 
		tmpSrc = *psrc;
		putbits(tmpSrc, nstart, PPW, pdst, ~((unsigned long)0));
		psrc++;
		pdst++;
	    } 
	    if (endmask) 
	    { 
		nend = xEnd & PIM; 
		getbits(psrc, 0, nend, tmpSrc);
		putbits(tmpSrc, nstart, nend, pdst, ~((unsigned long)0));
	    } 
	    pdst = pdstNext;
	} 
        ppt++;
	pwidth++;
    }
}
