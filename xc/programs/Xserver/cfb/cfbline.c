/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: mfbline.c,v 5.3 89/08/15 14:57:32 rws Exp $ */
#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"

#include "cfb.h"
#include "cfbmskbits.h"

/* single-pixel lines on a color frame buffer

   NON-SLOPED LINES
   horizontal lines are always drawn left to right; we have to
move the endpoints right by one after they're swapped.
   horizontal lines will be confined to a single band of a
region.  the code finds that band (giving up if the lower
bound of the band is above the line we're drawing); then it
finds the first box in that band that contains part of the
line.  we clip the line to subsequent boxes in that band.
   vertical lines are always drawn top to bottom (y-increasing.)
this requires adding one to the y-coordinate of each endpoint
after swapping.

   SLOPED LINES
   when clipping a sloped line, we bring the second point inside
the clipping box, rather than one beyond it, and then add 1 to
the length of the line before drawing it.  this lets us use
the same box for finding the outcodes for both endpoints.  since
the equation for clipping the second endpoint to an edge gives us
1 beyond the edge, we then have to move the point towards the
first point by one step on the major axis.
   eventually, there will be a diagram here to explain what's going
on.  the method uses Cohen-Sutherland outcodes to determine
outsideness, and a method similar to Pike's layers for doing the
actual clipping.

   DIVISION
   When clipping the lines, we want to round the answer, rather
than truncating.  We want to avoid floating point; we also
want to avoid the special code required when the dividend
and divisor have different signs.

    we work a little to make all the numbers in the division
positive.  we then use the signs of the major and minor axes
decide whether to add or subtract.  this takes the special-case 
code out of the rounding division (making it easier for a 
compiler or inline to do something clever).

   CEILING
   someties, we want the ceiling.  ceil(m/n) == floor((m+n-1)/n),
for n > 0.  in C, integer division results in floor.]

   MULTIPLICATION
   when multiplying by signdx or signdy, we KNOW that it will
be a multiplication by 1 or -1, but most compilers can't
figure this out.  if your compiler/hardware combination
does better at the ?: operator and 'move negated' instructions
that it does at multiplication, you should consider using
the alternate macros.

   OPTIMIZATION
   there has been no attempt to optimize this code.  there
are obviously many special cases, at the cost of increased
code space.  a few inline procedures (e.g. round, SignTimes,
ceiling, abs) would be very useful, since the macro expansions
are not very intelligent.
*/

/* NOTE
   maybe OUTCODES should take box (the one that includes all
edges) instead of pbox (the standard no-right-or-lower-edge one)?
*/
#define OUTCODES(result, x, y, pbox) \
    if (x < pbox->x1) \
	result |= OUT_LEFT; \
    if (y < pbox->y1) \
	result |= OUT_ABOVE; \
    if (x >= pbox->x2) \
	result |= OUT_RIGHT; \
    if (y >= pbox->y2) \
	result |= OUT_BELOW;

#define round(dividend, divisor) \
( (((dividend)<<1) + (divisor)) / ((divisor)<<1) )

#ifndef PURDUE
#define ceiling(m,n) ( ((m) + (n) -1)/(n) )
#else
#define ceiling(m,n)  (((m)-1)/(n) + 1)
#endif  /* PURDUE */

/*
#define SignTimes(sign, n) ((sign) * ((int)(n)))
*/

#define SignTimes(sign, n) \
    ( ((sign)<0) ? -(n) : (n) )

#ifndef PURDUE
#define SWAPPT(p1, p2, pttmp) \
pttmp = p1; \
p1 = p2; \
p2 = pttmp;

#define SWAPINT(i, j, t) \
t = i; \
i = j; \
j = t;

#else
#define SWAPINT(i, j) \
{  register int _t = i; \
   i = j; \
   j = _t; \
}

#define SWAPPT(i, j) \
{  register DDXPointRec _t; \
   _t = i; \
   i = j; \
   j = _t; \
}
#endif  /* PURDUE */
   

