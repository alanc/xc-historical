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
#include "cfb.h"
#include "mistruct.h"
#include "regionstr.h"
#include "cfbmskbits.h"

extern WindowPtr *WindowTable;

Bool
cfbCreateWindow(pWin)
    WindowPtr pWin;
{
    cfbPrivWin *pPrivWin;

    pPrivWin = (cfbPrivWin *)xalloc(sizeof(cfbPrivWin));
    pWin->devPrivates[cfbWindowPrivateIndex].ptr = (pointer)pPrivWin;
    if (!pPrivWin)
	return FALSE;
    pPrivWin->pRotatedBorder = NullPixmap;
    pPrivWin->pRotatedBackground = NullPixmap;
    pPrivWin->fastBackground = 0;
    pPrivWin->fastBorder = 0;

    return TRUE;
}

Bool
cfbDestroyWindow(pWin)
    WindowPtr pWin;
{
    cfbPrivWin *pPrivWin;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);

    if (pPrivWin->pRotatedBorder)
	cfbDestroyPixmap(pPrivWin->pRotatedBorder);
    if (pPrivWin->pRotatedBackground)
	cfbDestroyPixmap(pPrivWin->pRotatedBackground);
    xfree(pWin->devPrivates[cfbWindowPrivateIndex].ptr);
    return(TRUE);
}

/*ARGSUSED*/
Bool
cfbMapWindow(pWindow)
    WindowPtr pWindow;
{
    return(TRUE);
}

/* (x, y) is the upper left corner of the window on the screen 
   do we really need to pass this?  (is it a;ready in pWin->absCorner?)
   we only do the rotation for pixmaps that are 32 bits wide (padded
or otherwise.)
   cfbChangeWindowAttributes() has already put a copy of the pixmap
in pPrivWin->pRotated*
*/
/*ARGSUSED*/
Bool
cfbPositionWindow(pWin, x, y)
    WindowPtr pWin;
    int x, y;
{
    cfbPrivWin *pPrivWin;
    int setxy = 0;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);
    if (pWin->backgroundState == BackgroundPixmap &&
	(pPrivWin->fastBackground != 0))
    {
	cfbXRotatePixmap(pPrivWin->pRotatedBackground,
		      pWin->drawable.x - pPrivWin->oldRotate.x);
	cfbYRotatePixmap(pPrivWin->pRotatedBackground,
		      pWin->drawable.y - pPrivWin->oldRotate.y);
	setxy = 1;
    }

    if (!pWin->borderIsPixel &&	(pPrivWin->fastBorder != 0))
    {
	cfbXRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.x - pPrivWin->oldRotate.x);
	cfbYRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.y - pPrivWin->oldRotate.y);
	setxy = 1;
    }
    if (setxy)
    {
	pPrivWin->oldRotate.x = pWin->drawable.x;
	pPrivWin->oldRotate.y = pWin->drawable.y;
    }
    return (TRUE);
}

/*ARGSUSED*/
Bool
cfbUnmapWindow(pWindow)
    WindowPtr pWindow;
{
    return (TRUE);
}

/* UNCLEAN!
   this code calls the bitblt helper code directly.

   cfbCopyWindow copies only the parts of the destination that are
visible in the source.
*/


void 
cfbCopyWindow(pWin, ptOldOrg, prgnSrc)
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

    prgnDst = (* pWin->drawable.pScreen->RegionCreate)(NULL, 1);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    (* pWin->drawable.pScreen->TranslateRegion)(prgnSrc, -dx, -dy);
    (* pWin->drawable.pScreen->Intersect)(prgnDst, &pWin->borderClip, prgnSrc);

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);
    if(!(pptSrc = (DDXPointPtr )ALLOCATE_LOCAL(nbox * sizeof(DDXPointRec))))
	return;
    ppt = pptSrc;

    for (i = nbox; --i >= 0; ppt++, pbox++)
    {
	ppt->x = pbox->x1 + dx;
	ppt->y = pbox->y1 + dy;
    }

    cfbDoBitblt((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
		GXcopy, prgnDst, pptSrc);
    DEALLOCATE_LOCAL(pptSrc);
    (* pWin->drawable.pScreen->RegionDestroy)(prgnDst);
}



/* swap in correct PaintWindow* routine.  If we can use a fast output
routine (i.e. the pixmap is paddable to 32 bits), also pre-rotate a copy
of it in devPrivates[cfbWindowPrivateIndex].ptr.
*/
Bool
cfbChangeWindowAttributes(pWin, mask)
    WindowPtr pWin;
    unsigned long mask;
{
    register unsigned long index;
    register cfbPrivWin *pPrivWin;
    int width;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);
    while(mask)
    {
	index = lowbit (mask);
	mask &= ~index;
	switch(index)
	{
	  case CWBackPixmap:
	      if (pWin->backgroundState == None)
	      {
		  pPrivWin->fastBackground = 0;
	      }
	      else if (pWin->backgroundState == ParentRelative)
	      {
		  pPrivWin->fastBackground = 0;
	      }
	      else if (((width = (pWin->background.pixmap->drawable.width * PSZ)) <= 32) &&
		       !(width & (width - 1)))
	      {
		  if (pPrivWin->pRotatedBackground)
		      cfbDestroyPixmap(pPrivWin->pRotatedBackground);
		  pPrivWin->pRotatedBackground =
		    cfbCopyPixmap(pWin->background.pixmap);
		  if (pPrivWin->pRotatedBackground)
		  {
		      pPrivWin->fastBackground = 1;
		      pPrivWin->oldRotate.x = pWin->drawable.x;
		      pPrivWin->oldRotate.y = pWin->drawable.y;
		      (void)cfbPadPixmap(pPrivWin->pRotatedBackground);
		      cfbXRotatePixmap(pPrivWin->pRotatedBackground,
				       pWin->drawable.x);
		      cfbYRotatePixmap(pPrivWin->pRotatedBackground,
				       pWin->drawable.y);
		  }
		  else
		  {
		      pPrivWin->fastBackground = 0;
		  }
	      }
	      else
	      {
		  pPrivWin->fastBackground = 0;
	      }
	      break;

	  case CWBackPixel:
	      pPrivWin->fastBackground = 0;
	      break;

	  case CWBorderPixmap:
	      if (((width = (pWin->border.pixmap->drawable.width * PSZ)) <= 32) &&
		  !(width & (width - 1)))
	      {
		  if (pPrivWin->pRotatedBorder)
		      cfbDestroyPixmap(pPrivWin->pRotatedBorder);
		  pPrivWin->pRotatedBorder =
		    cfbCopyPixmap(pWin->border.pixmap);
		  if (pPrivWin->pRotatedBorder)
		  {
		      pPrivWin->fastBorder = 1;
		      pPrivWin->oldRotate.x = pWin->drawable.x;
		      pPrivWin->oldRotate.y = pWin->drawable.y;
		      (void)cfbPadPixmap(pPrivWin->pRotatedBorder);
		      cfbXRotatePixmap(pPrivWin->pRotatedBorder,
				       pWin->drawable.x);
		      cfbYRotatePixmap(pPrivWin->pRotatedBorder,
				       pWin->drawable.y);
		  }
		  else
		  {
		      pPrivWin->fastBorder = 0;
		  }
	      }
	      else
	      {
		  pPrivWin->fastBorder = 0;
	      }
	      break;
	    case CWBorderPixel:
	      pPrivWin->fastBorder = 0;
	      break;

	}
    }
    return (TRUE);
}

