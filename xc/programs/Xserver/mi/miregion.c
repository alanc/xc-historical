/***********************************************************
Copyright 1987, 1988, 1989 by 
Digital Equipment Corporation, Maynard, Massachusetts, and
the Massachusetts Institute of Technology, Cambridge, Massachusetts.

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
/* $XConsortium: miregion.c,v 1.38 88/10/03 14:58:06 jim Exp $ */

#include <stdio.h>
#include "miscstruct.h"
#include "regionstr.h"

#if defined (__GNUC__) && defined (__STDC__) && !defined (__STRICT_ANSI) && !defined (NO_INLINES)
#define INLINE	inline
#else
#define INLINE
#endif

/*
 * hack until callers of these functions can deal with out-of-memory
 */

extern Bool Must_have_memory;

#ifdef DEBUG
#define assert(expr) {if (!(expr)) \
		FatalError("Assertion failed file %s, line %d: expr\n", \
			__FILE__, __LINE__); }
#else
#define assert(expr)
#endif

/*
 * The functions in this file implement the Region abstraction used extensively
 * throughout the X11 sample server. A Region is simply a set of disjoint
 * (non-overlapping) rectangles, plus an "extent" rectangle which is the
 * smallest single rectangle that contains all the non-overlapping rectangles.
 *
 * A Region is implemented as a "y-x-banded" array of rectangles.  This array
 * imposes two degrees of order.  First, all rectangles are sorted by top side
 * y coordinate first (y1), and then by left side x coordinate (x1).
 *
 * Furthermore, the rectangles are grouped into "bands".  Each rectangle in a
 * band has the same top y coordinate (y1), and each has the same bottom y
 * coordinate (y2).  Thus all rectangles in a band differ only in their left
 * and right side (x1 and x2).  Bands are implicit in the array of rectangles:
 * there is no separate list of band start pointers.
 *
 * The y-x band representation does not minimize rectangles.  In particular,
 * if a rectangle vertically crosses a band (the rectangle has scanlines in 
 * the y1 to y2 area spanned by the band), then the rectangle may be broken
 * down into two or more smaller rectangles stacked one atop the other. 
 *
 *  -----------				    -----------
 *  |         |				    |         |		    band 0
 *  |         |  --------		    -----------  --------
 *  |         |  |      |  in y-x banded    |         |  |      |   band 1
 *  |         |  |      |  form is	    |         |  |      |
 *  -----------  |      |		    -----------  --------
 *               |      |				 |      |   band 2
 *               --------				 --------
 *
 * An added constraint on the rectangles is that they must cover as much
 * horizontal area as possible: no two rectangles within a band are allowed
 * to touch.
 *
 * Whenever possible, bands will be merged together to cover a greater vertical
 * distance (and thus reduce the number of rectangles). Two bands can be merged
 * only if the bottom of one touches the top of the other and they have
 * rectangles in the same places (of the same width, of course).
 *
 * I don't know who wrote the original code in this file.  I (Joel McCormack)
 * have substantially modified or rewritten most of the core arithmetic
 * routines.  I added miRegionValidate in order to support several speed
 * improvements to miValidateTree.
 */

/*  1: if two Boxes overlap.
 *  0: if two Boxes do not overlap.
 *  Remember, x2 and y2 are not in the region 
 */
#define EXTENTCHECK(r1, r2) \
      (!( ((r1)->x2 <= (r2)->x1)  || \
        ( ((r1)->x1 >= (r2)->x2)) || \
        ( ((r1)->y2 <= (r2)->y1)) || \
        ( ((r1)->y1 >= (r2)->y2)) ) )

static BoxRec EmptyBox = {0, 0, 0, 0};

#ifdef DEBUG
void
miprintRects(rgn)
    RegionPtr rgn;
{
    register int i;

    ErrorF(  "num: %d size: %d \n", rgn->numRects, rgn->size);
    ErrorF(  "   extents: %d %d %d %d\n",rgn->extents.x1, 
	    rgn->extents.y1, rgn->extents.x2, rgn->extents.y2);
    for (i = 0; i != rgn->numRects; i++)
      ErrorF(  "%d %d %d %d \n", rgn->rects[i].x1,rgn->rects[i].y1,
	      rgn->rects[i].x2,rgn->rects[i].y2);
    ErrorF(  "\n");
}


Bool
miRegionsEqual(reg1, reg2)
    RegionPtr reg1;
    RegionPtr reg2;
{
    int i;

    if (reg1->extents.x1 != reg2->extents.x1) return FALSE;
    if (reg1->extents.x2 != reg2->extents.x2) return FALSE;
    if (reg1->extents.y1 != reg2->extents.y1) return FALSE;
    if (reg1->extents.y2 != reg2->extents.y2) return FALSE;
    if (reg1->numRects != reg2->numRects) return FALSE;
    
    for (i = 0; i != reg1->numRects; i++) {
	if (reg1->rects[i].x1 != reg2->rects[i].x1) return FALSE;
	if (reg1->rects[i].x2 != reg2->rects[i].x2) return FALSE;
	if (reg1->rects[i].y1 != reg2->rects[i].y1) return FALSE;
	if (reg1->rects[i].y2 != reg2->rects[i].y2) return FALSE;
    }
    return TRUE;
}
#endif /* DEBUG */


/*****************************************************************
 *   RegionCreate(rect, size)
 *     This routine does a simple malloc to make a structure of
 *     REGION of "size" number of rectangles.
 *****************************************************************/

