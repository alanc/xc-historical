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
/* $XConsortium: mfbgetsp.c,v 5.8 93/09/20 20:21:13 dpw Exp $ */
#include "X.h"
#include "Xmd.h"

#include "misc.h"
#include "region.h"
#include "gc.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "mfb.h"
#include "maskbits.h"

#include "servermd.h"

/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
/*ARGSUSED*/
void
mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pchardstStart)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    char		*pchardstStart;	/* where to put the bits */
{
    PixelType		*pdstStart = (PixelType *)pchardstStart;
    register PixelType	*pdst;	/* where to put the bits */
    register PixelType	*psrc;	/* where to get the bits */
    register PixelType	tmpSrc;	/* scratch buffer for bits */
    PixelType		*psrcBase;	/* start of src bitmap */
    int			widthSrc;	/* width of pixmap in bytes */
    register DDXPointPtr pptLast;	/* one past last point to get */
    int         	xEnd;		/* last pixel to copy from */
    register int	nstart; 
    int	 		nend; 
    int	 		srcStartOver; 
    PixelType 		startmask, endmask;
    unsigned int	srcBit;
    int	 		nlMiddle, nl;
    int			w;
  
    pptLast = ppt + nspans;

    mfbGetPixelWidthAndPointer(pDrawable, widthSrc, psrcBase);
    pdst = pdstStart;

    while(ppt < pptLast)
    {
	/* XXX should this really be << PWSH, or * 8, or * PGSZB? */
	xEnd = min(ppt->x + *pwidth, widthSrc << PWSH);
	pwidth++;
	psrc = mfbScanline(psrcBase, ppt->x, ppt->y, widthSrc);
	w = xEnd - ppt->x;
	srcBit = ppt->x & PIM;

	if (srcBit + w <= PPW) 
	{ 
	    getandputbits0(psrc, srcBit, w, pdst);
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
		getandputbits0(psrc, srcBit, nstart, pdst);
		if(srcStartOver)
		    psrc++;
	    } 
	    nl = nlMiddle; 
#ifdef FASTPUTBITS
	    Duff(nl, putbits(*psrc, nstart, PPW, pdst); psrc++; pdst++;);
#else
	    while (nl--) 
	    { 
		tmpSrc = *psrc;
		putbits(tmpSrc, nstart, PPW, pdst);
		psrc++;
		pdst++;
	    } 
#endif
	    if (endmask) 
	    { 
		putbits(*psrc, nstart, nend, pdst);
		if(nstart + nend > PPW)
		    pdst++;
	    } 
	    if (startmask || endmask)
		pdst++; 
	} 
        ppt++;
    }
}
