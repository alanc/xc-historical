/* $Header: XRegion.c,v 11.12 87/06/10 18:34:35 jg Exp $ */
/************************************************************************
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

************************************************************************/

#include "region.h"
#include "poly.h"

extern char *_XAllocScratch();
/*	Create a new empty region	*/
Region
XCreateRegion()
{
    Region temp;

    temp = ( Region )Xmalloc( (unsigned) sizeof( REGION ));
    temp->rects = ( BOX * )Xmalloc( (unsigned) sizeof( BOX ));
    temp->numRects = 0;
    temp->extents.x1 = MAXSHORT;
    temp->extents.y1 = MAXSHORT;
    temp->extents.x2 = MINSHORT;
    temp->extents.y2 = MINSHORT;
    temp->size = 1;
    return( temp );
}


XClipBox( r, rect )
    Region r;
    XRectangle *rect;
{
    rect->x = r->extents.x1;
    rect->y = r->extents.y1;
    rect->width = r->extents.x2 - r->extents.x1;
    rect->height = r->extents.y2 - r->extents.y1;
}

XSetRegion( dpy, gc, r )
    Display *dpy;
    GC gc;
    register Region r;
{
    register int i;
    register XRectangle *xr;
    LockDisplay (dpy);
    xr = (XRectangle *) 
    	_XAllocScratch(dpy, (unsigned long)(r->numRects * sizeof (XRectangle)));
    for (i = 0; i < r->numRects; i++) {
        xr->x = r->rects[i].x1;
	xr->y = r->rects[i].y1;
	xr->width = r->rects[i].x2 - r->rects[i].x1;
	xr->height = r->rects[i].y2 - r->rects[i].y1;
      }
    _XSetClipRectangles(dpy, gc,r->extents.x1,r->extents.y1, xr,
    	r->numRects,Unsorted);
    UnlockDisplay(dpy);
    SyncHandle();
}

XDestroyRegion( r )
    Region r;
{
    Xfree( (char *) r->rects );
    Xfree( (char *) r );
}

XOffsetRegion( pRegion, x, y )
    register Region pRegion;
    register int x;
    register int y;
{
    register int nbox;
    register BOX *pbox;

    pbox = pRegion->rects;
    nbox = pRegion->numRects;

    while(nbox--)
    {
	pbox->x1 += x;
	pbox->x2 += x;
	pbox->y1 += y;
	pbox->y2 += y;
	pbox++;
    }
    pRegion->extents.x1 += x;
    pRegion->extents.x2 += x;
    pRegion->extents.y1 += y;
    pRegion->extents.y2 += y;
}

XShrinkRegion( r, dx, dy )
    Region r;
    int dx, dy;
{
    int i,h,w;
    float sx,sy;
    int cx,cy,newcx,newcy;
    BoxPtr rect;

    /* figure out ratios */
    h = r->extents.y2 - r->extents.y1;
    w = r->extents.x2 - r->extents.x1;
    sx = ((float)(w+dx))/w;
    sy = ((float)(h+dy))/h;
    rect = &(r->rects[0]);
    for(i=0; i<r->numRects; i++)
    {
    	cx = (rect->x1 + rect->x2)/2;
    	cy = (rect->y1 + rect->y2)/2;

	/* shrink or expand box */
    	rect->x1 = rect->x1 * sx;
    	rect->x2 = rect->x2 * sx;
    	rect->y1 = rect->y1 * sy;
    	rect->y2 = rect->y2 * sy;

	/* recenter box around old center */
    	newcx = (rect->x1 + rect->x2)/2;
    	newcy = (rect->y1 + rect->y2)/2;
	rect->x1 = rect->x1 - (newcx - cx);
	rect->x2 = rect->x2 - (newcx - cx);
	rect->y1 = rect->y1 - (newcy - cy);
	rect->y2 = rect->y2 - (newcy - cy);

    	/* adjust for shrinkage problems */
    	if ( dx < 0 )
    	{
    		if ( rect->x1 > cx ) rect->x1 = cx;
    		if ( rect->x2 < cx ) rect->x2 = cx;
    	}
    	if ( dy < 0 )
    	{
    		if ( rect->y1 > cy ) rect->y1 = cy;
    		if ( rect->y2 < cy ) rect->y2 = cy;
    	}
    	EXTENTS(rect,r);
    	(rect)++;
    }
}

/***********************************************************
 *     Bop down the array of rects until we have passed
 *     scanline y.  numRects is the size of the array.
 ***********************************************************/

static BOX 
*IndexRects(rects, numRects, y)
    register BOX *rects;
    register int numRects;
    register int y;
{
     while ((numRects--) && (rects->y2 <= y))
        rects++;
     return(rects);
}

