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

/* $XConsortium: mfbzerarc.c,v 5.16 94/01/07 09:43:41 dpw Exp $ */

/* Derived from:
 * "Algorithm for drawing ellipses or hyperbolae with a digital plotter"
 * by M. L. V. Pitteway
 * The Computer Journal, November 1967, Volume 10, Number 3, pp. 282-289
 */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "mfb.h"
#include "maskbits.h"
#include "mizerarc.h"
#include "mi.h"

#if (BITMAP_BIT_ORDER == MSBFirst)
#define LEFTMOST	((PixelType) (1 << PLST))
#else
#define LEFTMOST	((PixelType) 1)
#endif

#define PixelateWhite(addr,yoff,xoff) \
    *mfbScanlineOffset(addr, (yoff)+((xoff)>>PWSH)) |= \
	SCRRIGHT (LEFTMOST, ((xoff) & PIM))
#define PixelateBlack(addr,yoff,xoff) \
    *mfbScanlineOffset(addr, (yoff)+((xoff)>>PWSH)) &= \
	~(SCRRIGHT (LEFTMOST, ((xoff) & PIM)))

#define Pixelate(base,yoff,xoff) \
{ \
    paddr = mfbScanlineOffset(base, (yoff) + ((xoff)>>PWSH)); \
    pmask = SCRRIGHT(LEFTMOST, (xoff) & PIM); \
    *paddr = (*paddr & ~pmask) | (pixel & pmask); \
}

#define DoPix(bit,base,yoff,xoff) if (mask & bit) Pixelate(base,yoff,xoff);

static void
mfbZeroArcSS(pDraw, pGC, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
{
    miZeroArcRec info;
    Bool do360;
    register int x, y, a, b, d, mask;
    register int k1, k3, dx, dy;
    PixelType *addrl;
    PixelType *yorgl, *yorgol;
    PixelType pixel;
    int nlwidth, yoffset, dyoffset;
    PixelType pmask;
    register PixelType *paddr;

    if (((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->rop ==
	RROP_BLACK)
	pixel = 0;
    else
	pixel = ~0;

    mfbGetPixelWidthAndPointer(pDraw, nlwidth, addrl);
    do360 = miZeroArcSetup(arc, &info, TRUE);
    yorgl = addrl + ((info.yorg + pDraw->y) * nlwidth);
    yorgol = addrl + ((info.yorgo + pDraw->y) * nlwidth);
    info.xorg += pDraw->x;
    info.xorgo += pDraw->x;
    MIARCSETUP();
    yoffset = y ? nlwidth : 0;
    dyoffset = 0;
    mask = info.initialMask;
    if (!(arc->width & 1))
    {
	DoPix(2, yorgl, 0, info.xorgo);
	DoPix(8, yorgol, 0, info.xorgo);
    }
    if (!info.end.x || !info.end.y)
    {
	mask = info.end.mask;
	info.end = info.altend;
    }
    if (do360 && (arc->width == arc->height) && !(arc->width & 1))
    {
	int xoffset = nlwidth;
	PixelType *yorghl = mfbScanlineDelta(yorgl, info.h, nlwidth);
	int xorghp = info.xorg + info.h;
	int xorghn = info.xorg - info.h;

	if (pixel)
	{
	    while (1)
	    {
		PixelateWhite(yorgl, yoffset, info.xorg + x);
		PixelateWhite(yorgl, yoffset, info.xorg - x);
		PixelateWhite(yorgol, -yoffset, info.xorg - x);
		PixelateWhite(yorgol, -yoffset, info.xorg + x);
		if (a < 0)
		    break;
		PixelateWhite(yorghl, -xoffset, xorghp - y);
		PixelateWhite(yorghl, -xoffset, xorghn + y);
		PixelateWhite(yorghl, xoffset, xorghn + y);
		PixelateWhite(yorghl, xoffset, xorghp - y);
		xoffset += nlwidth;
		MIARCCIRCLESTEP(yoffset += nlwidth;);
	    }
	}
	else
	{
	    while (1)
	    {
		PixelateBlack(yorgl, yoffset, info.xorg + x);
		PixelateBlack(yorgl, yoffset, info.xorg - x);
		PixelateBlack(yorgol, -yoffset, info.xorg - x);
		PixelateBlack(yorgol, -yoffset, info.xorg + x);
		if (a < 0)
		    break;
		PixelateBlack(yorghl, -xoffset, xorghp - y);
		PixelateBlack(yorghl, -xoffset, xorghn + y);
		PixelateBlack(yorghl, xoffset, xorghn + y);
		PixelateBlack(yorghl, xoffset, xorghp - y);
		xoffset += nlwidth;
		MIARCCIRCLESTEP(yoffset += nlwidth;);
	    }
	}
	x = info.w;
	yoffset = info.h * nlwidth;
    }
    else if (do360)
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(dyoffset = nlwidth;);
	    Pixelate(yorgl, yoffset, info.xorg + x);
	    Pixelate(yorgl, yoffset, info.xorgo - x);
	    Pixelate(yorgol, -yoffset, info.xorgo - x);
	    Pixelate(yorgol, -yoffset, info.xorg + x);
	    MIARCSTEP(yoffset += dyoffset;, yoffset += nlwidth;);
	}
    }
    else
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(dyoffset = nlwidth;);
	    if ((x == info.start.x) || (y == info.start.y))
	    {
		mask = info.start.mask;
		info.start = info.altstart;
	    }
	    DoPix(1, yorgl, yoffset, info.xorg + x);
	    DoPix(2, yorgl, yoffset, info.xorgo - x);
	    DoPix(4, yorgol, -yoffset, info.xorgo - x);
	    DoPix(8, yorgol, -yoffset, info.xorg + x);
	    if ((x == info.end.x) || (y == info.end.y))
	    {
		mask = info.end.mask;
		info.end = info.altend;
	    }
	    MIARCSTEP(yoffset += dyoffset;, yoffset += nlwidth;);
	}
    }
    if ((x == info.start.x) || (y == info.start.y))
	mask = info.start.mask;
    DoPix(1, yorgl, yoffset, info.xorg + x);
    DoPix(4, yorgol, -yoffset, info.xorgo - x);
    if (arc->height & 1)
    {
	DoPix(2, yorgl, yoffset, info.xorgo - x);
	DoPix(8, yorgol, -yoffset, info.xorg + x);
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

    if (!pGC->planemask & 1)
	return;
    cclip = ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miCanZeroArc(arc))
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->height + 1;
	    if (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN)
		mfbZeroArcSS(pDraw, pGC, arc);
	    else
		miZeroPolyArc(pDraw, pGC, 1, arc);
	}
	else
	    miPolyArc(pDraw, pGC, 1, arc);
    }
}
