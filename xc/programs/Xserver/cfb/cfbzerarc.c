/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

********************************************************/

/* $XConsortium: cfbzerarc.c,v 5.0 89/09/02 15:29:28 rws Exp $ */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "mizerarc.h"

#if PPW == 4

static void
cfbZeroCircleSS8(pDraw, pGC, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
{
    register int x, y, d, dn, dp;
    register int mask;
    miZeroCircleRec info;
    register char *addrb;
    char *xorgb, *yorgb, *xorgob, *yorgob;
    unsigned long pixel = pGC->fgPixel;
    int rop = pGC->alu;
    int nlwidth, xoffset, yoffset;

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	addrb = (char *)
		(((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
		(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind);
    }
    else
    {
	addrb = (char *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDraw)->devKind);
    }
    miZeroCircleSetup(arc, &info);
    xorgb = addrb + ((info.xorg + pDraw->y) * nlwidth);
    yorgb = addrb + ((info.yorg + pDraw->y) * nlwidth);
    xorgob = addrb + ((info.xorgo + pDraw->y) * nlwidth);
    yorgob = addrb + ((info.yorgo + pDraw->y) * nlwidth);
    info.xorg += pDraw->x;
    info.xorgo += pDraw->x;
    y = info.y;
    yoffset = nlwidth * y;
    d = info.d;
    dn = info.dn;
    dp = info.dp;
    mask = info.initialMask;
    if (rop == GXcopy)
	for (x = info.x, xoffset = nlwidth * x; x <= y; x++, xoffset += nlwidth)
	{
	    if (x == info.startx)
		mask = info.startMask;
	    else if (x == info.endx)
		mask = info.endMask;
	    if (mask & 1)
		*(yorgob - xoffset + info.xorg + y) = pixel;
	    if (mask & 4)
		*(yorgob - yoffset + info.xorgo - x) = pixel;
	    if (mask & 16)
		*(yorgb + xoffset + info.xorgo - y) = pixel;
	    if (mask & 64)
		*(yorgb + yoffset + info.xorg + x) = pixel;
	    if ((x == y) || (x == 0))
		continue;
	    if (mask & 2)
		*(yorgob - yoffset + info.xorg + x) = pixel;
	    if (mask & 8)
		*(yorgob - xoffset + info.xorgo - y) = pixel;
	    if (mask & 32)
		*(yorgb + yoffset + info.xorgo - x) = pixel;
	    if (mask & 128)
		*(yorgb + xoffset + info.xorg + y) = pixel;
	    if (d < 0)
	    {
		d += (x << 2) + dn;
	    }
	    else
	    {
		d += ((x - y) << 2) + dp;
		y--;
		yoffset -= nlwidth;
	    }
	}
    else
	for (x = info.x, xoffset = nlwidth * x; x <= y; x++, xoffset += nlwidth)
	{
	    if (x == info.startx)
		mask = info.startMask;
	    else if (x == info.endx)
		mask = info.endMask;
	    if (mask & 1)
	    {
		addrb = yorgob - xoffset + info.xorg + y;
		*addrb = DoRop(rop, pixel, *addrb);
	    }
	    if (mask & 4)
	    {
		addrb = yorgob - yoffset + info.xorgo - x;
		*addrb = DoRop(rop, pixel, *addrb);
	    }
	    if (mask & 16)
	    {
		addrb = yorgb + xoffset + info.xorgo - y;
		*addrb = DoRop(rop, pixel, *addrb);
	    }
	    if (mask & 64)
	    {
		addrb = yorgb + yoffset + info.xorg + x;
		*addrb = DoRop(rop, pixel, *addrb);
	    }
	    if ((x == y) || (x == 0))
		continue;
	    if (mask & 2)
	    {
		addrb = yorgob - yoffset + info.xorg + x;
		*addrb = DoRop(rop, pixel, *addrb);
	    }
	    if (mask & 8)
	    {
		addrb = yorgob - xoffset + info.xorgo - y;
		*addrb = DoRop(rop, pixel, *addrb);
	    }
	    if (mask & 32)
	    {
		addrb = yorgb + yoffset + info.xorgo - x;
		*addrb = DoRop(rop, pixel, *addrb);
	    }
	    if (mask & 128)
	    {
		addrb = yorgb + xoffset + info.xorg + y;
		*addrb = DoRop(rop, pixel, *addrb);
	    }
	    if (d < 0)
	    {
		d += (x << 2) + dn;
	    }
	    else
	    {
		d += ((x - y) << 2) + dp;
		y--;
		yoffset -= nlwidth;
	    }
	}
}

void
cfbZeroPolyArcSS8(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc *arc;
    register int i;
    BoxRec box;
    RegionPtr cclip;

    cclip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (arc->width == arc->height)
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->width + 1;
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN)
		cfbZeroCircleSS8(pDraw, pGC, arc);
	    else
		miZeroPolyArc(pDraw, pGC, 1, arc);
	}
	else
	    miPolyArc(pDraw, pGC, 1, arc);
    }
}

#endif
