/*
 * $XConsortium$
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "cfbrrop.h"

#if defined(__GNUC__) && defined(mc68020)
#define STUPID volatile
#define REARRANGE
#else
#define STUPID
#endif

#define OUTCODES_IMPLICIT(result, x, y) \
    if (x < bx1) \
	result |= OUT_LEFT; \
    else if (x >= bx2) \
	result |= OUT_RIGHT; \
    if (y < by1) \
	result |= OUT_ABOVE; \
    else if (y >= by2) \
	result |= OUT_BELOW;

#define OUTCODES(result, x, y, box) \
    if (x < box->x1) \
	result |= OUT_LEFT; \
    else if (x >= box->x2) \
	result |= OUT_RIGHT; \
    if (y < box->y1) \
	result |= OUT_ABOVE; \
    else if (y >= box->y2) \
	result |= OUT_BELOW;

int
#ifdef POLYSEGMENT
RROP_NAME(cfb8SegmentSS1Rect) (pDrawable, pGC, nseg, pSegInit)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nseg;
    xSegment	*pSegInit;
#else
RROP_NAME(cfb8LineSS1Rect) (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int	mode;		/* Origin or Previous */
    int	npt;		/* number of points */
    DDXPointPtr pptInit;
#endif
{
    int		    bx1, bx2, by1, by2;
    int		    x1, y1;
    int		    x2, y2;
    int		    xorg, yorg;
    char	    *addr;
    int		    nwidth;
    cfbPrivGCPtr    devPriv;
    BoxPtr	    extents;
    int		    capStyle;
    int		    oc1;
    STUPID int	    oc2;
#ifndef REARRANGE
    char	    *addrb;
    int	    	    adx, ady;
    int		    e, e1, e3, len;
    int		    stepx, stepy;
    RROP_DECLARE
#endif

#ifndef POLYSEGMENT
    DDXPointPtr	    ppt;
#else
    xSegment	    *pSeg;
#endif

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
#ifndef REARRANGE
    RROP_FETCH_GCPRIV(devPriv);
#endif
    capStyle = pGC->capStyle;
    extents = &devPriv->pCompositeClip->extents;
    bx1 = extents->x1;
    by1 = extents->y1;
    bx2 = extents->x2;
    by2 = extents->y2;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addr = (char *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	nwidth = (int)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind);
    }
    else
    {
	addr = (char *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	nwidth = (int)(((PixmapPtr)pDrawable)->devKind);
    }
    xorg = pDrawable->x;
    yorg = pDrawable->y;
#ifdef POLYSEGMENT
    pSeg = pSegInit;
    while (nseg--)
#else
    ppt = pptInit;
    x2 = ppt->x + xorg;
    y2 = ppt->y + yorg;
    oc2 = 0;
    OUTCODES_IMPLICIT (oc2, x2, y2);
    --npt;
    while (npt--)
#endif
    {
#ifdef POLYSEGMENT
	x1 = pSeg->x1 + xorg;
	y1 = pSeg->y1 + yorg;
	x2 = pSeg->x2 + xorg;
	y2 = pSeg->y2 + yorg;
	pSeg++;
	oc1 = 0;
	oc2 = 0;
	OUTCODES_IMPLICIT (oc1, x1, y1);
	OUTCODES_IMPLICIT (oc2, x2, y2);
#else
	x1 = x2;
	y1 = y2;
	oc1 = oc2;
	++ppt;
	if (mode == CoordModePrevious)
	{
	    xorg = x1;
	    yorg = y1;
	}
	x2 = ppt->x + xorg;
	y2 = ppt->y + yorg;
	oc2 = 0;
	OUTCODES_IMPLICIT (oc2, x2, y2);
#endif
	if (oc1 & oc2)
	    continue;
	if (oc1 | oc2)
	{
#ifdef POLYSEGMENT
	    return pSeg - pSegInit;
#else
	    if (mode == CoordModePrevious)
	    {
		ppt[-1].x = x1 - pDrawable->x;
		ppt[-1].y = y1 - pDrawable->y;
		ppt[0].x = x2 - pDrawable->x;
		ppt[0].y = y2 - pDrawable->y;
	    }
	    return ppt - pptInit;
#endif
	}
#ifdef REARRANGE
	{
	register int e, e1, e3, len;
	register int stepx, stepy;
	int adx, ady;
#endif

	stepx = 1;
	if ((adx = x2 - x1) < 0)
	{
	    adx = -adx;
	    stepx = -1;
	}
	stepy = nwidth;
	if ((ady = y2 - y1) <= 0)
	{
	    ady = -ady;
	    stepy = -nwidth;
	}
	if (adx <= ady)
	{
	    int	t;
	    t = adx;
	    adx = ady;
	    ady = t;

	    t = stepy;
	    stepy = stepx;
	    stepx = t;
	}
	e1 = ady << 1;
	e3 = - (adx << 1);
	e = - adx;
	len = adx;

#ifdef POLYSEGMENT
	if (capStyle == CapNotLast)
#endif
	    --len;
#ifdef REARRANGE
	{
	register char	*addrb;
	RROP_DECLARE

	RROP_FETCH_GCPRIV(devPriv);
#endif

	addrb = addr + (y1 * nwidth) + x1;

#ifndef REARRANGE
	if (!ady)
	{
#define body	{ RROP_SOLID(addrb); addrb += stepx; }
	    while (len >= 4)
	    {
		body body body body
		len -= 4;
	    }
	    switch (len)
	    {
	    case  3: body case 2: body case 1: body
	    }
#undef body
	}
	else
#endif
	{
#define body {\
	    	RROP_SOLID(addrb); \
	    	addrb += stepx; \
	    	e += e1; \
	    	if (e >= 0) \
	    	{ \
		    addrb += stepy; \
		    e += e3; \
	     	 } \
	    }

#ifdef LARGE_INSTRUCTION_CACHE
	    while (len >= 4)
	    {
	    	body body body body
	    	len -= 4;
	    }
	    switch (len)
	    {
	    case  3: body case 2: body case 1: body
	    }
#else
	    while ((len -= 2) >= 0)
	    {
	    	body body
	    }
	    if (len & 1)
	    	body;
#endif
	}
	RROP_SOLID(addrb);
#undef body

#ifdef REARRANGE
	}
	}
#endif

    }