XIntersectRegion( reg1, reg2, newReg )
    Region newReg;                	/* destination Region 		*/
    Region reg1, reg2;           	/* source regions     		*/
{
    register BOX *r1Beg, *r2Beg;    	/* current position in ARTs 	*/
    register BOX *ART1End, *ART2End;    /* end of ARTs          	*/
    BOX *rects;                      	/* destination rects       	*/
    int y1, y2;                     	/* current y-bracket        	*/
    register BOX *ART1, *ART2;      	/* active rect tables       	*/
    BOX *r1End, *r2End;             	/* end of rect lists        	*/
    int x1, x2;                       	/* x vals for new rect      	*/
    BOX *FirstRect;                 	/* first rect in rects list 	*/
    int ymax;                       	/* ymax for intersection    	*/
    int oldSize, newSize;
    BOX *prectO, *prectN;		/* following are used in coalescing boxes */
    int irectPrevStart, irectBandStart;
    BOX *prectBandStart;
    int fSame, crects;			   
 
    /* check for trivial reject */
    if ( (!EXTENTCHECK(&reg1->extents, &reg2->extents)) ||
        (!(reg1->numRects)) || (!(reg2->numRects)) )
    {
        newReg->numRects = 0;
        return(1);
    }
 
    oldSize = newReg->size;
    newReg->size = newSize = (MAX(reg1->numRects, reg2->numRects) * 2);  

    rects = (BOX *) Xmalloc ((unsigned) (newSize * sizeof(BOX)));
    FirstRect = rects;

    ymax = MIN(reg1->extents.y2, reg2->extents.y2);
    y1 = MAX(reg1->extents.y1, reg2->extents.y1);

    /*  reset extents of new Region */

    newReg->extents.x1 = MAXSHORT;
    newReg->extents.x2 = MINSHORT; 
    newReg->extents.y1 = MAXSHORT;
    newReg->extents.y2 = MINSHORT;

    ART1 = IndexRects(reg1->rects, reg1->numRects, y1);
    ART2 = IndexRects(reg2->rects, reg2->numRects, y1);
    r1End = reg1->rects + reg1->numRects;
    r2End = reg2->rects + reg2->numRects;

    ART1End = ART1,  ART2End = ART2;

    irectPrevStart = -1; 	 /* initally, there are no previous rects */
    /* for each y bracket -- there is at least one since they overlap */ 
    newReg->numRects = 0;
    do 
    {
	irectBandStart = rects - FirstRect;
        /* find the y bracket */
        y1 = MAX(ART1->y1, ART2->y1);
        ART1 = IndexRects(ART1, MAX(0, r1End - ART1 - 1), y1);
        ART2 = IndexRects(ART2, MAX(0, r2End - ART2 - 1), y1);
        ART1End = ART1,  ART2End = ART2;
        y2 = MIN(ART1->y2, ART2->y2);

        /* add to the active rect tables */
        while ((ART1End < r1End) && (ART1End->y1 <= y1)) ART1End++;
        while ((ART2End < r2End) && (ART2End->y1 <= y1)) ART2End++;

        r1Beg = ART1;
        while (r1Beg != ART1End) 
	{
            r2Beg = ART2;
            while (r2Beg != ART2End) 
            {
                if (((r1Beg->x1 >= r2Beg->x1) && (r1Beg->x1 <= r2Beg->x2)) ||
                    ((r2Beg->x1 >= r1Beg->x1) && (r2Beg->x1 <= r1Beg->x2)))
                {
                    x1 = MAX(r1Beg->x1, r2Beg->x1);
                    x2 = MIN(r2Beg->x2, r1Beg->x2);
                    /*  this is where we used to check about subsuming */
                    MEMCHECK(newReg, rects, FirstRect);
                   ADDRECT(newReg, rects, x1, y1, x2, y2);
		}
                r2Beg++;
	    }
            r1Beg++;
	}
        /* flush the active rect tables if necessary */
        /* but make sure that they do not past end of array */
        if (ART1->y2 == y2)
            ART1 = MIN(ART1End, r1End-1);
        if (ART2->y2 == y2)
            ART2 = MIN(ART2End, r2End-1);

        /* now that we've carefully generated all the boxes for this y-band,	*/
	/* let's see if they can be coalesced with the previous y-band.		*/
	/* this will speed up routines that further manipulate the resulting	*/
	/* region. 								*/
	fSame = FALSE;
	if(irectPrevStart != -1)
	{
	    crects = irectBandStart - irectPrevStart;
	    if(crects == ((rects - FirstRect) - irectBandStart))
	    {
	        prectO = FirstRect + irectPrevStart;
	        prectN = prectBandStart = FirstRect + irectBandStart;
                if (prectO->y2 != prectN->y1)
		   fSame = FALSE;
                else
	        {
		    fSame = TRUE;
	            while(prectO < prectBandStart)
		    {
		        if((prectO->x1 != prectN->x1) || 
		               (prectO->x2 != prectN->x2))
		        {
			      fSame = FALSE;
			      break;
		        }
			prectO++;
			prectN++;
		    }
		}
		if (fSame)
		{
		    prectO = FirstRect + irectPrevStart;
	            prectN = prectBandStart;
		    while(prectO < prectBandStart)
		    {
			(prectO++)->y2 = (prectN++)->y2;
		    }
		    rects -= crects;
		    newReg->numRects -= crects;
		}
	    }
	}
	if(!fSame)
	    irectPrevStart = irectBandStart;
    } while (y2 < ymax);

    /* put together the new Region */ 

    if (newReg->numRects)
    {
        if (oldSize)
            Xfree((char *)newReg->rects);
        FirstRect = (BOX *) Xrealloc ((char *)FirstRect,
			(unsigned) (sizeof(BOX))*(newReg->numRects));
	newReg->size = newReg->numRects;
        newReg->rects = FirstRect;
    }
    else
    {
	newReg->size = oldSize;
	Xfree((char *)FirstRect);
    }
    return(1);
}

