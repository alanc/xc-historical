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
/* $Header: miregion.c,v 1.27 87/08/19 19:38:59 swick Locked $ */

#include "miscstruct.h"
#include "regionstr.h"

typedef BoxRec BOX;	/* this is to cut down on some gratuitous edits */

/*  1: if two BOXs overlap.
/*  0: if two BOXs do not overlap.
 *  Remember, x2 and y2 are not in the region 
 */
#define EXTENTCHECK(r1, r2) \
      (!( ((r1)->x2 <= (r2)->x1)  || \
        ( ((r1)->x1 >= (r2)->x2)) || \
        ( ((r1)->y2 <= (r2)->y1)) || \
        ( ((r1)->y1 >= (r2)->y2)) ) )

/*  add a rectangle to the given Region */
#define ADDRECTNOX(reg, r, rx1, ry1, rx2, ry2){\
            if ((rx1 < rx2) && (ry1 < ry2) && \
                CHECK_PREVIOUS((reg), (r), (rx1), (ry1), (rx2), (ry2))){\
              if (currentY != ry1) \
	      {\
                  currentY = ry1;\
		  irectPrevStart = irectBandStart;\
		  irectBandStart = r - FirstRect;\
		  if (irectPrevStart == irectBandStart)\
	              irectPrevStart -= 1;\
	      }\
              (r)->x1 = (rx1);\
              (r)->y1 = (ry1);\
              (r)->x2 = (rx2);\
              (r)->y2 = (ry2);\
              (reg)->numRects++;\
              (r)++;\
            }\
        }

void
miprintRects(rgn)
    RegionPtr rgn;
{
    register int i;

    ErrorF(  "num: %d size: %d \n", rgn->numRects, rgn->size);
    ErrorF(  "   extents: %d %d %d %d\n",rgn->extents.x1, 
	    rgn->extents.y1, rgn->extents.x2, rgn->extents.y2);
    for (i = 0; i < rgn->numRects; i++)
      ErrorF(  "%d %d %d %d \n", rgn->rects[i].x1,rgn->rects[i].y1,
	      rgn->rects[i].x2,rgn->rects[i].y2);
    ErrorF(  "\n");
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
     if (numRects < 0)
         ErrorF(  "THERE IS A BIG BIG PROBLEM in INDEXRECTS: %d \n", 
		 numRects);
     while ((numRects--) && (rects->y2 <= y))
        rects++;
     return(rects);
}


/*****************************************************************
 *   RegionCreate(rect, size)
 *     This routine does a simple malloc to make a structure of
 *     REGION of "size" number of rectangles.
 *****************************************************************/

RegionPtr
miRegionCreate(rect, size)
    register BOX *rect;
    register int size;
{
    register RegionPtr temp;       /*   new region  */
   
    size = max(1, size);
    temp = (RegionPtr ) Xalloc (sizeof (RegionRec));
    temp->rects = (BOX *) Xalloc (size * (sizeof(BOX)));
    if (rect == (BOX *)NULL)
    {
        temp->numRects = 0;
        temp->extents.x1 = 0;
        temp->extents.y1 = 0;
        temp->extents.x2 = 0;
        temp->extents.y2 = 0;
    }
    else
    {
        temp->extents = *rect;
        temp->rects[0] = *rect;
        temp->numRects = 1;
    }
    temp->size = size;
    return(temp);
}


void
miRegionCopy(dstrgn, rgn)
    register RegionPtr dstrgn;
    register RegionPtr rgn;

{
    if (dstrgn != rgn) /*  don't want to copy to itself */
    {  
        if (dstrgn->size < rgn->numRects)
        {
            if (dstrgn->rects)
            {
                dstrgn->rects = (BOX *) Xrealloc(dstrgn->rects, 
                                 rgn->numRects * (sizeof(BOX)));
            }
	    else
		ErrorF(  "RC HORRIBLE ERROR...\n");
            dstrgn->size = rgn->numRects;
	}
        dstrgn->numRects = rgn->numRects;
        dstrgn->extents.x1 = rgn->extents.x1;
        dstrgn->extents.y1 = rgn->extents.y1;
        dstrgn->extents.x2 = rgn->extents.x2;
        dstrgn->extents.y2 = rgn->extents.y2;

	bcopy(rgn->rects, dstrgn->rects, rgn->numRects * sizeof(BOX));   
    }
}


/*
 *    Inveerse takes a REGION and a BOX.  It returns a 
 *    REGION that is everything that is in the BOX but NOT
 *    the REGION.
 *    
 */