void
cfbLineSS(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr pGC;
    int mode;		/* Origin or Previous */
    int npt;		/* number of points */
    DDXPointPtr pptInit;
{
    int nboxInit;
    register int nbox;
    BoxPtr pboxInit;
    register BoxPtr pbox;
    int nptTmp;
    DDXPointPtr ppt;		/* pointer to list of translated points */

    DDXPointRec pt1;
    DDXPointRec pt2;

    unsigned int oc1;		/* outcode of point 1 */
    unsigned int oc2;		/* outcode of point 2 */

    int *addrl;		/* address of destination pixmap */
    int nlwidth;		/* width in longwords of destination pixmap */
    int xorg, yorg;		/* origin of window */

    int adx;		/* abs values of dx and dy */
    int ady;
    int signdx;		/* sign of dx and dy */
    int signdy;
    int e, e1, e2;		/* bresenham error and increments */
    int len;			/* length of segment */
    int axis;			/* major axis */

    int clipDone;		/* flag for clipping loop */
    DDXPointRec pt1Orig;	/* unclipped start point */
    DDXPointRec pt2Orig;	/* unclipped end point */
    int err;			/* modified bresenham error term */
    int clip1, clip2;		/* clippedness of the endpoints */

    int clipdx, clipdy;		/* difference between clipped and
				   unclipped start point */

				/* a bunch of temporaries */
    int tmp;
    int x1, x2, y1, y2;
    RegionPtr cclip;
    unsigned long   pixel;
    int		    alu;

    cclip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
    pboxInit = REGION_RECTS(cclip);
    nboxInit = REGION_NUM_RECTS(cclip);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrl = (int *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	addrl = (int *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDrawable)->devKind) >> 2;
    }

    /* translate the point list */
    ppt = pptInit;
    nptTmp = npt;
    if (mode == CoordModeOrigin)
    {
	while(nptTmp--)
	{
	    ppt->x += xorg;
	    ppt++->y += yorg;
	}
    }
    else
    {
	ppt->x += xorg;
	ppt->y += yorg;
	nptTmp--;
	while(nptTmp--)
	{
	    ppt++;
	    ppt->x += (ppt-1)->x;
	    ppt->y += (ppt-1)->y;
	}
    }

    alu = pGC->alu;
    pixel = pGC->fgPixel;
    if (alu == GXinvert)
    {
	pixel = pGC->planemask;
	alu = GXxor;
    }
    ppt = pptInit;
    while(--npt)
    {
	nbox = nboxInit;
	pbox = pboxInit;

	pt1 = *ppt++;
	pt2 = *ppt;

	if (pt1.x == pt2.x)
	{
	    /* make the line go top to bottom of screen, keeping
	       endpoint semantics
	    */
	    if (pt1.y > pt2.y)
	    {
#ifdef PURDUE
		register int tmp;
#endif
		tmp = pt2.y;
		pt2.y = pt1.y + 1;
		pt1.y = tmp + 1;
	    }

	    /* get to first band that might contain part of line */
	    while ((nbox) && (pbox->y2 <= pt1.y))
	    {
		pbox++;
		nbox--;
	    }

	    if (nbox)
	    {
		/* stop when lower edge of box is beyond end of line */
		while((nbox) && (pt2.y >= pbox->y1))
		{
		    if ((pt1.x >= pbox->x1) && (pt1.x < pbox->x2))
		    {
			/* this box has part of the line in it */
			y1 = max(pt1.y, pbox->y1);
			y2 = min(pt2.y, pbox->y2);
			if (y1 != y2)
			{
			    cfbVertS (alu, pixel,
				      addrl, nlwidth, 
				      pt1.x, y1, y2-y1);
			}
		    }
		    nbox--;
		    pbox++;
		}
	    }

	}
	else if (pt1.y == pt2.y)
	{
	    /* force line from left to right, keeping
	       endpoint semantics
	    */
	    if (pt1.x > pt2.x)
	    {
#ifdef PURDUE
		register int tmp;
#endif
		tmp = pt2.x;
		pt2.x = pt1.x + 1;
		pt1.x = tmp + 1;
	    }

	    /* find the correct band */
	    while( (nbox) && (pbox->y2 <= pt1.y))
	    {
		pbox++;
		nbox--;
	    }

	    /* try to draw the line, if we haven't gone beyond it */
	    if ((nbox) && (pbox->y1 <= pt1.y))
	    {
		/* when we leave this band, we're done */
		tmp = pbox->y1;
		while((nbox) && (pbox->y1 == tmp))
		{
		    if (pbox->x2 <= pt1.x)
		    {
			/* skip boxes until one might contain start point */
			nbox--;
			pbox++;
			continue;
		    }

		    /* stop if left of box is beyond right of line */
		    if (pbox->x1 >= pt2.x)
		    {
			nbox = 0;
			break;
		    }

		    x1 = max(pt1.x, pbox->x1);
		    x2 = min(pt2.x, pbox->x2);
		    if (x1 != x2)
		    {
			cfbHorzS (alu, pixel,
				  addrl, nlwidth, 
				  x1, pt1.y, x2-x1);
		    }
		    nbox--;
		    pbox++;
		}
	    }
	}
	else	/* sloped line */
	{

	    adx = pt2.x - pt1.x;
	    ady = pt2.y - pt1.y;
	    signdx = sign(adx);
	    signdy = sign(ady);
	    adx = abs(adx);
	    ady = abs(ady);

	    if (adx > ady)
	    {
		axis = X_AXIS;
		e1 = ady*2;
		e2 = e1 - 2*adx;
		e = e1 - adx;

	    }
	    else
	    {
		axis = Y_AXIS;
		e1 = adx*2;
		e2 = e1 - 2*ady;
		e = e1 - ady;
	    }

	    /* we have bresenham parameters and two points.
	       all we have to do now is clip and draw.
	    */

	    pt1Orig = pt1;
	    pt2Orig = pt2;

	    while(nbox--)
	    {

		BoxRec box;

		pt1 = pt1Orig;
		pt2 = pt2Orig;
		clipDone = 0;
		box.x1 = pbox->x1;
		box.y1 = pbox->y1;
		box.x2 = pbox->x2-1;
		box.y2 = pbox->y2-1;
		clip1 = 0;
		clip2 = 0;

		oc1 = 0;
		oc2 = 0;
		OUTCODES(oc1, pt1.x, pt1.y, pbox);
		OUTCODES(oc2, pt2.x, pt2.y, pbox);

		/*
		 * let the mfb helper routine do our work;
		 * better than duplicating code...
		 */
		if (oc1 & oc2)
		    clipDone = -1;
		else if ((oc1 | oc2) == 0)
		    clipDone = 1;
		else /* have to clip */
		    clipDone = mfbClipLine(pbox, box,
					   &pt1Orig, &pt1, &pt2, 
					   adx, ady, signdx, signdy, axis,
					   &clip1, &clip2);

		if (clipDone == -1)
		{
		    pbox++;
		}
		else
		{

		    if (axis == X_AXIS)
			len = abs(pt2.x - pt1.x);
		    else
			len = abs(pt2.y - pt1.y);

		    len += (clip2 != 0);
		    if (len)
		    {
			/* unwind bresenham error term to first point */
			if (clip1)
			{
			    clipdx = abs(pt1.x - pt1Orig.x);
			    clipdy = abs(pt1.y - pt1Orig.y);
			    if (axis == X_AXIS)
				err = e+((clipdy*e2) + ((clipdx-clipdy)*e1));
			    else
				err = e+((clipdx*e2) + ((clipdy-clipdx)*e1));
			}
			else
			    err = e;
			cfbBresS   
				 (alu, pixel,
				  addrl, nlwidth,
				  signdx, signdy, axis, pt1.x, pt1.y,
				  err, e1, e2, len);
		    }

		    /* if segment is unclipped, skip remaining rectangles */
		    if (!(clip1 || clip2))
			break;
		    else
			pbox++;
		}
	    } /* while (nbox--) */
	} /* sloped line */
    } /* while (nline--) */

    /* paint the last point if the end style isn't CapNotLast.
       (Assume that a projecting, butt, or round cap that is one
        pixel wide is the same as the single pixel of the endpoint.)
    */

    if ((pGC->capStyle != CapNotLast) &&
	((ppt->x != pptInit->x) ||
	 (ppt->y != pptInit->y) ||
	 (ppt == pptInit + 1)))
    {
	pt1 = *ppt;

	nbox = nboxInit;
	pbox = pboxInit;
	while (nbox--)
	{
	    if ((pt1.x >= pbox->x1) &&
		(pt1.y >= pbox->y1) &&
		(pt1.x <  pbox->x2) &&
		(pt1.y <  pbox->y2))
	    {
		unsigned long mask;
		unsigned long scrbits;
		unsigned long pix = PFILL (pixel);
		extern int cfbmask[4];

		mask = cfbmask[pt1.x & PIM];
		addrl += (pt1.y * nlwidth) + (pt1.x >> PWSH);
		scrbits = *addrl;
		*addrl = (scrbits & ~mask) |
			 (DoRop (alu, pix, scrbits) & mask);
		break;
	    }
	    else
		pbox++;
	}
    }
}