RegionPtr
miRegionCreate(rect, size)
    BoxPtr rect;
    int size;
{
    register RegionPtr temp;       /*   new region  */
   
    if (size <= 0) size = 1;
    Must_have_memory = TRUE; /* XXX */
    temp = (RegionPtr ) xalloc (sizeof (RegionRec));
    temp->rects = (BoxPtr) xalloc (size * (sizeof(BoxRec)));
    Must_have_memory = FALSE; /* XXX */
    if (rect == (BoxPtr)NULL)
    {
        temp->numRects = 0;
        temp->extents = EmptyBox;
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
    register int       numRects;

    if (dstrgn != rgn) /*  don't want to copy to itself */
    {  
	numRects = rgn->numRects;
        if (dstrgn->size < numRects)
        {
	    xfree (dstrgn->rects);
	    Must_have_memory = TRUE; /* XXX */
            dstrgn->rects = (BoxPtr) xalloc(numRects * (sizeof(BoxRec)));
	    Must_have_memory = FALSE; /* XXX */
            dstrgn->size = numRects;
	}
        dstrgn->numRects = numRects;
        dstrgn->extents = rgn->extents;

	/* Special-case the overwhelming majority of calls */
	if (numRects == 1) {
	   dstrgn->rects[0] = rgn->rects[0];
	   return;
	} else if (numRects == 0) {
	    return;
	}
	bcopy(rgn->rects, dstrgn->rects, numRects * sizeof(BoxRec));   
    }
}


/*======================================================================
 *	    Generic Region Operator
 *====================================================================*/

/*-
 *-----------------------------------------------------------------------
 * miCoalesce --
 *	Attempt to merge the boxes in the current band with those in the
 *	previous one.  We are guaranteed that the current band extends to
 *      the end of the rects array.  Used only by miRegionOp.
 *
 * Results:
 *	The new index for the previous band.
 *
 * Side Effects:
 *	If coalescing takes place:
 *	    - rectangles in the previous band will have their y2 fields
 *	      altered.
 *	    - pReg->numRects will be decreased.
 *
 *-----------------------------------------------------------------------
 */
INLINE static int
miCoalesce (pReg, prevStart, curStart)
    register RegionPtr	pReg;	    	/* Region to coalesce		      */
    int	    	  	prevStart;  	/* Index of start of previous band    */
    int	    	  	curStart;   	/* Index of start of current band     */
{
    register BoxPtr	pPrevBox;   	/* Current box in previous band	      */
    register BoxPtr	pCurBox;    	/* Current box in current band        */
    register int  	numRects;	/* Number rectangles in both bands    */
    register int	y2;		/* Bottom of current band	      */
    /*
     * Figure out how many rectangles are in the band.
     */
    numRects = curStart - prevStart;
    assert(numRects == pReg->numRects - curStart);

    if (numRects == 0) return curStart;

    /*
     * The bands may only be coalesced if the bottom of the previous
     * matches the top scanline of the current.
     */
    pPrevBox = &pReg->rects[prevStart];
    pCurBox = &pReg->rects[curStart];
    if (pPrevBox->y2 != pCurBox->y1) return curStart;

    /*
     * Make sure the bands have boxes in the same places. This
     * assumes that boxes have been added in such a way that they
     * cover the most area possible. I.e. two boxes in a band must
     * have some horizontal space between them.
     */
    y2 = pCurBox->y2;

    do {
	if ((pPrevBox->x1 != pCurBox->x1) || (pPrevBox->x2 != pCurBox->x2)) {
	    return (curStart);
	}
	pPrevBox++;
	pCurBox++;
	numRects--;
    } while (numRects != 0);

    /*
     * The bands may be merged, so set the bottom y of each box
     * in the previous band to the bottom y of the current band.
     */
    numRects = curStart - prevStart;
    pReg->numRects -= numRects;
    do {
	pPrevBox--;
	pPrevBox->y2 = y2;
	numRects--;
    } while (numRects != 0);
    return prevStart;
}


/* Quicky macro to avoid trivial reject procedure calls to miCoalesce */

#define Coalesce(newReg, prevBand, curBand)				\
    if (curBand - prevBand == newReg->numRects - curBand) {		\
	prevBand = miCoalesce(newReg, prevBand, curBand);		\
    } else {								\
	prevBand = curBand;						\
    }

#define ADDRECTANGLE(pNextRect, nx1, ny1, nx2, ny2)			\
{									\
    pNextRect->x1 = nx1;						\
    pNextRect->y1 = ny1;						\
    pNextRect->x2 = nx2;						\
    pNextRect->y2 = ny2;						\
    pNextRect++;							\
} /* ADDRECTANGLE */


/*-
 *-----------------------------------------------------------------------
 * miAppendNonO --
 *	Handle a non-overlapping band for the union and subtract operations.
 *      Just adds the (top/bottom-clipped) rectangles into the region.
 *      Doesn't have to check for subsumption or anything.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	pReg->numRects is incremented and the final rectangles overwritten
 *	with the rectangles we're passed.
 *
 *-----------------------------------------------------------------------
 */

INLINE static Bool
miAppendNonO (pReg, r, rEnd, y1, y2)
    register RegionPtr	pReg;
    register BoxPtr	r;
    BoxPtr  	  	rEnd;
    register int  	y1;
    register int  	y2;
{
    register BoxPtr	pNextRect;
    register int	newRects;
	     int	nextRect;

    newRects = rEnd - r;

    assert(y1 < y2);
    assert(newRects != 0);

    /* Make sure we have enough space for all rectangles to be added */
    nextRect = pReg->numRects;
    pReg->numRects += newRects;
    if (pReg->numRects > pReg->size) {
	pReg->size += 2 * newRects;
	pReg->rects =
	    (BoxPtr) xrealloc(pReg->rects, pReg->size * sizeof(BoxRec));
    }

    pNextRect = &pReg->rects[nextRect];
    do {
	assert(r->x1 < r->x2);
	ADDRECTANGLE(pNextRect, r->x1, y1, r->x2, y2);
	r++;
    } while (r != rEnd);

    return TRUE;
}

/*-
 *-----------------------------------------------------------------------
 * miRegionOp --
 *	Apply an operation to two regions. Called by miUnion, miInverse,
 *	miSubtract, miIntersect....  Both regions MUST have at least one
 *      rectangle.
 *
 * Results:
 *	TRUE if overlapFunc ever returns TRUE, FALSE otherwise.  (Used only
 *      for Union right now.)
 *
 * Side Effects:
 *	The new region is overwritten.
 *
 * Notes:
 *	The idea behind this function is to view the two regions as sets.
 *	Together they cover a rectangle of area that this function divides
 *	into horizontal bands where points are covered only by one region
 *	or by both. For the first case, the nonOverlapFunc is called with
 *	each the band and the band's upper and lower extents. For the
 *	second, the overlapFunc is called to process the entire band. It
 *	is responsible for clipping the rectangles in the band, though
 *	this function provides the boundaries.
 *	At the end of each band, the new region is coalesced, if possible,
 *	to reduce the number of rectangles in the region.
 *
 *-----------------------------------------------------------------------
 */
static Bool
miRegionOp(newReg, reg1, reg2, overlapFunc, appendNon1, appendNon2)
    RegionPtr       newReg;		    /* Place to store result	      */
    RegionPtr       reg1;		    /* First region in operation      */
    RegionPtr       reg2;		    /* 2d region in operation         */
    Bool	    (*overlapFunc)();       /* Function to call for over-
					     * lapping bands		      */
    Bool	    appendNon1;		    /* Append non-overlapping bands   */
					    /* in region 1 ? */
    Bool	    appendNon2;		    /* Append non-overlapping bands   */
					    /* in region 2 ? */
{
    register BoxPtr r1;			    /* Pointer into first region      */
    register BoxPtr r2;			    /* Pointer into 2d region	      */
    BoxPtr	    r1End;		    /* End of 1st region	      */
    BoxPtr	    r2End;		    /* End of 2d region		      */
	     short  ybot;		    /* Bottom of intersection	      */
	     short  ytop;		    /* Top of intersection	      */
    BoxPtr	    oldRects;		    /* Old rects for newReg	      */
    int		    prevBand;		    /* Index of start of
					     * previous band in newReg        */
    int		    curBand;		    /* Index of start of current
					     * band in newReg		      */
    register BoxPtr r1BandEnd;		    /* End of current band in r1      */
    register BoxPtr r2BandEnd;		    /* End of current band in r2      */
    short	    top;		    /* Top of non-overlapping band    */
    short	    bot;		    /* Bottom of non-overlapping band */
    register int    r1y1;		    /* Temps for r1->y1 and r2->y1    */
    register int    r2y1;
    Bool	    overlap;		    /* TRUE if overlapFunc TRUE       */
    int		    newSize;

    /*
     * Initialization:
     *	set r1, r2, r1End and r2End appropriately, save the rectangles
     * of the destination region until the end in case it's one of
     * the two source regions, then mark the "new" region empty, allocating
     * another array of rectangles for it to use.
     */

    overlap = FALSE;

    r1 = reg1->rects;
    r2 = reg2->rects;
    r1End = r1 + reg1->numRects;
    r2End = r2 + reg2->numRects;
    assert(r1 != r1End);
    assert(r2 != r2End);

    /*
     * careful here -- everything this function calls
     * better not clear this variable!
     */
    Must_have_memory = TRUE; /* XXX */

    oldRects = 0;
    newSize = max(reg1->numRects, reg2->numRects) * 2;
    if (newReg == reg1 || newReg == reg2 || newSize < newReg->size) {
	oldRects = newReg->rects;
    	
    	/*
     	 * Allocate a reasonable number of rectangles for the new region. The idea
     	 * is to allocate enough so the individual functions don't need to
     	 * reallocate and copy the array, which is time consuming, yet we don't
     	 * have to worry about using too much memory.
 	 */

    	newReg->rects = (BoxPtr) xalloc (sizeof(BoxRec) * newSize);
    	newReg->size = newSize;
    }
    
    EMPTY_REGION(newReg);

    /*
     * Initialize ybot.
     * In the upcoming loop, ybot and ytop serve different functions depending
     * on whether the band being handled is an overlapping or non-overlapping
     * band.
     * 	In the case of a non-overlapping band (only one of the regions
     * has points in the band), ybot is the bottom of the most recent
     * intersection and thus clips the top of the rectangles in that band.
     * ytop is the top of the next intersection between the two regions and
     * serves to clip the bottom of the rectangles in the current band.
     *	For an overlapping band (where the two regions intersect), ytop clips
     * the top of the rectangles of both regions and ybot clips the bottoms.
     */

    ybot = min(r1->y1, r2->y1);
    
    /*
     * prevBand serves to mark the start of the previous band so rectangles
     * can be coalesced into larger rectangles. qv. miCoalesce, above.
     * In the beginning, there is no previous band, so prevBand == curBand
     * (curBand is set later on, of course, but the first band will always
     * start at index 0). prevBand and curBand must be indices because of
     * the possible expansion, and resultant moving, of the new region's
     * array of rectangles.
     */
    prevBand = 0;
    
#define FindBand(r, rBandEnd, rEnd, ry1)		    \
{							    \
    ry1 = r->y1;					    \
    rBandEnd = r+1;					    \
    while ((rBandEnd != rEnd) && (rBandEnd->y1 == ry1)) {   \
	rBandEnd++;					    \
    }							    \
} /* FindBand */

    do {
	/*
	 * This algorithm proceeds one source-band (as opposed to a
	 * destination band, which is determined by where the two regions
	 * intersect) at a time. r1BandEnd and r2BandEnd serve to mark the
	 * rectangle after the last one in the current band for their
	 * respective regions.
	 */
	assert(r1 != r1End);
	assert(r2 != r2End);
    
	FindBand(r1, r1BandEnd, r1End, r1y1);
	FindBand(r2, r2BandEnd, r2End, r2y1);

	/*
	 * First handle the band that doesn't intersect, if any.
	 *
	 * Note that attention is restricted to one band in the
	 * non-intersecting region at once, so if a region has n
	 * bands between the current position and the next place it overlaps
	 * the other, this entire loop will be passed through n times.
	 */
	if (r1y1 < r2y1) {
	    if (appendNon1) {
		top = max(r1y1, ybot);
		bot = min(r1->y2, r2y1);
		if (top != bot)	{
		    curBand = newReg->numRects;
		    miAppendNonO(newReg, r1, r1BandEnd, top, bot);
		    Coalesce(newReg, prevBand, curBand);
		}
	    }
	    ytop = r2y1;
	} else if (r2y1 < r1y1) {
	    if (appendNon2) {
		top = max(r2y1, ybot);
		bot = min(r2->y2, r1y1);
		if (top != bot) {
		    curBand = newReg->numRects;
		    miAppendNonO(newReg, r2, r2BandEnd, top, bot);
		    Coalesce(newReg, prevBand, curBand);
		}
	    }
	    ytop = r1y1;
	} else {
	    ytop = r1y1;
	}

	/*
	 * Now see if we've hit an intersecting band. The two bands only
	 * intersect if ybot > ytop
	 */
	ybot = min(r1->y2, r2->y2);
	if (ybot > ytop) {
	    curBand = newReg->numRects;
	    overlap |= (* overlapFunc)
		(newReg, r1, r1BandEnd, r2, r2BandEnd, ytop, ybot);
	    Coalesce(newReg, prevBand, curBand);
	}

	/*
	 * If we've finished with a band (y2 == ybot) we skip forward
	 * in the region to the next band.
	 */
	if (r1->y2 == ybot) r1 = r1BandEnd;
	if (r2->y2 == ybot) r2 = r2BandEnd;

    } while (r1 != r1End && r2 != r2End);

    /*
     * Deal with whichever region (if any) still has rectangles left.
     *
     * We only need to worry about banding and coalescing for the very first
     * band left.  After that, we can just group all remaining boxes,
     * regardless of how many bands, into one final append to the list.
     */

#define	AppendRegions(newReg, r, rEnd)					\
{									\
    int newRects, nextRect;						\
    newRects = rEnd - r;						\
    if (newRects != 0) {						\
	/* Make sure room enough */					\
	nextRect = newReg->numRects;					\
	newReg->numRects += newRects;					\
	if (newReg->numRects > newReg->size) {				\
	    newReg->size = newReg->numRects;				\
	    newReg->rects =						\
	      (BoxPtr) xrealloc(newReg->rects, newReg->size * sizeof(BoxRec)); \
	}								\
	bcopy(r, &newReg->rects[nextRect], newRects * sizeof(BoxRec));  \
    }									\
} /* AppendRegions */

    if ((r1 != r1End) && appendNon1) {
	/* Do first nonOverlap1Func call, which may be able to coalesce */
	FindBand(r1, r1BandEnd, r1End, r1y1);
	curBand = newReg->numRects;
	miAppendNonO(newReg, r1, r1BandEnd, max(r1y1, ybot), r1->y2);
	Coalesce(newReg, prevBand, curBand);
	/* Just append the rest of the boxes  */
	AppendRegions(newReg, r1BandEnd, r1End);

    } else if ((r2 != r2End) && appendNon2) {
	/* Do first nonOverlap2Func call, which may be able to coalesce */
	FindBand(r2, r2BandEnd, r2End, r2y1);
	curBand = newReg->numRects;
	miAppendNonO(newReg, r2, r2BandEnd, max(r2y1, ybot), r2->y2);
	Coalesce(newReg, prevBand, curBand);
	/* Append rest of boxes */
	AppendRegions(newReg, r2BandEnd, r2End);
    }

    /*
     * A bit of cleanup. To keep regions from growing without bound,
     * we shrink the array of rectangles to match the new number of
     * rectangles in the region. This never goes to 0, however...
     *
     * Only do this stuff if the number of rectangles freed is large
     */
    if ((newReg->numRects < (newReg->size >> 1)) &&
	(newReg->size - newReg->numRects) > 100)
    {
	if (REGION_NOT_EMPTY(newReg))
	{
	    newReg->size = newReg->numRects;
	    newReg->rects = (BoxPtr) xrealloc (newReg->rects,
					       sizeof(BoxRec) * newReg->size);
	}
	else
	{
	    /*
	     * No point in doing the extra work involved in an xrealloc if
	     * the region is empty
	     */
	    newReg->size = 1;
	    xfree(newReg->rects);
	    newReg->rects = (BoxPtr) xalloc(sizeof(BoxRec));
	}
    }
    if (oldRects)
	xfree (oldRects);

    Must_have_memory = FALSE; /* XXX */

    return overlap;
}

/*-
 *-----------------------------------------------------------------------
 * miSetExtents --
 *	Reset the extents of a region to what they should be. Called by
 *	miSubtract and miIntersect as they can't figure it out along the
 *	way or do so easily, as miUnion can.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The region's 'extents' structure is overwritten.
 *
 *-----------------------------------------------------------------------
 */
static void
miSetExtents (pReg)
    RegionPtr	  	pReg;
{
    register BoxPtr	pBox,
			pBoxEnd,
			pExtents;

    if (pReg->numRects == 0) {
	pReg->extents = EmptyBox;
	return;
    }

    pExtents = &pReg->extents;
    pBox = pReg->rects;
    pBoxEnd = &pBox[pReg->numRects-1];

    /*
     * Since pBox is the first rectangle in the region, it must have the
     * smallest y1 and since pBoxEnd is the last rectangle in the region,
     * it must have the largest y2, because of banding. Initialize x1 and
     * x2 from  pBox and pBoxEnd, resp., as good things to initialize them
     * to...
     */
    pExtents->x1 = pBox->x1;
    pExtents->y1 = pBox->y1;
    pExtents->x2 = pBoxEnd->x2;
    pExtents->y2 = pBoxEnd->y2;

    assert(pExtents->y1 < pExtents->y2);
    while (pBox <= pBoxEnd) {
	if (pBox->x1 < pExtents->x1) {
	    pExtents->x1 = pBox->x1;
	}
	if (pBox->x2 > pExtents->x2) {
	    pExtents->x2 = pBox->x2;
	}
	pBox++;
    };

    assert(pExtents->x1 < pExtents->x2);
}


/*======================================================================
 *	    Region Inversion
 *====================================================================*/

/*-
 *-----------------------------------------------------------------------
 * miInverse --
 *	Take a region and a box and return a region that is everything
 *	in the box but not in the region. The careful reader will note
 *	that this is the same as subtracting the region from the box...
 *
 * Results:
 *	TRUE.
 *
 * Side Effects:
 *	newReg is overwritten.
 *
 *-----------------------------------------------------------------------
 */
int 
miInverse(newReg, reg1, invRect)
    RegionPtr 	  newReg;       /* Destination region */
    RegionPtr 	  reg1;         /* Region to invert */
    BoxPtr     	  invRect; 	/* Bounding box for inversion */
{
    RegionRec	  invReg;   	/* Quick and dirty region made from the
				 * bounding box */

    invReg.size = 	1;
    invReg.numRects = 	1;
    invReg.extents = 	*invRect;
    invReg.rects = 	invRect;
    return (miSubtract (newReg, &invReg, reg1));
}


/*======================================================================
 *	    Region Intersection
 *====================================================================*/
/*-
 *-----------------------------------------------------------------------
 * miIntersectO --
 *	Handle an overlapping band for miIntersect.
 *
 * Results:
 *	FALSE.
 *
 * Side Effects:
 *	Rectangles may be added to the region.
 *
 *-----------------------------------------------------------------------
 */
static Bool
miIntersectO (pReg, r1, r1End, r2, r2End, y1, y2)
    register RegionPtr	pReg;
    register BoxPtr	r1;
    BoxPtr  	  	r1End;
    register BoxPtr	r2;
    BoxPtr  	  	r2End;
    short    	  	y1;
    short    	  	y2;
{
    register int  	x1;
    register int  	x2;
    register BoxPtr	pNextRect;

    pNextRect = &pReg->rects[pReg->numRects];

    assert(y1 < y2);
    assert(r1 != r1End && r2 != r2End);

    do {
	x1 = max(r1->x1, r2->x1);
	x2 = min(r1->x2, r2->x2);

	/*
	 * If there's any overlap between the two rectangles, add that
	 * overlap to the new region.
	 */
	if (x1 < x2)
	{
	    MEMCHECK(pReg, pNextRect, pReg->rects);
	    ADDRECTANGLE(pNextRect, x1, y1, x2, y2);
	    pReg->numRects += 1;
	    assert(pReg->numRects <= pReg->size);
	}

	/*
	 * Advance the pointer(s) with the leftmost right side, since the next
	 * rectangle on that list may still overlap the other region's
	 * current rectangle.
	 */
	if (r1->x2 == x2) {
	    r1++;
	}
	if (r2->x2 == x2) {
	    r2++;
	}
    } while ((r1 != r1End) && (r2 != r2End));

    return(FALSE);
}


int 
miIntersect(newReg, reg1, reg2)
    register RegionPtr 	newReg;     /* destination Region */
    register RegionPtr 	reg1;
    register RegionPtr	reg2;       /* source regions     */
{
   /* check for trivial reject */
    if ( (!(reg1->numRects)) || (!(reg2->numRects))  ||
	(!EXTENTCHECK(&reg1->extents, &reg2->extents))) {
	/* Covers about 20% of all cases */
	newReg->numRects = 0;
	newReg->extents = EmptyBox;

    } else if (reg1->numRects == 1 && reg2->numRects == 1) {
	/* Covers about 80% of cases that aren't trivially rejected */
	newReg->extents.x1 = max(reg1->extents.x1, reg2->extents.x1);
	newReg->extents.y1 = max(reg1->extents.y1, reg2->extents.y1);
	newReg->extents.x2 = min(reg1->extents.x2, reg2->extents.x2);
	newReg->extents.y2 = min(reg1->extents.y2, reg2->extents.y2);
	newReg->numRects = 1;
	newReg->rects[0] = newReg->extents;

    /* Add case for one rectangle covering list? */
    } else {
	/* General purpose intersection */
	miRegionOp(newReg, reg1, reg2, miIntersectO, FALSE, FALSE);
	miSetExtents(newReg);
    }

    return(1);
}


/*======================================================================
 *	    Region Union
 *====================================================================*/

/*-
 *-----------------------------------------------------------------------
 * miUnionO --
 *	Handle an overlapping band for the union operation. Picks the
 *	left-most rectangle each time and merges it into the region.
 *
 * Results:
 *	TRUE if any boxes overlap.
 *
 * Side Effects:
 *	Rectangles are overwritten in pReg->rects and pReg->numRects will
 *	be changed.
 *
 *-----------------------------------------------------------------------
 */
static Bool
miUnionO (pReg, r1, r1End, r2, r2End, y1, y2)
    register RegionPtr	pReg;
    register BoxPtr	r1;
	     BoxPtr  	r1End;
    register BoxPtr	r2;
	     BoxPtr  	r2End;
	     short	y1;
	     short	y2;
{
    register BoxPtr     pNextRect;
    register int        x1;     /* left and right side of current union */
    register int        x2;
	     Bool       overlap;

    pNextRect = &pReg->rects[pReg->numRects];
    overlap = FALSE;

#define MERGERECT(r)						\
{								\
    if (r->x1 <= x2) {						\
	/* Merge with current rectangle */			\
	if (r->x1 < x2) overlap = TRUE;				\
	if (x2 < r->x2) x2 = r->x2;				\
    } else {							\
	/* Add current rectangle, start new one */		\
	MEMCHECK(pReg, pNextRect, pReg->rects);			\
	ADDRECTANGLE(pNextRect, x1, y1, x2, y2);		\
	pReg->numRects++;					\
	x1 = r->x1;						\
	x2 = r->x2;						\
    }								\
    r++;							\
} /* MERGERECT */

    assert (y1 < y2);
    assert(r1 != r1End && r2 != r2End);

    /* Start off current rectangle */
    if (r1->x1 < r2->x1) {
	x1 = r1->x1;
	x2 = r1->x2;
	r1++;
    } else {
	x1 = r2->x1;
	x2 = r2->x2;
	r2++;
    }
    while (r1 != r1End && r2 != r2End) {
	if (r1->x1 < r2->x1) MERGERECT(r1) else MERGERECT(r2);
    }

    /* Finish off whoever (if any) is left */
    if (r1 != r1End) {
	do {
	    MERGERECT(r1);
	} while (r1 != r1End);
    } else if (r2 != r2End) {
	do {
	    MERGERECT(r2);
	} while (r2 != r2End);
    }
    
    /* Add current rectangle */
    MEMCHECK(pReg, pNextRect, pReg->rects);
    ADDRECTANGLE(pNextRect, x1, y1, x2, y2);
    pReg->numRects++;

    return overlap;
} /* miIntersectO */

Bool 
miUnion(newReg, reg1, reg2)
    RegionPtr		newReg;                  /* destination Region */
    register RegionPtr 	reg1;
    register RegionPtr	reg2;             /* source regions     */
{
    /* Return TRUE if some overlap between reg1, reg2 */

    /*  checks all the simple cases */

    /*
     * Region 1 and 2 are the same
     */
    if (reg1 == reg2) {
        if (newReg != reg1)
            miRegionCopy(newReg, reg1);
        return(TRUE);
    }

    /*
     * Region 1 is empty
     */
    if (!(reg1->numRects)) {
        if (newReg != reg2)
            miRegionCopy(newReg, reg2);
        return(FALSE);
    }

    /*
     * Region 2 is empty
     */
    if (!(reg2->numRects)) {
        if (newReg != reg1)
            miRegionCopy(newReg, reg1);
        return(FALSE);
    }

    /*
     * Region 1 completely subsumes region 2
     */
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

    /*
     * Region 2 completely subsumes region 1
     */
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

    newReg->extents.x1 = min(reg1->extents.x1, reg2->extents.x1);
    newReg->extents.y1 = min(reg1->extents.y1, reg2->extents.y1);
    newReg->extents.x2 = max(reg1->extents.x2, reg2->extents.x2);
    newReg->extents.y2 = max(reg1->extents.y2, reg2->extents.y2);

    return miRegionOp(newReg, reg1, reg2, miUnionO, TRUE, TRUE);
}



/*======================================================================
 *	    Batch Rectangle Union
 *====================================================================*/

/*-
 *-----------------------------------------------------------------------
 * miRegionAppend --
 * 
 *      Just append the source region onto the end of the destination``region.''
 *      The destination is just a non-y-x-banded random collection of
 *      rectangles, and not a true region.  See miRegionValidate below.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *      The desination ``region'' may be modified.
 *
 */
void
miRegionAppend(dstrgn, rgn)
    register RegionPtr dstrgn;
    register RegionPtr rgn;
{
    register int numRects;

    numRects = rgn->numRects;

    if (numRects == 0) return;

    if (numRects + dstrgn->numRects > dstrgn->size) {
	dstrgn->size += 2 * numRects;
	Must_have_memory = TRUE; /* XXX */
	dstrgn->rects =
	    (BoxPtr) xrealloc(dstrgn->rects, dstrgn->size * sizeof(BoxRec));
	Must_have_memory = FALSE; /* XXX */
    }

    if (numRects == 1) {
	dstrgn->rects[dstrgn->numRects] = rgn->rects[0];
    } else {
	bcopy(rgn->rects, &(dstrgn->rects[dstrgn->numRects]),
	    numRects * sizeof(BoxRec));
    }
    dstrgn->numRects += numRects;
}

   

/*-
 *-----------------------------------------------------------------------
 * miRegionValidate --
 * 
 *      Take a ``region'' which is a non-y-x-banded random collection of
 *      rectangles, and compute a nice region which is the union of all the
 *      rectangles.
 *
 * Results:
 *	TRUE if there was any overlap in the rectangles, otherwise FALSE.
 *
 * Side Effects:
 *      The passed-in ``region'' may be modified.
 *
 * Strategy:
 *      Step 1. Sort the rectangles into ascending order with primary key y1
 *		and secondary key x1.
 *
 *      Step 2. Split the rectangles into the minimum number of proper y-x
 *		banded regions.  This may require horizontally merging
 *		rectangles, and vertically coalescing bands.  With any luck,
 *		this step in an identity tranformation (ala the Box widget),
 *		or a coalescing into 1 box (ala Menus).
 *
 *	Step 3. Merge the separate regions down to a single region by calling
 *		miUnion.  Maximize the work each miUnion call does by using
 *		a binary merge.
 *
 *-----------------------------------------------------------------------
 */

static void QuickSortRects(rects, numRects)
    register BoxRec     rects[];
    register int        numRects;
{
    register int	y1;
    register int	x1;
    register int        i, j;
    register BoxPtr     r;

/* Always called with numRects > 1 */

#define ExchangeRects(a, b) \
{			    \
    BoxRec     t;	    \
    t = rects[a];	    \
    rects[a] = rects[b];    \
    rects[b] = t;	    \
}

    do {
	if (numRects == 2) {
	    if (rects[0].y1 > rects[1].y1 ||
		    (rects[0].y1 == rects[1].y1 && rects[0].x1 > rects[1].x1)) {
		ExchangeRects(0, 1);
	    }
	    return;
	}

	/* Choose partition element, stick in location 0 */
        ExchangeRects(0, numRects / 2);
	y1 = rects[0].y1;
	x1 = rects[0].x1;

        /* Partition array */
        i = 0;
        j = numRects;
        do {
	    r = &(rects[i]);
	    do {
		r++;
		i++;
            } while (i != numRects &&
		(r->y1 < y1 || (r->y1 == y1 && r->x1 < x1)));
	    r = &(rects[j]);
	    do {
		r--;
		j--;
            } while (y1 < r->y1 || (y1 == r->y1 && x1 < r->x1));
            if (i < j) ExchangeRects(i, j);
        } while (i < j);

        /* Move partition element back to middle */
        ExchangeRects(0, j);

	/* Recurse */
        if (numRects-j-1 > 1) QuickSortRects(&rects[j+1], numRects-j-1);
        numRects = j;
    } while (numRects > 1);
} /* QuickSortRects */


Bool
miRegionValidate(badreg)
	    RegionPtr   badreg;     /* ``Region'' to clean up		    */
{
    /* Descriptor for regions under construction  in Step 2. */
    typedef struct {
	RegionPtr   reg;
	int	    prevBand;
	int	    curBand;
    } RegionInfo;

	     int	numRects;   /* Original badreg->numRects	    */
	     RegionInfo *ri;	    /* Array of current regions		    */
    register int	numRI;      /* Number of entries used in ri	    */
	     int	sizeRI;	    /* Number of entries available in ri    */
	     int	i;	    /* Index into rects			    */
    register int	j;	    /* Index into ri			    */
    register RegionInfo *rit;       /* &ri[j]				    */
    register RegionPtr  reg;	    /* ri[j].reg			    */
    register BoxPtr	box;	    /* Current box in rects		    */
    register BoxPtr	riBox;      /* Last box in ri[j].reg		    */
	     Bool	overlap;    /* Do any boxes overlap?		    */


    overlap = FALSE;
    numRects = badreg->numRects;

    /* Take care of simple 0 or 1 rectangle cases */
    if (numRects == 0) {
	return FALSE;
    } else if (numRects == 1) {
	badreg->extents = badreg->rects[0];
	return FALSE;
    }

    /*
     * careful here -- this routine "knows" that
     * miRegionCreate sets/clears Must_have_memory
     */

    Must_have_memory = TRUE; /* XXX */

    /* Step 1: Sort the rects array into ascending (y1, x1) order */
    QuickSortRects(badreg->rects, badreg->numRects);

    /* Step 2: Scatter the sorted array into the minimum number of regions */

    /* Set up the first region to be the first rectangle in badreg */
    /* Note that step 2 code will never overflow badreg->rects     */
    ri      = (RegionInfo *) xalloc(4 * sizeof(RegionInfo));
    sizeRI  = 4;
    numRI   = 1;
    ri[0].prevBand   = 0;
    ri[0].curBand    = 0;
    ri[0].reg	     = badreg;
    badreg->extents  = badreg->rects[0];
    badreg->numRects = 1;

    /* Now scatter rectangles into the minimum set of valid regions.  If the
       next rectangle to be added to a region would force an existing rectangle
       in the region to be split up in order to maintain y-x banding, just
       forget it.  Try the next region.  If it doesn't fit cleanly into any
       region, make a new one. */

    for (i = 1, box = badreg->rects+1; i != numRects; i++, box++) {
	/* Look for a region to append box to */
	for (j = 0, rit = ri; j != numRI; j++, rit++) {
	    reg = rit->reg;
	    riBox = &(reg->rects[reg->numRects-1]);

	    if (box->y1 == riBox->y1 && box->y2 == riBox->y2) {
		/* box in in same band as riBox.  Merge or append it */
		if (box->x1 <= riBox->x2) {
		    /* Merge it with riBox */
		    if (box->x1 < riBox->x2) overlap = TRUE;
		    if (box->x2 > riBox->x2) riBox->x2 = box->x2;
		} else {
		    /* Append to region */
		    if (reg->numRects == reg->size) {
			reg->size *= 2;
			reg->rects = (BoxPtr) xrealloc(reg->rects,
			    reg->size * sizeof(BoxRec));
		    }
		    reg->rects[reg->numRects] = *box;
		    reg->numRects++;
		}
		goto NextRect;   /* So sue me */

	    } else if (box->y1 >= riBox->y2) {
		/* Put box into new band */
		if (reg->extents.x2 < riBox->x2) reg->extents.x2 = riBox->x2;
		if (reg->extents.x1 > box->x1)   reg->extents.x1 = box->x1;
		Coalesce(reg, rit->prevBand, rit->curBand);
		rit->curBand = reg->numRects;
		if (reg->numRects == reg->size) {
		    reg->size *= 2;
		    reg->rects = (BoxPtr) xrealloc(reg->rects,
			reg->size * sizeof(BoxRec));
		}
		reg->rects[reg->numRects] = *box;
		reg->numRects++;
		goto NextRect;
	    }
	    /* Well, this region was inappropriate.  Try the next one. */
	} /* for j */

	/* Uh-oh.  No regions were appropriate.  Create a new one. */
	if (sizeRI == numRI) {
	    /* Oops, allocate space for new region information */
	    sizeRI *= 2;
	    ri = (RegionInfo *) xrealloc(ri, sizeRI * sizeof(RegionInfo));
	    rit = &(ri[numRI]);
	}
	numRI++;
	rit->prevBand = 0;
	rit->curBand = 0;
	rit->reg = miRegionCreate(box, (numRects-i+numRI) / numRI);

	Must_have_memory = TRUE; /* XXX */
NextRect: ;
    } /* for i */

    /* Make a final pass over each region in order to Coalesce and set
       extents.x2 and extents.y2 */

    for (j = 0, rit = ri; j != numRI; j++, rit++) {
	reg = rit->reg;
	riBox = &(reg->rects[reg->numRects-1]);
	reg->extents.y2 = riBox->y2;
	if (reg->extents.x2 < riBox->x2) reg->extents.x2 = riBox->x2;
	Coalesce(reg, rit->prevBand, rit->curBand);
    }

    Must_have_memory = FALSE; /* XXX */

    /* Step 3: Union all regions into a single region */
    while (numRI > 1) {
	int half = numRI/2;
	for (j = numRI & 1; j < (half + (numRI & 1)); j++) {
	    overlap |= miUnion(ri[j].reg, ri[j].reg, ri[j+half].reg);
	    miRegionDestroy(ri[j+half].reg);
	}
	numRI -= half;
    }

    /* Well, that's over with.  Just free up the ri array.  (Note that r[0].reg
       has not been freed, but this is good: it is the result region */
    xfree(ri);

    return overlap;
} /* miRegionFromRects */



/*======================================================================
 * 	    	  Region Subtraction
 *====================================================================*/


/*-
 *-----------------------------------------------------------------------
 * miSubtractO --
 *	Overlapping band subtraction. x1 is the left-most point not yet
 *	checked.
 *
 * Results:
 *	FALSE.
 *
 * Side Effects:
 *	pReg may have rectangles added to it.
 *
 *-----------------------------------------------------------------------
 */
static Bool
miSubtractO (pReg, r1, r1End, r2, r2End, y1, y2)
    register RegionPtr	pReg;
    register BoxPtr	r1;
    BoxPtr  	  	r1End;
    register BoxPtr	r2;
    BoxPtr  	  	r2End;
    register int  	y1;
             int  	y2;
{
    register BoxPtr	pNextRect;
    register int  	x1;

    x1 = r1->x1;
    
    assert(y1<y2);
    assert(r1 != r1End && r2 != r2End);

    pNextRect = &pReg->rects[pReg->numRects];

    do {
	if (r2->x2 <= x1) {
	    /*
	     * Subtrahend entirely to left of minuend: go to next subtrahend.
	     */
	    r2++;
	} else if (r2->x1 <= x1) {
	    /*
	     * Subtrahend preceeds minuend: nuke left edge of minuend.
	     */
	    x1 = r2->x2;
	    if (x1 >= r1->x2) {
		/*
		 * Minuend completely covered: advance to next minuend and
		 * reset left fence to edge of new minuend.
		 */
		r1++;
		if (r1 != r1End)
		    x1 = r1->x1;
	    } else {
		/*
		 * Subtrahend now used up since it doesn't extend beyond
		 * minuend
		 */
		r2++;
	    }
	} else if (r2->x1 < r1->x2) {
	    /*
	     * Left part of subtrahend covers part of minuend: add uncovered
	     * part of minuend to region and skip to next subtrahend.
	     */
	    assert(x1<r2->x1);
	    MEMCHECK(pReg, pNextRect, pReg->rects);
	    ADDRECTANGLE(pNextRect, x1, y1, r2->x1, y2);
	    pReg->numRects += 1;
	    assert(pReg->numRects<=pReg->size);

	    x1 = r2->x2;
	    if (x1 >= r1->x2) {
		/*
		 * Minuend used up: advance to new...
		 */
		r1++;
		if (r1 != r1End)
		    x1 = r1->x1;
	    } else {
		/*
		 * Subtrahend used up
		 */
		r2++;
	    }
	} else {
	    /*
	     * Minuend used up: add any remaining piece before advancing.
	     */
	    if (r1->x2 > x1) {
		MEMCHECK(pReg, pNextRect, pReg->rects);
		ADDRECTANGLE(pNextRect, x1, y1, r1->x2, y2);
		pReg->numRects += 1;
		assert(pReg->numRects<=pReg->size);
	    }
	    r1++;
	    if (r1 != r1End)
		x1 = r1->x1;
	}
    } while ((r1 != r1End) && (r2 != r2End));


    /*
     * Add remaining minuend rectangles to region.
     */
    while (r1 != r1End) {
	assert(x1<r1->x2);
	MEMCHECK(pReg, pNextRect, pReg->rects);
	ADDRECTANGLE(pNextRect, x1, y1, r1->x2, y2);
	pReg->numRects += 1;
	assert(pReg->numRects<=pReg->size);

	r1++;
	if (r1 != r1End) {
	    x1 = r1->x1;
	}
    }
    return FALSE;
}
	
/*-
 *-----------------------------------------------------------------------
 * miSubtract --
 *	Subtract regS from regM and leave the result in regD.
 *	S stands for subtrahend, M for minuend and D for difference.
 *
 * Results:
 *	TRUE.
 *
 * Side Effects:
 *	regD is overwritten.
 *
 *-----------------------------------------------------------------------
 */
int 
miSubtract(regD, regM, regS)
    register RegionPtr	regD;               
    register RegionPtr 	regM;
    register RegionPtr	regS;          
{
   /* check for trivial rejects */
    if ( (!(regM->numRects)) || (!(regS->numRects))  ||
	(!EXTENTCHECK(&regM->extents, &regS->extents)) )
    {
	miRegionCopy(regD, regM);
        return(1);
    }
 
    /* Add those rectangles in region 1 that aren't in region 2,
       do yucky substraction for overlaps, and
       just throw away rectangles in region 2 that aren't in region 1 */
    miRegionOp(regD, regM, regS, miSubtractO, TRUE, FALSE);

    /*
     * Can't alter newReg's extents before we call miRegionOp because
     * it might be one of the source regions and miRegionOp depends
     * on the extents of those regions being the unaltered. Besides, this
     * way there's no checking against rectangles that will be nuked
     * due to coalescing, so we have to examine fewer rectangles.
     */
    miSetExtents (regD);
    return(1);
}


/*
 *   RectIn(region, rect)
 *   This routine takes a pointer to a region and a pointer to a box
 *   and determines if the box is outside/inside/partly inside the region.
 *
 *   The idea is to travel through the list of rectangles trying to cover the
 *   passed box with them. Anytime a piece of the rectangle isn't covered
 *   by a band of rectangles, partOut is set TRUE. Any time a rectangle in
 *   the region covers part of the box, partIn is set TRUE. The process ends
 *   when either the box has been completely covered (we reached a band that
 *   doesn't overlap the box, partIn is TRUE and partOut is false), the
 *   box has been partially covered (partIn == partOut == TRUE -- because of
 *   the banding, the first time this is true we know the box is only
 *   partially in the region) or is outside the region (we reached a band
 *   that doesn't overlap the box at all and partIn is false)
 */

int
miRectIn(region, prect)
    register RegionPtr  region;
    register BoxPtr     prect;
{
    register int	x;
    register int	y;
    register BoxPtr     pbox;
    register BoxPtr     pboxEnd;
    int			partIn, partOut;


    /* this are (just) useful optimizations for the most common cases */
    if ((region->numRects == 0) || !EXTENTCHECK(&region->extents, prect))
        return(rgnOUT);

    if (region->numRects == 1) {
	/* We know that it must be rgnIN or rgnPART, because
	   region->extents == region->rects[0]. */
	if (	   region->extents.x1 <= prect->x1
		&& region->extents.x2 >= prect->x2
		&& region->extents.y1 <= prect->y1
		&& region->extents.y2 >= prect->y2)
	   return(rgnIN);
       else return(rgnPART);
    }

    partOut = FALSE;
    partIn = FALSE;

    /* (x,y) starts at upper left of rect, moving to the right and down */
    x = prect->x1;
    y = prect->y1;

    /* can stop when both partOut and partIn are TRUE, or we reach prect->y2 */
    for (pbox = region->rects, pboxEnd = pbox + region->numRects;
         pbox != pboxEnd;
         pbox++)
    {

        if (pbox->y2 <= y)
           continue;    /* getting up to speed or skipping remainder of band */

        if (pbox->y1 > y)
        {
           partOut = TRUE;      /* missed part of rectangle above */
           if (partIn || (pbox->y1 >= prect->y2))
              break;
           y = pbox->y1;        /* x guaranteed to be == prect->x1 */
        }

        if (pbox->x2 <= x)
           continue;            /* not far enough over yet */

        if (pbox->x1 > x)
        {
           partOut = TRUE;      /* missed part of rectangle to left */
           if (partIn)
              break;
        }

        if (pbox->x1 < prect->x2)
        {
            partIn = TRUE;      /* definitely overlap */
            if (partOut)
               break;
        }

        if (pbox->x2 >= prect->x2)
        {
           y = pbox->y2;        /* finished with this band */
           if (y >= prect->y2)
              break;
           x = prect->x1;       /* reset x out to left again */
        }
	else
	{
	    /*
	     * Because boxes in a band are maximal width, if the first box
	     * to overlap the rectangle doesn't completely cover it in that
	     * band, the rectangle must be partially out, since some of it
	     * will be uncovered in that band. partIn will have been set true
	     * by now...
	     */
	    partOut = TRUE;
	    break;
	}
    }

    return(partIn ? ((y < prect->y2) ? rgnPART : rgnIN) : rgnOUT);
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
    register BoxPtr pbox;

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
    xfree(pRegion->rects);
    xfree(pRegion);
}


void
miRegionReset(pRegion, pBox)
    RegionPtr pRegion;
    BoxPtr pBox;
{
    assert(pBox->x1<=pBox->x2);
    assert(pBox->y1<=pBox->y2);
    pRegion->extents = *pBox;
    pRegion->rects[0] = *pBox;
    
    if ((pBox->x1 == pBox->x2) || (pBox->y1 == pBox->y2))
	pRegion->numRects = 0;
    else
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
    BoxPtr box;     /* "return" value */
{
    register BoxPtr pbox, pboxEnd;

    if (pRegion->numRects == 0)
        return(FALSE);
    if (!INBOX(pRegion->extents, x, y))
        return(FALSE);
    for (pbox = pRegion->rects, pboxEnd = pbox + pRegion->numRects;
	 pbox != pboxEnd;
	 pbox++)
    {
        if (y >= pbox->y2)
	   continue;		/* not there yet */
	if ((y < pbox->y1) || (x < pbox->x1))
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

/* ||| I should apply the merge sort code to rectangle sorting above, and see
   if mapping time can be improved.  But right now I've been at work 12 hours,
   so forget it.
*/

static void QuickSortSpans(spans, widths, numSpans)
    register DDXPointRec    spans[];
    register int	    widths[];
    register int	    numSpans;
{
    register int	    y;
    register int	    i, j, m;
    register DDXPointPtr    r;

/* Always called with numSpans > 1 */
/* Sorts only by y, doesn't bother to sort by x */

#define ExchangeSpans(a, b)				    \
{							    \
    DDXPointRec     tpt;				    \
    register int    tw;					    \
							    \
    tpt = spans[a]; spans[a] = spans[b]; spans[b] = tpt;    \
    tw = widths[a]; widths[a] = widths[b]; widths[b] = tw;  \
}

    do {
	if (numSpans < 9) {
	    /* Do insertion sort */
	    register int yprev;

	    yprev = spans[0].y;
	    i = 1;
	    do { /* while i != numSpans */
		y = spans[i].y;
		if (yprev > y) {
		    /* spans[i] is out of order.  Move into proper location. */
		    DDXPointRec tpt;
		    int	    tw, k;

		    for (j = 0; y >= spans[j].y; j++) {}
		    tpt = spans[i];
		    tw  = widths[i];
		    for (k = i; k != j; k--) {
			spans[k] = spans[k-1];
			widths[k] = widths[k-1];
		    }
		    spans[j] = tpt;
		    widths[j] = tw;
		    y = spans[i].y;
		} /* if out of order */
		yprev = y;
		i++;
	    } while (i != numSpans);
	    return;
	}

	/* Choose partition element, stick in location 0 */
	m = numSpans / 2;
	if (spans[m].y > spans[0].y)		ExchangeSpans(m, 0);
	if (spans[m].y > spans[numSpans-1].y)   ExchangeSpans(m, numSpans-1);
	if (spans[m].y > spans[0].y)		ExchangeSpans(m, 0);
	y = spans[0].y;

        /* Partition array */
        i = 0;
        j = numSpans;
        do {
	    r = &(spans[i]);
	    do {
		r++;
		i++;
            } while (i != numSpans && r->y < y);
	    r = &(spans[j]);
	    do {
		r--;
		j--;
            } while (y < r->y);
            if (i < j) ExchangeSpans(i, j);
        } while (i < j);

        /* Move partition element back to middle */
        ExchangeSpans(0, j);

	/* Recurse */
        if (numSpans-j-1 > 1)
	    QuickSortSpans(&spans[j+1], &widths[j+1], numSpans-j-1);
        numSpans = j;
    } while (numSpans > 1);
} /* QuickSortSpans */

/*
    Clip a list of scanlines to a region.  The caller has allocated the
    spce.  FSorted is non-zero if the scanline origins are in ascending
    order.
    returns the number of new, clipped scanlines.
*/

int
miClipSpans(prgnDst, ppt, pwidth, nspans, pptNew, pwidthNew, fSorted)
    RegionPtr		    prgnDst;
    register DDXPointPtr    ppt;
    register int	    *pwidth;
    int			    nspans;
    register DDXPointPtr    pptNew;
    int			    *pwidthNew;
    int			    fSorted;
{
    register DDXPointPtr pptLast;
    int			*pwidthNewStart;	/* the vengeance of Xerox! */
    register int	y, x1, x2;
    register int	numRects;

    pptLast = ppt + nspans;
    pwidthNewStart = pwidthNew;
    numRects = prgnDst->numRects;

    if (numRects == 1) {
	/* Do special fast code with clip boundaries in registers */
	/* It doesn't pay much to make use of fSorted in this case, 
	   so we lump everything together. */

	register    int clipx1, clipx2, clipy1, clipy2;

	clipx1 = prgnDst->extents.x1;
	clipy1 = prgnDst->extents.y1;
	clipx2 = prgnDst->extents.x2;
	clipy2 = prgnDst->extents.y2;
	    
	for (; ppt != pptLast; ppt++, pwidth++) {
	    y = ppt->y;
	    x1 = ppt->x;
	    if (clipy1 <= y && y < clipy2) {
		x2 = x1 + *pwidth;
		if (x1 < clipx1)    x1 = clipx1;
		if (x2 > clipx2)    x2 = clipx2;
		if (x1 < x2) {
		    /* part of span in clip rectangle */
		    pptNew->x = x1;
		    pptNew->y = y;
		    *pwidthNew = x2 - x1;
		    pptNew++;
		    pwidthNew++;
		}
	    }
	} /* end for */

    } else if (numRects > 0) {
	/* Have to clip against many boxes */
	BoxPtr		pboxBandStart, pboxBandEnd;
	register BoxPtr pbox;
	register BoxPtr pboxLast;
	register int	clipy1, clipy2;

	/* In this case, taking advantage of sorted spans gains more than
	   the sorting costs. */
	if ((! fSorted) && (nspans > 1)) {
	    QuickSortSpans(ppt, pwidth, nspans);
	}

	pboxBandStart = prgnDst->rects;
	pboxLast = pboxBandStart + numRects;
    
#define NextBand()						    \
{								    \
    clipy1 = pboxBandStart->y1;					    \
    clipy2 = pboxBandStart->y2;					    \
    pboxBandEnd = pboxBandStart + 1;				    \
    while (pboxBandEnd != pboxLast && pboxBandEnd->y1 == clipy1) {  \
	pboxBandEnd++;						    \
    }								    \
    for (; ppt != pptLast && ppt->y < clipy1; ppt++, pwidth++) {} \
}

	NextBand();

	for (; ppt != pptLast; ) {
	    y = ppt->y;
	    if (y < clipy2) {
		/* span is in the current band */
		pbox = pboxBandStart;
		x1 = ppt->x;
		x2 = x1 + *pwidth;
		do { /* For each box in band */
		    register int    newx1, newx2;

		    newx1 = x1;
		    newx2 = x2;
		    if (newx1 < pbox->x1)   newx1 = pbox->x1;
		    if (newx2 > pbox->x2)   newx2 = pbox->x2;
		    if (newx1 < newx2) {
			/* Part of span in clip rectangle */
			pptNew->x = newx1;
			pptNew->y = y;
			*pwidthNew = newx2 - newx1;
			pptNew++;
			pwidthNew++;
		    }
		    pbox++;
		} while (pbox != pboxBandEnd);
		ppt++;
		pwidth++;
	    } else {
		/* Move to next band, adjust ppt as needed */
		pboxBandStart = pboxBandEnd;
		if (pboxBandStart == pboxLast) {
		    /* We're completely done */
		    break;
		}
		NextBand();
	    }
	}
    }
    return (pwidthNew - pwidthNewStart);
}

/* find the band in a region with the most rectangles */
int
miFindMaxBand(prgn)
RegionPtr prgn;
{
    register int nbox;
    register BoxPtr pbox;
    register int nThisBand;
    register int nMaxBand = 0;
    short yThisBand;

    nbox = prgn->numRects;
    pbox = prgn->rects;

    while(nbox > 0) {
	yThisBand = pbox->y1;
	nThisBand = 0;
	while((nbox > 0) && (pbox->y1 == yThisBand)) {
	    nbox--;
	    pbox++;
	    nThisBand++;
	}
	if (nThisBand > nMaxBand)
	    nMaxBand = nThisBand;
    }
    return (nMaxBand);
}
