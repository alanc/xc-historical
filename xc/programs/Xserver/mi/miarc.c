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
/* $XConsortium: miarc.c,v 1.56 88/10/15 08:45:17 rws Exp $ */
/* Author: Keith Packard */

#include "X.h"
#include "Xprotostr.h"
#include "misc.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "mifpoly.h"
#include "mi.h"

extern double sqrt(), cos(), sin(), atan();

/* these are from our <math.h>, but I'm told some systems don't have
 * math.h and that they're not in all versions of math.h. */

# define torad(xAngle)	(((double) (xAngle)) / 64.0 * M_PI/180.0)

#define M_PI	3.14159265358979323846
#define M_PI_2	1.57079632679489661923

#ifndef X_AXIS
# define X_AXIS 0
# define Y_AXIS 1
#endif X_AXIS

/* 360 degrees * 64 sub-degree positions */
#define FULLCIRCLE (64 * 360)

# define RIGHT_END	0
# define LEFT_END	1

typedef struct _miArcJoin {
	int	arcIndex0, arcIndex1;
	int	end0, end1;
} miArcJoinRec, *miArcJoinPtr;

typedef struct _miArcCap {
	int		arcIndex;
	int		end;		
} miArcCapRec, *miArcCapPtr;

typedef struct _miArcFace {
	SppPointRec	clock;
	SppPointRec	center;
	SppPointRec	counterClock;
} miArcFaceRec, *miArcFacePtr;

/*
 * This is an entire sequence of arcs, computed and categorized according
 * to operation.  miDashArcs generates either one or two of these.
 */

typedef struct _miArcData {
	xArc		arc;
	int		render;		/* non-zero means render after drawing */
	int		join;		/* related join */
	int		cap;		/* related cap */
	miArcFaceRec	bounds[2];
	double		x0, y0, x1, y1;
} miArcDataRec, *miArcDataPtr;

typedef struct _miPolyArc {
	int		narcs;
	miArcDataPtr	arcs;
	int		ncaps;
	miArcCapPtr	caps;
	int		njoins;
	miArcJoinPtr	joins;
} miPolyArcRec, *miPolyArcPtr;

static miPolyArcPtr miDashArcs (), miSolidArcs ();

#define GCValsFunction		0
#define GCValsForeground 	1
#define GCValsBackground 	2
#define GCValsLineWidth 	3
#define GCValsCapStyle 		4
#define GCValsJoinStyle		5
#define GCValsMask		(GCFunction | GCForeground | GCBackground | \
				 GCLineWidth | GCCapStyle | GCJoinStyle)
static CARD32 gcvals[6];

/*
 * draw one segment of the arc using the arc spans generation routines
 */

static void
miArcSegment(pDraw, pGC, tarc, right, left)
    DrawablePtr   pDraw;
    GCPtr         pGC;
    xArc          tarc;
    miArcFacePtr	right, left;
{
    int l = pGC->lineWidth;
    int	w, h;
    int a0, a1, startAngle, endAngle;
    int st, ct;
    miArcFacePtr	temp;

    /* random (necessary) paranoia checks.  specialized routines could be *
     * written for each of these cases, but who has the time?             */

    if (tarc.width == 0 || tarc.height == 0) {
	xRectangle	rect;

	rect.x = tarc.x;
	rect.y = tarc.y;
	if (tarc.width == 0) {
	    if (tarc.height == 0)
		return;
	    /* a vertical line */
	    rect.width = l;
	    rect.height = tarc.height;
	    ct = 0;
	    if (tarc.angle1 < 180 * 64)
	    	st = 1;
	    else
	    	st = -1;
	} else {
	    /* a horizontal line */
	    rect.width = tarc.width;
	    rect.height = l;
	    st = 0;
	    if (90 * 64 < tarc.angle1 && tarc.angle1 <= 270 * 64)
	    	ct = -1;
	    else
	    	ct = 1;
	}
	if (right) {
	    right->clock.y = tarc.y + tarc.height/2 + rect.height * (- ct - st);
	    right->clock.x = tarc.x + tarc.width/2 + rect.width * (ct - st);
	    right->center.y = tarc.y + tarc.height / 2 + rect.height * ( -st);
	    right->center.x = tarc.x + tarc.width/2 + rect.width * (ct);
	    right->counterClock.y = tarc.y + tarc.height/2 + rect.height * (ct - st);
	    right->counterClock.x = tarc.x + tarc.width/2 + rect.width * (ct + st);
	}
	if (left) {
	    left->counterClock.y = tarc.y + tarc.height/2 + rect.height * (- ct + st);
	    left->counterClock.x = tarc.x + tarc.width/2 + rect.width * (-ct - st);
	    left->center.y = tarc.y + tarc.height / 2 + rect.height * ( st);
	    left->center.x = tarc.x + tarc.width/2 + rect.width * (-ct);
	    left->clock.y = tarc.y + tarc.height/2 + rect.height * (ct + st);
	    left->clock.x = tarc.x + tarc.width/2 + rect.width * (-ct + st);
	}
	(*pGC->PolyFillRect) (pDraw, pGC, 1, &rect);
	return;
    }
    if (l < 1)
	l = 1;		/* for 0-width arcs */
    if (tarc.width == 0)
	tarc.width = 1;
    if (tarc.height == 0)
	tarc.height = 1;
    
    if (pGC->miTranslate && (pDraw->type == DRAWABLE_WINDOW)) {
	tarc.x += ((WindowPtr) pDraw)->absCorner.x;
	tarc.y += ((WindowPtr) pDraw)->absCorner.y;
    }

    a0 = tarc.angle1;
    a1 = tarc.angle2;
    if (a1 > FULLCIRCLE)
	a1 = FULLCIRCLE;
    else if (a1 < -FULLCIRCLE)
	a1 = -FULLCIRCLE;
    if (a1 < 0) {
    	startAngle = a0 + a1;
	endAngle = a0;
	temp = right;
	right = left;
	left = temp;
    } else {
	startAngle = a0;
	endAngle = a0 + a1;
    }
    /*
     * bounds check the two angles
     */
    if (startAngle < 0)
	startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
    if (startAngle >= FULLCIRCLE)
	startAngle = startAngle % FULLCIRCLE;
    if (endAngle < 0)
	endAngle = FULLCIRCLE - (-endAngle) % FULLCIRCLE;
    if (endAngle > FULLCIRCLE)
	endAngle = (endAngle-1) % FULLCIRCLE + 1;
    if (startAngle == endAngle) {
	startAngle = 0;
	endAngle = FULLCIRCLE;
    }

    drawArc ((int) tarc.x, (int) tarc.y,
             (int) tarc.width, (int) tarc.height, l, startAngle, endAngle,
	     right, left);
}

/*
 * miPolyArc strategy:
 *
 * If there's only 1 arc, or if the arc is draw with zero width lines, we 
 * don't have to worry about the rasterop or join styles.   
 * Otherwise, we set up pDrawTo and pGCTo according to the rasterop, then
 * draw using pGCTo and pDrawTo.  If the raster-op was "tricky," that is,
 * if it involves the destination, then we use PushPixels to move the bits
 * from the scratch drawable to pDraw. (See the wide line code for a
 * fuller explanation of this.)
 */