#ifndef POLYSEGMENT
    if (capStyle != CapNotLast && !oc2 &&
	(x2 != pptInit->x || y2 != pptInit->y))
    {
#ifdef REARRANGE
	register char	*addrb;
	RROP_DECLARE

	RROP_FETCH_GCPRIV(devPriv);
#endif
	addrb = addr + (y2 * nwidth) + x2;
	RROP_SOLID (addrb);
    }
#endif
    return -1;
}

#if RROP == GXset && !defined (POLYSEGMENT)

#define round(dividend, divisor) \
( (((dividend)<<1) + (divisor)) / ((divisor)<<1) )
#define ceiling(m,n)  (((m)-1)/(n) + 1)
#define SignTimes(sign,n)   (((sign) < 0) ? -(n) : (n))

static
cfbClipPoint (oc, xp, yp, dx, dy, boxp, right)
    int	oc;
    int	*xp, *yp;
    BoxPtr  boxp;
{
    int	x, y;
    int	adx, ady, signdx, signdy;
    int	utmp;

    x = *xp;
    y = *yp;
    signdx = 1;
    if (dx < 0)
    {
	signdx = -1;
	dx = -dx;
    }
    signdy = 1;
    if (dy  < 0)
    {
	signdy = -1;
	dy = -dy;
    }
    if (oc & OUT_LEFT)
    {
      x = boxp->x1;
      utmp = abs(boxp->x1 - *xp);
      utmp *= dy;
      if(dx > dy)
      {
	y = *yp + SignTimes(signdy, round(utmp, dx));
      }
      else
      {
	utmp <<= 1;
	if (right)
	    utmp += dy;
	else
	    utmp -= dy;
	y = *yp + SignTimes(signdy, ceiling(utmp, 2*dx));
	if (right)
	    y -= signdy;
      }
    }
    else if (oc & OUT_RIGHT)
    {
      x = boxp->x2;
      utmp = abs(*xp - boxp->x2);
      utmp *= dy;
      if (dx > dy)
      {
	y = *yp + SignTimes(signdy, round(utmp, dx));
      }
      else
      {
	utmp <<= 1;
	if (right)
	    utmp += dy;
	else
	    utmp -= dy;
	y = *yp + SignTimes(signdy, ceiling(utmp, 2*dx));
	if (right)
	    y -= signdy;
      }
    }
    if (oc & OUT_ABOVE)
    {
      y = boxp->y1;
      utmp = abs(boxp->y1 - *yp);
      utmp *= dx;
      if (dx <= dy)
      {
	x = *xp + SignTimes(signdx, round(utmp, dy));
      }
      else
      {
	utmp <<= 1;
	if (right)
	    utmp += dx;
	else
	    utmp -= dx;
	x = *xp + SignTimes(signdx, ceiling(utmp, 2*dy));
	if (right)
	    x -= signdx;
      }
    }
    else if (oc & OUT_BELOW)
    {
      y = boxp->y2;
      utmp = abs(*yp - boxp->y2);
      utmp *= dx;
      if (dx <= dy)
      {
	x = *xp + SignTimes(signdx, round(utmp, dy));
      }
      else
      {
	utmp <<= 1;
	if (right)
	    utmp += dx;
	else
	    utmp -= dx;
	x = *xp + SignTimes(signdx, ceiling(utmp, 2*dy));
	if (right)
	    x -= signdx;
      }
    }
    *xp = x;
    *yp = y;
}

