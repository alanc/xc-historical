/* $XConsortium: mfbwindow.c,v 5.0 89/06/09 15:07:08 keith Exp $ */
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
#include "scrnintstr.h"
#include "windowstr.h"
#include "mfb.h"
#include "mistruct.h"
#include "regionstr.h"

extern WindowPtr *WindowTable;

static WindowFuncs commonFuncs[] = {
    {
	mfbPaintWindow32,	/* PaintWindowBackground */
	mfbPaintWindow32,	/* PaintWindowBorder */
	mfbCopyWindow,		/* CopyWindow */
	miClearToBackground,	/* ClearToBackground */
    },
    {
	mfbPaintWindowSolid,	/* PaintWindowBackground */
	mfbPaintWindow32,	/* PaintWindowBorder */
	mfbCopyWindow,		/* CopyWindow */
	miClearToBackground,	/* ClearToBackground */
    },
    {
	mfbPaintWindowSolid,	/* PaintWindowBackground */
	mfbPaintWindowSolid,	/* PaintWindowBorder */
	mfbCopyWindow,		/* CopyWindow */
	miClearToBackground,	/* ClearToBackground */
    },
    {
	mfbPaintWindowNone,
	mfbPaintWindowPR,
	mfbCopyWindow,
	miClearToBackground,
    },
};

#define NumCommonFuncs	(sizeof (commonFuncs) / sizeof (commonFuncs[0]))

Bool
mfbCreateWindow(pWin)
    register WindowPtr pWin;
{
    register mfbPrivWin *pPrivWin;

    pWin->funcs = &commonFuncs[NumCommonFuncs-1];
    pPrivWin = (mfbPrivWin *)xalloc(sizeof(mfbPrivWin));
    pWin->devPrivates[mfbWindowPrivateIndex].ptr = (pointer)pPrivWin;
    if (!pPrivWin)
	 return (FALSE);
    pPrivWin->pRotatedBorder = NullPixmap;
    pPrivWin->pRotatedBackground = NullPixmap;
    pPrivWin->fastBackground = 0;
    pPrivWin->fastBorder = 0;

    return (TRUE);
}

/* This always returns true, because Xfree can't fail.  It might be possible
 * on some devices for Destroy to fail */
Bool 
mfbDestroyWindow(pWin)
    WindowPtr pWin;
{
    register mfbPrivWin *pPrivWin;

    pPrivWin = (mfbPrivWin *)(pWin->devPrivates[mfbWindowPrivateIndex].ptr);

    if (pPrivWin->pRotatedBorder)
	mfbDestroyPixmap(pPrivWin->pRotatedBorder);
    if (pPrivWin->pRotatedBackground)
	mfbDestroyPixmap(pPrivWin->pRotatedBackground);
    xfree(pWin->devPrivates[mfbWindowPrivateIndex].ptr);
    return (TRUE);
}

/*ARGSUSED*/
Bool mfbMapWindow(pWindow)
    WindowPtr pWindow;
{
    return (TRUE);
}

/* (x, y) is the upper left corner of the window on the screen 
   do we really need to pass this?  (is it a;ready in pWin->absCorner?)
   we only do the rotation for pixmaps that are 32 bits wide (padded
or otherwise.)
   mfbChangeWindowAttributes() has already put a copy of the pixmap
in pPrivWin->pRotated*
*/