void
miPolyArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register int		i;
    double			xMin, xMax, yMin, yMax, yOrg, xOrg, dx, dy;
    double			helperDx, helperDy;
    int				ifirst, count, width;
    Bool			fTricky;
    DrawablePtr			pDrawTo;
    unsigned long		fg, bg;
    GCPtr			pGCTo;
    miPolyArcPtr		polyArcs;
    int				cap, join;
    int				iphase;

    width = pGC->lineWidth;
    if(width == 0 && pGC->lineStyle == LineSolid)
    {
	for(i = 0; i < narcs; i++)
	    miArcSegment( pDraw, pGC, parcs[i],
 	    (miArcFacePtr) 0, (miArcFacePtr) 0 );
	fillSpans (pDraw, pGC);
    }
    else 
    {
	/* Set up pDrawTo and pGCTo based on the rasterop */
	switch(pGC->alu)
	{
	  case GXclear:		/* 0 */
	  case GXcopy:		/* src */
	  case GXcopyInverted:	/* NOT src */
	  case GXset:		/* 1 */
	    fTricky = FALSE;
	    pDrawTo = pDraw;
	    pGCTo = pGC;
	    if (pGC->miTranslate && (pDraw->type == DRAWABLE_WINDOW)) {
		xOrg = (double) ((WindowPtr)pDraw)->absCorner.x;
		yOrg = (double) ((WindowPtr)pDraw)->absCorner.y;
	    } else {
	    	xOrg = 0;
		yOrg = 0;
	    }
	    break;
	  default:
	    fTricky = TRUE;

	    xMin = yMin = MAXSHORT;
	    xMax = yMax = MINSHORT;

	    for(i = 0; i < narcs; i++)
	    {
		xMin = min (xMin, parcs[i].x);
		yMin = min (yMin, parcs[i].y);
		xMax = max (xMax, parcs[i].x + parcs[i].width);
		yMax = max (yMax, parcs[i].y + parcs[i].height);
	    }

	    pGCTo = GetScratchGC(1, pDraw->pScreen);
	    gcvals[GCValsFunction] = GXcopy;
	    gcvals[GCValsForeground] = 1;
	    gcvals[GCValsBackground] = 0;
	    gcvals[GCValsLineWidth] = pGC->lineWidth;
	    gcvals[GCValsCapStyle] = pGC->capStyle;
	    gcvals[GCValsJoinStyle] = pGC->joinStyle;
	    DoChangeGC(pGCTo, GCValsMask, gcvals, 0);
    
    	    xOrg = xMin - ((double) width + 1)/2;
	    yOrg = yMin - ((double) width + 1)/2;
	    dx = xMax - xMin + ((double) width) + 1;
	    dy = yMax - yMin + ((double) width) + 1;
	    for(i = 0; i < narcs; i++)
	    {
		parcs[i].x -= xOrg;
		parcs[i].y -= yOrg;
	    }
	    if (pGC->miTranslate && (pDraw->type == DRAWABLE_WINDOW))
	    {
		xOrg += (double) ((WindowPtr)pDraw)->absCorner.x;
		yOrg += (double) ((WindowPtr)pDraw)->absCorner.y;
	    }

	    /* allocate a 1 bit deep pixmap of the appropriate size, and
	     * validate it */
	    pDrawTo = (DrawablePtr)(*pDraw->pScreen->CreatePixmap)
					(pDraw->pScreen, dx, dy, 1, XYBitmap);
	    ValidateGC(pDrawTo, pGCTo);
	    miClearDrawable(pDrawTo, pGCTo);
	}

	fg = pGCTo->fgPixel;
	bg = pGCTo->bgPixel;
	if (pGC->lineStyle != LineSolid)
	    polyArcs = miDashArcs(parcs, narcs, pGC->dash, pGC->numInDashList,
				  pGC->dashOffset, (pGC->lineStyle == LineDoubleDash));
	else
	    polyArcs = miSolidArcs (parcs, narcs);

	if (!polyArcs)
	{
	    if (fTricky) {
		(*pDraw->pScreen->DestroyPixmap) (pDrawTo);
		FreeScratchGC (pGCTo);
	    }
	    return;
	}

	cap = 0;
	join = 0;
	for (iphase = ((pGC->lineStyle == LineDoubleDash) ? 1 : 0);
 	     iphase >= 0;
	     iphase--)
	{
	    for (i = 0; i < polyArcs[iphase].narcs; i++) {
		miArcDataPtr	arcData;
		int		j;

		arcData = &polyArcs[iphase].arcs[i];
		miArcSegment(pDrawTo, pGCTo, arcData->arc,
			     &arcData->bounds[RIGHT_END],
			     &arcData->bounds[LEFT_END]);
		if (polyArcs[iphase].arcs[i].render) {
		    if (iphase == 1) {
			gcvals[0] = bg;
			gcvals[1] = fg;
			DoChangeGC (pGCTo, GCForeground|GCBackground, gcvals, 0);
			ValidateGC (pDrawTo, pGCTo);
		    }
		    fillSpans (pDrawTo, pGCTo);
		    while (cap < polyArcs[iphase].arcs[i].cap) {
			int	arcIndex, end;
			miArcDataPtr	arcData0;

			arcIndex = polyArcs[iphase].caps[cap].arcIndex;
			end = polyArcs[iphase].caps[cap].end;
			arcData0 = &polyArcs[iphase].arcs[arcIndex];
			miArcCap (pDrawTo, pGCTo,
 				  &arcData0->bounds[end], end,
				  - arcData0->arc.x, - arcData0->arc.y,
				  (double) arcData0->arc.width / 2.0,
 				  (double) arcData0->arc.height / 2.0);
			++cap;
		    }
		    while (join < polyArcs[iphase].arcs[i].join) {
			int	arcIndex0, arcIndex1, end0, end1;
			miArcDataPtr	arcData0, arcData1;

			arcIndex0 = polyArcs[iphase].joins[join].arcIndex0;
			end0 = polyArcs[iphase].joins[join].end0;
			arcIndex1 = polyArcs[iphase].joins[join].arcIndex1;
			end1 = polyArcs[iphase].joins[join].end1;
			arcData0 = &polyArcs[iphase].arcs[arcIndex0];
			arcData1 = &polyArcs[iphase].arcs[arcIndex1];
			miArcJoin (pDrawTo, pGCTo,
				  &arcData0->bounds[end0],
 				  &arcData1->bounds[end1],
				  - arcData0->arc.x, - arcData0->arc.y,
				  (double) arcData0->arc.width / 2.0,
 				  (double) arcData0->arc.height / 2.0,
				  - arcData1->arc.x, - arcData1->arc.y,
				  (double) arcData1->arc.width / 2.0,
 				  (double) arcData1->arc.height / 2.0);
			++join;
		    }
		    if (fTricky) {
		    	(*pGC->PushPixels) (pGC, pDrawTo, pDraw, (int) dx,
					    (int) dy, (int) xOrg, (int) yOrg);
			miClearDrawable ((DrawablePtr) pDrawTo, pGCTo);
		    }
		    if (iphase == 1) {
			gcvals[0] = fg;
			gcvals[1] = bg;
			DoChangeGC (pGCTo, GCForeground|GCBackground, gcvals, 0);
			ValidateGC (pDrawTo, pGCTo);
		    }
		}
	    }
	    Xfree((pointer) polyArcs[iphase].arcs);
	    if (polyArcs[iphase].njoins > 0) {
		Xfree ((pointer) polyArcs[iphase].joins);
	    }
	    if (polyArcs[iphase].ncaps > 0) {
		Xfree ((pointer) polyArcs[iphase].caps);
	    }
	}
	Xfree((pointer) polyArcs);

	if(fTricky)
	{
	    (*pGCTo->pScreen->DestroyPixmap)((PixmapPtr)pDrawTo);
	    FreeScratchGC(pGCTo);
	}
    }
}

static double
angleBetween (center, point1, point2)
	SppPointRec	center, point1, point2;
{
	double	atan2 (), a1, a2;
	
	a1 = atan2 (point1.x - center.x, point1.y - center.y);
	a2 = atan2 (point2.x - center.x, point2.y - center.y);
	return a2 - a1;
}

translateBounds (b, x, y, fx, fy)
miArcFacePtr	b;
int		x, y;
double		fx, fy;
{
	b->clock.x += x + fx;
	b->clock.y += y + fy;
	b->center.x += x + fx;
	b->center.y += y + fy;
	b->counterClock.x += x + fx;
	b->counterClock.y += y + fy;
}

miArcJoin (pDraw, pGC, pRight, pLeft,
	   xOrgRight, yOrgRight, xFtransRight, yFtransRight,
	   xOrgLeft, yOrgLeft, xFtransLeft, yFtransLeft)
	DrawablePtr	*pDraw;
	GCPtr		pGC;
	miArcFacePtr	pRight, pLeft;
	int		xOrgRight, yOrgRight;
	double		xFtransRight, yFtransRight;
	int		xOrgLeft, yOrgLeft;
	double		xFtransLeft, yFtransLeft;
{
	SppPointRec	center, corner, otherCorner, end;
	SppPointRec	poly[5], e;
	SppPointPtr	pArcPts;
	int		cpt;
	SppArcRec	arc;
	miArcFaceRec	Right, Left;
	int		polyLen;
	int		xOrg, yOrg;
	double		xFtrans, yFtrans;
	double		angle[4];
	double		a;
	double		ae, ac2, ec2, bc2, de;
	double		width;
	
	xOrg = (xOrgRight + xOrgLeft) / 2;
	yOrg = (yOrgRight + yOrgLeft) / 2;
	xFtrans = (xFtransLeft + xFtransRight) / 2;
	yFtrans = (yFtransLeft + yFtransRight) / 2;
	Right = *pRight;
	translateBounds (&Right, xOrg - xOrgRight, yOrg - yOrgRight,
				 xFtrans - xFtransRight, yFtrans - yFtransRight);
	Left = *pLeft;
	translateBounds (&Left, xOrg - xOrgLeft, yOrg - yOrgLeft,
				 xFtrans - xFtransLeft, yFtrans - yFtransLeft);
	pRight = &Right;
	pLeft = &Left;

	center = pRight->center;
	if (0 <= (a = angleBetween (center, pRight->clock, pLeft->counterClock))
 	    && a < M_PI)
 	{
		corner = pRight->clock;
		otherCorner = pLeft->counterClock;
	} else {
		a = angleBetween (center, pLeft->clock, pRight->counterClock);
		corner = pLeft->clock;
		otherCorner = pRight->counterClock;
	}
	switch (pGC->joinStyle) {
	case JoinRound:
		width = (pGC->lineWidth ? pGC->lineWidth : 1);

		arc.x = center.x - width/2;
		arc.y = center.y - width/2;
		arc.width = width;
		arc.height = width;
		arc.angle1 = -atan2 (corner.y - center.y, corner.x - center.x);
		arc.angle2 = a;
		pArcPts = (SppPointPtr) Xalloc (sizeof (SppPointRec));
		pArcPts->x = center.x;
		pArcPts->y = center.y;
		if( cpt = miGetArcPts(&arc, 1, &pArcPts))
		{
			/* by drawing with miFillSppPoly and setting the endpoints of the arc
			 * to be the corners, we assure that the cap will meet up with the
			 * rest of the line */
			miFillSppPoly(pDraw, pGC, cpt, pArcPts, -xOrg, -yOrg, xFtrans, yFtrans);
			Xfree((pointer)pArcPts);
		}
		return;
	case JoinMiter:
		/*
		 * don't miter arcs with less than 11 degrees between them
		 */
		if (a < 169 * M_PI / 180.0) {
			poly[0] = corner;
			poly[1] = center;
			poly[2] = otherCorner;
			bc2 = (corner.x - otherCorner.x) * (corner.x - otherCorner.x) +
			      (corner.y - otherCorner.y) * (corner.y - otherCorner.y);
			ec2 = bc2 / 4;
			ac2 = (corner.x - center.x) * (corner.x - center.x) +
			      (corner.y - center.y) * (corner.y - center.y);
			ae = sqrt (ac2 - ec2);
			de = ec2 / ae;
			e.x = (corner.x + otherCorner.x) / 2;
			e.y = (corner.y + otherCorner.y) / 2;
			poly[3].x = e.x + de * (e.x - center.x) / ae;
			poly[3].y = e.y + de * (e.y - center.y) / ae;
			poly[4] = corner;
			polyLen = 5;
			break;
		}
	case JoinBevel:
		poly[0] = corner;
		poly[1] = center;
		poly[2] = otherCorner;
		poly[3] = corner;
		polyLen = 4;
		break;
	}
	miFillSppPoly (pDraw, pGC, polyLen, poly, -xOrg, -yOrg, xFtrans, yFtrans);
}

