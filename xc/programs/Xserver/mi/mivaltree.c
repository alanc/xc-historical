/*
 * mivaltree.c --
 *	Functions for recalculating window clip lists. Main function
 *	is miValidateTree.
 *
 * Copyright 1987, 1988, 1989 by 
 * Digital Equipment Corporation, Maynard, Massachusetts,
 * and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the names of Digital or MIT not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * 
 ******************************************************************/

 /* 
  * Aug '86: Susan Angebranndt -- original code
  * July '87: Adam de Boor -- substantially modified and commented
  * Summer '88: Joel McCormack -- so fast you wouldn't believe it possible.
  *             In particular, much improved code for window mapping and
  *             circulating.
  */

#ifndef lint
static char rcsid[] =
"$Header: mivaltree.c,v 5.2 89/07/05 20:22:03 rws Exp $ SPRITE (Berkeley)";
#endif lint

#include    <stdio.h>
#include    "X.h"
#include    "scrnintstr.h"
#include    "windowstr.h"
#include    "mi.h"
#include    "regionstr.h"

static void	(*clipNotify)() = 0;

extern void miRegionAppend();

/*
 * miClipNotify --
 *	Hook to let DDX request notification when the clipList of
 *	a window is recomputed.
 *
 *	clipNotify is expected to be a function receiving three arguments,
 *	a window, and an x and y offset -- the amount the window has
 *	been moved.
 */

void
miClipNotify (func)
void	(*func)();
{
	clipNotify = func;
}

/*-
 *-----------------------------------------------------------------------
 * miComputeClips --
 *	Recompute the clipList, borderClip, exposed and borderExposed
 *	regions for pParent and its children. Only viewable windows are
 *	taken into account.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	clipList, borderClip, exposed and borderExposed are altered.
 *	A VisibilityNotify event may be generated on the parent window.
 *
 *-----------------------------------------------------------------------
 */

