/*
 * Fill 32 bit tiled rectangles.  Used by both PolyFillRect and PaintWindow.
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

/* $XConsortium: cfbtile32.c,v 1.1 90/05/15 18:40:37 keith Exp $ */

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
#include "cfb8bit.h"

#include "mergerop.h"

#ifdef sparc
#define SHARED_IDCACHE
#endif

#define STORE(p)    (*(p) = MROP_SOLID(srcpix,*(p)))

#if (RROP == GXCopy) && defined(FAST_CONSTANT_OFFSET_MODE) && defined(SHARED_IDCACHE)
# define Expand(left,right) {\
    int part = nlwMiddle & 7; \
    nlwMiddle >>= 3; \
    while (h--) { \
	srcpix = psrc[srcy]; \
	++srcy; \
	if (srcy == tileHeight) \
	    srcy = 0; \
	left \
	p += part; \
	switch (part) { \
	case 7: \
	    STORE(p - 7); \
	case 6: \
	    STORE(p - 6); \
	case 5: \
	    STORE(p - 5); \
	case 4: \
	    STORE(p - 4); \
	case 3: \
	    STORE(p - 3); \
	case 2: \
	    STORE(p - 2); \
	case 1: \
	    STORE(p - 1); \
	} \
	nlw = nlwMiddle; \
	while (nlw) { \
	    STORE (p + 0); \
	    STORE (p + 1); \
	    STORE (p + 2); \
	    STORE (p + 3); \
	    STORE (p + 4); \
	    STORE (p + 5); \
	    STORE (p + 6); \
	    STORE (p + 7); \
	    p += 8; \
	    nlw--; \
	} \
	right \
	p += nlwExtra; \
    } \
}
#else
#define Expand(left,right) {\
    while (h--)	{ \
	srcpix = psrc[srcy]; \
	++srcy; \
	if (srcy == tileHeight) \
	    srcy = 0; \
	left \
	nlw = nlwMiddle; \
	while (nlw--) \
	{ \
	    *p = MROP_SOLID(srcpix, *p); \
	    p++; \
	} \
	right \
	p += nlwExtra; \
    } \
}
#endif

void
MROP_NAME(cfbFillRectTile32) (pDrawable, pGC, nBox, pBox)
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
    MROP_DECLARE_REG()

    tile = ((cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr))->pRotatedPixmap;
    tileHeight = tile->drawable.height;
    psrc = (int *)tile->devPrivate.ptr;

    MROP_INITIALIZE(pGC->alu, pGC->planemask);

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

	if ( ((pBox->x1 & PIM) + w) <= PPW)
	{
	    maskpartialbits(pBox->x1, w, startmask);
	    nlwExtra = nlwDst;
	    while (h--)
	    {
		srcpix = psrc[srcy];
		++srcy;
		if (srcy == tileHeight)
		    srcy = 0;
		*p = MROP_MASK (srcpix, *p, startmask);
		p += nlwExtra;
	    }
	}
	else
	{
	    maskbits(pBox->x1, w, startmask, endmask, nlwMiddle);
	    nlwExtra = nlwDst - nlwMiddle;

	    if (startmask)
	    {
		nlwExtra -= 1;
		if (endmask)
		{
		    Expand(*p = MROP_MASK(srcpix, *p, startmask); p++;,
			   *p = MROP_MASK(srcpix, *p, endmask);)
		}
		else
		{
		    Expand(*p = MROP_MASK(srcpix, *p, startmask); p++;,
			   )
		}
	    }
	    else
	    {
		if (endmask)
		{
		    Expand(,
			   *p = MROP_MASK(srcpix, *p, endmask);)
		}
		else
		{
		    Expand(,
			   )
		}
	    }
	}
        pBox++;
    }
}