miArcCap (pDraw, pGC, pFace, end, xOrg, yOrg, xFtrans, yFtrans)
	DrawablePtr	*pDraw;
	GCPtr		pGC;
	miArcFacePtr	pFace;
	int		end;
	int		xOrg, yOrg;
	double		xFtrans, yFtrans;
{
	SppPointRec	corner, otherCorner, center, endPoint, poly[5];

	corner = pFace->clock;
	otherCorner = pFace->counterClock;
	center = pFace->center;
	switch (pGC->capStyle) {
	case CapProjecting:
		poly[0].x = otherCorner.x;
		poly[0].y = otherCorner.y;
		poly[1].x = corner.x;
		poly[1].y = corner.y;
		poly[2].x = corner.x -
 				(center.y - corner.y);
		poly[2].y = corner.y +
 				(center.x - corner.x);
		poly[3].x = otherCorner.x -
 				(otherCorner.y - center.y);
		poly[3].y = otherCorner.y +
 				(otherCorner.x - center.x);
		poly[4].x = otherCorner.x;
		poly[4].y = otherCorner.y;
		miFillSppPoly (pDraw, pGC, 5, poly, -xOrg, -yOrg, xFtrans, yFtrans);
		break;
	case CapRound:
		/*
		 * miRoundCap just needs these to be unequal.
		 */
		endPoint = center;
		endPoint.x = endPoint.x + 100;
		miRoundCap (pDraw, pGC, center, endPoint, corner, otherCorner, 0,
			    xOrg, yOrg, xFtrans, yFtrans);
		break;
	}
}

/* MIPOLYFILLARC -- The public entry for the PolyFillArc request.
 * Since we don't have to worry about overlapping segments, we can just
 * fill each arc as it comes.  As above, we convert the arc into a set of
 * line segments and then fill the resulting polygon.
 */
void
miPolyFillArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    int	i, cpt;
    SppPointPtr pPts;
    SppArcRec	sppArc;
    int		angle1, angle2;

    for(i = 0; i < narcs; i++)
    {
	angle1 = parcs[i].angle1;
	if (angle1 >= FULLCIRCLE)
		angle1 = angle1 % FULLCIRCLE;
	else if (angle1 <= -FULLCIRCLE)
		angle1 = - (-angle1 % FULLCIRCLE);
	angle2 = parcs[i].angle2;
	if (angle2 > FULLCIRCLE)
		angle2 = FULLCIRCLE;
	else if (angle2 < -FULLCIRCLE)
		angle2 = -FULLCIRCLE;
	sppArc.x = parcs[i].x;
	sppArc.y = parcs[i].y;
	sppArc.width = parcs[i].width;
	sppArc.height = parcs[i].height;
	sppArc.angle1 = torad (angle1);
	sppArc.angle2 = torad (angle2);
	/* We do this test every time because a full circle PieSlice isn't
	 * really a slice, but a full pie, and the Chord code (below) should
	 * handle it better */
        if(pGC->arcMode == ArcPieSlice && parcs[i].angle2 < FULLCIRCLE)
	{
	    if (!(pPts = (SppPointPtr)Xalloc(sizeof(SppPointRec))))
		return;
	    if(cpt = miGetArcPts(&sppArc, 1, &pPts))
	    {
		pPts[0].x = sppArc.x + sppArc.width/2;
		pPts[0].y = sppArc.y + sppArc.height /2;
		miFillSppPoly(pDraw, pGC, cpt + 1, pPts, 0, 0, 0.0, 0.0);
		Xfree((pointer) pPts);
	    }
	}
        else /* Chord */
	{
	    pPts = (SppPointPtr)NULL;
	    if(cpt = miGetArcPts(&sppArc, 0, &pPts))
	    {
		miFillSppPoly(pDraw, pGC, cpt, pPts, 0, 0, 0.0, 0.0);
		Xfree((pointer) pPts);
	    }
	}
    }
}

#define REALLOC_STEP 10		/* how often to realloc */
/* MIGETARCPTS -- Converts an arc into a set of line segments -- a helper
 * routine for filled arc and line (round cap) code.
 * Returns the number of points in the arc.  Note that it takes a pointer
 * to a pointer to where it should put the points and an index (cpt).
 * This procedure allocates the space necessary to fit the arc points.
 * Sometimes it's convenient for those points to be at the end of an existing
 * array. (For example, if we want to leave a spare point to make sectors
 * instead of segments.)  So we pass in the Xalloc()ed chunk that contains the
 * array and an index saying where we should start stashing the points.
 * If there isn't an array already, we just pass in a null pointer and 
 * count on Xrealloc() to handle the null pointer correctly.
 */
int
miGetArcPts(parc, cpt, ppPts)
    SppArcPtr	parc;	/* points to an arc */
    int		cpt;	/* number of points already in arc list */
    SppPointPtr	*ppPts; /* pointer to pointer to arc-list -- modified */
{
    double 	st,	/* Start Theta, start angle */
                et,	/* End Theta, offset from start theta */
		dt,	/* Delta Theta, angle to sweep ellipse */
		cdt,	/* Cos Delta Theta, actually 2 cos(dt) */
    		x0, y0,	/* the recurrence formula needs two points to start */
		x1, y1,
		x2, y2, /* this will be the new point generated */
		xc, yc, /* the center point */
                xt, yt;	/* possible next point */
    int		count, i, axis, npts = 2; /* # points used thus far */
    double      asin(), fmax (), sin(), cos ();
    SppPointPtr	poly;
    DDXPointRec last;		/* last point on integer boundaries */

    /* The spec says that positive angles indicate counterclockwise motion.
     * Given our coordinate system (with 0,0 in the upper left corner), 
     * the screen appears flipped in Y.  The easiest fix is to negate the
     * angles given */
    
    st = - parc->angle1;

    et = - parc->angle2;

    /* Try to get a delta theta that is within 1/2 pixel.  Then adjust it
     * so that it divides evenly into the total.
     * I'm just using cdt 'cause I'm lazy.
     */
    cdt = fmax(parc->width, parc->height)/2.0;
    if(cdt <= 0)
	return 0;
    dt = asin( 1.0 / cdt ); /* minimum step necessary */
    count = et/dt;
    count = abs(count) + 1;
    dt = et/count;	
    count++;

    cdt = 2 * cos(dt);
#ifdef NOARCCOMPRESSION
    if (!(poly = (SppPointPtr) Xrealloc((pointer)*ppPts,
					(cpt + count) * sizeof(SppPointRec))))
	return(0);
    *ppPts = poly;
#else				/* ARCCOMPRESSION */
    if (!(poly = (SppPointPtr) Xrealloc((pointer)*ppPts,
					(cpt + 2) * sizeof(SppPointRec))))
	return(0);
#endif				/* ARCCOMPRESSION */

    xc = parc->width/2.0;		/* store half width and half height */
    yc = parc->height/2.0;
    axis = (xc >= yc) ? X_AXIS : Y_AXIS;
    
    x0 = xc * cos(st);
    y0 = yc * sin(st);
    x1 = xc * cos(st + dt);
    y1 = yc * sin(st + dt);
    xc += parc->x;		/* by adding initial point, these become */
    yc += parc->y;		/* the center point */

    poly[cpt].x = (xc + x0);
    poly[cpt].y = (yc + y0);
    last.x = ROUNDTOINT( poly[cpt + 1].x = (xc + x1) );
    last.y = ROUNDTOINT( poly[cpt + 1].y = (yc + y1) );

    for(i = 2; i < count; i++)
    {
	x2 = cdt * x1 - x0;
	y2 = cdt * y1 - y0;

#ifdef NOARCCOMPRESSION
 	poly[cpt + i].x = (xc + x2);
 	poly[cpt + i].y = (yc + y2);
#else				/* ARCCOMPRESSION */
	xt = xc + x2;
	yt = yc + y2;
 	if (((axis == X_AXIS) ?
	     (ROUNDTOINT(yt) != last.y) :
	     (ROUNDTOINT(xt) != last.x)) ||
	    i > count - 3)	/* insure 2 at the end */
 	{
	    /* allocate more space if we are about to need it */
	    /* include 1 extra in case minor axis swaps */
 	    if ((npts - 2) % REALLOC_STEP == 0)
	    {
 		if (!(poly = (SppPointPtr)
		      Xrealloc((pointer) poly,
			       ((npts + REALLOC_STEP + cpt) *
				sizeof(SppPointRec)))))
		    return(0);
	    }
	    /* check if we just switched direction in the minor axis */
	    if (((poly[cpt + npts - 2].y - poly[cpt + npts - 1].y > 0.0) ?
		 (yt - poly[cpt + npts - 1].y > 0.0) :
		 (poly[cpt + npts - 1].y - yt > 0.0)) ||
		((poly[cpt + npts - 2].x - poly[cpt + npts - 1].x > 0.0) ?
		 (xt - poly[cpt + npts - 1].x > 0.0) :
		 (poly[cpt + npts - 1].x - xt > 0.0)))
	    {
		/* Since the minor axis direction just switched, the final *
		 * point before the change must be included, or the        *
		 * following segment will begin before the minor swap.     */
		poly[cpt + npts].x = xc + x1;
		poly[cpt + npts].y = yc + y1;
		npts++;
		if ((npts - 2) % REALLOC_STEP == 0)
		{
		    if (!(poly = (SppPointPtr)
			  Xrealloc((pointer) poly,
				   ((npts + REALLOC_STEP + cpt) *
				    sizeof(SppPointRec)))))
			return(0);
		}
	    }
 	    last.x = ROUNDTOINT( poly[cpt + npts].x = xt );
 	    last.y = ROUNDTOINT( poly[cpt + npts].y = yt );
 	    npts++;
 	}
#endif				/* ARCCOMPRESSION */

	x0 = x1; y0 = y1;
	x1 = x2; y1 = y2;
    }
#ifndef NOARCCOMPRESSION	/* i.e.:  ARCCOMPRESSION */
    count = i = npts;
#endif				/* ARCCOMPRESSION */
    /* adjust the last point */
    if (abs(parc->angle2) >= FULLCIRCLE)
	poly[cpt +i -1] = poly[0];
    else {
	poly[cpt +i -1].x = (cos(st + et) * parc->width/2.0 + xc);
	poly[cpt +i -1].y = (sin(st + et) * parc->height/2.0 + yc);
    }

#ifndef NOARCCOMPRESSION	/* i.e.:  ARCCOMPRESSION */
    *ppPts = poly;		/* may have changed during reallocs */
#endif				/* ARCCOMPRESSION */
    return(count);
}

struct arcData {
	double	x0, y0, x1, y1;
	int	selfJoin;
};

