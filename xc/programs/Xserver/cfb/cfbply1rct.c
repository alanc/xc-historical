/*
 * $XConsortium: cfbply1rct.c,v 1.1 91/03/11 14:58:09 keith Exp $
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

#if IMAGE_BYTE_ORDER == MSBFirst
#define intToCoord(i,x,y)   (((x) = ((i) >> 16)), ((y) = ((i) & 0xFFFF)))
#define coordToInt(x,y)	(((x) << 16) | (y))
#define intToX(i)	((i) >> 16)
#define intToY(i)	((i) & 0xFFFF)
#else
#define intToCoord(i,x,y)   (((x) = ((i) & 0xFFFF)), ((y) = ((i) >> 16)))
#define coordToInt(x,y)	(((y) << 16) | (x))
#define intToX(i)	((i) & 0xFFFF)
#define intToY(i)	((i) >> 16)
#endif

void
RROP_NAME(cfbFillPoly1Rect) (pDrawable, pGC, shape, mode, count, ptsIn)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		count;
    DDXPointPtr	ptsIn;
{
    cfbPrivGCPtr    devPriv;
    int		    nwidth;
    unsigned long   *addrl, *addr;
    int		    maxy;
    int		    origin;
    register int    vertex1, vertex2;
    int		    c;
    BoxPtr	    extents;
    int		    clip;
    int		    y;
    int		    *vertex1p, *vertex2p;
    int		    *endp;
    int		    x1, x2;
    int		    dx1, dx2;
    int		    dy1, dy2;
    int		    e1, e2;
    int		    step1, step2;
    int		    sign1, sign2;
    int		    h;
    int		    l, r;
    unsigned long   mask, bits = ~0;
    int		    nmiddle;
    RROP_DECLARE

    if (mode == CoordModePrevious || shape != Convex)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, count, ptsIn);
	return;
    }
    cfbGetLongWidthAndPointer(pDrawable, nwidth, addrl);
    nwidth <<= 2;
    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    origin = *((int *) &pDrawable->x);
    extents = &devPriv->pCompositeClip->extents;
    RROP_FETCH_GCPRIV(devPriv);
    vertex1 = *((int *) &extents->x1) - origin;
    vertex2 = *((int *) &extents->x2) - origin - 0x00010001;
    clip = 0;
    y = 32767;
    maxy = 0;
    vertex2p = (int *) ptsIn;
    endp = vertex2p + count;
    while (count--)
    {
	c = *vertex2p;
	clip |= (c - vertex1) | (vertex2 - c);
	c = intToY(c);
	if (c < y) 
	{
	    y = c;
	    vertex1p = vertex2p;
	}
	vertex2p++;
	if (c > maxy)
	    maxy = c;
    }
    if (clip & 0x80008000)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, vertex2p - (int *) ptsIn, ptsIn);
	return;
    }

#define AddrYPlus(a,y)  (unsigned long *) (((char *) (a)) + (y) * nwidth)

    addrl = AddrYPlus(addrl,y + intToY(origin));
    origin = intToX(origin);
    vertex2p = vertex1p;
    vertex2 = vertex1 = *vertex2p++;
    if (vertex2p == endp)
	vertex2p = (int *) ptsIn;
#define Setup(c,x,vertex,dx,dy,e,sign,step) {\
    x = intToX(vertex); \
    if (dy = intToY(c) - y) { \
    	dx = intToX(c) - x; \
    	e = -dy; \
	step = 0; \
    	if (dx >= 0) \
    	{ \
	    sign = 1; \
	    if (dx >= dy) {\
	    	step = dx / dy; \
	    	dx = dx % dy; \
	    } \
    	} \
    	else \
    	{ \
	    sign = -1; \
	    dx = -dx; \
	    if (dx >= dy) { \
		step = - (dx / dy); \
		dx = dx % dy; \
	    } \
    	} \
    } \
    x += origin; \
    vertex = c; \
}

#define Step(x,dx,dy,e,sign,step) {\
    x += step; \
    if ((e += dx) > 0) \
    { \
	x += sign; \
	e -= dy; \
    } \
}
    while (y != maxy)
    {
	while (y == intToY(vertex1))
	{
	    if (vertex1p == (int *) ptsIn)
		vertex1p = endp;
	    c = *--vertex1p;
	    Setup (c,x1,vertex1,dx1,dy1,e1,sign1,step1)
	}
	while (y == intToY(vertex2))
	{
	    c = *vertex2p++;
	    if (vertex2p == endp)
		vertex2p = (int *) ptsIn;
	    Setup (c,x2,vertex2,dx2,dy2,e2,sign2,step2)
	}
	/* fill spans for this segment */
	h = dy1;
	if (dy2 < dy1)
	    h = dy2;
	y += h;
	for (;;)
	{
    	    if ((nmiddle = (r = x2) - (l = x1)) < 0) {
	    	nmiddle = -nmiddle;
	    	l = x2;
	    	r = x1;
    	    }
    	    c = l & PIM;
    	    addr = (unsigned long *) (((char *) addrl) + (l - c));
    	    if (c + nmiddle < PPW)
    	    {
	    	mask = SCRRIGHT (bits,c) ^ SCRRIGHT (bits,c+nmiddle);
	    	RROP_SOLID_MASK(addr,mask);
    	    }
    	    else
    	    {
	    	if (c)
	    	{
	    	    mask = SCRRIGHT(bits, c);
	    	    RROP_SOLID_MASK(addr,mask);
	    	    nmiddle -= (PPW - c);
	    	    addr++;
	    	}
	    	nmiddle >>= PWSH;
	    	RROP_SPAN(addr,nmiddle);
	    	if (mask = ~SCRRIGHT(bits, r & PIM))
	    	    RROP_SOLID_MASK(addr,mask);
    	    }
	    addrl = AddrYPlus (addrl, 1);
	    if (!--h)
		break;
	    Step(x1,dx1,dy1,e1,sign1,step1)
	    Step(x2,dx2,dy2,e2,sign2,step2)
	}
    }
}
