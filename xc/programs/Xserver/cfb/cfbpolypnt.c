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

/* $XConsortium: cfbpolypnt.c,v 5.3 89/08/29 15:25:17 keith Exp $ */

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
    unsigned long planemask = PFILL(pGC->planemask);
    register int *addr;
    unsigned long mask;
    int	x1, x2, y1, y2;

    if (!planemask)
	return;
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
    if ((rop == GXcopy) && ((planemask & PMSK) == PMSK))
    {
	addr = addrl;
	if (!(nlwidth & (nlwidth - 1)))
	{
	    nlwidth = ffs(nlwidth) - 1;
	    for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip);
		 --nbox >= 0;
		 pbox++)
	    {
		for (ppt = pptInit, i = npt; --i >= 0; ppt++)
		{
		    x = ppt->x + pDrawable->x;
		    y = ppt->y + pDrawable->y;
		    if ((x >= pbox->x1) && (x < pbox->x2) &&
			(y >= pbox->y1) && (y < pbox->y2))
		    {
			*((char *)(addr) + (y << nlwidth) + x) = pixel;
		    }
		}
	    }
	}
	else
	{
	    for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip);
		 --nbox >= 0;
		 pbox++)
	    {
		for (ppt = pptInit, i = npt; --i >= 0; ppt++)
		{
		    x = ppt->x + pDrawable->x;
		    y = ppt->y + pDrawable->y;
		    if ((x >= pbox->x1) && (x < pbox->x2) &&
			(y >= pbox->y1) && (y < pbox->y2))
		    {
			*((char *)(addr) + (y * nlwidth) + x) = pixel;
		    }
		}
	    }
	}
	return;
    }
#endif
    nlwidth >>= 2;
    pixel = PFILL(pixel);
    for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip);
	 --nbox >= 0;
	 pbox++)
    {
	for (ppt = pptInit, i = npt; --i >= 0; ppt++)
	{
	    x = ppt->x + pDrawable->x;
	    y = ppt->y + pDrawable->y;
	    if ((x >= pbox->x1) && (x < pbox->x2) &&
		(y >= pbox->y1) && (y < pbox->y2))
	    {
		addr = addrl + (y * nlwidth) + (x >> PWSH);
		mask = cfbmask[x & PIM] & planemask;
		*addr = (*addr & ~mask) | (DoRop(rop, pixel, *addr) & mask);
	    }
	}
    }
}