computeBackPoint (arc, x, y, dir, xp, yp)
	xArc	*arc;
	double	x, y;		/* absolute position of point on arc */
	double	dir;		/* direction along arc */
	double	*xp, *yp;
{
	double	w, h;
	double	arcx, arcy;
	double	dx, dy, magd;
	
	w = ((double) arc->width) / 2.0;
	h = ((double) arc->height) / 2.0;
	/*
	 * compute position relative to the center of the arc
	 */
	arcx = x - (((double) arc->x) + w);
	arcy = - (y - ((double) arc->y + h));
	if (w == 0)
		dx = 0;
	else
		dx = 2 * arcx / (w*w);
	if (h == 0)
		dy = 0;
	else
		dy = 2 * arcy / (h*h);
	magd = sqrt (dx*dx + dy*dy);
	dx /= magd;
	dy /= magd;
	*xp = arc->x + w + (arcx - dir * dy);
	*yp = arc->y + h - (arcy + dir * dx);
}

# define ADD_REALLOC_STEP	20

addCap (capsp, ncapsp, sizep, end, arcIndex)
	miArcCapPtr	*capsp;
	int		*ncapsp, *sizep;
	int		end, arcIndex;
{
	miArcCapPtr	cap;

	if (*ncapsp == *sizep)
		*capsp = (miArcCapPtr) Xrealloc (*capsp, (*sizep += ADD_REALLOC_STEP) * sizeof (**capsp));
	cap = &(*capsp)[*ncapsp];
	cap->end = end;
	cap->arcIndex = arcIndex;
	++*ncapsp;
}

addJoin (joinsp, njoinsp, sizep, end0, index0, end1, index1)
	miArcJoinPtr	*joinsp;
	int		*njoinsp, *sizep;
	int		end0, index0, end1, index1;
{
	miArcJoinPtr	join;

	if (*njoinsp == *sizep)
		*joinsp = (miArcJoinPtr) Xrealloc (*joinsp, (*sizep += ADD_REALLOC_STEP) * sizeof (**joinsp));
	join = &(*joinsp)[*njoinsp];
	join->end0 = end0;
	join->arcIndex0 = index0;
	join->end1 = end1;
	join->arcIndex1 = index1;
	++*njoinsp;
}

static miPolyArcPtr
miSolidArcs (parcs, narcs)
	xArc	*parcs;
	int	narcs;
{
	miPolyArcPtr	arcs;
	int		start, i, j, k;
	int		njoins = 0, joinSize = 0;
	int		ncaps = 0, capSize = 0;
	int		angle2;
	double		x0, y0, x1, y1, a0, a1, xc, yc;
	struct arcData	*data;

	arcs = (miPolyArcPtr) Xalloc (sizeof (*arcs));
	arcs->arcs = (miArcDataPtr) Xalloc (narcs * sizeof (miArcDataRec));
	arcs->narcs = narcs;
	data = (struct arcData *) ALLOCATE_LOCAL (narcs * sizeof (struct arcData));
	for (i = 0; i < narcs; i++) {
		a0 = torad (parcs[i].angle1);
		angle2 = parcs[i].angle2;
		if (angle2 > FULLCIRCLE)
			angle2 = FULLCIRCLE;
		else if (angle2 < -FULLCIRCLE)
			angle2 = -FULLCIRCLE;
		data[i].selfJoin = angle2 == FULLCIRCLE || angle2 == -FULLCIRCLE;
		a1 = torad (parcs[i].angle1 + angle2);
		data[i].x0 = parcs[i].x + (double) parcs[i].width / 2 * (1 + cos (a0));
		data[i].y0 = parcs[i].y + (double) parcs[i].height / 2 * (1 - sin (a0));
		data[i].x1 = parcs[i].x + (double) parcs[i].width / 2 * (1 + cos (a1));
		data[i].y1 = parcs[i].y + (double) parcs[i].height / 2 * (1 - sin (a1));
	}

	arcs->njoins = 0;
	arcs->joins = 0;
	joinSize = 0;

	arcs->ncaps = 0;
	arcs->caps = 0;
	capSize = 0;

	if (narcs == 1) {
	    arcs->arcs[0].arc = parcs[0];
	    addCap (&arcs->caps, &arcs->ncaps, &capSize, RIGHT_END, 0);
	    addCap (&arcs->caps, &arcs->ncaps, &capSize, LEFT_END, 0);
	    arcs->arcs[0].join = 0;
	    arcs->arcs[0].cap = arcs->ncaps;
	    return arcs;
	}
	for (i = narcs - 1; i >= 0; i--) {
		j = i + 1;
		if (j == narcs)
			j = 0;
		if (!data[i].selfJoin && 
		     (UNEQUAL (data[i].x1, data[j].x0) ||
		      UNEQUAL (data[i].y1, data[j].y0)))
 		{
			addCap (&arcs->caps, &arcs->ncaps, &capSize, RIGHT_END, i);
			break;
		}
	}
	start = i + 1;
	if (start == narcs)
		start = 0;
	k = 0;
	i = start;
	for (;;) {
		j = i + 1;
		if (j == narcs)
			j = 0;
		arcs->arcs[k].arc = parcs[i];
		if (ISEQUAL (data[i].x1, data[j].x0) &&
		    ISEQUAL (data[i].y1, data[j].y0))
 		{
			addJoin (&arcs->joins, &arcs->njoins, &joinSize,
 				 RIGHT_END, k == narcs-1 ? 0 : k+1,
 				 LEFT_END, k);
			arcs->arcs[k].render = 0;
			arcs->arcs[k].join = arcs->njoins;
			arcs->arcs[k].cap = arcs->ncaps;
		} else {
			if (!data[i].selfJoin)
				addCap (&arcs->caps, &arcs->ncaps, &capSize, LEFT_END, k);
			arcs->arcs[k].join = arcs->njoins;
			arcs->arcs[k].cap = arcs->ncaps;
			if (k != narcs-1 && !data[j].selfJoin)
				addCap (&arcs->caps, &arcs->ncaps, &capSize, RIGHT_END, k+1);
			arcs->arcs[k].render = 1;
		}
		++k;
		if (++i == narcs)
			i = 0;
		if (i == start)
			break;
	}
	/*
	 * make sure the last section is rendered
	 */
	arcs->arcs[narcs-1].render = 1;
	return arcs;
}

/* b > 0 only */

# define mod(a,b)	((a) >= 0 ? (a) % (b) : (b) - (-a) % (b))

/*
 * compute the angle of an elipse which cooresponds to
 * the given path length.  Note that the correct solution
 * to this problem is an eliptic integral, we'll punt and
 * approximate (it's only for dashes anyway).  The approximation
 * used is a diamond (well, sort of anyway)
 *
 * The remaining portion of len is stored in *lenp -
 * this will be negative if the arc extends beyond
 * len and positive if len extends beyond the arc.
 */

static int
computeAngleFromPath (a0, a1, w, h, lenp, backwards)
	int	a0, a1;		/* normalized absolute angles in *64 degrees;
	int	w, h;		/* elipse width and height */
	int	*lenp;
	int	backwards;
{
	double	len;
	double	t0, t1, t, l, x0, y0, x1, y1, sidelen;
	int	a, startq, endq, q;
	double	atan2 (), floor ();

	len = *lenp;
	if (backwards) {
		a0 = FULLCIRCLE - a0;
		a1 = FULLCIRCLE - a1;
	}
	if (a1 < a0)
		a1 += FULLCIRCLE;
	startq = floor ((double) a0 / (90.0 * 64.0));
	endq = floor ((double) a1 / (90.0 * 64.0));
	a = a0;
	a0 = a0 - startq * 90 *64;
	a1 = a1 - endq * 90 * 64;
	for (q = startq; q <= endq && len > 0; ++q) {
		/*
		 * compute the end points of this arc
		 * in this quadrant
		 */
		if (q == startq && a0 != (q&1) * 90 * 64) {
			t0 = torad (a0 + startq * 90 * 64);
			x0 = (double) w / 2 * cos (t0);
			y0 = (double) h / 2* sin (t0);
		} else {
			x0 = 0;
			y0 = 0;
			switch (mod (q, 4)) {
			case 0: x0 = (double) w/2;	break;
			case 2:	x0 = - (double) w/2;	break;
			case 1:	y0 = (double) h/2;	break;
			case 3:	y0 = -(double) h/2;	break;
			}
		}
		if (q == endq && a1 != (1-(q&1)) * 90-64) {
			t1 = torad (a1 + endq * 90 * 64);
			x1 = (double) w / 2 * cos(t1);
			y1 = (double) h / 2 * sin(t1);
		} else {
			x1 = 0;
			y1 = 0;
			switch (mod (q, 4)) {
			case 0:	y1 = (double) h/2;	break;
			case 2:	y1 = - (double) h/2;	break;
			case 1:	x1 = -(double) w/2;	break;
			case 3:	x1 = (double) w/2;	break;
			}
		}
		/*
		 * compute the "length" of the arc in this quadrant --
		 * this should be the eliptic integral, we'll
		 * punt and assume it's close to a straight line
		 */
		sidelen = sqrt ((x1-x0)*(x1-x0) + (y1-y0) * (y1-y0));
		if (sidelen >= len) {
			/*
			 * compute the distance to the next axis
			 */
			x1 = 0;
			y1 = 0;
			switch (mod (q, 4)) {
			case 0:	y1 = (double) h/2;	break;
			case 2:	y1 = -(double) h/2;	break;
			case 1:	x1 = -(double) w/2;	break;
			case 3:	x1 = (double) w/2;	break;
			}
			sidelen = sqrt ((x1-x0) * (x1-x0) + (y1-y0) * (y1-y0));
			/*
			 * now pick the point "len" away from x0,y0
			 */
			y1 = y0 + (y1 - y0) * len / sidelen;
			x1 = x0 + (x1 - x0) * len / sidelen;
			/*
			 * translate the point to the angle on the
			 * elipse (remember that elipse angles don't
			 * match the actual angle
			 */
			if (y1 == 0 && x1 == 0)
				t1 = 0;
			else
				t1 = atan2 (y1 * w, x1 * h);
			a1 = (t1 * 180/M_PI) * 64.0;
			if (a1 < 0)
				a1 += FULLCIRCLE;
 			a1 -= q * 90 * 64;
			len = 0;
		} else
			len -= sidelen;
	}
	*lenp = len;
	a1 = a1 + (q-1) * (90*64);
	if (a1 > FULLCIRCLE)
		a1 -= FULLCIRCLE;
	if (backwards)
		a1 = FULLCIRCLE -a1;
	return a1;
}