void bcopy();
static void
miRegionCopy(dstrgn, rgn)
    register Region dstrgn;
    register Region rgn;
{
    if (dstrgn != rgn)  /*  don't want to copy to itself */
    {  
        if (dstrgn->size < rgn->numRects)
        {
            if (dstrgn->rects)
            {
                dstrgn->rects = (BOX *) Xrealloc((char *)dstrgn->rects, 
                       (unsigned) rgn->numRects * (sizeof(BOX)));
            }
        dstrgn->size = rgn->numRects;
	}
        dstrgn->numRects = rgn->numRects;
        dstrgn->extents.x1 = rgn->extents.x1;
        dstrgn->extents.y1 = rgn->extents.y1;
        dstrgn->extents.x2 = rgn->extents.x2;
        dstrgn->extents.y2 = rgn->extents.y2;

	(void) bcopy((char *)rgn->rects, 
		     (char *) dstrgn->rects, rgn->numRects * sizeof(BOX));   
    }
}

/*
 *  combinRegs(newReg, reg1, reg2)
 *    if one region is above or below the other.
*/ 

static void
combineRegs(newReg, reg1, reg2)
    register Region newReg;
    Region reg1;
    Region reg2;
{
    register Region tempReg;
    register BOX *rects;
    register BOX *rects1;
    register BOX *rects2;
    register int total;

    rects1 = reg1->rects;
    rects2 = reg2->rects;

    total = reg1->numRects + reg2->numRects;
    tempReg = XCreateRegion();
    tempReg->size = total;
    /*  region 1 is below region 2  */
    if (reg1->extents.y1 > reg2->extents.y1)
    {
        miRegionCopy(tempReg, reg2);
        rects = &tempReg->rects[tempReg->numRects];
        total -= tempReg->numRects;
        while (total--)
            *rects++ = *rects1++;
    }
    else
    {
        miRegionCopy(tempReg, reg1);
        rects = &tempReg->rects[tempReg->numRects];
        total -= tempReg->numRects;
        while (total--)
            *rects++ = *rects2++;
    }
    tempReg->extents = reg1->extents;
    tempReg->numRects = reg1->numRects + reg2->numRects;
    EXTENTS(&reg2->extents, tempReg);  
    miRegionCopy(newReg, tempReg);
    Xfree((char *)tempReg);
}

/*
 *  QuickCheck checks to see if it does not have to go through all the
 *  the ugly code for the region call.  It returns 1 if it did all
 *  the work for Union, otherwise 0 - still work to be done.
*/ 

static int
QuickCheck(newReg, reg1, reg2)
    Region newReg, reg1, reg2;
{

    /*  if unioning with itself or no rects to union with  */
    if ( (reg1 == reg2) || (!(reg1->numRects)) )
    {
        miRegionCopy(newReg, reg2);
        return(TRUE);
    }

    /*   if nothing to union   */
    if (!(reg2->numRects))
    {
        miRegionCopy(newReg, reg1);
        return(TRUE);
    }

    /*   could put an extent check to see if add above or below */

    if ((reg1->extents.y1 >= reg2->extents.y2) ||
        (reg2->extents.y1 >= reg1->extents.y2) )
    {
        combineRegs(newReg, reg1, reg2);
        return(TRUE);
    }
    return(FALSE);
}

/*   TopRects(rects, reg1, reg2)
 * N.B. We now assume that reg1 and reg2 intersect.  Therefore we are
 * NOT checking in the two while loops for stepping off the end of the
 * region.  
 */ 

static int
TopRects(newReg, rects, reg1, reg2, FirstRect)
    register Region newReg;
    register BOX *rects;
    register Region reg1;
    register Region reg2; 
    BOX *FirstRect;
{
    register BOX *tempRects;

    /*  need to add some rects from region 1 */
    if (reg1->extents.y1 < reg2->extents.y1)
    {
        tempRects = reg1->rects;
        while(tempRects->y1 < reg2->extents.y1)
	{
	    MEMCHECK(newReg, rects, FirstRect);
            ADDRECTNOX(newReg,rects, tempRects->x1, tempRects->y1, 
		       tempRects->x2, MIN(tempRects->y2, reg2->extents.y1));
            tempRects++;
	}
    }
    /*  need to add some rects from region 2 */
    if (reg2->extents.y1 < reg1->extents.y1)
    {
        tempRects = reg2->rects;
        while (tempRects->y1 < reg1->extents.y1)
        {
            MEMCHECK(newReg, rects, FirstRect);
            ADDRECTNOX(newReg, rects, tempRects->x1,tempRects->y1, 
		       tempRects->x2, MIN(tempRects->y2, reg1->extents.y1));
            tempRects++;
	}
    }
    return(1);
}

