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

/* $XConsortium: mfbzerarc.c,v 5.0 89/09/03 17:12:01 keith Exp $ */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "mfb.h"
#include "maskbits.h"
#include "mizerarc.h"

#if (BITMAP_BIT_ORDER == MSBFirst)
#define LEFTMOST	((unsigned int) 0x80000000)
#else
#define LEFTMOST	((unsigned int) 1)
#endif

#define PixelateBlack(addr,off) \
    (addr)[(off)>>5] |= SCRRIGHT (LEFTMOST, ((off) & 0x1f))
#define PixelateWhite(addr,off) \
    (addr)[(off)>>5] &= ~(SCRRIGHT (LEFTMOST, ((off) & 0x1f)))
#define PixelateInvert(addr,off) \
    (addr)[(off)>>5] ^= SCRRIGHT (LEFTMOST, ((off) & 0x1f))

#define PixelateAll \
    if ((info.initialMask == 0xff) && (info.startx < 0)) \
    { \
	PixelateAllCircle \
    } \
    else \
    { \
	for (x = info.x, xoffset = nlwidth * x; x <= y; x++, xoffset += nlwidth) \
	{ \
	    if (x == info.startx) \
		mask = info.startMask; \
	    if (mask & 1) \
		Pixelate (yorgol - xoffset, info.xorg + y); \
	    if (mask & 4) \
		Pixelate (yorgol - yoffset, info.xorgo - x); \
	    if (mask & 16) \
		Pixelate (yorgl + xoffset, info.xorgo - y); \
	    if (mask & 64) \
		Pixelate (yorgl + yoffset, info.xorg + x); \
	    if (x == y) \
		break; \
	    if (x) \
	    { \
		if (mask & 2) \
		    Pixelate (yorgol - yoffset, info.xorg + x); \
		if (mask & 8) \
		    Pixelate (yorgol - xoffset, info.xorgo - y); \
		if (mask & 32) \
		    Pixelate (yorgl + yoffset, info.xorgo - x); \
		if (mask & 128) \
		    Pixelate (yorgl + xoffset, info.xorg + y); \
	    } \
	    if (x == info.endx) \
		mask = info.endMask; \
	    if (d < 0) \
	    { \
		d += (x << 2) + dn; \
	    } \
	    else \
	    { \
		d += ((x - y) << 2) + dp; \
		y--; \
		yoffset -= nlwidth; \
	    } \
	} \
    }

#define PixelateAllCircle \
    for (x = info.x, xoffset = nlwidth * x; x <= y; x++, xoffset += nlwidth) \
    { \
	Pixelate (yorgol - xoffset, info.xorg + y); \
	Pixelate (yorgol - yoffset, info.xorgo - x); \
	Pixelate (yorgl + xoffset, info.xorgo - y); \
	Pixelate (yorgl + yoffset, info.xorg + x); \
    	if (x == y) \
	    break; \
	if (x) \
	{ \
	    Pixelate (yorgol - yoffset, info.xorg + x); \
	    Pixelate (yorgol - xoffset, info.xorgo - y); \
	    Pixelate (yorgl + yoffset, info.xorgo - x); \
	    Pixelate (yorgl + xoffset, info.xorg + y); \
	} \
	if (d < 0) \
	{ \
	    d += (x << 2) + dn; \
	} \
	else \
	{ \
	    d += ((x - y) << 2) + dp; \
	    y--; \
	    yoffset -= nlwidth; \
	} \
    }

static void
mfbZeroCircleSS(pDraw, pGC, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
{
    register int x, y, d, dn, dp;
    register int mask;
    miZeroCircleRec info;
    register unsigned *addrl;
    unsigned *xorgl, *yorgl, *xorgol, *yorgol;
    int rop = ((mfbPrivGC *) (pGC->devPrivates[mfbGCPrivateIndex].ptr))->rop;
    int nlwidth, xoffset, yoffset;

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	addrl = (unsigned *)
		(((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
		(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	addrl = (unsigned *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDraw)->devKind) >> 2;
    }
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

    if (rop == RROP_BLACK)
    {
#define Pixelate(addr,off) PixelateBlack(addr,off)
	PixelateAll
#undef Pixelate
    }
    else if (rop == RROP_WHITE)
    {
#define Pixelate(addr,off) PixelateWhite(addr,off)
	PixelateAll
#undef Pixelate
    }
    else
    {
#define Pixelate(addr,off) PixelateInvert(addr,off)
	PixelateAll
#undef Pixelate
    }
}

void
mfbZeroPolyArcSS(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc *arc;
    register int i;
    BoxRec box;
    RegionPtr cclip;

    cclip = ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (arc->width == arc->height)
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->width + 1;
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN)
		mfbZeroCircleSS(pDraw, pGC, arc);
	    else
		miZeroPolyArc(pDraw, pGC, 1, arc);
	}
	else
	    miPolyArc(pDraw, pGC, 1, arc);
    }
}