/*
 * only does on/off dashes now
 */

static miPolyArcPtr
miDashArcs (pArcs, nArcs, pDash, nDashes, dashOffset, isDoubleDash)
	xArc		*pArcs;
	int		nArcs;
	unsigned char	*pDash;
	int		nDashes;
	int		dashOffset;
{
	int		dashRemaining;
	int		dashLen;
	int		iDash;
	int		iArc;
	int		startAngle;
	int		dashAngle;
	int		prevDashAngle;
	int		spanAngle;
	int		endAngle;
	int		backwards;
	int		iphase;
	int		nextArc;
	xArc		*Xarcs = 0;
	int		nXarcs = 0;
	int		sizeXarcs = 0;
	double		x0, y0, x1, y1;
	miPolyArcPtr	ret;

	iDash = 0;
	dashRemaining = pDash[0];
	iphase = 0;
 	while (dashOffset > 0) {
		if (dashOffset > dashRemaining) {
			dashOffset -= dashRemaining;
			iphase = iphase ? 0 : 1;
			iDash++;
			dashRemaining = pDash[iDash];
		} else {
			dashRemaining -= dashOffset;
			dashOffset = 0;
		}
	}
	for (iArc = 0; iArc < nArcs; iArc++) {
		startAngle = pArcs[iArc].angle1;
		spanAngle = pArcs[iArc].angle2;
		if (spanAngle > FULLCIRCLE)
			spanAngle = FULLCIRCLE;
		else if (spanAngle < -FULLCIRCLE)
			spanAngle = -FULLCIRCLE;
		if (startAngle < 0)
			startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
		if (startAngle >= FULLCIRCLE)
			startAngle = startAngle % FULLCIRCLE;
		endAngle = startAngle + spanAngle;
		backwards = spanAngle < 0;
		prevDashAngle = startAngle;
		while (prevDashAngle != endAngle) {
			dashAngle = computeAngleFromPath (prevDashAngle, endAngle,
	 						  pArcs[iArc].width, pArcs[iArc].height,
							  &dashRemaining, backwards);
			if (iphase == 0) {
				if (nXarcs == sizeXarcs) {
					Xarcs = (xArc *) Xrealloc ((pointer) Xarcs,
 						(sizeXarcs += 20) * sizeof (xArc));
				}
				Xarcs[nXarcs] = pArcs[iArc];
				Xarcs[nXarcs].angle1 = prevDashAngle;
				if (backwards) {
					spanAngle = dashAngle - prevDashAngle;
					if (dashAngle > prevDashAngle)
						spanAngle = - 360 * 64 + spanAngle;
				} else {
					spanAngle = dashAngle - prevDashAngle;
					if (dashAngle < prevDashAngle)
						spanAngle = 360 * 64 + spanAngle;
				}
				Xarcs[nXarcs].angle2 = spanAngle;
				++nXarcs;
			}
			if (dashRemaining <= 0) {
				++iDash;
				if (iDash == nDashes)
					iDash = 0;
				iphase = iphase ? 0:1;
				dashRemaining = pDash[iDash];
			}
			prevDashAngle = dashAngle;
		}
		nextArc = iArc+1;
		if (nextArc == nArcs)
			nextArc = 0;
		/*
		 * check to see if the two arcs meet -- if not,
		 * restart the dash list
		 */
		x0 = pArcs[nextArc].x + (double) pArcs[nextArc].width / 2 *
 					(1 + cos (torad (pArcs[nextArc].angle1)));
		y0 = pArcs[nextArc].y + (double) pArcs[nextArc].height / 2 *
 					(1 - sin (torad (pArcs[nextArc].angle1)));
		x1 = pArcs[iArc].x + (double) pArcs[iArc].width / 2 *
 					(1 + cos (torad (endAngle)));
		y1 = pArcs[iArc].y + (double) pArcs[iArc].height / 2 *
 					(1 - sin (torad (endAngle)));
		if (UNEQUAL (x0, x1) || UNEQUAL (y0, y1)) {
			iDash = 0;
			dashRemaining = pDash[iDash];
			iphase = 0;
		}
	}
	if (Xarcs) {
		ret = miSolidArcs (Xarcs, nXarcs);
		Xfree (Xarcs);
		return ret;
	}
	return 0;
}

/*
 * create two lists of arcs from the input arcs
 * and dash pattern
 */

static miPolyArcPtr
miOldDashArcs(pArc, nArcs, pDash, nDashes, dashOffset, isDoubleDash)
    xArc	  *pArc;	/* array of arcs to dash */
    int		  nArcs;	/* number of arcs in passed arc array */
    unsigned char *pDash;
    int           nDashes;
    int           dashOffset;	/* for dashes -- from gc */
    Bool          isDoubleDash;	/* TRUE if double dashed, otherwise onoff */
{
#ifdef NOTDEF
    double 	 st,	/* Start Theta, start angle */
                 et,	/* End Theta, offset from start theta */
    int		 count, i, istart, axis;
    double       asin();
    DDXPointRec  last;		/* last point on integer boundaries */
    int          lenCur;	/* used part of current dash */
    int          lenMax;	/* the desired length of this dash */
    int          iDash = 0;	/* index of current dash segment */
    miDashedArcPtr pDashed;
    int          which;		/* EVEN_DASH or ODD_DASH */
    int          iphase;	/* 0 = even, 1 = odd */
    int          a0, a1;
    int		startAngle, endAngle;
    int		backwards;	/* this arc drawn backwards */

    if (!(pDashed = (miDashedArcPtr) Xalloc((isDoubleDash ? 2 : 1) *
					    sizeof(miDashedArcRec))))
	return( NULL );		/* hosed */

    for ( ; nArcs > 0; nArcs--, pArc++, nSegs++)
    {
	a0 = pArc->angle1;
	a1 = pArc->angle2;
	if (a1 > FULLCIRCLE)
	    a1 = FULLCIRCLE;
	else if (a1 < -FULLCIRCLE)
	    a1 = -FULLCIRCLE;
	backwards = a1 < 0;
	startAngle = a0;
	endAngle = a0 + a1;
	/*
	 * bounds check the two angles
	 */
	if (startAngle < 0)
	    startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
	if (startAngle >= FULLCIRCLE)
	    startAngle = startAngle % FULLCIRCLE;
	if (endAngle < 0)
	    endAngle = FULLCIRCLE - (-endAngle) % FULLCIRCLE;
	if (endAngle > FULLCIRCLE)
	    endAngle = (endAngle-1) % FULLCIRCLE + 1;
	xc = pArc->x + pArc->width/2.0;
	yc = pArc->y + pArc->height/2.0;
	
	/* Handle dash offset if necessary.  May be retained from before. */
	if (nDash > 0)
	{			/* we've already generated at least 1 arc */
	    if (ROUNDTOINT(xt) !=
		ROUNDTOINT(poly[1].x) ||
		ROUNDTOINT(yt) !=
		ROUNDTOINT(poly[1].y))
	    {			/* get next dash position */
		lenCur = dashOffset;
		which = EVEN_DASH;
		while (lenCur > pDash[iDash])
		{
		    lenCur -= pDash[iDash];
		    iDash++;
		    if (iDash >= nDashes)
			iDash = 0;
		    which = ~which;
		}
		lenMax = pDash[iDash];
	    }			/* otherwise retain old dash position */
	} else {		/* first segment, so restart dashes */
	    lenCur = dashOffset;
	    which = EVEN_DASH;
	    while (lenCur > pDash[iDash])
	    {
		lenCur -= pDash[iDash];
		iDash++;
		if (iDash >= nDashes)
		    iDash = 0;
		which = ~which;
	    }
	    lenMax = pDash[iDash];
	}			/* now dashes are set to correct offset */
	
		if (which == EVEN_DASH)
		    iphase = 0;
	        else if (isDoubleDash)
		    iphase = 1;
		else		/* odd_dash but style = on-off-dashed */
		{
		    iphase = 2;
		    Xfree((pointer) poly); /* XXX dont generate this!! */
		}
		if (iphase < 2)
		{
		    if ((pDashed[iphase].ndashes) % REALLOC_STEP == 0)
			pDashed[iphase].dashes = (miArcDashPtr)
			    Xrealloc((pointer) pDashed[iphase].dashes,
				     (pDashed[iphase].ndashes+REALLOC_STEP) *
				     sizeof(miArcDashRec));
		    pDashed[iphase].dashes[pDashed[iphase].ndashes].arc =
			*pArc;
		    pDashed[iphase].dashes[pDashed[iphase].ndashes].arc.angle2
			= a1 - dt * i * 180 * 64 / M_PI -
			    pArc->angle1;
		    pDashed[iphase].ndashes++;
		}
		pArc->angle1 = a1 -
		    dt * 180 * 64 * i / M_PI; /* XXX - ack! */
		npts = 2;
		/* next segment starts where this one ended */
		if (!(poly = (SppPointPtr)
		      Xalloc(((1+(nalloced = 1 + REALLOC_STEP)) *
			      sizeof(SppPointRec)))))
		    return ( NULL );
		poly[1] = lastPt;
		istart = i;	/* for next arc segment collection */
		which = ~which;
		iDash++;
		if (iDash >= nDashes)
		    iDash = 0;
		lenMax = pDash[iDash];
		lenCur = 0;
	    }
	    
	    x0 = x1; y0 = y1;
	    x1 = x2; y1 = y2;
	}
	count = i = npts;

	/* put points generated for this arc into this dash segment */
	if (which == EVEN_DASH)
	    iphase = 0;
	else if (isDoubleDash)
	    iphase = 1;
	else		/* odd_dash but style = on-off-dashed */
	{
	    iphase = 2;
	    Xfree((pointer) poly);
	}
	if (iphase < 2)
	{
	    if ((pDashed[iphase].ndashes) % REALLOC_STEP == 0)
		pDashed[iphase].dashes = (miArcDashPtr)
		    Xrealloc((pointer) pDashed[iphase].dashes,
			     (pDashed[iphase].ndashes+REALLOC_STEP) *
			     sizeof(miArcDashRec));
	    pDashed[iphase].dashes[pDashed[iphase].ndashes].arc =
		*pArc;
	    pDashed[iphase].dashes[pDashed[iphase].ndashes].arc.angle2
		= a1 + pArc->angle2 - pArc->angle1;
	    pDashed[iphase].ndashes++;
	}
    }
    
    return( pDashed );