static
drawClippedLine (pDrawable, pGC, x1, y1, x2, y2, shorten)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		x1, y1, x2, y2;
    Bool	shorten;
{
    int		    oc1, oc2;
    int		    signdx, signdy, axis, e, e1, e2, len;
    int		    adx, ady;
    BoxPtr	    box;
    cfbPrivGCPtr    devPriv;
    unsigned long   *addrl;
    int		    nlwidth;
    
    x1 += pDrawable->x;
    y1 += pDrawable->y;
    x2 += pDrawable->x;
    y2 += pDrawable->y;
    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    box = &devPriv->pCompositeClip->extents;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrl = (unsigned long *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	addrl = (unsigned long *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDrawable)->devKind) >> 2;
    }
    signdx = 1;
    if ((adx = x2 - x1) < 0)
    {
	adx = -adx;
	signdx = -1;
    }
    signdy = 1;
    if ((ady = y2 - y1) < 0)
    {
	ady = -ady;
	signdy = -1;
    }
    axis = X_AXIS;
    if (adx <= ady)
    {
	int	t;

	t = adx;
	adx = ady;
	ady = t;

	axis = Y_AXIS;
    }
    e1 = ady << 1;
    e2 = e1 - (adx << 1);
    e = e1 - adx;
    len = adx;
    oc1 = 0;
    oc2 = 0;
    OUTCODES (oc1, x1, y1, box);
    OUTCODES (oc2, x2, y2, box);
    if (oc1 & oc2)
	return;
    if (oc2)
    {
	int xt = x2, yt = y2;
	int	dx = x2 - x1, dy = y2 - y1;
	int change;

	cfbClipPoint (oc2, &xt, &yt, dx, dy, box, TRUE);
	if (axis == Y_AXIS)
	    change = y2 - yt;
	else
	    change = x2 - xt;
	if (change < 0)
	    change = -change;
	len -= change;
    }
    if (oc1)
    {
	int	xt = x1, yt = y1;
	int	dx = x2 - x1, dy = y2 - y1;
	int	changex, changey;

	cfbClipPoint (oc1, &xt, &yt, dx, dy, box, FALSE);
	changex = x1 - xt;
	if (changex < 0)
	    changex = -changex;
	changey = y1 - yt;
	if (changey < 0)
	    changey = -changey;
	if (axis == Y_AXIS)
	{
	    len -= changey;
	    e = e + (changey * e2) + ((changex - changey) * e1);
	}
	else
	{
	    len -= changex;
	    e = e + (changex * e2) + ((changey - changex) * e1);
	}
	x1 = xt;
	y1 = yt;
    }
    if (shorten && !oc2)
	--len;
    cfbBresS (devPriv->rop, devPriv->and, devPriv->xor, addrl, nlwidth,
	      signdx, signdy, axis, x1, y1, e, e1, e2, len);
}

extern int cfb8LineSS1RectCopy(), cfb8LineSS1RectXor(), cfb8LineSS1RectGeneral(); 

void
cfb8LineSS1Rect (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		mode;
    int		npt;
    DDXPointPtr	pptInit;
{
    int	(*func)();
    int	drawn;

    switch (((cfbPrivGC *) pGC->devPrivates[cfbGCPrivateIndex].ptr)->rop)
    {
    case GXcopy:
	func = cfb8LineSS1RectCopy;
	break;
    case GXxor:
	func = cfb8LineSS1RectXor;
	break;
    default:
	func = cfb8LineSS1RectGeneral;
	break;
    }
    while (npt)
    {
	drawn = (*func) (pDrawable, pGC, mode, npt, pptInit);
	if (drawn == -1)
	    break;
	drawClippedLine (pDrawable, pGC,
			 pptInit[drawn-1].x, pptInit[drawn-1].y,
			 pptInit[drawn].x, pptInit[drawn].y,
			 drawn != npt - 1 || pGC->capStyle == CapNotLast);
	pptInit += drawn + 1;
	npt -= drawn + 1;
    }
}

extern int cfb8SegmentSS1RectCopy(), cfb8SegmentSS1RectXor(), cfb8SegmentSS1RectGeneral(); 

void
cfb8SegmentSS1Rect (pDrawable, pGC, nseg, pSegInit)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nseg;
    xSegment	    *pSegInit;
{
    int	(*func)();
    int	drawn;

    switch (((cfbPrivGC *) pGC->devPrivates[cfbGCPrivateIndex].ptr)->rop)
    {
    case GXcopy:
	func = cfb8SegmentSS1RectCopy;
	break;
    case GXxor:
	func = cfb8SegmentSS1RectXor;
	break;
    default:
	func = cfb8SegmentSS1RectGeneral;
	break;
    }
    while (nseg)
    {
	drawn = (*func) (pDrawable, pGC, nseg, pSegInit);
	if (drawn == -1)
	    break;
	drawClippedLine (pDrawable, pGC,
			 pSegInit[drawn-1].x1, pSegInit[drawn-1].y1,
			 pSegInit[drawn-1].x2, pSegInit[drawn-1].y2,
			 pGC->capStyle == CapNotLast);
	pSegInit += drawn;
	nseg -= drawn;
    }
}

#endif
