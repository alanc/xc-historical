/* $XConsortium$ */
/*
 * Fill rectangles.
 */

/*
 * Copyright 1989 by the Massachusetts Institute of Technology
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 * 
 * Modified for the 8514/A by Kevin E. Martin (martin@cs.unc.edu)
 * 
 * KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 * 
 */


/* $XConsortium: cfbfillrct.c,v 5.13 90/05/15 18:40:19 keith Exp $ */

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "mergerop.h"

#include "s3.h"
#include "regs3.h"

#define NUM_STACK_RECTS	1024
extern int s3MAX_SLOTS;

void
s3PolyFillRect(pDrawable, pGC, nrectFill, prectInit)
     DrawablePtr pDrawable;
     register GCPtr pGC;
     int   nrectFill;		/* number of rectangles to fill */
     xRectangle *prectInit;	/* Pointer to first rectangle to fill */
{
   xRectangle *prect;
   RegionPtr prgnClip;
   register BoxPtr pbox;
   register BoxPtr pboxClipped;
   BoxPtr pboxClippedBase;
   BoxPtr pextent;
   BoxRec stackRects[NUM_STACK_RECTS];
   cfbPrivGC *priv;
   int   numRects;
   int   n;
   int   xorg, yorg;
   int   width, height;
   PixmapPtr pPix;
   int   pixWidth;
   int   xrot, yrot;

   if (!xf86VTSema)
   {
      cfbPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
      return;
   }

   priv = (cfbPrivGC *) pGC->devPrivates[cfbGCPrivateIndex].ptr;
   prgnClip = priv->pCompositeClip;

   prect = prectInit;
   xorg = pDrawable->x;
   yorg = pDrawable->y;
   if (xorg || yorg) {
      prect = prectInit;
      n = nrectFill;
      while (n--) {
	 prect->x += xorg;
	 prect->y += yorg;
	 prect++;
      }
   }
   prect = prectInit;

   numRects = REGION_NUM_RECTS(prgnClip) * nrectFill;
   if (numRects > NUM_STACK_RECTS) {
      pboxClippedBase = (BoxPtr) ALLOCATE_LOCAL(numRects * sizeof(BoxRec));
      if (!pboxClippedBase)
	 return;
   } else
      pboxClippedBase = stackRects;

   pboxClipped = pboxClippedBase;

   if (REGION_NUM_RECTS(prgnClip) == 1) {
      int   x1, y1, x2, y2, bx2, by2;

      pextent = REGION_RECTS(prgnClip);
      x1 = pextent->x1;
      y1 = pextent->y1;
      x2 = pextent->x2;
      y2 = pextent->y2;
      while (nrectFill--) {
	 if ((pboxClipped->x1 = prect->x) < x1)
	    pboxClipped->x1 = x1;

	 if ((pboxClipped->y1 = prect->y) < y1)
	    pboxClipped->y1 = y1;

	 bx2 = (int)prect->x + (int)prect->width;
	 if (bx2 > x2)
	    bx2 = x2;
	 pboxClipped->x2 = bx2;

	 by2 = (int)prect->y + (int)prect->height;
	 if (by2 > y2)
	    by2 = y2;
	 pboxClipped->y2 = by2;

	 prect++;
	 if ((pboxClipped->x1 < pboxClipped->x2) &&
	     (pboxClipped->y1 < pboxClipped->y2)) {
	    pboxClipped++;
	 }
      }
   } else {
      int   x1, y1, x2, y2, bx2, by2;

      pextent = (*pGC->pScreen->RegionExtents) (prgnClip);
      x1 = pextent->x1;
      y1 = pextent->y1;
      x2 = pextent->x2;
      y2 = pextent->y2;
      while (nrectFill--) {
	 BoxRec box;

	 if ((box.x1 = prect->x) < x1)
	    box.x1 = x1;

	 if ((box.y1 = prect->y) < y1)
	    box.y1 = y1;

	 bx2 = (int)prect->x + (int)prect->width;
	 if (bx2 > x2)
	    bx2 = x2;
	 box.x2 = bx2;

	 by2 = (int)prect->y + (int)prect->height;
	 if (by2 > y2)
	    by2 = y2;
	 box.y2 = by2;

	 prect++;

	 if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    continue;

	 n = REGION_NUM_RECTS(prgnClip);
	 pbox = REGION_RECTS(prgnClip);

       /*
        * clip the rectangle to each box in the clip region this is logically
        * equivalent to calling Intersect()
        */
	 while (n--) {
	    pboxClipped->x1 = max(box.x1, pbox->x1);
	    pboxClipped->y1 = max(box.y1, pbox->y1);
	    pboxClipped->x2 = min(box.x2, pbox->x2);
	    pboxClipped->y2 = min(box.y2, pbox->y2);
	    pbox++;

	  /* see if clipping left anything */
	    if (pboxClipped->x1 < pboxClipped->x2 &&
		pboxClipped->y1 < pboxClipped->y2) {
	       pboxClipped++;
	    }
	 }
      }
   }

   if (pboxClipped != pboxClippedBase) {
      n = pboxClipped - pboxClippedBase;
      switch (pGC->fillStyle) {
	case FillSolid:
	   BLOCK_CURSOR;
	   WaitQueue(3);
	   S3_OUTW(FRGD_COLOR, (short)(pGC->fgPixel));
	   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | s3alu[pGC->alu]);
	   S3_OUTW(WRT_MASK, (short)pGC->planemask);

	   pboxClipped = pboxClippedBase;
	   while (n--) {
	      WaitQueue(5);
	      S3_OUTW(CUR_X, (short)(pboxClipped->x1));
	      S3_OUTW(CUR_Y, (short)(pboxClipped->y1));
	      S3_OUTW(MAJ_AXIS_PCNT,
		    (short)(pboxClipped->x2 - pboxClipped->x1 - 1));
	      S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT |
		    (short)(pboxClipped->y2 - pboxClipped->y1 - 1));
	      S3_OUTW(CMD, CMD_RECT | INC_Y | INC_X | DRAW | PLANAR | WRTDATA);

	      pboxClipped++;
	   }

	   WaitQueue(2);
	   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
	   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
	   UNBLOCK_CURSOR;
	   break;
	case FillTiled:
	   xrot = pDrawable->x + pGC->patOrg.x;
	   yrot = pDrawable->y + pGC->patOrg.y;

	   pPix = pGC->tile.pixmap;
	   width = pPix->drawable.width;
	   height = pPix->drawable.height;
	   pixWidth = PixmapBytePad(width, pPix->drawable.depth);

	   pboxClipped = pboxClippedBase;
#ifdef PIXPRIV
	   if (s3MAX_SLOTS && s3CacheTile(pPix, (pextent->x2 - pboxClipped->x1)
					      - width)) {
	      while (n--) {
	        int w, h;
		w = pboxClipped->x2 - pboxClipped->x1;
		h = pboxClipped->y2 - pboxClipped->y1;
		if ((w > 9) || (h > 9))
		 s3CImageFill(pPix->slot,
			      pboxClipped->x1, pboxClipped->y1,
			      w, h,
			      xrot, yrot,
			      s3alu[pGC->alu], pGC->planemask);
		 else
		   (s3ImageFillFunc) (pboxClipped->x1, pboxClipped->y1,
				    w, h,
				    pPix->devPrivate.ptr, pixWidth,
				    width, height, xrot, yrot,
				    s3alu[pGC->alu], pGC->planemask);
		 pboxClipped++;
	      }
	   } else
#endif
	   {
	      while (n--) {
		 (s3ImageFillFunc) (pboxClipped->x1, pboxClipped->y1,
				    pboxClipped->x2 - pboxClipped->x1,
				    pboxClipped->y2 - pboxClipped->y1,
				    pPix->devPrivate.ptr, pixWidth,
				    width, height, xrot, yrot,
				    s3alu[pGC->alu], pGC->planemask);
		 pboxClipped++;
	      }
	   }
	   break;
	case FillStippled:
	   xrot = pDrawable->x + pGC->patOrg.x;
	   yrot = pDrawable->y + pGC->patOrg.y;

	   pPix = pGC->stipple;
	   width = pPix->drawable.width;
	   height = pPix->drawable.height;
	   pixWidth = PixmapBytePad(width, pPix->drawable.depth);

	   pboxClipped = pboxClippedBase;
#ifdef PIXPRIV
	   if (s3MAX_SLOTS &&
	       s3CacheStipple(pPix, (pextent->x2 - pboxClipped->x1) - width)) {
	      while (n--) {
	         int w, h;
	         w = pboxClipped->x2 - pboxClipped->x1;
		 h = pboxClipped->y2 - pboxClipped->y1;
		 if ((w > 9) || (h > 9))
	       		  s3CImageStipple(pPix->slot,
				 pboxClipped->x1, pboxClipped->y1,
				 w, h,
				 xrot, yrot, pGC->fgPixel,
				 s3alu[pGC->alu], pGC->planemask);
		 else
		  s3ImageStipple(pboxClipped->x1, pboxClipped->y1,
				w, h,
				pPix->devPrivate.ptr, pixWidth,
				width, height, xrot, yrot,
				pGC->fgPixel,
				s3alu[pGC->alu], pGC->planemask);
		 pboxClipped++;
	      }
	   } else
#endif
	   {
	      while (n--) {
		 s3ImageStipple(pboxClipped->x1, pboxClipped->y1,
				pboxClipped->x2 - pboxClipped->x1,
				pboxClipped->y2 - pboxClipped->y1,
				pPix->devPrivate.ptr, pixWidth,
				width, height, xrot, yrot,
				pGC->fgPixel,
				s3alu[pGC->alu], pGC->planemask);
		 pboxClipped++;
	      }
	   }
	   break;
	case FillOpaqueStippled:
	   xrot = pDrawable->x + pGC->patOrg.x;
	   yrot = pDrawable->y + pGC->patOrg.y;

	   pPix = pGC->stipple;
	   width = pPix->drawable.width;
	   height = pPix->drawable.height;
	   pixWidth = PixmapBytePad(width, pPix->drawable.depth);

	   pboxClipped = pboxClippedBase;
#ifdef PIXPRIV
	   if (s3MAX_SLOTS && s3CacheOpStipple(pPix,
				(pextent->x2 - pboxClipped->x1) - width)) {
	      while (n--) {
      	         int w, h;
	         w = pboxClipped->x2 - pboxClipped->x1;
		 h = pboxClipped->y2 - pboxClipped->y1;
		 if ((w > 9) || (h > 9))
		  s3CImageOpStipple(pPix->slot,
				   pboxClipped->x1, pboxClipped->y1,
				   w, h,
				   xrot, yrot,
				   pGC->fgPixel, pGC->bgPixel,
				   s3alu[pGC->alu],
				   pGC->planemask);
		  else
		     s3ImageOpStipple(pboxClipped->x1, pboxClipped->y1,
				  w, h,
				  pPix->devPrivate.ptr, pixWidth,
				  width, height, xrot, yrot,
				  pGC->fgPixel, pGC->bgPixel,
				  s3alu[pGC->alu],
				  pGC->planemask);		   
		 pboxClipped++;
	      }
	   } else
#endif
	   {
	      while (n--) {
		 s3ImageOpStipple(pboxClipped->x1, pboxClipped->y1,
				  pboxClipped->x2 - pboxClipped->x1,
				  pboxClipped->y2 - pboxClipped->y1,
				  pPix->devPrivate.ptr, pixWidth,
				  width, height, xrot, yrot,
				  pGC->fgPixel, pGC->bgPixel,
				  s3alu[pGC->alu],
				  pGC->planemask);
		 pboxClipped++;
	      }
	   }
	   break;
      }
   }
   if (pboxClippedBase != stackRects)
      DEALLOCATE_LOCAL(pboxClippedBase);
}