/*ARGSUSED*/
Bool 
mfbPositionWindow(pWin, x, y)
    register WindowPtr pWin;
    int x, y;
{
    register mfbPrivWin *pPrivWin;
    int	reset = 0;

    pPrivWin = (mfbPrivWin *)(pWin->devPrivates[mfbWindowPrivateIndex].ptr);
    if (pWin->backgroundState == BackgroundPixmap &&
	(pPrivWin->fastBackground != 0))
    {
	mfbXRotatePixmap(pPrivWin->pRotatedBackground,
			 pWin->drawable.x - pPrivWin->oldRotate.x);
	mfbYRotatePixmap(pPrivWin->pRotatedBackground,
			 pWin->drawable.y - pPrivWin->oldRotate.y);
	reset = 1;
    }

    if (pWin->borderIsPixel == FALSE &&
	(pPrivWin->fastBorder != 0))
    {
	mfbXRotatePixmap(pPrivWin->pRotatedBorder,
			 pWin->drawable.x - pPrivWin->oldRotate.x);
	mfbYRotatePixmap(pPrivWin->pRotatedBorder,
			 pWin->drawable.y - pPrivWin->oldRotate.y);
	reset = 1;
    }
    if (reset)
    {
	pPrivWin->oldRotate.x = pWin->drawable.x;
	pPrivWin->oldRotate.y = pWin->drawable.y;
    }

    /* XXX  THIS IS THE WRONG FIX TO THE RIGHT PROBLEM   XXX
     * When the window is moved, we need to invalidate any RotatedTile or
     * RotatedStipple that exists in any GC currently validated against
     * this window.  Bumping the serialNumber here is an expensive way to
     * accomplish this.  A better fix is to have the rotated versions
     * computed on demand by the routines that need them.  Have ValidateGC
     * simply destroy the rotated versions when invalidated, add a flag to
     * mfbPrivWin to indicate that the position has changed, set that flag
     * here, and have routines that need to get the rotated versions check
     * for a null pixmap or for the flag being set, and if so call a routine
     * to recompute the correct rotations.  However, it is unknown how many
     * ddx layers not under our control would break as a result, so for the
     * moment we play it safe (and slow).
     */
    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    /* Again, we have no failure modes indicated by any of the routines
     * we've called, so we have to assume it worked */
    return (TRUE);
}

/*ARGSUSED*/
Bool 
mfbUnmapWindow(pWindow)
    WindowPtr pWindow;
{
    return (TRUE);
}

/* UNCLEAN!
   this code calls the bitblt helper code directly.

   mfbCopyWindow copies only the parts of the destination that are
visible in the source.
*/


void 
mfbCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    RegionPtr prgnDst;
    register BoxPtr pbox;
    register int dx, dy;
    register int i, nbox;
    WindowPtr pwinRoot;

    pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];

    prgnDst = (* pWin->drawable.pScreen->RegionCreate)(NULL, 
					       pWin->borderClip->numRects);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    (* pWin->drawable.pScreen->TranslateRegion)(prgnSrc, -dx, -dy);
    (* pWin->drawable.pScreen->Intersect)(prgnDst, pWin->borderClip, prgnSrc);

    pbox = prgnDst->rects;
    nbox = prgnDst->numRects;
    if(!(pptSrc = (DDXPointPtr )ALLOCATE_LOCAL( prgnDst->numRects *
      sizeof(DDXPointRec))))
	return;
    ppt = pptSrc;

#ifndef PURDUE
    for (i=0; i<nbox; i++, ppt++, pbox++)
    {
	ppt->x = pbox->x1 + dx;
	ppt->y = pbox->y1 + dy;
    }
#else
    i = nbox;
    Duff(i, ppt->x = pbox->x1 + dx; ppt->y = pbox->y1 + dy; ppt++; pbox++);
#endif  /* PURDUE */

    mfbDoBitblt((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
		GXcopy, prgnDst, pptSrc);
    DEALLOCATE_LOCAL(pptSrc);
    (* pWin->drawable.pScreen->RegionDestroy)(prgnDst);
}