XUnionRegion( reg1, reg2, newReg )
    Region newReg;       		/* destination Region 		*/
    Region reg1, reg2;              	/* source regions     		*/
{
    register BOX *r1Beg, *r2Beg;    	/* current position in ARTs 	*/ 
    BOX *ART1End, *ART2End;           	/* end of ARTs            	*/ 
    register BOX *rects;            	/* destination rects        	*/ 
    int y1;                         	/* current y-bracket        	*/  
    int y2;
    register BOX *ART1, *ART2;      	/* active rect tables       	*/ 
    BOX *r1End, *r2End;             	/* end of rect lists        	*/ 
    int x1, x2;                     	/* x vals for new rect      	*/ 
    register BOX *FirstRect;          	/* first rect in rects list	*/ 
    int ymax;                       	/* ymax for intersection    	*/ 
    BOX *pART2, *pART1;
    int prevY;
    int num1, num2;               	/*  size of region1, region 2 	*/ 
    int oldSize, newSize;
    int px1, py1, px2, py2;       	/* previous values            	*/ 
    int tempmin, tempmax;         	/* temp variables for macros 	*/ 
    BOX *prectO, *prectN;	    	/* following are used in coalescing boxes */ 
    int irectPrevStart, irectBandStart;
    BOX *prectBandStart;
    int fSame, crects;			   
    
    /*  checks all the simple cases */

    if (QuickCheck(newReg, reg1, reg2))
    {
        newReg->extents.x1 = MIN(reg1->extents.x1, reg2->extents.x1);
        newReg->extents.y1 = MIN(reg1->extents.y1, reg2->extents.y1);
        newReg->extents.x2 = MAX(reg1->extents.x2, reg2->extents.x2);
        newReg->extents.y2 = MAX(reg1->extents.y2, reg2->extents.y2);
        return(1);
    }
    oldSize = newReg->size;
    newSize = newReg->size = MAX(reg1->numRects, reg2->numRects);
   
    FirstRect = rects = (BOX *)Xmalloc((unsigned) (newSize * sizeof(BOX)));

    ymax = MIN(reg1->extents.y2, reg2->extents.y2);
    y1 = MAX(reg1->extents.y1, reg2->extents.y1);

    /*  add rectangles beteen the mininum y1 extent and maxinum y1 */ 

    num1 = reg1->numRects;
    num2 = reg2->numRects;
    newReg->numRects = 0;
    (void) TopRects(newReg, rects, reg1, reg2, FirstRect);

    /*   find the first end of the bracket        */ 
    ART1 = IndexRects(reg1->rects, num1, y1);
    ART2 = IndexRects(reg2->rects, num2, y1);
    /* what is the address of the last rectangle in region 1 and 2 */ 

    r1End = reg1->rects + num1;    /*  this is a biggie */
    r2End = reg2->rects + num2;

    ART1End = ART1,  ART2End = ART2;

    /* for each y bracket -- there is at least one since they overlap */

    pART1 = ART1;
    pART2 = ART2;
    prevY = y1;
  
    /*  needed for when no intersection at all */
    while ((ART1End < r1End) && (ART1End->y1 <= y1)) 
         ART1End++;
    while ((ART2End < r2End) && (ART2End->y1 <= y1))
        ART2End++;
    irectPrevStart = -1; 	 /* initally, there are no previous rects */
    /* for each y bracket -- there is at least one since they overlap */ 
    do
    {   /* find the y bracket */
        y1 = MAX(ART1->y1, ART2->y1);
        ART1 = IndexRects(ART1, MAX(0, ART1End - ART1 - 1), y1);
        ART2 = IndexRects(ART2, MAX(0, ART2End - ART2 - 1), y1);
        y2 = MIN(ART1->y2, ART2->y2);

       /*  Since it is possible that the brackets are not continuous,     */
       /*  we must add the rectangles that could have been missed because */ 
       /*  there is no intersection.                                      */  
        if ( (y1 > prevY) && (y1 == ART1->y1) )
        {
            while (ART2End != pART2)
            {
                MEMCHECK(newReg, rects, FirstRect);
                tempmax = MAX(prevY, pART2->y1);
                tempmin = MIN(y1, pART2->y2);
                ADDRECTNOX(newReg, rects, pART2->x1, tempmax,pART2->x2, 
			   tempmin);
                pART2++;
            }
        }
        if ( (y1 > prevY) && (y1 == ART2->y1) )
        {
            while (ART1End != pART1)
            {
                MEMCHECK(newReg, rects, FirstRect);
                tempmin = MIN(y1, pART1->y2);
                tempmax = MAX(prevY, pART1->y1);
                ADDRECTNOX(newReg, rects, pART1->x1,tempmax, pART1->x2, 
			   tempmin);
                pART1++;
	    }
	}
        prevY = y2;
        pART2 = ART2;
        pART1 = ART1;
        ART1End = ART1; ART2End = ART2;

        /* add to the active rect tables */
        while ((ART1End < r1End) && (ART1End->y1 <= y1)) 
            ART1End++;
        while ((ART2End < r2End) && (ART2End->y1 <= y1))
            ART2End++;

        /* for each y bracket -- there is at least one since they overlap */
        r1Beg = ART1;
        px1 = py1 = px2 = py2 = 0;
        while (r1Beg != ART1End) 
        {
            irectBandStart = rects - FirstRect;
            r2Beg = ART2;
            while (r2Beg != ART2End) 
	    {
                /*  if they intersect */
                if (((r1Beg->x1 >= r2Beg->x1) && (r1Beg->x1 <= r2Beg->x2)) ||
                    ((r2Beg->x1 >= r1Beg->x1) && (r2Beg->x1 <= r1Beg->x2))) 
                {
                    x1 = MIN(r1Beg->x1, r2Beg->x1);
                    x2 = MAX(r2Beg->x2, r1Beg->x2);
                    /*  check to see if it can subsume */
                    if ((rects!= FirstRect) && ((rects-1)->y1 == y1) &&
                       ((rects-1)->y2 == y2) && ((rects-1)->x1 < x2) && 
                       ((rects-1)->x2 > x1) && ((rects-1)->x2 < x2))
		    {
                        (rects-1)->x2 = x2;
                        newReg->extents.x2 = MAX(newReg->extents.x2, x2);
		    }
                    else
                    {
                        MEMCHECK(newReg, rects, FirstRect);
                        ADDRECTNOX(newReg, rects, x1, y1, x2, y2);
		    }
		}
                else
                {   /*  if there is no intersection */
                    tempmin = MIN(y2, r2Beg->y2);
                    tempmax = MAX(y1, r2Beg->y1);
                    if (r2Beg->x1 < r1Beg->x1)
                    {
                        MEMCHECK(newReg, rects, FirstRect);
                        ADDRECTNOX(newReg, rects, r2Beg->x1, tempmax,
                               r2Beg->x2,tempmin);
                     /*  check to see that was not previously added */
                        if ((px1 != r1Beg->x1) &&
                            (py1 != tempmax) &&
                            (px2 != r1Beg->x1) &&
                            (py2 != tempmin))
                        {
                            MEMCHECK(newReg, rects, FirstRect);
                            ADDRECTNOX(newReg, rects, r1Beg->x1, tempmax,
                                   r1Beg->x2, tempmin);
                            px1 = r1Beg->x1;
                            py1 = tempmax;
                            px2 = r1Beg->x1;
                            py2 = tempmin;
			}
		    }
                    else 
                    {
                        /*  check to see that was not previously added */ 
                        if ((px1 != r1Beg->x1) &&
                            (py1 != tempmax) &&
                            (px2 != r1Beg->x1) &&
                            (py2 != tempmin))
                        {
                            MEMCHECK(newReg, rects, FirstRect);
                            ADDRECTNOX(newReg, rects, r1Beg->x1, tempmax,
                                   r1Beg->x2, tempmin);
                            px1 = r1Beg->x1;
                            py1 = tempmax;
                            px2 = r1Beg->x1;
                            py2 = tempmin;
			}
                        MEMCHECK(newReg, rects, FirstRect);
                        ADDRECTNOX(newReg, rects, r2Beg->x1, tempmax,
                            r2Beg->x2, tempmin);
		    }
		}
                r2Beg++; 
	    }   /* while */ 
             r1Beg++;
	}  /* while */
        /* flush the active rect tables if necessary */
	if (ART1->y2 == y2)
           ART1 = MIN(ART1End, r1End -1);
        if (ART2->y2 == y2)
           ART2 = MIN(ART2End, r2End -1);

        /* now that we've carefully generated all the boxes for this y-band,
	 * let's see if they can be coalesced with the previous y-band.
	 * this will speed up routines that further manipulate the resulting
	 * region. */
	fSame = FALSE;
	if(irectPrevStart != -1)
	{
	    crects = irectBandStart - irectPrevStart;
	    if(crects == ((rects - FirstRect) - irectBandStart))
	    {
	        prectO = FirstRect + irectPrevStart;
	        prectN = prectBandStart = FirstRect + irectBandStart;
                if (prectO->y2 != prectN->y1)
		   fSame = FALSE;
                else
	        {
		    fSame = TRUE;
	            while(prectO < prectBandStart)
		    {
		        if((prectO->x1 != prectN->x1) || 
		               (prectO->x2 != prectN->x2))
		        {
			      fSame = FALSE;
			      break;
		        }
			prectO++;
			prectN++;
		    }
		}
		if (fSame)
		{
		    prectO = FirstRect + irectPrevStart;
	            prectN = prectBandStart;
		    while(prectO < prectBandStart)
		    {
			(prectO++)->y2 = (prectN++)->y2;
		    }
		    rects -= crects;
		    newReg->numRects -= crects;
		}
	    }
	}
	if(!fSame)
	    irectPrevStart = irectBandStart;

    }  while (y2 < ymax);
            /*  add the rectangles that are below region 2's extents */ 
    if (ymax != reg1->extents.y2)
    {
        while (pART1 != r1End)
        {
            MEMCHECK(newReg, rects, FirstRect);
            tempmax = MAX(ymax, pART1->y1);
            ADDRECTNOX(newReg, rects, pART1->x1, tempmax, pART1->x2, 
		       pART1->y2);
            pART1++;
	}
    }
    else 
    {
        while (pART2 < r2End)
        {
            MEMCHECK(newReg, rects, FirstRect);
            tempmax = MAX(ymax, pART2->y1);
            ADDRECTNOX(newReg, rects, pART2->x1, tempmax, pART2->x2,  
		       pART2->y2);
            pART2++;
	}
    }
             /* put together the new Region */
    rects = (BOX *)Xrealloc((char *)FirstRect, 
			    (unsigned) (sizeof(BOX) * newReg->numRects));
    newReg->extents.x1 = MIN(reg1->extents.x1, reg2->extents.x1);
    newReg->extents.y1 = MIN(reg1->extents.y1, reg2->extents.y1);
    newReg->extents.x2 = MAX(reg1->extents.x2, reg2->extents.x2);
    newReg->extents.y2 = MAX(reg1->extents.y2, reg2->extents.y2);
    newReg->size = newReg->numRects; 
    if (oldSize)
        Xfree((char *)newReg->rects); 
    newReg->rects = rects;
    return(1);
}

