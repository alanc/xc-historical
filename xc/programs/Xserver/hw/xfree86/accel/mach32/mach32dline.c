/* $XConsortium: mach32dline.c,v 1.1 94/03/28 21:07:26 dpw Exp $ */
/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL AND KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL DIGITAL OR KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

Modified for the 8514/A by Kevin E. Martin (martin@cs.unc.edu)
Modified for the mach32 by Mike Bernson    (mike@mbsun.mlb.org)

*/

/* s3dline.c from s3line.c with help from cfbresd.c and cfbline.c - Jon */

#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"
#include "miline.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "misc.h"
#include "xf86.h"
#include "mach32.h"
#include "regmach32.h"

#define NextDash {\
    dashIndexTmp++; \
    if (dashIndexTmp == numInDashList) \
        dashIndexTmp = 0; \
    dashRemaining = pDash[dashIndexTmp]; \
    thisDash = dashRemaining; \
    }

#define FillDashPat {\
   int i; \
\
   for (i = 0; i < 16; i++) {\
      dashPat <<= 1;\
      if (tmp + i < len) {\
	 if (!(dashIndexTmp & 1))\
	    dashPat |= 1;\
	 if (--thisDash == 0)\
	    NextDash\
      }\
   }\
}

static	int low_translate[16] = 
	{
	  0x0000,  0x0010, 0x0008, 0x0018,
	  0x0004,  0x0014, 0x000c, 0x001c,
	  0x0006,  0x001c, 0x000e, 0x001e,
	  0x0002,  0x001a, 0x000a, 0x001a
	};

static	int high_translate[16] = 
	{
	  0x0000,  0x1000, 0x0800, 0x1800,
	  0x0400,  0x1400, 0x0c00, 0x1c00,
	  0x0600,  0x1c00, 0x0e00, 0x1e00,
	  0x0200,  0x1a00, 0x0a00, 0x1a00
	};



#define	HIGH_HIGH_NIBLE(x)	high_translate[((x) >> 12) & 0x0f]
#define LOW_HIGH_NIBLE(x)	low_translate[((x) >> 8) & 0x0f]
#define HIGH_LOW_NIBLE(x)	high_translate[((x) >> 4) & 0x0f]
#define	LOW_LOW_NIBLE(x)	low_translate[(x) & 0x0f]

/*
 * Dashed lines through the graphics engine.
 * Known Bugs: Jon 13/9/93
 * - Dash offset isn't caclulated correctly for clipped lines. [fixed?]
 * - Dash offset isn't updated correctly. [fixed?]
 * - Dash patters which are a power of 2 and < 16 can be done faster through
 *   the color compare register.
 * - DoubleDashed lines are are probably very incorrect.
 * - line caps are possible wrong too.
 * - Caclulating the dashes could probably be done more optimally,
 *   e.g. We could producing the pattern stipple before hand?
 */