static void
miComputeClips (pParent, pScreen, universe, kind, exposed)
    register WindowPtr	pParent;
    register ScreenPtr	pScreen;
    register RegionPtr	universe;
    VTKind		kind;
    RegionPtr		exposed; /* for intermediate calculations */
{
    int			dx,
			dy;
    RegionPtr		childUniverse;
    register WindowPtr	pChild;
    int     	  	oldVis;
    register BoxPtr  	borderSize;
    RegionPtr		childUnion;
    Bool		overlap;
    
    /*
     * Figure out the new visibility of this window.
     * The extent of the universe should be the same as the extent of
     * the borderSize region. If the window is unobscured, this rectangle
     * will be completely inside the universe (the universe will cover it
     * completely). If the window is completely obscured, none of the
     * universe will cover the rectangle.
     */
    borderSize = (* pScreen->RegionExtents) (pParent->borderSize);
    
    oldVis = pParent->visibility;
    switch ((* pScreen->RectIn) (universe, borderSize)) 
    {
	case rgnIN:
	    if (((borderSize->x2 - borderSize->x1) ==
		 (pParent->drawable.width + (wBorderWidth (pParent) << 1)))
		&&
		((borderSize->y2 - borderSize->y1) ==
		 (pParent->drawable.height + (wBorderWidth (pParent) << 1))))
		pParent->visibility = VisibilityUnobscured;
	    else
		pParent->visibility = VisibilityPartiallyObscured;
	    break;
	case rgnPART:
	    pParent->visibility = VisibilityPartiallyObscured;
	    break;
	default:
	    pParent->visibility = VisibilityFullyObscured;
	    break;
    }
    if (oldVis != pParent->visibility && pParent->realized)
	SendVisibilityNotify(pParent);

    /*
     * avoid computations when dealing with simple operations
     */

    dx = pParent->drawable.x - pParent->valdata->oldAbsCorner.x;
    dy = pParent->drawable.y - pParent->valdata->oldAbsCorner.y;
    
    switch (kind) {
    case VTMap:
    case VTStack:
    case VTUnmap:
	break;
    case VTMove:
    	/*
     	 * To calculate exposures correctly, we have to translate the old
     	 * borderClip and clipList regions to the window's new location so there
     	 * is a correspondence between pieces of the new and old clipping regions.
     	 */
    	if (dx || dy) 
    	{
	    /*
	     * We translate the old clipList because that will be exposed or copied
	     * if gravity is right.
	     */
	    (* pScreen->TranslateRegion) (pParent->borderClip, dx, dy);
	    (* pScreen->TranslateRegion) (pParent->clipList, dx, dy);
    	} 
	break;
    default:
    	/*
     	 * To calculate exposures correctly, we have to translate the old
     	 * borderClip and clipList regions to the window's new location so there
     	 * is a correspondence between pieces of the new and old clipping regions.
     	 */
    	if (dx || dy) 
    	{
	    /*
	     * We translate the old clipList because that will be exposed or copied
	     * if gravity is right.
	     */
	    (* pScreen->TranslateRegion) (pParent->borderClip, dx, dy);
	    (* pScreen->TranslateRegion) (pParent->clipList, dx, dy);
    	} 
    	else if (pParent->borderWidth) 
    	{
	    /*
	     * If the window has shrunk, we have to be careful about figuring the
	     * exposure of the right and bottom borders -- they should be exposed
	     * if the window shrank on that side and they aren't being obscured
	     * by a sibling -- so we add those edges to the borderExposed region.
	     * This is all necessary because sometimes what was the internal window
	     * in the old borderClip will overlap the new border and cause the
	     * right and bottom edges of the new border not to appear in the
	     * borderExposed region and it's a royal pain to figure out what to
	     * remove from the old borderClip.
	     * Note that borderSize is clipped to the window's parent, thus its
	     * extents are not necessarily those of the window's border, thus we
	     * must use physical position to find the actual extents of the window.
	     * XXX: Isn't there a nicer way to do this?  Yes, but we'll
	     * have to save the old clip lists while validating.
	     */
	    BoxPtr	oldExtents;
	    BoxPtr	newExtents;
	    
	    oldExtents = (* pScreen->RegionExtents) (pParent->borderClip);
	    newExtents = (* pScreen->RegionExtents) (universe);
    
	    if ((* pScreen->RegionNotEmpty) (universe) &&
	    	(* pScreen->RegionNotEmpty) (pParent->borderClip) &&
	    	((newExtents->x2 < oldExtents->x2) ||
	     	 (newExtents->y2 < oldExtents->y2))) 
            {
	    	BoxRec 	borderBox;
	    	RegionPtr 	borderRegion;
	    	int		x1, y1, x2, y2, v;
    
	    	borderRegion = (* pScreen->RegionCreate) (NULL, 1);
	    	/* Worry about overflow: don't use a Box.  Sigh.
	     	 * We know we'll clip to universe later, so do it now as well.
	     	 */
	    	x1 = pParent->drawable.x - wBorderWidth (pParent);
	    	if (x1 < newExtents->x1)
		    x1 = newExtents->x1;
	    	y1 = pParent->drawable.y - wBorderWidth (pParent);
	    	if (y1 < newExtents->y1)
		    y1 = newExtents->y1;
	    	x2 = pParent->drawable.x +
		     (int)pParent->drawable.width +
		     wBorderWidth (pParent);
	    	if (x2 > newExtents->x2)
		    x2 = newExtents->x2;
	    	y2 = pParent->drawable.y +
		     (int)pParent->drawable.height +
		     wBorderWidth (pParent);
	    	if (y2 > newExtents->y2)
		    y2 = newExtents->y2;
	    	if (x1 > x2)
		    x2 = x1;
	    	if (y1 > y2)
		    y2 = y1;
    
	    	if (newExtents->x2 < oldExtents->x2) 
            	{
		    /*
		     * Add the right edge.
		     */
		    v = pParent->drawable.x + (int)pParent->drawable.width;
		    if (v < x1)
		    	v = x1;
		    if ((v < x2) && (y1 < y2))
		    {
		    	borderBox.x1 = v;
		    	borderBox.y1 = y1;
		    	borderBox.x2 = x2;
		    	borderBox.y2 = y2;
		    	(* pScreen->RegionReset) (borderRegion, &borderBox);
		    	(* pScreen->Union) (pParent->valdata->borderExposed,
					    pParent->valdata->borderExposed,
					    borderRegion);
		    }
	    	}
	    	if (newExtents->y2 < oldExtents->y2) 
            	{
		    /*
		     * Add the bottom edge.
		     */
		    v = pParent->drawable.y + (int)pParent->drawable.height;
		    if (v < y1)
		    	v = y1;
		    if ((v < y2) && (x1 < x2))
		    {
		    	borderBox.x1 = x1;
		    	borderBox.y1 = v;
		    	borderBox.x2 = x2;
		    	borderBox.y2 = y2;
		    	(* pScreen->RegionReset) (borderRegion, &borderBox);
		    	(* pScreen->Union) (pParent->valdata->borderExposed,
					    pParent->valdata->borderExposed,
					    borderRegion);
		    }
	    	}
	    	(* pScreen->RegionDestroy) (borderRegion);
    
	    	/*
	     	 * To make sure we don't expose a border that's supposed to
	     	 * be clipped, clip the regions we just added to borderExposed...
	     	 */
	    	(* pScreen->Intersect) (pParent->valdata->borderExposed,
					universe,
				    	pParent->valdata->borderExposed);
	    }
    	}
    }

    pParent->valdata->oldAbsCorner.x = pParent->drawable.x;
    pParent->valdata->oldAbsCorner.y = pParent->drawable.y;

    /*
     * Since the borderClip must not be clipped by the children, we do
     * the border exposure first...
     *
     * 'universe' is the window's borderClip. To figure the exposures, remove
     * the area that used to be exposed (the old borderClip) from the new.
     * This leaves a region of pieces that weren't exposed before. Border
     * exposures accumulate until they're taken care of.
     */

    (* pScreen->Subtract) (exposed, universe, pParent->borderClip);
    (* pScreen->Subtract) (exposed, exposed, pParent->winSize);

    (* pScreen->Union) (pParent->valdata->borderExposed,
			pParent->valdata->borderExposed,
			exposed);

    (* pScreen->RegionCopy) (pParent->borderClip, universe);

    /*
     * To get the right clipList for the parent, and to make doubly sure
     * that no child overlaps the parent's border, we remove the parent's
     * border from the universe before proceeding.
     */

    (* pScreen->Intersect) (universe, universe, pParent->winSize);
    
    if (pParent->firstChild) {
	childUniverse = (* pScreen->RegionCreate) (NULL, 1);
	
	childUnion = (* pScreen->RegionCreate) (NULL, 200);
	for (pChild = pParent->firstChild; pChild; pChild = pChild->nextSib) {
	    if (pChild->viewable) {
		miRegionAppend
		/* (* pScreen->RegionAppend) */ (childUnion, pChild->borderSize);
	    }
	}
	overlap = miRegionValidate /* (* pScreen->RegionValidate) */ (childUnion);

	for (pChild = pParent->firstChild;
	     pChild;
	     pChild = pChild->nextSib)
 	{
	    if (pChild->viewable) {
		/*
		 * If the child is viewable, we want to remove its extents
		 * from the current universe, but we only re-clip it if
		 * it's been marked.
		 */
		if (pChild->valdata) {
		    /*
		     * Figure out the new universe from the child's
		     * perspective and recurse.
		     */
		    (* pScreen->Intersect) (childUniverse,
					    universe,
					    pChild->borderSize);
		    miComputeClips (pChild, pScreen, childUniverse, kind,
				    exposed);
		}
		/*
		 * Once the child has been processed, we remove its extents
		 * from the current universe, thus denying its space to any
		 * other sibling.
		 */
		if (overlap)
		    (* pScreen->Subtract)
			(universe, universe, pChild->borderSize);
	    }
	}
	if (!overlap)
	    (* pScreen->Subtract) (universe, universe, childUnion);
	(* pScreen->RegionDestroy) (childUnion);
	(* pScreen->RegionDestroy) (childUniverse);
    } /* if any children */

    /*
     * 'universe' now contains the new clipList for the parent window.
     *
     * To figure the exposure of the window we subtract the old clip from the
     * new, just as for the border. Again, exposures accumulate.
     */

    (* pScreen->Subtract) (exposed, universe, pParent->clipList);
    (* pScreen->Union) (pParent->valdata->exposed,
			pParent->valdata->exposed, exposed);

    /*
     * One last thing: backing storage. We have to try to save what parts of
     * the window are about to be obscured. We can just subtract the universe
     * from the old clipList and get the areas that were in the old but aren't
     * in the new and, hence, are about to be obscured.
     */
    if (pParent->backStorage)
    {
	(* pScreen->Subtract) (exposed, pParent->clipList, universe);
	(*pParent->backStorage->funcs->SaveDoomedAreas)(
					    pParent, exposed, dx, dy);
    }
    
    (* pScreen->RegionCopy) (pParent->clipList, universe);

    pParent->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    if (clipNotify)
	(* clipNotify) (pParent, dx, dy);
}