/* Subtract regS from regM and puts result in regD --
 * M is for minuend, S for subtrahend, D for difference
 */ 
XSubtractRegion( regM, regS, regD )
    Region regD;               
    Region regM, regS;          
{
    register BOX *artM, *artS;		/* active rect tables       	*/ 
    register BOX *artMEnd, *artSEnd;    /* end of ARTs       	        */ 
    register BOX *rMBeg, *rSBeg;    	/* current position in ARTs	*/ 
    BOX *rMEnd, *rSEnd;             	/* end of rect lists        	*/ 
    BOX *rects;                     	/* destination rects        	*/  
    int y1, y2;                     	/* current y-bracket        	*/ 
    int x1, x2;                     	/* x vals for new rect      	*/ 
    int ymax;                       	/* ymax for intersection    	*/ 
    BOX *FirstRect;                 	/* first rect in rects list 	*/ 
    BOX	*tempRect;		    	/* temporary pointer to a box 	*/
    int oldSize, oldy1, oldy2, oldMSize;

    BOX *prectO, *prectN;	    /* these are used in coalescing boxes */ 
    int irectPrevStart, irectBandStart;
    BOX *prectBandStart;
    int fSame, crects;			   
 
    /* check for trivial reject */ 
    if ( (!EXTENTCHECK(&regM->extents, &regS->extents)) ||
        (!(regM->numRects)) || (!(regS->numRects)) )
    {
	miRegionCopy(regD, regM);
        return(1);
    }
 
    oldSize = regD->size;
    oldy1 = regM->extents.y1;
    oldy2 = regM->extents.y2;
    oldMSize = regM->size;
    regD->size =  (MAX(regM->numRects, regS->numRects) * 2);  

    rects = (BOX *) Xmalloc ((unsigned) (regD->size * sizeof(BOX)));
    FirstRect = rects;

    ymax = MIN(regM->extents.y2, regS->extents.y2);
    y1 = MAX(regM->extents.y1, regS->extents.y1);

    /* setting y2 = y1 lets first band be treated like others */
    y2 = y1;

    /*  reset extents of new Region */
    regD->extents.x1 = MAXSHORT;
    regD->extents.x2 = MINSHORT; 
    regD->extents.y1 = MAXSHORT;
    regD->extents.y2 = MINSHORT;


    artM = IndexRects(regM->rects, regM->numRects, y1);
    artS = IndexRects(regS->rects, regS->numRects, y1);
    rMEnd = regM->rects + regM->numRects;
    rSEnd = regS->rects + regS->numRects;
    regD->numRects = 0;

    /* Add rectangles from regM before intersection */
    if (oldy1 < y1) 
    {
	tempRect = regM->rects;
	while((tempRect < rMEnd) && (tempRect->y1 < y1))
	{
	    MEMCHECK(regD, rects, FirstRect);
	    ADDRECT(regD, rects, tempRect->x1, tempRect->y1, tempRect->x2,
		MIN(tempRect->y2, y1));
	    tempRect++;
	}
    }

    artMEnd = artM,  artSEnd = artS;

    irectPrevStart = -1; 	 /* initally, there are no previous rects */
    /* for each y bracket do the following */
    /* -- there is at least one y bracket since the regions  overlap */
    do 
    {
        /* find the y bracket */
        y1 = MAX(artM->y1, artS->y1);
	/* add any parts of the minuend that fall between y-brackets */
	if(y1 != artM->y1)
	{
	    tempRect = artM;
	    while((tempRect < rMEnd) && (tempRect->y1 < y1))
	    {
		MEMCHECK(regD, rects, FirstRect);
		ADDRECT(regD, rects, tempRect->x1, MAX(tempRect->y1, y2),
		        tempRect->x2, MIN(tempRect->y2, y1));
		tempRect++;
	    }
	}

        artM = IndexRects(artM, MAX(0, rMEnd - artM - 1), y1);
        artS = IndexRects(artS, MAX(0, rSEnd - artS - 1), y1);
        artMEnd = artM,  artSEnd = artS;
        y2 = MIN(artM->y2, artS->y2);
        /* add to the active rect tables */
        while ((artMEnd < rMEnd) && (artMEnd->y1 <= y1))
	    artMEnd++;
        while ((artSEnd < rSEnd) && (artSEnd->y1 <= y1))
	    artSEnd++;

        rMBeg = artM;
        while (rMBeg != artMEnd) 
	{
            irectBandStart = rects - FirstRect;
            rSBeg = artS;
	    x1 = rMBeg->x1;
            while (rSBeg != artSEnd) 
            {
		if(rSBeg->x2 <= rMBeg->x1)
		{
		    /* subtrahend entirely to left of minuend, it's not */
		    /* interesting */
		    rSBeg++;
		    continue;
		}

		if(rSBeg->x1 <= x1)
		{
		    /* subtrahend begins before or when minuend begins 
		     * left part of this minuend is subtracted out, but 
		     * keep looking */
		    x1 = x2 = rSBeg->x2;
		    if(x2 >= rMBeg->x2)
		    {
			/* subtrahend rectangle cross all of this minuend
			 * none of minuend between y1 and rSBeg->y2 could
			 * be included
			 */ 
			break;
		    }

		}
		else if (rSBeg->x1 < rMBeg->x2)
		{
		    /* left part of this is in difference. Include it, then
		     * skip over rest of this subtrahend */
		    x2 = rSBeg->x1;
                    MEMCHECK(regD, rects, FirstRect);
                    ADDRECT(regD, rects, x1, y1, x2, y2);
		    x1 = x2 = rSBeg->x2;
		}
		else
		{
		    x2 = rMBeg->x2;
                    MEMCHECK(regD, rects, FirstRect);
                    ADDRECT(regD, rects, x1, y1, x2, y2);
		    /* All the remaining artS rects must be to the right 
		     * of rMBeg so we can skip looking at them */
		    break;  
		} 
		    
                rSBeg++;
	    }

	    if(rMBeg->x2 > x1)
	    {
		MEMCHECK(regD, rects, FirstRect);
		ADDRECT(regD, rects, x1, y1, rMBeg->x2, y2);
	    }
            rMBeg++;
	}
        /* flush the active rect tables if necessary */
        /* but make sure that they do not past end of array */
        if (artM->y2 == y2)
            artM = MIN(artMEnd, rMEnd-1);
        if (artS->y2 == y2)
            artS = MIN(artSEnd, rSEnd-1);

        /* now that we've carefully generated all the boxes for this y-band,
	 * let's see if they can be coalesced with the previous y-band.
	 * this will speed up routines that further manipulate the resulting
	 * region. */ 
	fSame = FALSE;
	if(irectPrevStart != -1)
	{
	    crects = irectBandStart - irectPrevStart;
	    if(crects == ((rects - FirstRect) - irectBandStart))
	    {
	        prectO = FirstRect + irectPrevStart;
	        prectN = prectBandStart = FirstRect + irectBandStart;
                if (prectO->y2 != prectN->y1)
		   fSame = FALSE;
                else
	        {
		    fSame = TRUE;
	            while(prectO < prectBandStart)
		    {
		        if((prectO->x1 != prectN->x1) || 
		               (prectO->x2 != prectN->x2))
		        {
			      fSame = FALSE;
			      break;
		        }
			prectO++;
			prectN++;
		    }
		}
		if (fSame)
		{
		    prectO = FirstRect + irectPrevStart;
	            prectN = prectBandStart;
		    while(prectO < prectBandStart)
		    {
			(prectO++)->y2 = (prectN++)->y2;
		    }
		    rects -= crects;
		    regD->numRects -= crects;
		}
	    }
	}
	if(!fSame)
	    irectPrevStart = irectBandStart;

    } while (y2 < ymax);

    /* Add rectangles from regM after the intersection */
    if (oldy2 > y2) 
    {
        tempRect = IndexRects(artM, oldMSize, y2); 
	while ((tempRect < rMEnd) && (tempRect->y2 <= oldy2))
	{
	    MEMCHECK(regD, rects, FirstRect);
	    ADDRECT(regD, rects, tempRect->x1, MAX(tempRect->y1, ymax),
	            tempRect->x2, tempRect->y2);
	    tempRect++;
	}
    }

    /* put together the new Region */

    if (regD->numRects)
    {
        if (oldSize)
            Xfree((char *)regD->rects);
        FirstRect = (BOX *) Xrealloc ((char *)FirstRect,
			(unsigned) (sizeof(BOX))*(regD->numRects));
	regD->size = regD->numRects;
        regD->rects = FirstRect;
    }
    else
    {
	regD->size = oldSize;
	Xfree((char *)FirstRect);
    }
    return(1);
}

