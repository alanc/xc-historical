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

/* $XConsortium: cfbzerarc.c,v 5.1 89/09/04 11:07:19 rws Exp $ */

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
cfbZeroCircleSS8Copy(pDraw, pGC, arc)
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
    if ((mask == 0xff) && (info.startx < 0))
	for (x = info.x, xoffset = nlwidth * x;
	     x <= y;
	     x++, xoffset += nlwidth)
	{
	    *(yorgob - xoffset + info.xorg + y) = pixel;
	    *(yorgob - yoffset + info.xorgo - x) = pixel;
	    *(yorgb + xoffset + info.xorgo - y) = pixel;
	    *(yorgb + yoffset + info.xorg + x) = pixel;
	    if (x == y)
		break;
	    if (x)
	    {
		*(yorgob - yoffset + info.xorg + x) = pixel;
		*(yorgob - xoffset + info.xorgo - y) = pixel;
		*(yorgb + yoffset + info.xorgo - x) = pixel;
		*(yorgb + xoffset + info.xorg + y) = pixel;
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
    else
	for (x = info.x, xoffset = nlwidth * x;
	     x <= y;
	     x++, xoffset += nlwidth)
	{
	    if (x == info.startx)
		mask = info.startMask;
	    if (mask & 1)
		*(yorgob - xoffset + info.xorg + y) = pixel;
	    if (mask & 4)
		*(yorgob - yoffset + info.xorgo - x) = pixel;
	    if (mask & 16)
		*(yorgb + xoffset + info.xorgo - y) = pixel;
	    if (mask & 64)
		*(yorgb + yoffset + info.xorg + x) = pixel;
	    if (x == y)
		break;
	    if (x)
	    {
		if (mask & 2)
		    *(yorgob - yoffset + info.xorg + x) = pixel;
		if (mask & 8)
		    *(yorgob - xoffset + info.xorgo - y) = pixel;
		if (mask & 32)
		    *(yorgb + yoffset + info.xorgo - x) = pixel;
		if (mask & 128)
		    *(yorgb + xoffset + info.xorg + y) = pixel;
	    }
	    if (x == info.endx)
		mask = info.endMask;
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
#endif

#define DoPix(bit, x, y) \
if (mask & bit) \
{ \
    addrl = x + ((y) >> PWSH); \
    pmask = cfbmask[(y) & PIM] & planemask; \
    *addrl = (*addrl & ~pmask) | (DoRop(rop, pixel, *addrl) & pmask); \
}

static void
cfbZeroCircleSS(pDraw, pGC, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
{
    register int x, y, d, dn, dp;
    register int mask;
    miZeroCircleRec info;
    register int *addrl;
    int *xorgl, *yorgl, *xorgol, *yorgol;
    unsigned long pixel = PFILL(pGC->fgPixel);
    unsigned long planemask = PFILL(pGC->planemask);
    unsigned long pmask;
    int rop = pGC->alu;
    int nlwidth, xoffset, yoffset;

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	addrl = (int *)
		(((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
		(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind);
    }
    else
    {
	addrl = (int *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDraw)->devKind);
    }
    nlwidth >>= 2;
    miZeroCircleSetup(arc, &info);
    xorgl = addrl + ((info.xorg + pDraw->y) * nlwidth);
    yorgl = addrl + ((info.yorg + pDraw->y) * nlwidth);
    xorgol = addrl + ((info.xorgo + pDraw->y) * nlwidth);
    yorgol = addrl + ((info.yorgo + pDraw->y) * nlwidth);
    info.xorg += pDraw->x;
    info.xorgo += pDraw->x;
    y = info.y;
    yoffset = nlwidth * y;
    d = info.d;
    dn = info.dn;
    dp = info.dp;
    mask = info.initialMask;
    for (x = info.x, xoffset = nlwidth * x; x <= y; x++, xoffset += nlwidth)
    {
	if (x == info.startx)
	    mask = info.startMask;
	DoPix(1, yorgol - xoffset, info.xorg + y);
	DoPix(4, yorgol - yoffset, info.xorgo - x);
	DoPix(16, yorgl + xoffset, info.xorgo - y);
	DoPix(64, yorgl + yoffset, info.xorg + x);
	if (x == y)
	    break;
	if (x)
	{
	    DoPix(2, yorgol - yoffset, info.xorg + x);
	    DoPix(8, yorgol - xoffset, info.xorgo - y);
	    DoPix(32, yorgl + yoffset, info.xorgo - x);
	    DoPix(128, yorgl + xoffset, info.xorg + y);
	}
	if (x == info.endx)
	    mask = info.endMask;
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
cfbZeroPolyArcSS(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc *arc;
    register int i;
    BoxRec box;
    RegionPtr cclip;
    void (*func)();

#if PPW == 4
    if ((pGC->alu == GXcopy) && ((pGC->planemask & PMSK) == PMSK))
	func = cfbZeroCircleSS8Copy;
    else
#endif
	func = cfbZeroCircleSS;
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
		(*func)(pDraw, pGC, arc);
	    else
		miZeroPolyArc(pDraw, pGC, 1, arc);
	}
	else
	    miPolyArc(pDraw, pGC, 1, arc);
    }
}
