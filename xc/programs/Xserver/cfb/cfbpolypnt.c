/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
no- tice appear in all copies and that both that copyright
no- tice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

********************************************************/

/* $XConsortium: cfbpolypnt.c,v 5.15 93/12/13 17:22:22 dpw Exp $ */

#include "X.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "cfb.h"
#include "cfbmskbits.h"

#define isClipped(c,ul,lr)  ((((c) - (ul)) | ((lr) - (c))) & ClipMask)

/* WARNING: pbox contains two shorts. This code assumes they are packed
 * and can be referenced together as an INT32.
 */

#define PointLoop(fill) { \
    for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip); \
	 --nbox >= 0; \
	 pbox++) \
    { \
	c1 = *((INT32 *) &pbox->x1) - off; \
	c2 = *((INT32 *) &pbox->x2) - off - 0x00010001; \
	for (ppt = (INT32 *) pptInit, i = npt; --i >= 0;) \
	{ \
	    pt = *ppt++; \
	    if (!isClipped(pt,c1,c2)) { \
		fill \
	    } \
	} \
    } \
}

void
cfbPolyPoint(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr pGC;
    int mode;
    int npt;
    xPoint *pptInit;
{
    register INT32   pt;
    register INT32   c1, c2;
    register unsigned long   ClipMask = 0x80008000;
    register unsigned long   xor;
#ifdef PIXEL_ADDR
    register PixelType   *addrp;
    register int    npwidth;
    PixelType	    *addrpt;
#else
    register unsigned long    *addrl;
    register int    nlwidth;
    register int    xoffset;
    unsigned long   *addrlt;
#endif
    register INT32  *ppt;
    RegionPtr	    cclip;
    int		    nbox;
    register int    i;
    register BoxPtr pbox;
    unsigned long   and;
    int		    rop = pGC->alu;
    int		    off;
    cfbPrivGCPtr    devPriv;
    xPoint	    *pptPrev;

    devPriv =cfbGetGCPrivate(pGC);
    rop = devPriv->rop;
    if (rop == GXnoop)
	return;
    cclip = devPriv->pCompositeClip;
    xor = devPriv->xor;
    if ((mode == CoordModePrevious) && (npt > 1))
    {
	for (pptPrev = pptInit + 1, i = npt - 1; --i >= 0; pptPrev++)
	{
	    pptPrev->x += (pptPrev-1)->x;
	    pptPrev->y += (pptPrev-1)->y;
	}
    }
    off = *((int *) &pDrawable->x);
    off -= (off & 0x8000) << 1;
#ifdef PIXEL_ADDR
    cfbGetPixelWidthAndPointer(pDrawable, npwidth, addrp);
    addrp = addrp + pDrawable->y * npwidth + pDrawable->x;
    if (rop == GXcopy)
    {
	if (!(npwidth & (npwidth - 1)))
	{
	    npwidth = ffs(npwidth) - 1;
	    PointLoop(*(addrp + (intToY(pt) << npwidth) + intToX(pt)) = xor;)
	}
#ifdef sun
	else if (npwidth == 1152)
	{
	    register int    y;
	    PointLoop(y = intToY(pt); *(addrp + (y << 10) + (y << 7) + intToX(pt)) = xor;)
	}
#endif
	else
	{
	    PointLoop(*(addrp + intToY(pt) * npwidth + intToX(pt)) = xor;)
	}
    }
    else
    {
	and = devPriv->and;
	PointLoop(  addrpt = addrp + intToY(pt) * npwidth + intToX(pt);
		    *addrpt = DoRRop (*addrpt, and, xor);)
    }
#else /* !PIXEL_ADDR */
    cfbGetLongWidthAndPointer(pDrawable, nlwidth, addrl);
    addrl = addrl + pDrawable->y * nlwidth + (pDrawable->x >> PWSH);
    xoffset = pDrawable->x & PIM;
    and = devPriv->and;
    PointLoop(   addrlt = addrl + intToY(pt) * nlwidth
 	                   + ((intToX(pt) + xoffset) >> PWSH);
 		   *addrlt = DoRRop (*addrlt,
 			   and | ~cfbmask[(intToX(pt) + xoffset) & PIM],
 			   xor & cfbmask[(intToX(pt) + xoffset) & PIM]);
	     )
#endif /* PIXEL_ADDR */
}
