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

/* $XConsortium: cfbpolypnt.c,v 5.2 89/08/28 14:42:59 rws Exp $ */

#include "X.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "cfb.h"
#include "cfbmskbits.h"

void
cfbPolyPoint(pDrawable, pGC, mode, npt, pptInit)
    register DrawablePtr pDrawable;
    GCPtr pGC;
    int mode;
    int npt;
    xPoint *pptInit;
{
    register int i;
    register xPoint *ppt;
    RegionPtr cclip;
    int nbox;
    register BoxPtr pbox;
    register int x, y;
    int *addrl;
    int nlwidth;
    int rop = pGC->alu;
    unsigned long pixel = pGC->fgPixel;
#if PPW == 4
    register char *addrb;
#else
    register int *addr;
    unsigned long mask;
#endif
    int	x1, x2, y1, y2;

    if ((mode == CoordModePrevious) && (npt > 1))
    {
	for (ppt = pptInit + 1, i = npt - 1; --i >= 0; ppt++)
	{
	    ppt->x += (ppt-1)->x;
	    ppt->y += (ppt-1)->y;
	}
    }
    cclip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrl = (int *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind);
    }
    else
    {
	addrl = (int *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDrawable)->devKind);
    }
#if PPW == 4
    if ((rop == GXcopy) && !(nlwidth & (nlwidth - 1)))
    {
	nlwidth = ffs(nlwidth) - 1;
	for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip);
	     --nbox >= 0;
	     pbox++)
	{
	    x1 = pbox->x1;
	    x2 = pbox->x2;
	    y1 = pbox->y1;
	    y2 = pbox->y2;
	    for (ppt = pptInit, i = npt; --i >= 0; ppt++)
	    {
		x = ppt->x + pDrawable->x;
		y = ppt->y + pDrawable->y;
		if ((x >= x1) && (x < x2) &&
		    (y >= y1) && (y < y2))
		{
		    addrb = (char *)(addrl) + (y << nlwidth) + x;
		    *addrb = pixel;
		}
	    }
	}
	return;
    }
#else
    nlwidth >>= 2;
    pixel = PFILL(pixel);
#endif
    for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip);
	 --nbox >= 0;
	 pbox++)
    {
	x1 = pbox->x1;
	x2 = pbox->x2;
	y1 = pbox->y1;
	y2 = pbox->y2;
	for (ppt = pptInit, i = npt; --i >= 0; ppt++)
	{
	    x = ppt->x + pDrawable->x;
	    y = ppt->y + pDrawable->y;
	    if ((x >= x1) && (x < x2) &&
		(y >= y1) && (y < y2))
	    {
#if PPW == 4
		addrb = (char *)(addrl) + (y * nlwidth) + x;
	    	*addrb = DoRop(rop, pixel, *addrb);
#else
		addr = addrl + (y * nlwidth) + (x >> PWSH);
		mask = cfbmask[x & PIM];
		*addr = (*addr & ~mask) | (DoRop(rop, pixel, *addr) & mask);
#endif
	    }
	}
    }
}
