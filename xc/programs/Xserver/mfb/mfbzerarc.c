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

/* $XConsortium: mfbzerarc.c,v 5.6 89/09/17 16:54:09 rws Exp $ */

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

extern void miPolyArc(), miZeroPolyArc();

#if (BITMAP_BIT_ORDER == MSBFirst)
#define LEFTMOST	((unsigned int) 0x80000000)
#else
#define LEFTMOST	((unsigned int) 1)
#endif

#define PixelateWhite(addr,off) \
    (addr)[(off)>>5] |= SCRRIGHT (LEFTMOST, ((off) & 0x1f))
#define PixelateBlack(addr,off) \
    (addr)[(off)>>5] &= ~(SCRRIGHT (LEFTMOST, ((off) & 0x1f)))

#define Pixelate(base,off) \
{ \
    paddr = base + ((off)>>5); \
    pmask = SCRRIGHT(LEFTMOST, (off) & 0x1f); \
    *paddr = (*paddr & ~pmask) | (pixel & pmask); \
}

#define DoPix(bit,base,off) if (mask & bit) Pixelate(base,off);

static void
mfbZeroArcSS(pDraw, pGC, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
{
    miZeroArcRec info;
    Bool do360;
    register int x, y, a, b, d, mask;
    register int k1, k3, dx1, dy1;
    int *addrl;
    int *yorgl, *yorgol;
    unsigned long pixel;
    int nlwidth, yoffset, dyoffset;
    int pmask;
    register int *paddr;

    if (((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->rop ==
	RROP_BLACK)
	pixel = 0;
    else
	pixel = ~0L;

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	addrl = (int *)
		(((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
		(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	addrl = (int *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDraw)->devKind) >> 2;
    }
    do360 = miZeroArcSetup(arc, &info, TRUE);
    yorgl = addrl + ((info.yorg + pDraw->y) * nlwidth);
    yorgol = addrl + ((info.yorgo + pDraw->y) * nlwidth);
    info.xorg += pDraw->x;
    info.xorgo += pDraw->x;
    x = info.x;
    y = info.y;
    yoffset = 0;
    k1 = info.k1;
    k3 = info.k3;
    a = info.a;
    b = info.b;
    d = info.d;
    dx1 = info.dx1;
    dy1 = info.dy1;
    dyoffset = 0;
    mask = info.initialMask;
    if (x && !(arc->width & 1))
    {
	DoPix(2, yorgl, info.xorgo);
	DoPix(8, yorgol, info.xorgo);
    }
    if (!info.endx)
	mask = info.endMask;
    if (do360 && (arc->width == arc->height) && !(arc->width & 1))
    {
	int xoffset = nlwidth;
	int *yorghl = yorgl + (info.h * nlwidth);
	int xorghp = info.xorg + info.h;
	int xorghn = info.xorg - info.h;

	if (pixel)
	{
	    while (1)
	    {
		PixelateWhite(yorgl + yoffset, info.xorg + x);
		PixelateWhite(yorgl + yoffset, info.xorg - x);
		PixelateWhite(yorgol- yoffset, info.xorg - x);
		PixelateWhite(yorgol - yoffset, info.xorg + x);
		if (a < 0)
		    break;
		PixelateWhite(yorghl - xoffset, xorghp - y);
		PixelateWhite(yorghl - xoffset, xorghn + y);
		PixelateWhite(yorghl + xoffset, xorghn + y);
		PixelateWhite(yorghl + xoffset, xorghp - y);
		b -= k1;
		x++;
		xoffset += nlwidth;
		if (d < 0)
		{
		    a += k1;
		    d += b;
		}
		else
		{
		    y++;
		    yoffset += nlwidth;
		    a += k3;
		    d -= a;
		}
	    }
	}
	else
	{
	    while (1)
	    {
		PixelateBlack(yorgl + yoffset, info.xorg + x);
		PixelateBlack(yorgl + yoffset, info.xorg - x);
		PixelateBlack(yorgol- yoffset, info.xorg - x);
		PixelateBlack(yorgol - yoffset, info.xorg + x);
		if (a < 0)
		    break;
		PixelateBlack(yorghl - xoffset, xorghp - y);
		PixelateBlack(yorghl - xoffset, xorghn + y);
		PixelateBlack(yorghl + xoffset, xorghn + y);
		PixelateBlack(yorghl + xoffset, xorghp - y);
		b -= k1;
		x++;
		xoffset += nlwidth;
		if (d < 0)
		{
		    a += k1;
		    d += b;
		}
		else
		{
		    y++;
		    yoffset += nlwidth;
		    a += k3;
		    d -= a;
		}
	    }
	}
	x = info.w;
	yoffset = info.h * nlwidth;
    }
    else if (do360)
    {
	while (y < info.h)
	{
	    if (a < 0)
	    {
		dx1 = 0;
		dy1 = 1;
		dyoffset = nlwidth;
		k1 = info.alpha << 1;
		k3 = -k3;
		b = b + a - info.alpha;
		d = b - (a >> 1) - d + (k3 >> 3);
		a = (info.alpha - info.beta) - a;
	    }
	    Pixelate(yorgl + yoffset, info.xorg + x);
	    Pixelate(yorgl + yoffset, info.xorgo - x);
	    Pixelate(yorgol - yoffset, info.xorgo - x);
	    Pixelate(yorgol - yoffset, info.xorg + x);
	    b -= k1;
	    if (d < 0)
	    {
		x += dx1;
		y += dy1;
		yoffset += dyoffset;
		a += k1;
		d += b;
	    }
	    else
	    {
		x++;
		y++;
		yoffset += nlwidth;
		a += k3;
		d -= a;
	    }
	}
    }
    else
    {
	while (y < info.h)
	{
	    if (a < 0)
	    {
		dx1 = 0;
		dy1 = 1;
		dyoffset = nlwidth;
		k1 = info.alpha << 1;
		k3 = -k3;
		b = b + a - info.alpha;
		d = b - (a >> 1) - d + (k3 >> 3);
		a = (info.alpha - info.beta) - a;
	    }
	    if ((x == info.startx) || (y == info.starty))
		mask = info.startMask;
	    DoPix(1, yorgl + yoffset, info.xorg + x);
	    DoPix(2, yorgl + yoffset, info.xorgo - x);
	    DoPix(4, yorgol - yoffset, info.xorgo - x);
	    DoPix(8, yorgol - yoffset, info.xorg + x);
	    if ((x == info.endx) || (y == info.endy))
		mask = info.endMask;
	    b -= k1;
	    if (d < 0)
	    {
		x += dx1;
		y += dy1;
		yoffset += dyoffset;
		a += k1;
		d += b;
	    }
	    else
	    {
		x++;
		y++;
		yoffset += nlwidth;
		a += k3;
		d -= a;
	    }
	}
    }
    for (; x <= info.w; x++)
    {
	if ((x == info.startx) || (y == info.starty))
	    mask = info.startMask;
	DoPix(1, yorgl + yoffset, info.xorg + x);
	DoPix(4, yorgol - yoffset, info.xorgo - x);
	if (!arc->height || (arc->height & 1))
	{
	    DoPix(2, yorgl + yoffset, info.xorgo - x);
	    DoPix(8, yorgol - yoffset, info.xorg + x);
	}
	if (x == info.endx)
	    mask = info.endMask;
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
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN)
		mfbZeroArcSS(pDraw, pGC, arc);
	    else
		miZeroPolyArc(pDraw, pGC, 1, arc);
	}
	else
	    miPolyArc(pDraw, pGC, 1, arc);
    }
}