#endif
}

/*
 * scan convert wide arcs.
 */

#undef fabs
#undef min
#undef max

extern double	ceil (), floor (), fabs (), sin (), cos (), sqrt (), pow ();

# define Dsin(d)	((d) == 0.0 ? 0.0 : ((d) == 90.0 ? 1.0 : sin(d*M_PI/180.0)))
# define Dcos(d)	((d) == 0.0 ? 1.0 : ((d) == 90.0 ? 0.0 : cos(d*M_PI/180.0)))

# define BINARY_LIMIT	(0.1)
# define NEWTON_LIMIT	(0.0000001)

struct bound {
	double	min, max;
};

struct line {
	double	m, b;
	int	valid;
};

/*
 * these are all y value bounds
 */

struct arc_bound {
	struct bound	elipse;
	struct bound	inner;
	struct bound	outer;
	struct bound	right;
	struct bound	left;
};

struct accelerators {
	double		tail_y;
	double		h2;
	double		w2;
	double		h4;
	double		w4;
	double		h2mw2;
	double		wh2mw2;
	double		wh4;
	struct line	left, right;
};

struct arc_def {
	double	w, h, l;
	double	a0, a1;
};

double
Sqrt (x)
double	x;
{
	if (x < 0) {
		if (x > -NEWTON_LIMIT)
			return 0;
		else
			abort ();
	}
	return sqrt (x);
}

double
fmax (a, b)
double	a,b;
{
	return a > b? a : b;
}

double
fmin (a, b)
double	a, b;
{
	return a < b ? a : b;
}

boundedLt (value, bounds)
double	value;
struct bound	bounds;
{
	return bounds.min <= value && value < bounds.max;
}

boundedLe (value, bounds)
double	value;
struct bound	bounds;
{
	return bounds.min <= value && value <= bounds.max;
}

/*
 * this computes the elipse y value associated with the
 * bottom of the tail.
 */

# define CUBED_ROOT_2	1.2599210498948732038115849718451499938964
# define CUBED_ROOT_4	1.5874010519681993173435330390930175781250

double
tailElipseY (w, h, l)
	double	w, h, l;
{
	extern double	Sqrt (), pow ();
	double		t;

	if (w != h) {
		t = (pow (h * l * w, 2.0/3.0) - CUBED_ROOT_4 * h*h) /
		    (w*w - h*h);
		if (t < 0)
			return 0;	/* no tail */
		return h / CUBED_ROOT_2 * Sqrt (t);
	} else
		return 0;
}

/*
 * inverse functions -- compute edge coordinates
 * from the elipse
 */

double
outerXfromXY (x, y, def, acc)
	double			x, y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	return x + (x * acc->h2 * def->l) /
		   (2 * Sqrt (x*x *acc->h4 + y*y * acc->w4));
}

double
outerXfromY (y, def, acc)
	double			y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	double	x;

	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);

	return x + (x * acc->h2 * def->l) /
		   (2 * Sqrt (x*x *acc->h4 + y*y * acc->w4));
}