int 
miInverse(newReg, reg1, invRect)
    RegionPtr newReg;             /* destination Region */
    RegionPtr reg1;               /* destination Region */
    BOX *invRect;
{
    register BOX *r1Beg;           /* current position in ARTs */
    register BOX *ART1End;         /* end of ARTs          */
    register BOX *rects;           /* destination rects        */
    register int y1, y2;            /* current y-bracket        */
    register BOX *ART1;            /* active rect tables       */
    BOX *r1End;                    /* end of rect lists        */
    BOX *FirstRect;                /* first rect in rects list */
    int minX, maxX;
    int yprev;
    int openX;
    int ymax;
    int size;
    int oldsize;
    int tempmin;
 
    if ((reg1->numRects == 0) || !(EXTENTCHECK(&(reg1->extents), invRect)))
    {
        newReg->extents = *invRect;
	bcopy(invRect, newReg->rects, sizeof(BOX));
        newReg->numRects = 1;
        return(1);
    }
    oldsize = newReg->size;
    newReg->size = size = reg1->numRects * 2;
    FirstRect = rects = (BOX *) Xalloc (sizeof(BOX) * size);
    minX = invRect->x1;
    maxX = invRect->x2;

    ymax = reg1->extents.y2;
    y1 = invRect->y1;

                   /*  reset extents of new Region */

    newReg->extents.x1 = newReg->extents.y1 = MAXSHORT;
    newReg->extents.y2 = newReg->extents.x2 = MINSHORT;

    ART1 = IndexRects(reg1->rects, reg1->numRects, y1);

    r1End = reg1->rects + reg1->numRects;

    ART1End = ART1;

    yprev = invRect->y1;

         /* for each y bracket -- there is at least one since they overlap */

    newReg->numRects = 0;
    do 
    {
        y1 = max(ART1->y1, invRect->y1);
        y2 = ART1->y2;
        if (y1 > yprev)
        {
            MEMCHECK(newReg, rects, FirstRect);
            ADDRECT(newReg, rects, minX, yprev, maxX, y1);
        }
        while ((ART1End < r1End) && (ART1End->y1 <= y1)) ART1End++;

        r1Beg = ART1;
        openX = minX;
        while ( (r1Beg != ART1End) && (openX < maxX))
        {
            if ( (r1Beg->x1 > openX) && (r1Beg->x1 <  maxX) )
            {
                MEMCHECK(newReg, rects, FirstRect);
                ADDRECT(newReg, rects, openX, y1, r1Beg->x1, y2 );
            }
            openX = r1Beg->x2;
            r1Beg++;
	}
        if (openX < maxX)
        {
            MEMCHECK(newReg, rects, FirstRect);
            tempmin = min(y2, invRect->y2);
            ADDRECT(newReg, rects, openX, y1, maxX, tempmin);
        }
        yprev = y2;
        ART1 = ART1End;
        }   while ( (ymax != y2) && (y2 < invRect->y2) && (ART1 != r1End) );

    if (ymax < invRect->y2)
    {
        MEMCHECK(newReg, rects, FirstRect);
        ADDRECT(newReg, rects, minX, ymax, maxX, invRect->y2);
    }

   /* put together the new Region  */

    newReg->numRects = rects - FirstRect;
    if (newReg->numRects)
    {
       if (oldsize)
           Xfree(newReg->rects);
/*
       newReg->size = rects - FirstRect;
       rects = (BOX *)Xrealloc(FirstRect, sizeof(BOX) * newReg->numRects);
       newReg->rects = rects;
*/
       newReg->rects = FirstRect;
    }
    else
    {
	newReg->extents.x1 = newReg->extents.y1 = 0;
	newReg->extents.x2 = newReg->extents.y2 = 0;
	newReg->numRects = 0;
	newReg->size = oldsize;
	Xfree(FirstRect);
    }
    return(1);
}

