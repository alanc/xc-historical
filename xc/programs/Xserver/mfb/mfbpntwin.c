/* $XConsortium: mfbpntwin.c,v 5.10 93/10/12 11:28:52 dpw Exp $ */
/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
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

#include "X.h"

#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "mfb.h"
#include "maskbits.h"
#include "mi.h"

void
mfbPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    register mfbPrivWin	*pPrivWin;

    pPrivWin = (mfbPrivWin *)(pWin->devPrivates[mfbWindowPrivateIndex].ptr);
    
    switch (what) {
    case PW_BACKGROUND:
	switch (pWin->backgroundState) {
	case None:
	    return;
	case ParentRelative:
	    do {
		pWin = pWin->parent;
	    } while (pWin->backgroundState == ParentRelative);
	    (*pWin->drawable.pScreen->PaintWindowBackground)(pWin, pRegion,
							     what);
	    return;
	case BackgroundPixmap:
	    if (pPrivWin->fastBackground)
	    {
		mfbTileAreaPPWCopy((DrawablePtr)pWin, REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion), GXcopy,
				  pPrivWin->pRotatedBackground);
		return;
	    }
	    break;
	case BackgroundPixel:
	    if (pWin->background.pixel & 1)
		mfbSolidWhiteArea((DrawablePtr)pWin, REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion), GXset, NullPixmap);
	    else
		mfbSolidBlackArea((DrawablePtr)pWin, REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion), GXclear, NullPixmap);
	    return;
    	}
    	break;
    case PW_BORDER:
	if (pWin->borderIsPixel)
	{
	    if (pWin->border.pixel & 1)
		mfbSolidWhiteArea((DrawablePtr)pWin, REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion), GXset, NullPixmap);
	    else
		mfbSolidBlackArea((DrawablePtr)pWin, REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion), GXclear, NullPixmap);
	    return;
	}
	else if (pPrivWin->fastBorder)
	{
	    mfbTileAreaPPWCopy((DrawablePtr)pWin, REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion), GXcopy,
				  pPrivWin->pRotatedBorder);
	    return;
	}
	break;
    }
    miPaintWindow(pWin, pRegion, what);
}