XXorRegion( sra, srb, dr )
    Region sra, srb, dr;
{
    Region tra, trb;

    tra = (Region)Xmalloc((unsigned) sizeof(REGION));
    trb = (Region)Xmalloc((unsigned) sizeof(REGION));
    (void) XSubtractRegion(sra,srb,tra);
    (void) XSubtractRegion(srb,sra,trb);
    (void) XUnionRegion(tra,trb,dr);
}

/*
 * Check to see if the region is empty.  Assumes a region is passed 
 * as a parameter
 */
int 
XEmptyRegion( r )
    Region r;
{
    if( r->size == 0 && r->numRects == 0 && r->extents.x1 == 0 && 
    	r->extents.x2 == 0 && r->extents.y1 == 0 && r->extents.y2 ==0 )
    return( TRUE );
    else  return( FALSE );
}

/*
 *	Check to see if two regions are equal	
 */
int 
XEqualRegion( r1, r2 )
    Region r1, r2;
{
    int i;

    if ( r1->size != r2->size ) return( FALSE );
    else if( r1->numRects != r2->numRects ) return( FALSE );
    else if ( r1->extents.x1 != r2->extents.x1 ) return( FALSE );
    else if ( r1->extents.x2 != r2->extents.x2 ) return( FALSE );
    else if ( r1->extents.y1 != r2->extents.y1 ) return( FALSE );
    else if ( r1->extents.y2 != r2->extents.y2 ) return( FALSE );
    else for( i=0; i < r1->numRects; i++ ) {
    	if ( r1->rects[i].x1 != r2->rects[i].x1 ) return( FALSE );
    	else if ( r1->rects[i].x2 != r2->rects[i].x2 ) return( FALSE );
    	else if ( r1->rects[i].y1 != r2->rects[i].y1 ) return( FALSE );
    	else if ( r1->rects[i].y2 != r2->rects[i].y2 ) return( FALSE );
    }
    return( TRUE );
}