int 
miIntersect(newReg, reg1, reg2)
    RegionPtr newReg;               /* destination Region */
    RegionPtr reg1, reg2;          /* source regions     */
{
    register BOX *r1Beg, *r2Beg;   /* current position in ARTs */
    register BOX *ART1End, *ART2End;   /* end of ARTs          */
    BOX *rects;                    /* destination rects        */
    int y1, y2;                    /* current y-bracket        */
    register BOX *ART1, *ART2;     /* active rect tables       */
    BOX *r1End, *r2End;            /* end of rect lists        */
    int x1, x2;                    /* x vals for new rect      */
    BOX *FirstRect;                /* first rect in rects list */
    int ymax;                      /* ymax for intersection    */
    int oldSize, newSize;
    BOX *prectO, *prectN;	   /* following are used in coalescing boxes */
    int irectPrevStart, irectBandStart;
    BOX *prectBandStart;
    int fSame, crects;			   
 
   /* check for trivial reject */
    if ( (!(reg1->numRects)) || (!(reg2->numRects))  ||
	(!EXTENTCHECK(&reg1->extents, &reg2->extents)))
    {
        newReg->numRects = 0;
        return(1);
    }
 
    oldSize = newReg->size;
    newReg->size = newSize = 4;

    rects = (BOX *) Xalloc (newSize * sizeof(BOX));
    FirstRect = rects;

    ymax = min(reg1->extents.y2, reg2->extents.y2);
    y1 = max(reg1->extents.y1, reg2->extents.y1);

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

    irectPrevStart = -1; 	/* initally, there are no previous rects */
       /* for each y bracket -- there is at least one since they overlap */
    newReg->numRects = 0;
    do 
    {
	irectBandStart = rects - FirstRect;
             /* find the y bracket */
        y1 = max(ART1->y1, ART2->y1);
        ART1 = IndexRects(ART1, max(0, r1End - ART1 - 1), y1);
        ART2 = IndexRects(ART2, max(0, r2End - ART2 - 1), y1);
        ART1End = ART1,  ART2End = ART2;
        y2 = min(ART1->y2, ART2->y2);

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
                    x1 = max(r1Beg->x1, r2Beg->x1);
                    x2 = min(r2Beg->x2, r1Beg->x2);
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
            ART1 = min(ART1End, r1End-1);
        if (ART2->y2 == y2)
            ART2 = min(ART2End, r2End-1);

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
    } while (y2 < ymax);

   /* put together the new Region */

    if (newReg->numRects)
    {
        if (oldSize)
            Xfree((char *)newReg->rects);
/*
        FirstRect = (BOX *) Xrealloc (FirstRect,
				     (sizeof(BOX))*(newReg->numRects));
	newReg->size = newReg->numRects;
        newReg->rects = FirstRect;
*/
	newReg->rects = FirstRect;
    }
    else
    {
	newReg->size = oldSize;
	Xfree(FirstRect);
    }
    return(1);
}


/*
 *   RectIn(region, rect)
 *   This routine takes a pointer to a region and a pointer to a box
 *   and determines if the box is outside/inside/partly inside the region.
 *   Code courtesy of Robert Scheifler.
*/

int 
miRectIn(region, prect)
    register RegionPtr	region;
    register BoxPtr 	prect;
{
    register short x;
    register short y;
    register BoxPtr pbox;
    register BoxPtr pboxEnd;
    int      partIn, partOut;

    /* this is (just) a useful optimization */
    if ((region->numRects == 0) || !EXTENTCHECK(&region->extents, prect))
        return(rgnOUT);

    partOut = FALSE;
    partIn = FALSE;

    /* (x,y) starts at upper left of rect, moving to the right and down */
    x = prect->x1;
    y = prect->y1;

    /* can stop when both partOut and partIn are TRUE, or we reach prect->y2 */
    for (pbox = region->rects, pboxEnd = pbox + region->numRects;
	 pbox < pboxEnd;
	 pbox++)
    {

	if (pbox->y2 <= y)
	   continue;	/* getting up to speed or skipping remainder of band */

	if (pbox->y1 > y)
	{
	   partOut = TRUE;	/* missed part of rectangle above */
	   if (partIn || (pbox->y1 >= prect->y2))
	      break;
	   y = pbox->y1;	/* x guaranteed to be == prect->x1 */
	}

	if (pbox->x2 <= x)
	   continue;		/* not far enough over yet */

	if (pbox->x1 > x)
	{
	   partOut = TRUE;	/* missed part of rectangle to left */
	   if (partIn)
	      break;
	}

	if (pbox->x1 < prect->x2)
	{
	    partIn = TRUE;	/* definitely overlap */
	    if (partOut)
	       break;
	}

	if (pbox->x2 >= prect->x2)
	{
	   y = pbox->y2;	/* finished with this band */
	   if (y >= prect->y2)
	      break;
	   x = prect->x1;	/* reset x out to left again */
	} else
	   x = pbox->x2;	/* eat as much of corner as we found */
    }

    return(partIn ? ((y < prect->y2) ? rgnPART : rgnIN) : rgnOUT);
}


/************************************************************
 *
 *     Written by Michael L. Bidun && Brian Kelleher; Nov 1985
 *     Butchered by Todd Newman; Sept 1986
 *   attempted to make faster by making  FirstRect a register variable.
 *****************************************************************/