/*
    this code does not pretend to be efficient, but it does recycle a
lot of the line code and use the miDashLine() code too.  a better
implementation is to use the solid line code to clip and
translate, and then call cfbBresD(), to do the dashes as the
line is drawn.  a Bres() procedure entry in the devPrivate
part of the GC would make this easy to do, as well as possibly speeding
up solid lines to (by avoiding the test of rop for each segment.)

    the error term at the start of each dash is computed for us by
miDashLine.  if the segment we draw is not clipped, we can use this
error term; if the first point of the dash is clipped, we have to
calculate a new error term based on e at the first point of the line.
*/

void
cfbDashLine( pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr pGC;
    int mode;		/* Origin or Previous */
    int npt;		/* number of points */
    DDXPointPtr pptInit;
{
    int nseg;			/* number of dashed segments */
    miDashPtr pdash;		/* list of dashes */
    miDashPtr pdashInit;

    int nboxInit;
    int nbox;
    BoxPtr pboxInit;
    BoxPtr pbox;
    int nptTmp;
    DDXPointPtr ppt;		/* pointer to list of translated points */

    DDXPointRec pt1;
    DDXPointRec pt2;

    unsigned int oc1;		/* outcode of point 1 */
    unsigned int oc2;		/* outcode of point 2 */

    int *addrl;			/* address of destination pixmap */
    int nlwidth;		/* width in longwords of destination pixmap */
    int xorg, yorg;		/* origin of window */
    unsigned long pixel;	/* set to color for this dash */
				/* these are all per original line */
    int adx;			/* abs values of dx and dy */
    int ady;
    int signdx;			/* sign of dx and dy */
    int signdy;
    int e;			/* error term for first point of
				   original line */
    int e1, e2;			/* i wonder what these are? */


				/* these are all per dash */
    int err;			/* bres error term for first drawn point */
    int len;			/* length of segment */
    int axis;			/* major axis */

    int clipDone;		/* flag for clipping loop */
    DDXPointRec pt1Orig;	/* unclipped start point */
    int clip1, clip2;		/* clippedness of the endpoints */

    int clipdx, clipdy;		/* difference between clipped and
				   unclipped start point */
    RegionPtr cclip;

    cclip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
    pboxInit = REGION_RECTS(cclip);
    nboxInit = REGION_NUM_RECTS(cclip);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrl = (int *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	addrl = (int *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDrawable)->devKind) >> 2;
    }

    /* translate the point list */
    ppt = pptInit;
    nptTmp = npt;
    if (mode == CoordModeOrigin)
    {
	while(nptTmp--)
	{
	    ppt->x += xorg;
	    ppt++->y += yorg;
	}
    }
    else
    {
	ppt->x += xorg;
	ppt->y += yorg;
	nptTmp--;
	while(nptTmp--)
	{
	    ppt++;
	    ppt->x += (ppt-1)->x;
	    ppt->y += (ppt-1)->y;
	}
    }

    pdash = miDashLine(npt, pptInit, 
		       pGC->numInDashList, pGC->dash, pGC->dashOffset,
		       &nseg);
    pdashInit = pdash;

    pixel = pGC->fgPixel;

    for (; --nseg >= 0; pdash++)
    {
	if (pdash->newLine)
	{
	    pt1 = *pptInit++;
	    pt1Orig = pt1;
	    pt2 = *pptInit;
	    adx = pt2.x - pt1.x;
	    ady = pt2.y - pt1.y;
	    signdx = sign(adx);
	    signdy = sign(ady);
	    adx = abs(adx);
	    ady = abs(ady);
	    e = pdash->e;
	    e1 = pdash->e1;
	    e2 = pdash->e2;
	    if (adx > ady)
		axis = X_AXIS;
	    else
		axis = Y_AXIS;
	}
	if (pGC->lineStyle == LineOnOffDash)
	{
	    if (pdash->which == ODD_DASH)
		continue;
	}
	else if (pGC->lineStyle == LineDoubleDash)
	{
	    /* use a different color for odd dashes */
	    if (pdash->which == EVEN_DASH)
		pixel = pGC->fgPixel;
	    else
		pixel = pGC->bgPixel;
	}

	nbox = nboxInit;
	pbox = pboxInit;
	while(nbox--)
	{
	    BoxRec box;

	    clipDone = 0;
	    pt1 = pdash->pt;
	    pt2 = (pdash+1)->pt;
	    box.x1 = pbox->x1;
	    box.y1 = pbox->y1;
	    box.x2 = pbox->x2-1;
	    box.y2 = pbox->y2-1;
	    clip1 = 0;
	    clip2 = 0;

	    oc1 = 0;
	    oc2 = 0;
	    OUTCODES(oc1, pt1.x, pt1.y, pbox);
	    OUTCODES(oc2, pt2.x, pt2.y, pbox);

	    if (oc1 & oc2)
		clipDone = -1;
	    else if ((oc1 | oc2) == 0)
		clipDone = 1;
	    else /* have to clip */
		clipDone = mfbClipLine(pbox, box,
				       &pt1Orig, &pt1, &pt2, 
				       adx, ady, signdx, signdy, axis,
				       &clip1, &clip2);

	    if (clipDone == -1)
	    {
		    pbox++;
	    }
	    else
	    {
		if (axis == X_AXIS)
		    len = abs(pt2.x - pt1.x);
		else
		    len = abs(pt2.y - pt1.y);

		len += (clip2 != 0);
		if (len)
		{
		    if (clip1)
		    {
			/* unwind bres error term to first visible point */
			clipdx = abs(pt1.x - pt1Orig.x);
			clipdy = abs(pt1.y - pt1Orig.y);
			if (axis == X_AXIS)
			    err = e+((clipdy*e2) + ((clipdx-clipdy)*e1));
			else
			    err = e+((clipdx*e2) + ((clipdy-clipdx)*e1));
		    }
		    else
		    {
			/* use error term calculated with the dash */
			err = pdash->e;
		    }

		    cfbBresS
			( pGC->alu, pixel,
			      addrl, nlwidth,
			      signdx, signdy, axis, pt1.x, pt1.y,
			      err, e1, e2, len);
		}

		/* if segment is unclipped, skip remaining rectangles */
		if (!(clip1 || clip2))
			break;
		else
			pbox++;
	    }
	} /* while (nbox--) */
    } /* for */

    xfree(pdashInit);
}