void
mach32Dline(pDrawable, pGC, mode, npt, pptInit)
     DrawablePtr pDrawable;
     GCPtr pGC;
     int   mode;		/* Origin or Previous */
     int   npt;			/* number of points */
     DDXPointPtr pptInit;
{
   int   nboxInit;
   register int nbox;
   BoxPtr pboxInit;
   register BoxPtr pbox;
   register DDXPointPtr ppt;	/* pointer to list of translated points */

   unsigned int oc1;		/* outcode of point 1 */
   unsigned int oc2;		/* outcode of point 2 */

   int   xorg, yorg;		/* origin of window */

   int   adx;			/* abs values of dx and dy */
   int   ady;
   int   signdx;		/* sign of dx and dy */
   int   signdy;
   int   e, e1, e2;		/* bresenham error and increments */
   int   len;			/* length of segment */
   int   axis;			/* major axis */
   short direction;
   unsigned char *pDash;
   int   dashOffset;   
   int numInDashList;
   int dashIndex;
   int dashIndexTmp, dashOffsetTmp, thisDash, dashRemaining;
   int unclippedlen;
   short dashPat;
 /* a bunch of temporaries */
   int   tmp;
   register int y1, y2;
   register int x1, x2;
   RegionPtr cclip;
   cfbPrivGCPtr devPriv;
   short fix;

   if (!xf86VTSema)
   {
      cfbLineSD(pDrawable, pGC, mode, npt, pptInit);
      return;
   }

   devPriv = (cfbPrivGC *) (pGC->devPrivates[cfbGCPrivateIndex].ptr);
   cclip = devPriv->pCompositeClip;
   pboxInit = REGION_RECTS(cclip);
   nboxInit = REGION_NUM_RECTS(cclip);

   WaitQueue(6);
   outw(FRGD_MIX, FSS_FRGDCOL | mach32alu[pGC->alu]);
   if (pGC->lineStyle == LineDoubleDash) {
      outw(BKGD_COLOR, (short)pGC->bgPixel);
      outw(BKGD_MIX, BSS_BKGDCOL | mach32alu[pGC->alu]);      
   } else
      outw(BKGD_MIX, BSS_BKGDCOL | MIX_DST);
   outw(WRT_MASK, (short)pGC->planemask);
   outw(FRGD_COLOR, (short)pGC->fgPixel);
   outw (MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_EXPPC | COLCMPOP_F);
   
   xorg = pDrawable->x;
   yorg = pDrawable->y;
   ppt = pptInit;

   x2 = ppt->x + xorg;
   y2 = ppt->y + yorg;

   pDash = (unsigned char *) pGC->dash;
   numInDashList = pGC->numInDashList;
    
   dashIndex = 0;
   dashOffset = 0;
   miStepDash ((int)pGC->dashOffset, &dashIndex, pDash,
                numInDashList, &dashOffset);

   dashRemaining = pDash[dashIndex] - dashOffset;
   thisDash = dashRemaining ;
		  
   
   while (--npt) {
      nbox = nboxInit;
      pbox = pboxInit;

      x1 = x2;
      y1 = y2;
      ++ppt;
      if (mode == CoordModePrevious) {
	 xorg = x1;
	 yorg = y1;
      }
      x2 = ppt->x + xorg;
      y2 = ppt->y + yorg;

#ifdef fastaxislinesfixed
      if (x1 == x2) {

	 if (y1 > y2) {
	    unclippedlen = y1 = y2;
	 } else {
	    direction |= INC_X;
	    unclippedlen = y2 - y1;	    
	 }
	 
       /* get to first band that might contain part of line */
	 while ((nbox) && (pbox->y2 <= y1)) {
	    pbox++;
	    nbox--;
	 }

	 if (nbox) {
	  /* stop when lower edge of box is beyond end of line */
	    while ((nbox) && (y2 >= pbox->y1)) {
	       if ((x1 >= pbox->x1) && (x1 < pbox->x2)) {
		  int   y1t, y2t;


		/* this box has part of the line in it */
		  y1t = max(y1, pbox->y1);
		  y2t = min(y2, pbox->y2);
		  if (y1t != y2t) {
		     /* use tmp dash index and offsets */
		     dashIndexTmp = dashIndex;    
		     dashOffsetTmp = dashOffset; 

		     if (y1t != y1) { /* advance the dash index */
			miStepDash (y1t - y1, &dashIndexTmp, pDash,
				 numInDashList, &dashOffsetTmp);
		     }
		     dashRemaining = pDash[dashIndexTmp] - dashOffsetTmp;
		     thisDash = dashRemaining ;
		     
		     WaitQueue(4);
		     outw(CUR_X, (short)x1);
		     outw(CUR_Y, (short)y1t);
		     len = y2t - y1t;
		     outw(MAJ_AXIS_PCNT, (short)(len - 1));
		     outw(CMD, CMD_LINE | DRAW | LINETYPE | PLANAR |
			PCDATA | _16BIT | WRTDATA | INC_Y | YMAJAXIS);

		     for (tmp = 0 ; tmp < len; tmp+=16) {
#ifdef	NEVER

			FillDashPat;			
			outw(PIX_TRANS, dashPat);
#else
	    outw(PIX_TRANS, HIGH_HIGH_NIBLE(dashPat) | LOW_HIGH_NIBLE(dashPat));
	    outw(PIX_TRANS, HIGH_LOW_NIBLE(dashPat) | LOW_LOW_NIBLE(dashPat));
#endif

		     }		  
		  }
	       }
	       nbox--;
	       pbox++;
	    }
	 }
	 y2 = ppt->y + yorg;
      } else if (y1 == y2) {

       /*
        * force line from left to right, keeping endpoint semantics
        */
	 if (x1 > x2) {
	    register int tmp;

	    tmp = x2;
	    x2 = x1 + 1;
	    x1 = tmp + 1;
	 }
	 unclippedlen = x2 - x1;
       /* find the correct band */
	 while ((nbox) && (pbox->y2 <= y1)) {
	    pbox++;
	    nbox--;
	 }

       /* try to draw the line, if we haven't gone beyond it */
	 if ((nbox) && (pbox->y1 <= y1)) {
	  /* when we leave this band, we're done */
	    tmp = pbox->y1;
	    while ((nbox) && (pbox->y1 == tmp)) {
	       int   x1t, x2t;

	       if (pbox->x2 <= x1) {
		/* skip boxes until one might contain start point */
		  nbox--;
		  pbox++;
		  continue;
	       }
	     /* stop if left of box is beyond right of line */
	       if (pbox->x1 >= x2) {
		  nbox = 0;
		  break;
	       }
	       x1t = max(x1, pbox->x1);
	       x2t = min(x2, pbox->x2);	       
	       if (x1t != x2t) {
		  dashIndexTmp = dashIndex;    
		  dashOffsetTmp = dashOffset; 

		  if (x1t != x1) { /* advance the dash index */
		     miStepDash (x1t - x1, &dashIndexTmp, pDash,
				 numInDashList, &dashOffsetTmp);
		  }
		  dashRemaining = pDash[dashIndexTmp] - dashOffsetTmp;
		  thisDash = dashRemaining ;
		  
		  WaitQueue(4);
		  outw(CUR_X, (short)x1t);
		  outw(CUR_Y, (short)y1);
		  len = x2t - x1t;
		  outw(MAJ_AXIS_PCNT, (short)(len - 1));
		  outw(CMD, CMD_LINE | DRAW | LINETYPE | PLANAR |
			PCDATA | _16BIT | WRTDATA);
		  for (tmp = 0 ; tmp < len; tmp+=16) {
#ifdef	NEVER
		     FillDashPat;
		     outw(PIX_TRANS, dashPat);
#else
	    outw(PIX_TRANS, HIGH_HIGH_NIBLE(dashPat) | LOW_HIGH_NIBLE(dashPat));
	    outw(PIX_TRANS, HIGH_LOW_NIBLE(dashPat) | LOW_LOW_NIBLE(dashPat));

#endif
		  }		 
	       }
	       nbox--;
	       pbox++;
	    }
	 }
	 x2 = ppt->x + xorg;
      }
      else
#endif
      {			/* sloped line */
	 direction = 0x0000;
	 signdx = 1;
	 if ((adx = x2 - x1) < 0) {
	    adx = -adx;
	    signdx = -1;
	    fix = 0;
	 } else {
	    direction |= INC_X;
	    fix = -1;
	 }
	 signdy = 1;
	 if ((ady = y2 - y1) < 0) {
	    ady = -ady;
	    signdy = -1;
	 } else {
	    direction |= INC_Y;
	 }

	 if (adx > ady) {
	    axis = X_AXIS;
	    e1 = ady << 1;
	    e2 = e1 - (adx << 1);
	    e = e1 - adx;
	    unclippedlen = adx;
	 } else {
	    axis = Y_AXIS;
	    e1 = adx << 1;
	    e2 = e1 - (ady << 1);
	    e = e1 - ady;
	    direction |= YMAJAXIS;
	    unclippedlen = ady;	    
	 }

       /*
        * we have bresenham parameters and two points. all we have to do now
        * is clip and draw.
        */

	 while (nbox--) {
	    oc1 = 0;
	    oc2 = 0;
	    OUTCODES(oc1, x1, y1, pbox);
	    OUTCODES(oc2, x2, y2, pbox);
	    if ((oc1 | oc2) == 0) {
	       if (axis == X_AXIS) 
		  len = adx;
	       else 
		  len = ady;
	     dashIndexTmp = dashIndex;    
	     dashOffsetTmp = dashOffset;
	     /* No need to adjust dash offset */
	     /*
	      * NOTE:  The 8514/A hardware routines for generating lines do
	      * not match the software generated lines of mi, cfb, and mfb.
	      * This is a problem, and if I ever get time, I'll figure out
	      * the 8514/A algorithm and implement it in software for mi,
	      * cfb, and mfb.
	      * 2-sep-93 TCG: apparently only change needed is
	      * addition of 'fix' stuff in cfbline.c
	      */
	       WaitQueue(7);
	       outw(CUR_X, (short)x1);
	       outw(CUR_Y, (short)y1);
	       outw(ERR_TERM, (short)(e + fix));
	       outw(DESTY_AXSTP, (short)e1);
	       outw(DESTX_DIASTP, (short)e2);
	       outw(MAJ_AXIS_PCNT, (short)len);
	       outw(CMD, CMD_LINE | DRAW | LASTPIX | PLANAR | direction |
		       PCDATA | _16BIT | WRTDATA);
	       for (tmp = 0 ; tmp < len; tmp+=16) {
#ifdef	NEVER
		  FillDashPat;
		  outw(PIX_TRANS, dashPat);
#else
	    outw(PIX_TRANS, HIGH_HIGH_NIBLE(dashPat) | LOW_HIGH_NIBLE(dashPat));
	    outw(PIX_TRANS, HIGH_LOW_NIBLE(dashPat) | LOW_LOW_NIBLE(dashPat));
#endif
	       }
	       break;
	    } else if (oc1 & oc2) {
	       pbox++;
	    } else {

	     /*
	      * let the mi helper routine do our work; better than
	      * duplicating code...
	      */
	       int   err;		/* modified bresenham error term */
	       int   clip1=0, clip2=0;	/* clippedness of the endpoints */

	       int   clipdx, clipdy;	/* difference between clipped and
					 * unclipped start point */
	       int dlen;
	       int new_x1 = x1, new_y1 = y1, new_x2 = x2, new_y2 = y2;

               if (miZeroClipLine(pbox->x1, pbox->y1,
					pbox->x2-1, pbox->y2-1,
					&new_x1, &new_y1,
					&new_x2, &new_y2,
					adx, ady,
					&clip1, &clip2,
					axis, (signdx == signdy),
					oc1, oc2) == -1)
		{
		     pbox++;
		     continue;
		}
		dashIndexTmp = dashIndex;
		dashOffsetTmp = dashOffset; 
			       
		if (axis == X_AXIS) {
		     dlen = abs(new_x1 - x1);
		     len = abs(new_x2 - new_x1);
		} else {
		     dlen = abs(new_y1 - y1);
		     len = abs(new_y2 - new_y1);
		}

		len += (clip2 != 0);
		if (len) {
		   /* unwind bresenham error term to first point */
		     if (clip1) {
			clipdx = abs(new_x1 - x1);
			clipdy = abs(new_y1 - y1);
			if (axis == X_AXIS)
			   err = e + ((clipdy * e2) + ((clipdx - clipdy) * e1));
			else
			   err = e + ((clipdx * e2) + ((clipdy - clipdx) * e1));
		     } else
			err = e;

		     /*
		      * Here is a problem, the unwound error terms could be
		      * upto 16bit now. The poor MACH32 is only 12 or 13 bit.
		      * The rounding error is probably small I favor scaling
		      * the error terms, although re-evaluation is also an
		      * option I think it might give visable errors
		      * - Jon 12/9/93.
		      */
		      
		     if (abs(err) > 4096  || abs(e1) > 4096 || abs(e2) > 4096) {
#if 1
			int div;

			if (abs(err) > abs(e1))
			    div = (abs(err) > abs(e2)) ?
			    (abs(err) + 4095)/ 4096 : (abs(e2) + 4095)/ 4096;
			else
			    div = (abs(e1) > abs(e2)) ?
			    (abs(e1) + 4095)/ 4096 : (abs(e2) + 4095)/ 4096;

			err /= div;
			e1 /= div;
			e2 /= div;
#else
			int minor;
			if (axis == X_AXIS) {			
			   minor = abs(new_y2 - new_y1);
			   err = 2 * minor - len;			   
			} else {
			   minor = abs(new_x2 - new_x1);
			   err = 2 * minor - len - 1;	   
			}
			e1 = minor << 1;
			e2 = e1 - (len << 1);
#endif
		     }
		     miStepDash (dlen, &dashIndexTmp, pDash,
				 numInDashList, &dashOffsetTmp);
		     WaitQueue(7);
		     outw(CUR_X, (short)new_x1);
		     outw(CUR_Y, (short)new_y1);
		     outw(ERR_TERM, (short)(err + fix));
		     outw(DESTY_AXSTP, (short)e1);
		     outw(DESTX_DIASTP, (short)e2);
		     outw(MAJ_AXIS_PCNT, (short)len);
		     outw(CMD, CMD_LINE | DRAW | LASTPIX | PLANAR |
			     direction | PCDATA | _16BIT | WRTDATA);

		     for (tmp = 0 ; tmp < len; tmp+=16) {
#ifdef	NEVER

			FillDashPat;
			outw(PIX_TRANS, dashPat);
#else
	    outw(PIX_TRANS, HIGH_HIGH_NIBLE(dashPat) | LOW_HIGH_NIBLE(dashPat));
	    outw(PIX_TRANS, HIGH_LOW_NIBLE(dashPat) | LOW_LOW_NIBLE(dashPat));
#endif
		     }
		}
	       pbox++;
	    }
	 }			/* while (nbox--) */
      }/* sloped line */
      miStepDash (unclippedlen, &dashIndex, pDash,
		  numInDashList, &dashOffset);
   } /* while (nline--) */

 /*
  * paint the last point if the end style isn't CapNotLast. (Assume that a
  * projecting, butt, or round cap that is one pixel wide is the same as the
  * single pixel of the endpoint.)
  */

   if ((pGC->capStyle != CapNotLast) &&
       ((ppt->x + xorg != pptInit->x + pDrawable->x) ||
	(ppt->y + yorg != pptInit->y + pDrawable->y) ||
	(ppt == pptInit + 1))) {
      nbox = nboxInit;
      pbox = pboxInit;
      while (nbox--) {
	 if ((x2 >= pbox->x1) &&
	     (y2 >= pbox->y1) &&
	     (x2 < pbox->x2) &&
	     (y2 < pbox->y2)) {
	    WaitQueue(4);
	    outw(CUR_X, (short)x2);
	    outw(CUR_Y, (short)y2);
	    outw(MAJ_AXIS_PCNT, 0);
	    outw(CMD, CMD_LINE | DRAW | LINETYPE | PLANAR | WRTDATA);

	    break;
	 } else
	    pbox++;
      }
   }
   WaitQueue(3);
   outw(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   outw(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   outw (MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_FRGDMIX | COLCMPOP_F);  
}