int 
miUnion(newReg, reg1, reg2)
    RegionPtr newReg;                  /* destination Region */
    RegionPtr reg1, reg2;             /* source regions     */
{
    register BOX *r1Beg, *r2Beg;   /* current position in ARTs */
    BOX *ART1End, *ART2End;        /* end of ARTs              */
    register BOX *rects;                   /* destination rects        */
    int y1;                        /* current y-bracket        */
    int y2;
    register BOX *ART1, *ART2;     /* active rect tables       */
    BOX *r1End, *r2End;            /* end of rect lists        */
    int x1, x2;                    /* x vals for new rect      */
    register BOX *FirstRect;       /* first rect in rects list */
    int ymax;                      /* ymax for intersection    */
    BOX *pART2, *pART1;
    int prevY, prevX;
    int num1, num2;              /*  size of region1, region 2 */
    int oldSize, newSize;
    int tempmin, tempmax;        /* temp variables for macros */
    BOX *prectO, *prectN;	   /* following are used in coalescing boxes */
    int irectPrevStart, irectBandStart;
    BOX *prectBandStart;
    int fSame, crects;			   
    int currentY;
    
   /*  checks all the simple cases */

    if ( (reg1 == reg2) || (!(reg1->numRects)) )
    {
        if (newReg != reg2)
            miRegionCopy(newReg, reg2);
        return(1);
    }

       /*   if nothing to union   */
    if (!(reg2->numRects))
    {
        if (newReg != reg1)
            miRegionCopy(newReg, reg1);
        return(1);
    }

    if ((reg1->numRects == 1) && 
	(reg1->extents.x1 <= reg2->extents.x1) &&
	(reg1->extents.y1 <= reg2->extents.y1) &&
	(reg1->extents.x2 >= reg2->extents.x2) &&
	(reg1->extents.y2 >= reg2->extents.y2))
    {
        if (newReg != reg1)
            miRegionCopy(newReg, reg1);
        return(TRUE);
    }
    if ((reg2->numRects == 1) && 
	(reg2->extents.x1 <= reg1->extents.x1) &&
	(reg2->extents.y1 <= reg1->extents.y1) &&
	(reg2->extents.x2 >= reg1->extents.x2) &&
	(reg2->extents.y2 >= reg1->extents.y2))
    {
        if (newReg != reg2)
            miRegionCopy(newReg, reg2);
        return(TRUE);
    }
    oldSize = newReg->size;
    newSize = newReg->size = max(reg1->numRects, reg2->numRects);

    FirstRect = rects = (BOX *)Xalloc(newSize * sizeof(BOX));

    ymax = min(reg1->extents.y2, reg2->extents.y2);

        /*  add rectangles beteen the mininum y1 extent and maxinum y1 */

    num1 = reg1->numRects;
    num2 = reg2->numRects;
    newReg->numRects = 0;

    currentY = -1;
    irectPrevStart = -1; 	/* initally, there are no previous rects */
    irectBandStart = -1;

       /*  what is the address of the last rectangle in region 1 and 2 */
    r1End = reg1->rects + num1;   /*  this is a biggie */
    r2End = reg2->rects + num2;

    /***************** Top rects */

       /*
        * If region 1 extends above region2, then add any rectangles
        * from region 1 which are completely above region 2. Since the
        * rectangles are banded, we can stop as soon as we find one that
        * extends into region 2...
        */

    if (reg1->extents.y1 < reg2->extents.y1)
    {
        y1 = reg2->extents.y1;
        ART1 = reg1->rects;
        while ((ART1 < r1End) &&
	       (ART1->y1 < y1) &&
	       (ART1->y2 <= y1))
	{
	    MEMCHECK(newReg, (rects), FirstRect);
            ADDRECTNOX(newReg,(rects), ART1->x1, ART1->y1,ART1->x2, ART1->y2);
            ART1++;
	}
       /*   find the first end of the bracket        */
        ART2 = reg2->rects;

    }
        /*  
        * Similarly for region 2 extending above region 1
        */
    else if (reg2->extents.y1 < reg1->extents.y1)
    {
        y1 = reg1->extents.y1;
        ART2 = reg2->rects;
        while ((ART2 < r2End) &&
	       (ART2->y1 < y1) &&
	       (ART2->y2 <= y1))
        {
            MEMCHECK(newReg, (rects), FirstRect);
            ADDRECTNOX(newReg, (rects), ART2->x1,ART2->y1, 
		       ART2->x2, ART2->y2);
            ART2++;
	}
        ART1 = reg1->rects;
    }
    else   /* they overlap immediately */
    {
        ART1 = reg1->rects;
        ART2 = reg2->rects;
    }

    /*************** end top rects */

       /*
        * If we exhausted one of the regions, go add the rest of the
        * other region
        */
 
    if ((ART1 == r1End) || (ART2 == r2End))
        goto AddEnd;

     /* for each y bracket -- there is at least one since they overlap */

    y1 = min(ART1->y1, ART2->y1);
    ART1End = ART1,  ART2End = ART2;

    pART1 = ART1;
    pART2 = ART2;

    prevY = y1;
    prevX = -1;  
        /*  needed for when no intersection at all */
    while ((ART1End < r1End) && (ART1End->y1 <= y1)) 
         ART1End++;
    while ((ART2End < r2End) && (ART2End->y1 <= y1))
        ART2End++;

    do
    { 
        y1 = max(ART1->y1, ART2->y1);
        ART1 = IndexRects(ART1, max(0, r1End - ART1 - 1), y1);
        ART2 = IndexRects(ART2, max(0, r2End - ART2 - 1), y1);
        y2 = min(pART1->y2, pART2->y2);

      /* 
       * Due to coalescing, we might not overlap at this point.
       * Check this out first.  Want to try coalescing again too.
       */

        if (pART1->y2 < ART2->y1)
        {
            while ((pART1->y2 < ART2->y1) && (pART1 != r1End))
            {
                 MEMCHECK(newReg, rects, FirstRect);
		 tempmin = max(prevY, pART1->y1);
                 ADDRECTNOX(newReg, rects, pART1->x1, tempmin,
                             pART1->x2, pART1->y2);
		 prevX = pART1->x2;
                 pART1++;
            }             
            ART1 = pART1;
            goto Coalesce;
        }
        if (pART2->y2 < ART1->y1) 
        {
            while ((pART2->y2 < ART1->y1)&& (pART2 != r2End))
            {
                 MEMCHECK(newReg, rects, FirstRect);
		 tempmin = max(prevY, pART2->y1);
                 ADDRECTNOX(newReg, rects, pART2->x1, tempmin,
                             pART2->x2, pART2->y2);
		 prevX = pART2->x2;
                 pART2++;
            }             
            ART2 = pART2;
            goto Coalesce;
        }

      /*
       * y1 is now the top of the intersection of the two active rectangles
       * and y2 is the bottom of that intersection.
       * Since it is possible that the brackets are not continuous,     
       * we must add the rectangles that could have been missed because 
       * there is no intersection.                                      
       * If the boundary is beyond the y1 used last time, we probably
       * missed something from the region whose y1 is smaller, so we add
       * whatever rectangles we missed, trimming them to not extend into the
       * new intersection.
       */
        y2 = min(ART1->y2, ART2->y2);

        if ( (y1 > prevY) && (y1 == ART1->y1) )
        {
	    BOX *stop;
            stop = max(ART2, ART2End);
            while (pART2 < stop)
            {
                MEMCHECK(newReg, rects, FirstRect);
                tempmin = max(prevY, pART2->y1);
                tempmax = min(y1, pART2->y2);
                ADDRECTNOX(newReg, rects, pART2->x1, tempmin, pART2->x2, 
			   tempmax);
		prevX = pART2->x2;
                pART2++;
            }
        }
        if ( (y1 > prevY) && (y1 == ART2->y1) )
        {
            BOX *stop;
            stop = max(ART1, ART1End);
            while (pART1 < stop)
            {
                MEMCHECK(newReg, rects, FirstRect);
                tempmin = max(prevY, pART1->y1);
                tempmax = min(y1, pART1->y2);
                ADDRECTNOX(newReg, rects, pART1->x1, tempmin, pART1->x2, 
			   tempmax);
		prevX = pART1->x2;
                pART1++;
	    }
	}

	if ((ART1->y2 != y1) && (ART2->y2 !=  y1))
	{
            prevY = y2;
	    prevX = -1;
	    pART2 = ART2;
	    pART1 = ART1;
	    ART1End = ART1; ART2End = ART2;

            /* add to the active rect tables */
	    while ((ART1End < r1End) && (ART1End->y1 <= y1)) 
			ART1End++;
	    while ((ART2End < r2End) && (ART2End->y1 <= y1))
			ART2End++;
	}

       /* make sure they do overlap.... */
       if ((y1 >= y2) &&
	   (((ART1->y1 > ART2->y1) && (ART1->y2 > ART2->y2)) ||
	   ((ART2->y1 > ART1->y1) && (ART2->y2 > ART1->y2))))
       {
	  if ((ART1 == ART1End) && (ART2 == ART2End))
              y2 = ymax;    /* get out of loop */
          goto Coalesce;
       }
            
       /* for each y bracket -- there is at least one since they overlap */
        r1Beg = ART1;
        while (r1Beg < ART1End) 
        {
            r2Beg = ART2;
            while (r2Beg < ART2End)
/*
           while ((r2Beg != ART2End) && (r1Beg != ART1End) )
*/
	    {
                     /*  if they intersect */
                if (((r1Beg->x1 >= r2Beg->x1) && (r1Beg->x1 <= r2Beg->x2)) ||
                    ((r2Beg->x1 >= r1Beg->x1) && (r2Beg->x1 <= r1Beg->x2))) 
                {
                    x1 = min(r1Beg->x1, r2Beg->x1);
                    x2 = max(r2Beg->x2, r1Beg->x2);
                        /*  check to see if it can subsume */

                    if ((rects!= FirstRect) && ((rects-1)->y1 == y1) &&
                       ((rects-1)->y2 == y2) && ((rects-1)->x1 < x2) && 
                       ((rects-1)->x2 > x1) && ((rects-1)->x2 < x2))
		    {
                        (rects-1)->x2 = x2;
		    }
                    else
                    {
                        MEMCHECK(newReg, rects, FirstRect);
                        ADDRECTNOX(newReg, rects, x1, y1, x2, y2);
			
		    }
		    prevX = x2;
		}
                else
                {  /*  if there is no intersection */
                    tempmin = min(y2, r2Beg->y2);
                    tempmax = max(y1, r2Beg->y1);
                    if (r2Beg->x1 < r1Beg->x1)
                    {
			if (r2Beg->x2 > prevX)
		       {
                            MEMCHECK(newReg, rects, FirstRect);
                            ADDRECTNOX(newReg, rects, r2Beg->x1, tempmax,
                               r2Beg->x2,tempmin);
		            prevX = r2Beg->x2;
			}
			if ((r2Beg+1 == ART2End) && 
			    (r1Beg->x2 > r2Beg->x2))
			{
                            MEMCHECK(newReg, rects, FirstRect);
                            ADDRECTNOX(newReg, rects, r1Beg->x1, tempmax,
                                r1Beg->x2, tempmin);
		            prevX = r1Beg->x2;
			}
		    }
                    else  
                    {
			if (r1Beg->x2 > prevX)
			{
                            MEMCHECK(newReg, rects, FirstRect);
                            ADDRECTNOX(newReg, rects, r1Beg->x1, tempmax,
                                r1Beg->x2, tempmin);
		            prevX = r1Beg->x2;
			}
			if ((r1Beg+1 == ART1End) &&
			    (r2Beg->x2 > r1Beg->x2))
			{
                            MEMCHECK(newReg, rects, FirstRect);
                            ADDRECTNOX(newReg, rects, r2Beg->x1, tempmax,
                               r2Beg->x2,tempmin);
		            prevX = r2Beg->x2;
			}
		    }
		}
                r2Beg++; 
	    }  /* while */
             r1Beg++;
	}  /* while */
Coalesce:
                      /* flush the active rect tables if necessary */
        prevX = -1;
	if (ART1->y2 == y2)
           ART1 = min(ART1End, r1End -1);
        if (ART2->y2 == y2)
           ART2 = min(ART2End, r2End -1);

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
			(prectO++)->y2 = (prectN++)->y2;
		    rects -= crects;
		    newReg->numRects -= crects;
		    irectBandStart = irectPrevStart;

		}
	    }
	}
        y1 = min(ART1->y1, ART2->y1);
        ART1End = ART1,  ART2End = ART2;


        pART1 = ART1;
        pART2 = ART2;

	prevY = (rects-1)->y2;
	prevX = -1;  
	    /*  needed for when no intersection at all */
	while ((ART1End < r1End) && (ART1End->y1 <= y1)) 
            ART1End++;
	while ((ART2End < r2End) && (ART2End->y1 <= y1))
	    ART2End++;
    }  while (y2 < ymax);