double
outerYfromXY (x, y, def, acc)
	double		x, y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	return y + (y * acc->w2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

double
outerYfromY (y, def, acc)
	double	y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	double	x;

	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);

	return y + (y * acc->w2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

double
innerXfromXY (x, y, def, acc)
	double			x, y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	return x - (x * acc->h2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

double
innerXfromY (y, def, acc)
	double			y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	double	x;

	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);
	
	return x - (x * acc->h2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

double
innerYfromXY (x, y, def, acc)
	double			x, y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	return y - (y * acc->w2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

double
innerYfromY (y, def, acc)
	double	y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	double	x;

	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);

	return y - (y * acc->w2 * def->l) /
		   (2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

computeLine (x1, y1, x2, y2, line)
	double		x1, y1, x2, y2;
	struct line	*line;
{
	if (y1 == y2)
		line->valid = 0;
	else {
		line->m = (x1 - x2) / (y1 - y2);
		line->b = x1  - y1 * line->m;
		line->valid = 1;
	}
}

double
intersectLine (y, line)
	double		y;
	struct line	*line;
{
	return line->m * y + line->b;
}

computeAcc (def, acc)
	struct arc_def		*def;
	struct accelerators	*acc;
{
	acc->h2 = def->h * def->h;
	acc->w2 = def->w * def->w;
	acc->h4 = acc->h2 * acc->h2;
	acc->w4 = acc->w2 * acc->w2;
	acc->h2mw2 = acc->h2 - acc->w2;
	acc->wh2mw2 = def->w * acc->h2mw2;
	acc->wh4 = def->w * acc->h4;
	acc->tail_y = tailElipseY (def->w, def->h, def->l);
}
		
computeBound (def, bound, acc, right, left)
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
	miArcFacePtr		right, left;
{
	double		t, elipseX ();
	double		innerTaily;
	double		tail_y;
	struct bound	innerx, outerx;
	struct bound	elipsex;

	bound->elipse.min = Dsin (def->a0) * def->h;
	bound->elipse.max = Dsin (def->a1) * def->h;
	if (def->a0 == 45 && def->w == def->h)
		elipsex.min = bound->elipse.min;
	else
		elipsex.min = Dcos (def->a0) * def->w;
	if (def->a1 == 45 && def->w == def->h)
		elipsex.max = bound->elipse.max;
	else
		elipsex.max = Dcos (def->a1) * def->w;
	bound->outer.min = outerYfromXY (elipsex.min, bound->elipse.min, def, acc);
	bound->outer.max = outerYfromXY (elipsex.max, bound->elipse.max, def, acc);
	bound->inner.min = innerYfromXY (elipsex.min, bound->elipse.min, def, acc);
	bound->inner.max = innerYfromXY (elipsex.max, bound->elipse.max, def, acc);

	outerx.min = outerXfromXY (elipsex.min, bound->elipse.min, def, acc);
	outerx.max = outerXfromXY (elipsex.max, bound->elipse.max, def, acc);
	innerx.min = innerXfromXY (elipsex.min, bound->elipse.min, def, acc);
	innerx.max = innerXfromXY (elipsex.max, bound->elipse.max, def, acc);
	
	/*
	 * save the line end points for the
	 * cap code to use
	 */

	if (right) {
		right->counterClock.y = bound->outer.min;
		right->counterClock.x = outerx.min;
		right->center.y = bound->elipse.min;
		right->center.x = elipsex.min;
		right->clock.y = bound->inner.min;
		right->clock.x = innerx.min;
	}

	if (left) {
		left->clock.y = bound->outer.max;
		left->clock.x = outerx.max;
		left->center.y = bound->elipse.max;
		left->center.x = elipsex.max;
		left->counterClock.y = bound->inner.max;
		left->counterClock.x = innerx.max;
	}

	bound->left.min = bound->inner.max;
	bound->left.max = bound->outer.max;
	bound->right.min = bound->inner.min;
	bound->right.max = bound->outer.min;

	computeLine (innerx.min, bound->inner.min, outerx.min, bound->outer.min,
		      &acc->right);
	computeLine (innerx.max, bound->inner.max, outerx.max, bound->outer.max,
		     &acc->left);

	if (bound->inner.min > bound->inner.max) {
		t = bound->inner.min;
		bound->inner.min = bound->inner.max;
		bound->inner.max = t;
	}
	tail_y = acc->tail_y;
	if (tail_y > bound->elipse.max)
		tail_y = bound->elipse.max;
	else if (tail_y < bound->elipse.min)
		tail_y = bound->elipse.min;
	innerTaily = innerYfromY (tail_y, def, acc);
	if (bound->inner.min > innerTaily)
		bound->inner.min = innerTaily;
	if (bound->inner.max < innerTaily)
		bound->inner.max = innerTaily;
}

/*
 * using newtons method and a binary search, compute the elipse y value
 * associated with the given edge value (either outer or
 * inner)
 */

double
elipseY (edge_y, def, bound, acc, outer, y0, y1)
	double			edge_y;
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
	double			y0, y1;
{
	double	w, h, l, h2, h4, w2, w4, x, y2;
	double	newtony, binaryy;
	double	value0, value1, valuealt;
	double	newtonvalue, binaryvalue;
	double	minY, maxY;
	double	(*f)();
	
	/*
	 * compute some accelerators
	 */
	w = def->w;
	h = def->h;
	f = outer ? outerYfromY : innerYfromY;
	l = outer ? def->l : -def->l;
	h2 = acc->h2;
	h4 = acc->h4;
	w2 = acc->w2;
	w4 = acc->w4;

	value0 = f (y0, def, acc) - edge_y;
	if (value0 == 0)
		return y0;
	value1 = f (y1, def, acc) - edge_y;
	maxY = y1;
	minY = y0;
	if (y0 > y1) {
		maxY = y0;
		minY = y1;
	}
	if (value1 == 0)
		return y1;
	if (value1 > 0 == value0 > 0)
		return -1.0;	/* an illegal value */
	/*
	 * binary search for a while
	 */
	do {
		if (y0 == y1 || value0 == value1)
			return maxY+1;
		binaryy = (y0 + y1) / 2;

		/*
		 * inline expansion of the function
		 */

		y2 = binaryy*binaryy;
		x = w * Sqrt ((h2 - (y2)) / h2);

		binaryvalue = ( binaryy + (binaryy * w2 * l) /
			      (2 * Sqrt (x*x * h4 + y2 * w4))) - edge_y;

		if (binaryvalue == 0)
			return binaryy;
		if (binaryvalue > 0 == value0 > 0) {
			y0 = binaryy;
			value0 = binaryvalue;
		} else {
			y1 = binaryy;
			value1 = binaryvalue;
		}
	} while (fabs (value1) > BINARY_LIMIT);

	/*
	 * clean up the estimate with newtons method
	 */

	while (fabs (value1) > NEWTON_LIMIT) {
		newtony = y1 - value1 * (y1 - y0) / (value1 - value0);
		if (newtony > maxY)
			newtony = maxY;
		if (newtony < minY)
			newtony = minY;
		/*
		 * inline expansion of the function
		 */

		y2 = newtony*newtony;
		x = w * Sqrt ((h2 - (y2)) / h2);

		newtonvalue = ( newtony + (newtony * w2 * l) /
			      (2 * Sqrt (x*x * h4 + y2 * w4))) - edge_y;

		if (newtonvalue == 0)
			return newtony;
		if (fabs (value0) > fabs (value1)) {
			y0 = newtony;
			value0 = newtonvalue;
		} else {
			y1 = newtony;
			value1 = newtonvalue;
		}
	}
	return y1;
}

double
elipseX (elipse_y, def, acc)
	double			elipse_y;
	struct arc_def		*def;
	struct accelerators	*acc;
{
	return def->w / def->h * Sqrt (acc->h2 - elipse_y * elipse_y);
}

double
outerX (outer_y, def, bound, acc)
	double			outer_y;
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
{
	double	y;

	if (outer_y == bound->outer.min)
		y = bound->elipse.min;
	if (outer_y == bound->outer.max)
		y = bound->elipse.max;
	else
		y = elipseY (outer_y, def, bound, acc, 1,
 			     bound->elipse.min, bound->elipse.max, -1.0);
	return outerXfromY (y, def, acc);
}

/*
 * this equation has two solutions -- it's not a function
 */

innerXs (inner_y, def, bound, acc, innerX1p, innerX2p)
	double			inner_y;
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
	double			*innerX1p, *innerX2p;
{
	double	x1, x2, xalt, y0, y1, altY, elipse_y1, elipse_y2;

	if (boundedLe (acc->tail_y, bound->elipse)) {
		if (def->h > def->w) {
			y0 = bound->elipse.min;
			y1 = acc->tail_y;
			altY = bound->elipse.max;
		} else {
			y0 = bound->elipse.max;
			y1 = acc->tail_y;
			altY = bound->elipse.min;
		}
		elipse_y1 = elipseY (inner_y, def, bound, acc, 0, y0, y1);
		elipse_y2 = elipseY (inner_y, def, bound, acc, 0, y1, altY);
		if (elipse_y1 == -1.0)
			elipse_y1 = elipse_y2;
		if (elipse_y2 == -1.0)
			elipse_y2 = elipse_y1;
	} else {
		elipse_y1 = elipseY (inner_y, def, bound, acc, 0,
				     bound->elipse.min, bound->elipse.max);
		elipse_y2 = elipse_y1;
	}
	x2 = x1 = innerXfromY (elipse_y1, def, acc);
	if (elipse_y1 != elipse_y2)
		x2 = innerXfromY (elipse_y2, def, acc);
	if (acc->left.valid && boundedLe (inner_y, bound->left)) {
		xalt = intersectLine (inner_y, &acc->left);
		if (xalt < x2 && xalt < x1)
			x2 = xalt;
		if (xalt < x1)
			x1 = xalt;
	}
	if (acc->right.valid && boundedLe (inner_y, bound->right)) {
		xalt = intersectLine (inner_y, &acc->right);
		if (xalt < x2 && xalt < x1)
			x2 = xalt;
		if (xalt < x1)
			x1 = xalt;
	}
	*innerX1p = x1;
	*innerX2p = x2;
}

/*
 * this section computes the x value of the span at y 
 * intersected with the specified face of the elipse.
 *
 * this is the min/max X value over the set of normal
 * lines to the entire elipse,  the equation of the
 * normal lines is:
 *
 *     elipse_x h^2                   h^2
 * x = ------------ y + elipse_x (1 - --- )
 *     elipse_y w^2                   w^2
 *
 * compute the derivative with-respect-to elipse_y and solve
 * for zero:
 *    
 *       (w^2 - h^2) elipse_y^3 + h^4 y
 * 0 = - ----------------------------------
 *       h w elipse_y^2 sqrt (h^2 - elipse_y^2)
 *
 *            (   h^4 y     )
 * elipse_y = ( ----------  ) ^ (1/3)
 *            ( (h^2 - w^2) )
 *
 * The other two solutions to the equation are imaginary.
 *
 * This gives the position on the elipse which generates
 * the normal with the largest/smallest x intersection point.
 *
 * Now compute the second derivative to check whether
 * the intersection is a minimum or maximum:
 *
 *    h (y0^3 (w^2 - h^2) + h^2 y (3y0^2 - 2h^2))
 * -  -------------------------------------------
 *          w y0^3 (sqrt (h^2 - y^2)) ^ 3
 *
 * as we only care about the sign,
 *
 * - (y0^3 (w^2 - h^2) + h^2 y (3y0^2 - 2h^2))
 *
 * or (to use accelerators),
 *
 * y0^3 (h^2 - w^2) - h^2 y (3y0^2 - 2h^2) 
 *
 */

/*
 * computes the position on the elipse whose normal line
 * intersects the given scan line maximally
 */

double
hookElipseY (scan_y, def, bound, acc, left)
	double			scan_y;
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
{
	double	ret;

	if (acc->h2mw2 == 0) {
		if (scan_y > 0 && !left || scan_y < 0 && left)
			return bound->elipse.min;
		return bound->elipse.max;
	}
	ret = (acc->h4 * scan_y) / (acc->h2mw2);
	if (ret >= 0)
		return pow (ret, 1.0/3.0);
	else
		return -pow (-ret, 1.0/3.0);
}

/*
 * computes the X value of the intersection of the
 * given scan line with the right side of the lower hook
 */

double
hookX (scan_y, def, bound, acc, left)
	double			scan_y;
	struct arc_def		*def;
	struct arc_bound	*bound;
	struct accelerators	*acc;
	int			left;
{
	double	elipse_y, elipse_x, x, xalt;
	double	maxMin;

	elipse_y = hookElipseY (scan_y, def, bound, acc, left);
	if (boundedLe (elipse_y, bound->elipse)) {
		/*
		 * compute the value of the second
		 * derivative
		 */
		maxMin = elipse_y*elipse_y*elipse_y * acc->h2mw2 -
		 acc->h2 * scan_y * (3 * elipse_y*elipse_y - 2*acc->h2);
		if ((left && maxMin > 0) || (!left && maxMin < 0)) {
			if (elipse_y == 0)
				return def->w + left ? -def->l/2 : def->l/2;
			x = (acc->h2 * scan_y - elipse_y * acc->h2mw2) *
				Sqrt (acc->h2 - elipse_y * elipse_y) /
			 	(def->h * def->w * elipse_y);
			return x;
		}
	}
	if (left) {
		if (acc->left.valid && boundedLe (scan_y, bound->left)) {
			x = intersectLine (scan_y, &acc->left);
		} else {
			if (acc->right.valid)
				x = intersectLine (scan_y, &acc->right);
			else
				x = def->w - def->l/2;
		}
	} else {
		if (acc->right.valid && boundedLe (scan_y, bound->right)) {
			x = intersectLine (scan_y, &acc->right);
		} else {
			if (acc->left.valid)
				x = intersectLine (scan_y, &acc->left);
			else
				x = def->w - def->l/2;
		}
	}
	return x;
}

arcSpan (y, def, bounds, acc)
	double			y;
	struct arc_def		*def;
	struct arc_bound	*bounds;
	struct accelerators	*acc;
{
	double	innerx1, innerx2, outerx1, outerx2;

	if (boundedLe (y, bounds->inner)) {
		/*
		 * intersection with inner edge
		 */
		innerXs (y, def, bounds, acc, &innerx1, &innerx2);
	} else {
		/*
		 * intersection with left face
		 */
		innerx2 = innerx1 = hookX (y, def, bounds, acc, 1);
		if (acc->right.valid && boundedLe (y, bounds->right))
		{
			innerx2 = intersectLine (y, &acc->right);
			if (innerx2 < innerx1)
				innerx1 = innerx2;
		}
	}
	if (boundedLe (y, bounds->outer)) {
		/*
		 * intersection with outer edge
		 */
		outerx1 = outerx2 = outerX (y, def, bounds, acc);
	} else {
		/*
		 * intersection with right face
		 */
		outerx2 = outerx1 = hookX (y, def, bounds, acc, 0);
		if (acc->left.valid && boundedLe (y, bounds->left))
 		{
			outerx2 = intersectLine (y, &acc->left);
			if (outerx2 < outerx1)
				outerx2 = outerx1;
		}
	}
	/*
	 * there are a very few cases when two spans will be
	 * generated.
	 */
	if (innerx1 < outerx1 && outerx1 < innerx2 && innerx2 < outerx2) {
		span (innerx1, outerx1);
		span (innerx2, outerx2);
	} else
		span (innerx1, outerx2);
}

/*
 * create whole arcs out of pieces.  This code is
 * very bad.
 */

static double	arcXcenter, arcYcenter;
static int	arcXoffset, arcYoffset;

static struct finalSpan	*firstSpan;
static int		nspans;		/* total spans, not just y coords */

struct finalSpan {
	struct finalSpan	*next;
	int			y;
	struct xrange {
		struct xrange	*next;
		int		min, max;
	}			*spans;
};

fillSpans (pDrawable, pGC)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
{
	struct finalSpan	*f, *next;
	struct xrange		*x, *nextx;
	DDXPointPtr		xSpans, xSpan;
	int			*xWidths, *xWidth;
	int			i;

	if (nspans == 0)
		return;
	xSpan = xSpans = (DDXPointPtr) Xalloc (nspans * sizeof (DDXPointRec));
	xWidth = xWidths = (int *) Xalloc (nspans * sizeof (int));
	i = 0;
	for (f = firstSpan; f; f=next) {
		next = f->next;
		for (x = f->spans; x; x=nextx) {
			nextx = x->next;
			if (x->max > x->min) {
				xSpan->x = x->min;
				xSpan->y = f->y;
				++xSpan;
				*xWidth++ = x->max - x->min;
				++i;
			}
			Xfree (x);
		}
		Xfree (f);
	}
	(*pGC->FillSpans) (pDrawable, pGC, i, xSpans, xWidths, TRUE);
	Xfree (xSpans);
	Xfree (xWidths);
	firstSpan = 0;
	nspans = 0;
}

struct finalSpan *
findSpan (y)
{
	struct finalSpan	*f, *new, *prev;

	prev = 0;
	for (f = firstSpan; f; f=f->next) {
		if (f->y == y)
			return f;
		if (f->y > y)
			break;
		prev = f;
	}
	new = (struct finalSpan *) Xalloc (sizeof (struct finalSpan));
	new->y = y;
	new->spans = 0;
	new->next = f;
	if (prev)
		prev->next = new;
	else
		firstSpan = new;
	return new;
}

newFinalSpan (y, xmin, xmax)
{
	struct finalSpan	*f;
	struct xrange		*x, *oldx, *prev;

	f = findSpan (y);
	oldx = 0;
	for (;;) {
		prev = 0;
		for (x = f->spans; x; x=x->next) {
			if (x == oldx) {
				prev = x;
				continue;
			}
			if (x->min <= xmax && xmin <= x->max) {
				if (oldx) {
					oldx->min = min (x->min, xmin);
					oldx->max = max (x->max, xmax);
					if (prev)
						prev->next = x->next;
					else
						f->spans = x->next;
					--nspans;
					free (x);
				} else {
					x->min = min (x->min, xmin);
					x->max = max (x->max, xmax);
					oldx = x;
				}
				xmin = oldx->min;
				xmax = oldx->max;
				break;
			}
			prev = x;
		}
		if (!x)
			break;
	}
	if (!oldx) {
		x = (struct xrange *) Xalloc (sizeof (struct xrange));
		x->min = xmin;
		x->max = xmax;
		x->next = f->spans;
		f->spans = x;
		++nspans;
	}
}

mirrorSppPoint (quadrant, sppPoint)
	int		quadrant;
	SppPointPtr	sppPoint;
{
	switch (quadrant) {
	case 0:
		break;
	case 1:
		sppPoint->x = -sppPoint->x;
		break;
	case 2:
		sppPoint->x = -sppPoint->x;
		sppPoint->y = -sppPoint->y;
		break;
	case 3:
		sppPoint->y = -sppPoint->y;
		break;
	}
	/*
	 * and translate to X coordinate system
	 */
	sppPoint->y = -sppPoint->y;
}

mirrorSpan (quadrant, y, min, max)
	double		y;
	double		min, max;
{
	int		spany, xmin, xmax;
	double		t;

	switch (quadrant) {
	case 0:
		break;
	case 1:
		t = -max;
		max = -min;
		min = t;
		break;
	case 2:
		t = -max;
		max = -min;
		min = t;
		y = -y;
		break;
	case 3:
		y = -y;
		break;
	}
	xmin = (int) ceil (min + arcXcenter) + arcXoffset;
	xmax = (int) ceil (max + arcXcenter) + arcXoffset;
	spany = (int) (ceil (arcYcenter - y)) + arcYoffset;
	if (xmax > xmin)
		newFinalSpan (spany, xmin, xmax);
}

static int	quadrantMask;

mergeSpan (y, min, max)
	double	y, min, max;
{
	if (quadrantMask & 1)
		mirrorSpan (0, y, min, max);
	if (quadrantMask & 2)
		mirrorSpan (1, y, min, max);
	if (quadrantMask & 4)
		mirrorSpan (2, y, min, max);
	if (quadrantMask & 8)
		mirrorSpan (3, y, min, max);
}

static double	spanY;

drawArc (x0, y0, w, h, l, a0, a1, right, left)
	int	x0, y0, w, h, l, a0, a1;
	miArcFacePtr	right, left;	/* save end line points */
{
	struct arc_def		def;
	struct accelerators	acc;
	struct span		*result;
	int			startq, endq, curq;
	int			rightq, leftq, righta, lefta;
	miArcFacePtr		passRight, passLeft;
	int			q0, q1, mask;
	struct band {
		int	a0, a1;
		int	mask;
	}	band[5], sweep[20];
	int			bandno, sweepno;
	int			i, j, k;

	def.w = ((double) w) / 2;
	def.h = ((double) h) / 2;
	arcXoffset = x0;
	arcYoffset = y0;
	arcXcenter = def.w;
	arcYcenter = def.h;
	def.l = (double) l;
	if (a1 < a0)
		a1 += 360 * 64;
	startq = a0 / (90 * 64);
	endq = (a1-1) / (90 * 64);
	bandno = 0;
	curq = startq;
	for (;;) {
		switch (curq) {
		case 0:
			if (a0 > 90 * 64)
				q0 = 0;
			else
				q0 = a0;
			if (a1 < 360 * 64)
				q1 = min (a1, 90 * 64);
			else
				q1 = 90 * 64;
			if (curq == startq && a0 == q0) {
				righta = q0;
				rightq = curq;
			}
			if (curq == endq && a1 == q1) {
				lefta = q1;
				leftq = curq;
			}
			break;
		case 1:
			if (a1 < 90 * 64)
				q0 = 0;
			else
				q0 = 180 * 64 - min (a1, 180 * 64);
			if (a0 > 180 * 64)
				q1 = 90 * 64;
			else
				q1 = 180 * 64 - max (a0, 90 * 64);
			if (curq == startq && 180 * 64 - a0 == q1) {
				righta = q1;
				rightq = curq;
			}
			if (curq == endq && 180 * 64 - a1 == q0) {
				lefta = q0;
				leftq = curq;
			}
			break;
		case 2:
			if (a0 > 270 * 64)
				q0 = 0;
			else
				q0 = max (a0, 180 * 64) - 180 * 64;
			if (a1 < 180 * 64)
				q1 = 90 * 64;
			else
				q1 = min (a1, 270 * 64) - 180 * 64;
			if (curq == startq && a0 - 180*64 == q0) {
				righta = q0;
				rightq = curq;
			}
			if (curq == endq && a1 - 180 * 64 == q1) {
				lefta = q1;
				leftq = curq;
			}
			break;
		case 3:
			if (a1 < 270 * 64)
				q0 = 0;
			else
				q0 = 360 * 64 - min (a1, 360 * 64);
			q1 = 360 * 64 - max (a0, 270 * 64);
			if (curq == startq && 360 * 64 - a0 == q1) {
				righta = q1;
				rightq = curq;
			}
			if (curq == endq && 360 * 64 - a1 == q0) {
				lefta = q0;
				leftq = curq;
			}
			break;
		}
		band[bandno].a0 = q0;
		band[bandno].a1 = q1;
		band[bandno].mask = 1 << curq;
		bandno++;
		if (curq == endq)
			break;
		curq++;
		if (curq == 4) {
			a0 = 0;
			a1 -= 360 * 64;
			curq = 0;
			endq -= 4;
		}
	}
	sweepno = 0;
	for (;;) {
		q0 = 90 * 64;
		mask = 0;
		/*
		 * find left-most point
		 */
		for (i = 0; i < bandno; i++)
			if (band[i].a0 < q0) {
				q0 = band[i].a0;
				q1 = band[i].a1;
				mask = band[i].mask;
			}
		if (!mask)
			break;
		/*
		 * locate next point of change
		 */
		for (i = 0; i < bandno; i++)
			if (!(mask & band[i].mask)) {
				if (band[i].a0 == q0) {
					if (band[i].a1 < q1)
						q1 = band[i].a1;
					mask |= band[i].mask;
 				} else if (band[i].a0 < q1)
					q1 = band[i].a0;
			}
		/*
		 * create a new sweep
		 */
		sweep[sweepno].a0 = q0;
		sweep[sweepno].a1 = q1;
		sweep[sweepno].mask = mask;
		sweepno++;
		/*
		 * subtract the sweep from the affected bands
		 */
		for (i = 0; i < bandno; i++)
			if (band[i].a0 == q0) {
				band[i].a0 = q1;
				/*
				 * check if this band is empty
				 */
				if (band[i].a0 == band[i].a1)
					band[i].a1 = band[i].a0 = 90 * 64;
			}
	}
	computeAcc (&def, &acc);
	for (j = 0; j < sweepno; j++) {
		mask = sweep[j].mask;
		passRight = passLeft = 0;
 		if (mask & (1 << rightq)) {
			if (sweep[j].a0 == righta)
				passRight = right;
			if (sweep[j].a1 == righta)
				passLeft = right;
		}
		if (mask & (1 << leftq)) {
			if (sweep[j].a0 == lefta)
				passRight = left;
			if (sweep[j].a1 == lefta)
				passLeft = left;
		}
		drawQuadrant (&def, &acc, sweep[j].a0, sweep[j].a1, mask, 
 			      passRight, passLeft);
	}
	if (right) {
		mirrorSppPoint (rightq, &right->clock);
		mirrorSppPoint (rightq, &right->center);
		mirrorSppPoint (rightq, &right->counterClock);
	}
	if (left) {
		mirrorSppPoint (leftq,  &left->counterClock);
		mirrorSppPoint (leftq,  &left->center);
		mirrorSppPoint (leftq,  &left->clock);
	}
}

drawQuadrant (def, acc, a0, a1, mask, right, left)
	struct arc_def		*def;
	struct accelerators	*acc;
	int			a0, a1;
	int			mask;
	miArcFacePtr		right, left;
{
	struct arc_bound	bound;
	double			miny, maxy, y;
	int			minIsInteger;

	def->a0 = ((double) a0) / 64.0;
	def->a1 = ((double) a1) / 64.0;
	computeBound (def, &bound, acc, right, left);
	miny = fmin (bound.inner.min, bound.outer.min);
	minIsInteger = miny == ceil (miny);
	miny = ceil (miny);
	maxy = fmax (floor (bound.inner.max), floor (bound.outer.max));
	for (y = miny; y <= maxy; y = y + 1.0) {
		if (y == miny && minIsInteger)
			quadrantMask = mask & 0xc;
		else
			quadrantMask = mask;
		spanY = y;
		arcSpan (y, def, &bound, acc);
	}
	/*
	 * add the pixel at the top of the arc
	 */
	if (a1 == 90 * 64 && (quadrantMask & 1) && ((int) (def->w * 2 + def->l)) & 1)
		mirrorSpan (0, def->h + def->l/2, 0.0, 1.0);
}

max (x, y)
{
	return x>y? x:y;
}

min (x, y)
{
	return x<y? x:y;
}

span (left, right)
double	left, right;
{
	mergeSpan (spanY, left, right);
}
