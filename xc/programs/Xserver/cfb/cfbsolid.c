/*
 * $XConsortium$
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */


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
#include "cfbrrop.h"

#if defined(FAST_CONSTANT_OFFSET_MODE) && (RROP != GXcopy)
# define Expand(left,right) {\
    int part = nmiddle & 3; \
    while (h--) { \
	pdst = pdstRect; \
	left \
	m = nmiddle; \
	pdst += part; \
	switch (part) { \
	case 3: \
	    RROP_SOLID(pdst - 3); \
	case 2: \
	    RROP_SOLID(pdst - 2); \
	case 1: \
	    RROP_SOLID(pdst - 1); \
	} \
	while ((m -=4) >= 0) { \
	    RROP_SOLID (pdst + 0); \
	    RROP_SOLID (pdst + 1); \
	    RROP_SOLID (pdst + 2); \
	    RROP_SOLID (pdst + 3); \
	    pdst += 4; \
	} \
	right \
	pdstRect += widthDst; \
    } \
}
#else
# define Expand(left, right) { \
    while (h--) { \
	pdst = pdstRect; \
	left \
	m = nmiddle; \
	while (m--) {\
	    RROP_SOLID(pdst); \
	    pdst++; \
	} \
	right \
	pdstRect += widthDst; \
    } \
}
#endif
	

void
RROP_NAME(cfbFillRectSolid) (pDrawable, pGC, nBox, pBox)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nBox;
    BoxPtr	    pBox;
{
    register int    m;
    register unsigned long   *pdst;
    RROP_DECLARE
    register unsigned long   leftMask, rightMask;
    unsigned long   *pdstBase, *pdstRect;
    int		    nmiddle;
    int		    h;
    int		    w;
    int		    widthDst;
    cfbPrivGCPtr    devPriv;

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

    RROP_FETCH_GC(pGC)
    
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

	    while (h--)
	    {
		RROP_SOLID (pdstb);
		pdstb += incr;
	    }
	}
	else
	{
#endif
	pdstRect += (pBox->x1 >> PWSH);
	if ((pBox->x1 & PIM) + w <= PPW)
	{
	    maskpartialbits(pBox->x1, w, leftMask);
	    pdst = pdstRect;
	    while (h--) {
		RROP_SOLID_MASK (pdst, leftMask);
		pdst += widthDst;
	    }
	}
	else
	{
	    maskbits (pBox->x1, w, leftMask, rightMask, nmiddle);
	    if (leftMask)
	    {
		if (rightMask)	/* left mask and right mask */
		{
		    Expand(RROP_SOLID_MASK (pdst, leftMask); pdst++;,
			   RROP_SOLID_MASK (pdst, rightMask);)
		}
		else	/* left mask and no right mask */
		{
		    Expand(RROP_SOLID_MASK (pdst, leftMask); pdst++;,
			   )
		}
	    }
	    else
	    {
		if (rightMask)	/* no left mask and right mask */
		{
		    Expand(,
			   RROP_SOLID_MASK (pdst, rightMask);)
		}
		else	/* no left mask and no right mask */
		{
		    Expand(,
			    )
		}
	    }
	}
#if PPW == 4
	}
#endif
    }
}

void
RROP_NAME(cfbSolidSpans) (pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
    unsigned long   *pdstBase;
    int		    widthDst;

    RROP_DECLARE
    
    register unsigned long  *pdst;
    register int	    nlmiddle;
    register unsigned long  startmask, endmask;
    register int	    w;
    int			    x;
    
				/* next three parameters are post-clip */
    int		    n;		/* number of spans to fill */
    DDXPointPtr	    ppt;	/* pointer to list of start points */
    int		    *pwidthFree;/* copies of the pointers to free */
    DDXPointPtr	    pptFree;
    int		    *pwidth;
    cfbPrivGCPtr    devPriv;

    devPriv = (cfbPrivGCPtr)pGC->devPrivates[cfbGCPrivateIndex].ptr;
    RROP_FETCH_GCPRIV(devPriv)
    
    n = nInit * miFindMaxBand(devPriv->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	if (pptFree) DEALLOCATE_LOCAL(pptFree);
	if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(devPriv->pCompositeClip,
		     pptInit, pwidthInit, nInit,
		     ppt, pwidth, fSorted);

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

    while (n--)
    {
	x = ppt->x;
	pdst = pdstBase + (ppt->y * widthDst);
	++ppt;
	w = *pwidth++;
	if (!w)
	    continue;
#if PPW == 4
	if (w <= 4)
	{
	    register char   *addrb;

	    addrb = ((char *) pdst) + x;
	    while (w--)
	    {
		RROP_SOLID (addrb);
		addrb++;
	    }
	}
#else
	if ((x & PIM) + width <= PPW)
	{
	    pdst += x >> PWSH;
	    maskpartialbits (x, w, startmask);
	    RROP_SOLID_MASK (pdst, startmask);
	}
#endif
	else
	{
	    pdst += x >> PWSH;
	    maskbits (x, w, startmask, endmask, nlmiddle);
	    if (startmask)
	    {
		RROP_SOLID_MASK (pdst, startmask);
		++pdst;
	    }
	    while (nlmiddle--)
	    {
		RROP_SOLID(pdst);
		++pdst;
	    }
	    if (endmask)
	    {
		RROP_SOLID_MASK (pdst, endmask);
	    }
	}
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}