AddEnd:

           /*  add the rectangles that are below region 2's extents */
    if (ymax != reg1->extents.y2)
    {
        prevY = FirstRect[newReg->numRects-1].y1;
        while (ART1 != r1End)
        {
            MEMCHECK(newReg, rects, FirstRect);
            tempmax = max(ymax, ART1->y1);
            ADDRECTNOX(newReg, rects, ART1->x1, tempmax, ART1->x2, 
		       ART1->y2);
            if (tempmax != prevY)
	    {
                prevY = tempmax;
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
	    }
            ART1++;
	}
    }
    else 
    {
        prevY = FirstRect[newReg->numRects-1].y1;                
        while (ART2 < r2End)
        {
            MEMCHECK(newReg, rects, FirstRect);
            tempmax = max(ymax, ART2->y1);
            ADDRECTNOX(newReg, rects, ART2->x1, tempmax, ART2->x2,  
		       ART2->y2);
            if (tempmax != prevY)
	    {
                prevY = tempmax;
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
	    }
            ART2++;
	}
    }
    rects = (BOX *)Xrealloc(FirstRect, (sizeof(BOX) * newReg->numRects));
    newReg->extents.x1 = min(reg1->extents.x1, reg2->extents.x1);
    newReg->extents.y1 = min(reg1->extents.y1, reg2->extents.y1);
    newReg->extents.x2 = max(reg1->extents.x2, reg2->extents.x2);
    newReg->extents.y2 = max(reg1->extents.y2, reg2->extents.y2);
    newReg->size = newReg->numRects; 
    if (oldSize)
        Xfree(newReg->rects); 
    newReg->rects = rects;

    return(1);
}