/* swap in correct PaintWindow* routine.  If we can use a fast output
routine (i.e. the pixmap is paddable to 32 bits), also pre-rotate a copy
of it in devPrivate.
*/
Bool
mfbChangeWindowAttributes(pWin, mask)
    register WindowPtr pWin;
    register unsigned long mask;
{
    register unsigned long index;
    register mfbPrivWin *pPrivWin;
    WindowFuncs		newFuncs;

    pPrivWin = (mfbPrivWin *)(pWin->devPrivates[mfbWindowPrivateIndex].ptr);
    newFuncs = *pWin->funcs;
    while(mask)
    {
	index = lowbit (mask);
	mask &= ~index;
	switch(index)
	{
#ifdef NOTDEF
	  case CWBackingStore:
	      if (pWin->backingStore != NotUseful)
	      {
		  miInitBackingStore(pWin, mfbSaveAreas, mfbRestoreAreas, (void (*)()) 0);
	      }
	      else
	      {
		  miFreeBackingStore(pWin);
	      }
	      /*
	       * XXX: The changing of the backing-store status of a window
	       * is serious enough to warrant a validation, since otherwise
	       * the backing-store stuff won't work.
	       */
	      pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	      break;
#endif

	  case CWBackPixmap:
	      if (pWin->backgroundState == None)
	      {
		  newFuncs.PaintWindowBackground = mfbPaintWindowNone;
		  pPrivWin->fastBackground = 0;
	      }
	      else if (pWin->backgroundState == ParentRelative)
	      {
		  newFuncs.PaintWindowBackground = mfbPaintWindowPR;
		  pPrivWin->fastBackground = 0;
	      }
	      else if ((pWin->background.pixmap->drawable.width <= 32) &&
		       !(pWin->background.pixmap->drawable.width &
			 (pWin->background.pixmap->drawable.width - 1)))
	      {
		  if (pPrivWin->pRotatedBackground)
		      mfbDestroyPixmap(pPrivWin->pRotatedBackground);
		  pPrivWin->pRotatedBackground =
		    mfbCopyPixmap(pWin->background.pixmap);
		  if (pPrivWin->pRotatedBackground)
		  {
		      pPrivWin->fastBackground = 1;
		      (void)mfbPadPixmap(pPrivWin->pRotatedBackground);
		      mfbXRotatePixmap(pPrivWin->pRotatedBackground,
				       pWin->drawable.x);
		      mfbYRotatePixmap(pPrivWin->pRotatedBackground,
				       pWin->drawable.y);
		      pPrivWin->oldRotate.x = pWin->drawable.x;
		      pPrivWin->oldRotate.y = pWin->drawable.y;
		      newFuncs.PaintWindowBackground = mfbPaintWindow32;
		  }
		  else
		  {
		      pPrivWin->fastBackground = 0;
		      newFuncs.PaintWindowBackground = miPaintWindow;
		  }
	      }
	      else
	      {
		  pPrivWin->fastBackground = 0;
		  newFuncs.PaintWindowBackground = miPaintWindow;
	      }
	      break;

	  case CWBackPixel:
              newFuncs.PaintWindowBackground = mfbPaintWindowSolid;
	      pPrivWin->fastBackground = 0;
	      break;

	  case CWBorderPixmap:
	      if ((pWin->border.pixmap->drawable.width <= 32) &&
		  !(pWin->border.pixmap->drawable.width &
		    (pWin->border.pixmap->drawable.width - 1)))
	      {
		  if (pPrivWin->pRotatedBorder)
		      mfbDestroyPixmap(pPrivWin->pRotatedBorder);
		  pPrivWin->pRotatedBorder = mfbCopyPixmap(pWin->border.pixmap);
		  if (pPrivWin->pRotatedBorder)
		  {
		      pPrivWin->fastBorder = 1;
		      pPrivWin->oldRotate.x = pWin->drawable.x;
		      pPrivWin->oldRotate.y = pWin->drawable.y;
		      (void)mfbPadPixmap(pPrivWin->pRotatedBorder);
		      mfbXRotatePixmap(pPrivWin->pRotatedBorder,
				       pWin->drawable.x);
		      mfbYRotatePixmap(pPrivWin->pRotatedBorder,
				       pWin->drawable.y);
		      newFuncs.PaintWindowBorder = mfbPaintWindow32;
		  }
		  else
		  {
		      pPrivWin->fastBorder = 0;
		      newFuncs.PaintWindowBorder = miPaintWindow;
		  }
	      }
	      else
	      {
		  pPrivWin->fastBorder = 0;
		  newFuncs.PaintWindowBorder = miPaintWindow;
	      }
	      break;
	    case CWBorderPixel:
	      newFuncs.PaintWindowBorder = mfbPaintWindowSolid;
	      pPrivWin->fastBorder = 0;
	      break;

	}
    }
    if (newFuncs.PaintWindowBorder != pWin->funcs->PaintWindowBorder ||
        newFuncs.PaintWindowBackground != pWin->funcs->PaintWindowBackground)
    {
	int	i;

	for (i = 0; i < NumCommonFuncs; i++)
	    if (newFuncs.PaintWindowBorder == commonFuncs[i].PaintWindowBorder &&
	        newFuncs.PaintWindowBackground == commonFuncs[i].PaintWindowBackground)
	    {
		break;
	    }
	if (i < NumCommonFuncs) {
	    if (pWin->funcs->devPrivate.val)
	    	xfree (pWin->funcs);
	    pWin->funcs = &commonFuncs[i];
	} else {
	    if (!pWin->funcs->devPrivate.val)
		pWin->funcs = (WindowFuncs *) xalloc (sizeof (WindowFuncs));
	    *pWin->funcs = newFuncs;
	    pWin->funcs->devPrivate.val = 1;
	}
    }
    /* Again, we have no failure modes indicated by any of the routines
     * we've called, so we have to assume it worked */
    return (TRUE);
}