int 
XPointInRegion( pRegion, x, y )
    Region pRegion;
    int x, y;
{
    int i;

    if (pRegion->numRects == 0)
        return(FALSE);
    if (!INBOX(pRegion->extents, x, y))
        return(FALSE);
    for (i=0; i<pRegion->numRects; i++)
    {
        if (INBOX (pRegion->rects[i], x, y))
	    return(TRUE);
    }
    return(FALSE);
}

int 
XRectInRegion( region, rx, ry, rwidth, rheight )
    Region region;
    int rx, ry;
    unsigned int rwidth, rheight;
{
    BOX *rect;
    register int flag;
    register int startX;
    int endX;
    register int x;
    register int startY;
    int endY;
    register int y;
    register BOX *r;
    register BOX *rEnd;
    int partflag = 0;

    rect = (BOX *)Xmalloc((unsigned) sizeof(BOX));
    rect->x1 = rx;
    rect->y1 = ry;
    rect->x2 = rwidth + rx;
    rect->y2 = rheight + ry;

    if (!EXTENTCHECK(&region->extents, rect))
        return(rgnOUT);
    if (region->numRects == 1)
    {
        if ( (region->extents.y1 <= rect->y1) &&
             (region->extents.x1 <= rect->x1) &&
             (region->extents.y2 >= rect->y2) &&
             (region->extents.x2 >= rect->x2) )
              return(rgnIN);
         else
              return(rgnPART);
    }
    if ((region->extents.x1 > rect->x1) && (region->extents.x2 < rect->x2) ||
        (region->extents.y1 > rect->y1) && (region->extents.y2 < rect->y2))
        return(rgnPART);
    startX = rect->x1;
    startY = rect->y1;
    endX = rect->x2;
    endY = rect->y2;
    x = startX;
    y = startY;

    r = region->rects;
    rEnd = region->rects + region->numRects;

    while ( (r < rEnd) && (r->y2 < startY) ) 
        r++;      
    if (r != region->rects)
        r--;
    if (r == rEnd)
       return(rgnOUT);
    y = r->y2;
    flag = 1;
    while (flag) 
    {
	if (r->y2 > y)
        {
	    if ((x < endX) || (r->y1 != y))
	        flag = 0;
	    else
            {
		x = startX;
		y = r->y2;
            }
	}
	if ((r->x1 <= x) && (r->x2 >= x))
	{
	    partflag = 1;
	    x = r->x2;
	}
        else
        {
	    if ((r->x1 <= endX) && (r->x1 >= startX))
	    {
		partflag = 1;
		flag = 0;
	    }
	}
	r++; 
	if ((r >= rEnd) || (r->y1 >= endY))
            flag = 0;
    }
    if ((y >= endY) && (x >= endX))
        return(rgnIN);
    if ((x == startX) && (partflag != 1))
	return(rgnOUT);
    return(rgnPART);

}
