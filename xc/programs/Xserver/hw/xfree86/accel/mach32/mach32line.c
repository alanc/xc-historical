/* $XConsortium$ */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
Copyright 1993 by Kevin E. Martin, Chapel Hill, North Carolina.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL AND KEVIN E. MARTIN AND RICKARD E. FAITH AND CRAIG E. GROESCHEL
DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL DIGITAL OR
KEVIN E. MARTIN OR RICKARD E. FAITH OR CRAIG E. GROESCHEL BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

Modified for the 8514/A by Kevin E. Martin (martin@cs.unc.edu)
Modified for the Mach-8 by Rickard E. Faith (faith@cs.unc.edu)
Modified for the Mach32 by Kevin E. Martin (martin@cs.unc.edu)
mach32Line1Rect by Craig E. Groeschel (craig@adikia.sccsi.com),
  based on an ATI preclipping code example

******************************************************************/


/* $XConsortium: cfbline.c,v 1.19 91/08/13 18:48:42 keith Exp $ */

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
#include "regmach32.h"
#include "mach32.h"

void
#ifdef ONE_RECT
mach32Line1Rect(pDrawable, pGC, mode, npt, pptInit)
#else
mach32Line(pDrawable, pGC, mode, npt, pptInit)
#endif
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;		/* number of points */
    DDXPointPtr pptInit;
{
    int nboxInit;
    register int nbox;
    BoxPtr pboxInit;
    register BoxPtr pbox;
    register DDXPointPtr ppt;	/* pointer to list of translated points */

    unsigned int oc1;		/* outcode of point 1 */
    unsigned int oc2;		/* outcode of point 2 */

    int xorg, yorg;		/* origin of window */

    int adx;		/* abs values of dx and dy */
    int ady;
    int signdx;		/* sign of dx and dy */
    int signdy;
    int e, e1, e2;		/* bresenham error and increments */
    int len;			/* length of segment */
    int axis;			/* major axis */
    short cmd = CMD_LINE | DRAW | PLANAR | WRTDATA | LASTPIX;
    short cmd2;
    short fix;

				/* a bunch of temporaries */
    int tmp;
    register int y1, y2;
    register int x1, x2;
    RegionPtr cclip;
    cfbPrivGCPtr    devPriv;

    if (!xf86VTSema)
    {
#ifdef ONE_RECT
	cfb8LineSS1Rect(pDrawable, pGC, mode, npt, pptInit);
#else
	cfbLineSS(pDrawable, pGC, mode, npt, pptInit);
#endif
	return;
    }

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    cclip = devPriv->pCompositeClip;
    pboxInit = REGION_RECTS(cclip);
    nboxInit = REGION_NUM_RECTS(cclip);

#ifdef ONE_RECT
    WaitQueue(11);
    outw(LINEDRAW_OPT, CLIP_MODE_1 | LASTPIX);
    outw(EXT_SCISSOR_L, pboxInit->x1);
    outw(EXT_SCISSOR_T, pboxInit->y1);
    outw(EXT_SCISSOR_R, pboxInit->x2-1);
    outw(EXT_SCISSOR_B, pboxInit->y2-1);
#else
    WaitQueue(3);
#endif
    outw(FRGD_MIX, FSS_FRGDCOL | mach32alu[pGC->alu]);
    outw(WRT_MASK, (short)pGC->planemask);
    outw(FRGD_COLOR, (short)pGC->fgPixel);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    ppt = pptInit;
    x2 = ppt->x + xorg;
    y2 = ppt->y + yorg;
#ifdef ONE_RECT
    outw(LINEDRAW_INDEX, 0);	/* reset the state machine; set current x,y */
    outw(LINEDRAW, x2);
    outw(LINEDRAW, y2);
#endif

    while(--npt)
    {
#ifdef ONE_RECT
      /* Send a burst of endpoints before checking for a clip exception */
      int overrun;
      int nburst = min(14, npt);
      npt -= nburst - 1;

      while (nburst--)
      {
#endif
	x1 = x2;
	y1 = y2;
	++ppt;
	if (mode == CoordModePrevious)
	{
	    xorg = x1;
	    yorg = y1;
	}
	x2 = ppt->x + xorg;
	y2 = ppt->y + yorg;
#ifdef ONE_RECT
	WaitQueue(2);
	outw(LINEDRAW, x2);	/* write destination x,y; draw line */
	outw(LINEDRAW, y2);
      } /* while (nburst--) */

      WaitIdleEmpty();	/* 8-( */
/*
 * Read CLIP_OVERRUN and, if an exception has occurred, put the endpoints
 * of the offending line into (x1, y1),(x2, y2) and let the software
 * clip and draw.
 */
      if ((overrun = (inw(EXT_GE_STATUS) & 0x0f))) {
	ppt -= overrun;

	/* Put the start point of the line that caused the clip exception
	 * into (x2, y2).  If (mode == CoordModePrevious), we have to start
	 * from the beginning (pptInit) to calculate xorg and yorg.
	 */
	x2 = pDrawable->x;
	y2 = pDrawable->y;
	if (mode == CoordModePrevious) {
	    DDXPointPtr ppt_tmp;
	    for (ppt_tmp = pptInit; ppt_tmp != ppt; ppt_tmp++) {
		xorg = x2;
		yorg = y2;
		x2 = ppt_tmp->x + xorg;
		y2 = ppt_tmp->y + yorg;
	    }
	}
	x2 += ppt->x;
	y2 += ppt->y;
        
	/* Now draw the line that caused the clip exception. */
	x1 = x2;
	y1 = y2;
	++ppt;
	if (mode == CoordModePrevious)
	{
	    xorg = x1;
	    yorg = y1;
	}
	x2 = ppt->x + xorg;
	y2 = ppt->y + yorg;
#endif

	nbox = nboxInit;
	pbox = pboxInit;

	if (x1 == x2)
	{
	    /* make the line go top to bottom of screen, keeping
	       endpoint semantics
	    */
	    if (y1 > y2)
	    {
		register int tmp;

		tmp = y2;
		y2 = y1 + 1;
		y1 = tmp + 1;
	    }
	    /* get to first band that might contain part of line */
	    while ((nbox) && (pbox->y2 <= y1))
	    {
		pbox++;
		nbox--;
	    }

	    if (nbox)
	    {
		/* stop when lower edge of box is beyond end of line */
		while((nbox) && (y2 >= pbox->y1))
		{
		    if ((x1 >= pbox->x1) && (x1 < pbox->x2))
		    {
			int y1t, y2t;
			/* this box has part of the line in it */
			y1t = max(y1, pbox->y1);
			y2t = min(y2, pbox->y2);
			if (y1t != y2t)
			{
			    WaitQueue(4);
			    outw(CUR_X, (short)x1);
			    outw(CUR_Y, (short)y1t);
			    outw(MAJ_AXIS_PCNT, (short)(y2t-y1t-1));
			    outw(CMD, CMD_LINE | DRAW | LINETYPE | PLANAR |
				  WRTDATA | (6 << 5));
			}
		    }
		    nbox--;
		    pbox++;
		}
	    }
	    y2 = ppt->y + yorg;
	}
	else if (y1 == y2)
	{
	    /* force line from left to right, keeping
	       endpoint semantics
	    */
	    if (x1 > x2)
	    {
		register int tmp;

		tmp = x2;
		x2 = x1 + 1;
		x1 = tmp + 1;
	    }

	    /* find the correct band */
	    while( (nbox) && (pbox->y2 <= y1))
	    {
		pbox++;
		nbox--;
	    }

	    /* try to draw the line, if we haven't gone beyond it */
	    if ((nbox) && (pbox->y1 <= y1))
	    {
		/* when we leave this band, we're done */
		tmp = pbox->y1;
		while((nbox) && (pbox->y1 == tmp))
		{
		    int	x1t, x2t;

		    if (pbox->x2 <= x1)
		    {
			/* skip boxes until one might contain start point */
			nbox--;
			pbox++;
			continue;
		    }

		    /* stop if left of box is beyond right of line */
		    if (pbox->x1 >= x2)
		    {
			nbox = 0;
			break;
		    }

		    x1t = max(x1, pbox->x1);
		    x2t = min(x2, pbox->x2);
		    if (x1t != x2t)
		    {
			WaitQueue(4);
			outw(CUR_X, (short)x1t);
			outw(CUR_Y, (short)y1);
			outw(MAJ_AXIS_PCNT, (short)(x2t-x1t-1));
			outw(CMD, CMD_LINE | DRAW | LINETYPE | PLANAR |
			      WRTDATA);
		    }
		    nbox--;
		    pbox++;
		}
	    }
	    x2 = ppt->x + xorg;
	}
	else	/* sloped line */
	{
	    cmd2 = cmd;
	    signdx = 1;
	    if ((adx = x2 - x1) < 0)
	    {
		adx = -adx;
		signdx = -1;
		fix = 0;
	    }
	    else
	    {
		cmd2 |= INC_X;
		fix = -1;
	    }
	    signdy = 1;
	    if ((ady = y2 - y1) < 0)
	    {
		ady = -ady;
		signdy = -1;
	    }
	    else
	    {
		cmd2 |= INC_Y;
	    }

	    if (adx > ady)
	    {
		axis = X_AXIS;
		e1 = ady << 1;
		e2 = e1 - (adx << 1);
		e = e1 - adx;
	    }
	    else
	    {
		axis = Y_AXIS;
		e1 = adx << 1;
		e2 = e1 - (ady << 1);
		e = e1 - ady;
		cmd2 |= YMAJAXIS;
	    }

	    /* we have bresenham parameters and two points.
	       all we have to do now is clip and draw.
	    */

	    while(nbox--)
	    {
		oc1 = 0;
		oc2 = 0;
		OUTCODES(oc1, x1, y1, pbox);
		OUTCODES(oc2, x2, y2, pbox);
		if ((oc1 | oc2) == 0)
		{
		    if (axis == X_AXIS)
			len = adx;
		    else
			len = ady;

		    /* NOTE:  The 8514/A hardware routines for generating
		       lines do not match the software generated lines
		       of mi, cfb, and mfb.  This is a problem, and if I
		       ever get time, I'll figure out the 8514/A algorithm
		       and implement it in software for mi, cfb, and mfb.
		       2-sep-93 TCG: apparently only change needed is
		       addition of 'fix' stuff in cfbline.c
		       */
		    WaitQueue(7);
		    outw(CUR_X, (short)x1);
		    outw(CUR_Y, (short)y1);
		    outw(ERR_TERM, (short)(e + fix));
		    outw(DESTY_AXSTP, (short)e1);
		    outw(DESTX_DIASTP, (short)e2);
		    outw(MAJ_AXIS_PCNT, (short)len);
		    outw(CMD, cmd2);

		    break;
		}
		else if (oc1 & oc2)
		{
		    pbox++;
		}
		else
		{
	    	    /*
	     	     * let the mi helper routine do our work;
	     	     * better than duplicating code...
	     	     */
    	    	    int err;		 /* modified bresenham error term */
    	    	    int clip1=0, clip2=0;/* clippedness of the endpoints */
    	    	
    	    	    int clipdx, clipdy;	 /* difference between clipped and
				       	       	   unclipped start point */
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
		    if (axis == X_AXIS)
			len = abs(new_x2 - new_x1);
		    else
			len = abs(new_y2 - new_y1);
    
		    len += (clip2 != 0);
		    if (len)
		    {
			/* unwind bresenham error term to first point */
			if (clip1)
			{
			   clipdx = abs(new_x1 - x1);
			    clipdy = abs(new_y1 - y1);
			    if (axis == X_AXIS)
				err = e+((clipdy*e2) + ((clipdx-clipdy)*e1));
			    else
				err = e+((clipdx*e2) + ((clipdy-clipdx)*e1));
			}
			else
			    err = e;

			WaitQueue(7);
			outw(CUR_X, (short)new_x1);
			outw(CUR_Y, (short)new_y1);
			outw(ERR_TERM, (short)(err + fix));
			outw(DESTY_AXSTP, (short)e1);
			outw(DESTX_DIASTP, (short)e2);
			outw(MAJ_AXIS_PCNT, (short)len);
			outw(CMD, cmd2);
		    }
		    pbox++;
		}
	    } /* while (nbox--) */
	} /* sloped line */
#ifdef ONE_RECT
	npt += overrun - 1;
	WaitQueue(3);
	outw(LINEDRAW_INDEX, 0);  /* clear CLIP_OVERRUN; write current x,y */
	outw(LINEDRAW, x2);
	outw(LINEDRAW, y2);
      } /* if (overrun) */
#endif
    } /* while (--npt) */

#ifdef ONE_RECT
    WaitQueue(10);
    outw(LINEDRAW_OPT, CLIP_MODE_0);
#else
    WaitQueue(5);
#endif

    /* paint the last point if the end style isn't CapNotLast.
       (Assume that a projecting, butt, or round cap that is one
        pixel wide is the same as the single pixel of the endpoint.)
    */

    if ((pGC->capStyle != CapNotLast) &&
	((ppt->x + xorg != pptInit->x + pDrawable->x) ||
	 (ppt->y + yorg != pptInit->y + pDrawable->y) ||
	 (ppt == pptInit + 1)))
    {
	nbox = nboxInit;
	pbox = pboxInit;
	while (nbox--)
	{
	    if ((x2 >= pbox->x1) &&
		(y2 >= pbox->y1) &&
		(x2 <  pbox->x2) &&
		(y2 <  pbox->y2))
	    {
		outw(CUR_X, (short)x2);
		outw(CUR_Y, (short)y2);
		outw(MAJ_AXIS_PCNT, 0);
		outw(CMD, CMD_LINE | DRAW | LINETYPE | PLANAR | WRTDATA);

		break;
	    }
	    else
		pbox++;
	}
    }

#ifdef ONE_RECT
    outw(EXT_SCISSOR_L, 0);
    outw(EXT_SCISSOR_T, 0);
    outw(EXT_SCISSOR_R, mach32MaxX);
    outw(EXT_SCISSOR_B, mach32MaxY);
#endif
    outw(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);

    WaitIdleEmpty(); /* Make sure that all commands have finished */
}