/* TranslateRegion(pRegion, x, y)
   translates in place
   added by raymond
*/

void
miTranslateRegion(pRegion, x, y)
    register RegionPtr pRegion;
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

void
miRegionDestroy(pRegion)
    RegionPtr pRegion;
{
    Xfree(pRegion->rects);
    Xfree(pRegion);
}


/* Subtract regS from regM and puts result in regD --
 * M is for minuend, S for subtrahend, D for difference
 */
int 
miSubtract(regD, regM, regS)
    RegionPtr regD;               
    RegionPtr regM, regS;          
{
    register BOX *artM, *artS;     /* active rect tables       */
    register BOX *artMEnd, *artSEnd;   /* end of ARTs          */
    register BOX *rMBeg, *rSBeg;   /* current position in ARTs */
    BOX *rMEnd, *rSEnd;            /* end of rect lists        */
    BOX *rects;                    /* destination rects        */
    int y1, y2;                    /* current y-bracket        */
    int x1, x2;                    /* x vals for new rect      */
    int ymax;                      /* ymax for intersection    */
    BOX *FirstRect;                /* first rect in rects list */
    BOX	*tempRect;		   /* temporary pointer to a box */
    int oldSize, oldy1, oldy2, oldMSize;

    BOX *prectO, *prectN;	   /* these are used in coalescing boxes */
    int irectPrevStart, irectBandStart;
    BOX *prectBandStart;
    int fSame, crects;			   
 
   /* check for trivial reject */
    if ( (!(regM->numRects)) || (!(regS->numRects))  ||
	(!EXTENTCHECK(&regM->extents, &regS->extents)) )
    {
	miRegionCopy(regD, regM);
        return(1);
    }
 
    oldSize = regD->size;
    oldy1 = regM->extents.y1;
    oldy2 = regM->extents.y2;
    oldMSize = regM->size;
    regD->size =  (max(regM->numRects, regS->numRects) * 2);  

    rects = (BOX *) Xalloc (regD->size * sizeof(BOX));
    FirstRect = rects;

    ymax = min(regM->extents.y2, regS->extents.y2);
    y2 = max(regM->extents.y1, regS->extents.y1);

                   /*  reset extents of new Region */
    regD->extents.x1 = MAXSHORT;
    regD->extents.x2 = MINSHORT; 
    regD->extents.y1 = MAXSHORT;
    regD->extents.y2 = MINSHORT;


    artM = IndexRects(regM->rects, regM->numRects, y2);
    artS = IndexRects(regS->rects, regS->numRects, y2);
    rMEnd = regM->rects + regM->numRects;
    rSEnd = regS->rects + regS->numRects;
    regD->numRects = 0;

    /* Add rectangles from regM before intersection */
    if (oldy1 < y2) 
    {
	tempRect = regM->rects;
	while((tempRect < rMEnd) && (tempRect->y1 < y2))
	{
	    MEMCHECK(regD, rects, FirstRect);
	    ADDRECT(regD, rects, tempRect->x1, tempRect->y1, tempRect->x2,
		min(tempRect->y2, y2));
	    tempRect++;
	}
    }

    irectPrevStart = -1; 	/* initally, there are no previous rects */
       /* for each y bracket do the following
        * -- there is at least one y bracket since the regions  overlap */
    do 
    {
             /* find the y bracket */
        y1 = max(artM->y1, artS->y1);
	    /* add any parts of the minuend that fall between y-brackets */
	if(y1 != artM->y1)
	{
	    tempRect = artM;
	    while((tempRect < rMEnd) && (tempRect->y1 < y1))
	    {
		MEMCHECK(regD, rects, FirstRect);
		ADDRECT(regD, rects, tempRect->x1, max(tempRect->y1, y2),
		        tempRect->x2, min(tempRect->y2, y1));
		tempRect++;
	    }
	}

        artM = IndexRects(artM, max(0, rMEnd - artM - 1), y1);
        artS = IndexRects(artS, max(0, rSEnd - artS - 1), y1);
        artMEnd = artM,  artSEnd = artS;
        y2 = min(artM->y2, artS->y2);
            /* add to the active rect tables */
        while ((artMEnd < rMEnd) && (artMEnd->y1 <= y1))
	    artMEnd++;
        while ((artSEnd < rSEnd) && (artSEnd->y1 <= y1))
	    artSEnd++;

        irectBandStart = rects - FirstRect;
        rMBeg = artM;
        while (rMBeg != artMEnd) 
	{
            rSBeg = artS;
	    x1 = rMBeg->x1;
            while (rSBeg != artSEnd) 
            {
		if(rSBeg->x2 <= rMBeg->x1)
		{
		    /* subtrahend entirely to left of minuend, it's not
		     * interesting */
		    rSBeg++;
		    continue;
		}

		if(rSBeg->x1 <= x1)
		{
		    /* subtrahend begins before or when minuend begins */

		    /* left part of this minuend is subtracted out, but
		     * keep looking */
		    x1 = rSBeg->x2;
		    if(x1 >= rMBeg->x2)
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
		    x1 = rSBeg->x2;
		    if (x1 >= rMBeg->x2)
		      break;     /* minuend is now all gone */
		}
		else
		{
		    /* All the remaining artS rects must be to the right 
		     * of rMBeg so we can skip looking at them */
		    break;  
		} 
		    
                rSBeg++;
	    }

	    x2 = rMBeg->x2;
	    if(x2 > x1)
	    {
		MEMCHECK(regD, rects, FirstRect);
		ADDRECT(regD, rects, x1, y1, x2, y2);
	    }
            rMBeg++;
	}
              /* flush the active rect tables if necessary */
             /* but make sure that they do not past end of array */
        if (artM->y2 == y2)
            artM = min(artMEnd, rMEnd-1);
        if (artS->y2 == y2)
            artS = min(artSEnd, rSEnd-1);

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
	    ADDRECT(regD, rects, tempRect->x1, max(tempRect->y1, ymax),
	            tempRect->x2, tempRect->y2);
	    tempRect++;
	}
    }

   /* put together the new Region */

    if (regD->numRects)
    {
        if (oldSize)
            Xfree((char *)regD->rects);
/*
        FirstRect = (BOX *) Xrealloc (FirstRect,
				     (sizeof(BOX))*(regD->numRects));
	regD->size = regD->numRects;
*/
        regD->rects = FirstRect;
    }
    else
    {
	regD->size = oldSize;
	Xfree(FirstRect);
    }
    return(1);
}