static void
miTreeObscured(pParent)
    register WindowPtr pParent;
{
    register WindowPtr pChild;

    pChild = pParent;
    while (1)
    {
	if (pChild->viewable)
	{
	    pChild->visibility = VisibilityFullyObscured;
	    SendVisibilityNotify(pChild);
	    if (pChild->firstChild)
	    {
		pChild = pChild->firstChild;
		continue;
	    }
	}
	while (!pChild->nextSib && (pChild != pParent))
	    pChild = pChild->parent;
	if (pChild == pParent)
	    break;
	pChild = pChild->nextSib;
    }
}

/*-
 *-----------------------------------------------------------------------
 * miValidateTree --
 *	Recomputes the clip list for pParent and all its inferiors.
 *
 * Results:
 *	Always returns 1.
 *
 * Side Effects:
 *	The clipList, borderClip, exposed, and borderExposed regions for
 *	each marked window are altered.
 *
 * Notes:
 *	This routine assumes that all affected windows have been marked
 *	(valdata created) and their winSize and borderSize regions
 *	adjusted to correspond to their new positions. The borderClip and
 *	clipList regions should not have been touched.
 *
 *	The top-most level is treated differently from all lower levels
 *	because pParent is unchanged. For the top level, we merge the
 *	regions taken up by the marked children back into the clipList
 *	for pParent, thus forming a region from which the marked children
 *	can claim their areas. For lower levels, where the old clipList
 *	and borderClip are invalid, we can't do this and have to do the
 *	extra operations done in miComputeClips, but this is much faster
 *	e.g. when only one child has moved...
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
int
miValidateTree (pParent, pChild, kind)
    WindowPtr	  	pParent;    /* Parent to validate */
    WindowPtr	  	pChild;     /* First child of pParent that was
				     * affected */
    VTKind    	  	kind;       /* What kind of configuration caused call */
{
    RegionPtr	  	totalClip;  /* Total clipping region available to
				     * the marked children. pParent's clipList
				     * merged with the borderClips of all
				     * the marked children. */
    RegionPtr	  	childClip;  /* The new borderClip for the current
				     * child */
    RegionPtr		exposed;    /* For intermediate calculations */
    register ScreenPtr	pScreen;
    register WindowPtr	pWin;

    pScreen = pParent->drawable.pScreen;
    if (pChild == NullWindow)
	pChild = pParent->firstChild;

    childClip = (* pScreen->RegionCreate) (NULL, 1);
    exposed = (* pScreen->RegionCreate) (NULL, 1);

    /*
     * compute the area of the parent window occupied
     * by the marked children + the parent itself.  This
     * is the area which can be divied up among the marked
     * children in their new configuration.
     */
    totalClip = (* pScreen->RegionCreate) (NULL, 200);
    for (pWin = pChild; pWin != NullWindow; pWin = pWin->nextSib)
    {
	if (pWin->valdata)
	    /* (* pScreen->RegionAppend) */
	    miRegionAppend (totalClip, pWin->borderClip);
    }
    
    (void) /* (* pScreen->RegionValidate) */ miRegionValidate (totalClip);

    /*
     * Now go through the children of the root and figure their new
     * borderClips from the totalClip, passing that off to miComputeClips
     * to handle recursively. Once that's done, we remove the child
     * from the totalClip to clip any siblings below it.
     */

    if (kind != VTStack)
	(* pScreen->Union) (totalClip, totalClip, pParent->clipList);

    for (pWin = pChild;
	 pWin != NullWindow;
	 pWin = pWin->nextSib)
    {
	if (pWin->viewable) {
	    if (pWin->valdata) {
		(* pScreen->Intersect) (childClip,
					totalClip,
 					pWin->borderSize);
		miComputeClips (pWin, pScreen, childClip, kind, exposed);
		(* pScreen->Subtract) (totalClip,
				       totalClip,
				       pWin->borderSize);
	    } else if (pWin->visibility == VisibilityNotViewable) {
		miTreeObscured(pWin);
	    }
	} else {
	    if (pWin->valdata) {
		(* pScreen->RegionEmpty)(pWin->clipList);
		(* pScreen->RegionEmpty)(pWin->borderClip);
	    }
	}
    }

    (* pScreen->RegionDestroy) (childClip);

    /*
     * each case below is responsible for updating the
     * clipList and serial number for the parent window
     */

    switch (kind) {
    case VTStack:
	break;
    default:
	/*
	 * totalClip contains the new clipList for the parent. Figure out
	 * exposures and obscures as per miComputeClips and reset the parent's
	 * clipList.
	 */
	(* pScreen->Subtract) (exposed, totalClip, pParent->clipList);
	(* pScreen->Union) (pParent->valdata->exposed,
			    pParent->valdata->exposed, exposed);
	/* fall through */
    case VTMap:
	if (pParent->backStorage) {
	    (* pScreen->Subtract) (exposed, pParent->clipList, totalClip);
	    (*pParent->backStorage->funcs->SaveDoomedAreas)(
					    pParent, exposed, 0, 0);
	}
	
	(* pScreen->RegionCopy) (pParent->clipList, totalClip);
	pParent->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	break;
    }

    (* pScreen->RegionDestroy) (totalClip);
    (* pScreen->RegionDestroy) (exposed);
    return (1);
}
