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

/* $XConsortium: mfbfillarc.c,v 5.0 89/10/20 15:04:02 rws Exp $ */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "mfb.h"
#include "maskbits.h"
#include "mifillarc.h"

extern void miPolyFillArc();

static void
mfbFillEllipseSolid(pDraw, pGC, arc, rop)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
    int rop;
{
    int iscircle;
    int x, y, e, ex;
    int yk, xk, ym, xm, dx, dy, xorg, yorg;
    int slw;
    miFillArcRec info;
    int *addrlt, *addrlb;
    register int *addrl;
    register int n;
    int nlwidth;
    register int fill, xpos;
    int startmask, endmask, nlmiddle;

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	addrlt = (int *)
	       (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
	       (((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	addrlt = (int *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDraw)->devKind) >> 2;
    }
    if (rop == RROP_BLACK)
	fill = 0;
    else
	fill = ~0;
    miFillArcSetup(arc, &info);
    MIFILLARCSETUP();
    xorg += pDraw->x;
    yorg += pDraw->y;
    addrlb = addrlt;
    addrlt += nlwidth * (yorg - y);
    addrlb += nlwidth * (yorg + y + dy);
    iscircle = (arc->width == arc->height);
    while (y)
    {
	addrlt += nlwidth;
	addrlb -= nlwidth;
	if (iscircle)
	{
	    MIFILLCIRCSTEP(slw);
	}
	else
	{
	    MIFILLELLSTEP(slw);
	    if (!slw)
		continue;
	}
	xpos = xorg - x;
	addrl = addrlt + (xpos >> 5);
	if (((xpos & 0x1f) + slw) < 32)
	{
	    maskpartialbits(xpos, slw, startmask);
	    if (rop == RROP_INVERT)
		*addrl ^= startmask;
	    else
		*addrl = (*addrl & ~startmask) | (fill & startmask);
	    if (miFillArcLower(slw))
	    {
		addrl = addrlb + (xpos >> 5);
		if (rop == RROP_INVERT)
		    *addrl ^= startmask;
		else
		    *addrl = (*addrl & ~startmask) | (fill & startmask);
	    }
	    continue;
	}
	maskbits(xpos, slw, startmask, endmask, nlmiddle);
	if (startmask)
	{
	    if (rop == RROP_INVERT)
		*addrl ^= startmask;
	    else
		*addrl = (*addrl & ~startmask) | (fill & startmask);
	    addrl++;
	}
	if (rop == RROP_INVERT)
	    for (n = nlmiddle; n--; )
		*addrl++ ^= fill;
	else
	    for (n = nlmiddle; n--; )
		*addrl++ = fill;
	if (endmask)
	{
	    if (rop == RROP_INVERT)
		*addrl ^= endmask;
	    else
		*addrl = (*addrl & ~endmask) | (fill & endmask);
	}
	if (!miFillArcLower(slw))
	    continue;
	addrl = addrlb + (xpos >> 5);
	if (startmask)
	{
	    if (rop == RROP_INVERT)
		*addrl ^= startmask;
	    else
		*addrl = (*addrl & ~startmask) | (fill & startmask);
	    addrl++;
	}
	if (rop == RROP_INVERT)
	    for (n = nlmiddle; n--; )
		*addrl++ ^= fill;
	else
	    for (n = nlmiddle; n--; )
		*addrl++ = fill;
	if (endmask)
	{
	    if (rop == RROP_INVERT)
		*addrl ^= endmask;
	    else
		*addrl = (*addrl & ~endmask) | (fill & endmask);
	}
    }
}

void
mfbPolyFillArcSolid(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    mfbPrivGC *priv;
    register xArc *arc;
    register int i;
    BoxRec box;
    RegionPtr cclip;
    int rop;

    priv = (mfbPrivGC *) pGC->devPrivates[mfbGCPrivateIndex].ptr;
    rop = priv->rop;
    if ((rop == RROP_NOP) || !(pGC->planemask & 1))
	return;
    cclip = priv->pCompositeClip;
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miFillArcEmpty(arc))
	    continue;
	if (((arc->angle2 >= FULLCIRCLE) || (arc->angle2 <= -FULLCIRCLE)) &&
	    miCanFillArc(arc))
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->height + 1;
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN)
	    {
		mfbFillEllipseSolid(pDraw, pGC, arc, rop);
		continue;
	    }
	}
	miPolyFillArc(pDraw, pGC, 1, arc);
    }
}