void
miRegionReset(pRegion, pBox)
    RegionPtr pRegion;
    BOX *pBox;
{
    pRegion->extents.x1 = pRegion->rects->x1 = pBox->x1;
    pRegion->extents.y1 = pRegion->rects->y1 = pBox->y1;
    pRegion->extents.x2 = pRegion->rects->x2 = pBox->x2;
    pRegion->extents.y2 = pRegion->rects->y2 = pBox->y2;

    pRegion->numRects = 1;
}

#define INBOX(r, x, y) \
      ( ( ((r).x2 >  x)) && \
        ( ((r).x1 <= x)) && \
        ( ((r).y2 >  y)) && \
        ( ((r).y1 <= y)) )

Bool
miPointInRegion(pRegion, x, y, box)
    register RegionPtr pRegion;
    register int x, y;
    BOX *box;     /* "return" value */
{
    register BOX *pbox, *pboxEnd;

    if (pRegion->numRects == 0)
        return(FALSE);
    if (!INBOX(pRegion->extents, x, y))
        return(FALSE);
    for (pbox = pRegion->rects, pboxEnd = pbox + pRegion->numRects;
	 pbox < pboxEnd;
	 pbox++)
    {
        if (y >= pbox->y2)
	   continue;		/* not there yet */
	if ((y < box->y1) || (x < box->x1))
	   break;		/* missed it */
	if (x >= pbox->x2)
	   continue;		/* not there yet */
	*box = *pbox;
	return(TRUE);
    }
    return(FALSE);
}

Bool
miRegionNotEmpty(pRegion)
    RegionPtr pRegion;
{
    return(pRegion->numRects != 0);
}


void
miRegionEmpty(pRegion)
    RegionPtr pRegion;
{
    pRegion->numRects = 0;
}

BoxPtr
miRegionExtents(pReg)
    RegionPtr pReg;
{
    return((BoxPtr) &pReg->extents);
}


/*
 * 1 if equal, 0 if not 
 */
int 
miSingleRectRegionEqual(r1, r2)
    RegionPtr r1, r2;
{   
    return(((r1->numRects == 1) && (r2->numRects == 1) && 
          ((r1->rects)->x1 == (r2->rects)->x1) && 
          ((r1->rects)->y1 == (r2->rects)->y1) && 
          ((r1->rects)->x2 == (r2->rects)->x2) && 
          ((r1->rects)->y2 == (r2->rects)->y2) )
          ? 1 : 0);
}



