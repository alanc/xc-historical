/*
 * $XConsortium: sunGX.c,v 1.5 91/07/26 19:17:25 keith Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"mistruct.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#include	"mergerop.h"

#include	"sun.h"
#include	"sunGX.h"

#define sunGXFillSpan(gx,y,x1,x2,r) {\
    (gx)->arecty = (y); \
    (gx)->arectx = (x1); \
    (gx)->arecty = (y); \
    (gx)->arectx = (x2); \
    GXDrawDone(gx,r); \
}

/*
   rop_tables
   ==========
   lookup tables for GX raster ops, with the plane_mask,pixel_mask,pattern_mask
   ,attrib_sel, polygon_draw,raster_mode encoded into the top half.
   There is a lookup table for each commonly used combination.
*/

/* rops for bit blit / copy area
   with:
       Plane Mask - use plane mask reg.
       Pixel Mask - use all ones.
       Patt  Mask - use all ones.
*/

#define ROP_STANDARD	(GX_PLANE_MASK |\
			GX_PIXEL_ONES |\
			GX_POLYG_OVERLAP |\
			GX_ATTR_SUPP |\
			GX_RAST_BOOL |\
			GX_PLOT_PLOT)

/* fg = don't care  bg = don't care */

#define ROP_BLIT(O,I)	(ROP_STANDARD | \
			GX_PATTERN_ONES |\
			GX_ROP_11_1(I) |\
			GX_ROP_11_0(O) |\
			GX_ROP_10_1(I) |\
			GX_ROP_10_0(O) |\
			GX_ROP_01_1(I) |\
			GX_ROP_01_0(O) |\
			GX_ROP_00_1(I) |\
			GX_ROP_00_0(O))

/* fg = fgPixel	    bg = don't care */

#define ROP_FILL(O,I)	(ROP_STANDARD | \
			GX_PATTERN_ONES |\
			GX_ROP_11_1(I) |\
			GX_ROP_11_0(I) |\
			GX_ROP_10_1(I) |\
			GX_ROP_10_0(I) | \
			GX_ROP_01_1(O) |\
			GX_ROP_01_0(O) |\
			GX_ROP_00_1(O) |\
			GX_ROP_00_0(O))

/* fg = fgPixel	    bg = don't care */
 
#define ROP_STIP(O,I)   (ROP_STANDARD |\
			GX_PATTERN_MASK |\
			GX_ROP_11_1(I) |\
			GX_ROP_11_0(GX_ROP_NOOP) |\
			GX_ROP_10_1(I) |\
			GX_ROP_10_0(GX_ROP_NOOP) | \
			GX_ROP_01_1(O) |\
			GX_ROP_01_0(GX_ROP_NOOP) |\
			GX_ROP_00_1(O) |\
			GX_ROP_00_0(GX_ROP_NOOP))

/* fg = fgPixel	    bg = bgPixel */
			    
#define ROP_OSTP(O,I)   (ROP_STANDARD |\
			GX_PATTERN_MASK |\
			GX_ROP_11_1(I) |\
			GX_ROP_11_0(I) |\
			GX_ROP_10_1(I) |\
			GX_ROP_10_0(O) |\
			GX_ROP_01_1(O) |\
			GX_ROP_01_0(I) |\
			GX_ROP_00_1(O) |\
			GX_ROP_00_0(O))

static Uint gx_blit_rop_table[16]={
    ROP_BLIT(GX_ROP_CLEAR,  GX_ROP_CLEAR),	/* GXclear */
    ROP_BLIT(GX_ROP_CLEAR,  GX_ROP_NOOP),	/* GXand */
    ROP_BLIT(GX_ROP_CLEAR,  GX_ROP_INVERT),	/* GXandReverse */
    ROP_BLIT(GX_ROP_CLEAR,  GX_ROP_SET),	/* GXcopy */
    ROP_BLIT(GX_ROP_NOOP,   GX_ROP_CLEAR),	/* GXandInverted */
    ROP_BLIT(GX_ROP_NOOP,   GX_ROP_NOOP),	/* GXnoop */
    ROP_BLIT(GX_ROP_NOOP,   GX_ROP_INVERT),	/* GXxor */
    ROP_BLIT(GX_ROP_NOOP,   GX_ROP_SET),	/* GXor */
    ROP_BLIT(GX_ROP_INVERT, GX_ROP_CLEAR),	/* GXnor */
    ROP_BLIT(GX_ROP_INVERT, GX_ROP_NOOP),	/* GXequiv */
    ROP_BLIT(GX_ROP_INVERT, GX_ROP_INVERT),	/* GXinvert */
    ROP_BLIT(GX_ROP_INVERT, GX_ROP_SET),	/* GXorReverse */
    ROP_BLIT(GX_ROP_SET,    GX_ROP_CLEAR),	/* GXcopyInverted */
    ROP_BLIT(GX_ROP_SET,    GX_ROP_NOOP),	/* GXorInverted */
    ROP_BLIT(GX_ROP_SET,    GX_ROP_INVERT),	/* GXnand */
    ROP_BLIT(GX_ROP_SET,    GX_ROP_SET),	/* GXset */
};

/* rops for solid drawing
   with:
       Plane Mask - use plane mask reg.
       Pixel Mask - use all ones.
       Patt  Mask - use all ones.
*/

static Uint gx_solid_rop_table[16]={
    ROP_FILL(GX_ROP_CLEAR,  GX_ROP_CLEAR),	/* GXclear */
    ROP_FILL(GX_ROP_CLEAR,  GX_ROP_NOOP),	/* GXand */
    ROP_FILL(GX_ROP_CLEAR,  GX_ROP_INVERT),	/* GXandReverse */
    ROP_FILL(GX_ROP_CLEAR,  GX_ROP_SET),	/* GXcopy */
    ROP_FILL(GX_ROP_NOOP,   GX_ROP_CLEAR),	/* GXandInverted */
    ROP_FILL(GX_ROP_NOOP,   GX_ROP_NOOP),	/* GXnoop */
    ROP_FILL(GX_ROP_NOOP,   GX_ROP_INVERT),	/* GXxor */
    ROP_FILL(GX_ROP_NOOP,   GX_ROP_SET),	/* GXor */
    ROP_FILL(GX_ROP_INVERT, GX_ROP_CLEAR),	/* GXnor */
    ROP_FILL(GX_ROP_INVERT, GX_ROP_NOOP),	/* GXequiv */
    ROP_FILL(GX_ROP_INVERT, GX_ROP_INVERT),	/* GXinvert */
    ROP_FILL(GX_ROP_INVERT, GX_ROP_SET),	/* GXorReverse */
    ROP_FILL(GX_ROP_SET,    GX_ROP_CLEAR),	/* GXcopyInverted */
    ROP_FILL(GX_ROP_SET,    GX_ROP_NOOP),	/* GXorInverted */
    ROP_FILL(GX_ROP_SET,    GX_ROP_INVERT),	/* GXnand */
    ROP_FILL(GX_ROP_SET,    GX_ROP_SET),	/* GXset */
};

static Uint gx_stipple_rop_table[16]={
    ROP_STIP(GX_ROP_CLEAR,  GX_ROP_CLEAR),	/* GXclear */
    ROP_STIP(GX_ROP_CLEAR,  GX_ROP_NOOP),	/* GXand */
    ROP_STIP(GX_ROP_CLEAR,  GX_ROP_INVERT),	/* GXandReverse */
    ROP_STIP(GX_ROP_CLEAR,  GX_ROP_SET),	/* GXcopy */
    ROP_STIP(GX_ROP_NOOP,   GX_ROP_CLEAR),	/* GXandInverted */
    ROP_STIP(GX_ROP_NOOP,   GX_ROP_NOOP),	/* GXnoop */
    ROP_STIP(GX_ROP_NOOP,   GX_ROP_INVERT),	/* GXxor */
    ROP_STIP(GX_ROP_NOOP,   GX_ROP_SET),	/* GXor */
    ROP_STIP(GX_ROP_INVERT, GX_ROP_CLEAR),	/* GXnor */
    ROP_STIP(GX_ROP_INVERT, GX_ROP_NOOP),	/* GXequiv */
    ROP_STIP(GX_ROP_INVERT, GX_ROP_INVERT),	/* GXinvert */
    ROP_STIP(GX_ROP_INVERT, GX_ROP_SET),	/* GXorReverse */
    ROP_STIP(GX_ROP_SET,    GX_ROP_CLEAR),	/* GXcopyInverted */
    ROP_STIP(GX_ROP_SET,    GX_ROP_NOOP),	/* GXorInverted */
    ROP_STIP(GX_ROP_SET,    GX_ROP_INVERT),	/* GXnand */
    ROP_STIP(GX_ROP_SET,    GX_ROP_SET),	/* GXset */
};

static Uint gx_opaque_stipple_rop_table[16]={
    ROP_OSTP(GX_ROP_CLEAR,  GX_ROP_CLEAR),	/* GXclear */
    ROP_OSTP(GX_ROP_CLEAR,  GX_ROP_NOOP),	/* GXand */
    ROP_OSTP(GX_ROP_CLEAR,  GX_ROP_INVERT),	/* GXandReverse */
    ROP_OSTP(GX_ROP_CLEAR,  GX_ROP_SET),	/* GXcopy */
    ROP_OSTP(GX_ROP_NOOP,   GX_ROP_CLEAR),	/* GXandInverted */
    ROP_OSTP(GX_ROP_NOOP,   GX_ROP_NOOP),	/* GXnoop */
    ROP_OSTP(GX_ROP_NOOP,   GX_ROP_INVERT),	/* GXxor */
    ROP_OSTP(GX_ROP_NOOP,   GX_ROP_SET),	/* GXor */
    ROP_OSTP(GX_ROP_INVERT, GX_ROP_CLEAR),	/* GXnor */
    ROP_OSTP(GX_ROP_INVERT, GX_ROP_NOOP),	/* GXequiv */
    ROP_OSTP(GX_ROP_INVERT, GX_ROP_INVERT),	/* GXinvert */
    ROP_OSTP(GX_ROP_INVERT, GX_ROP_SET),	/* GXorReverse */
    ROP_OSTP(GX_ROP_SET,    GX_ROP_CLEAR),	/* GXcopyInverted */
    ROP_OSTP(GX_ROP_SET,    GX_ROP_NOOP),	/* GXorInverted */
    ROP_OSTP(GX_ROP_SET,    GX_ROP_INVERT),	/* GXnand */
    ROP_OSTP(GX_ROP_SET,    GX_ROP_SET),	/* GXset */
};

int	sunGXScreenPrivateIndex;
int	sunGXGCPrivateIndex;
int	sunGXWindowPrivateIndex;
int	sunGXGeneration;

/*
  sunGXDoBitBlt
  =============
  Bit Blit for all window to window blits.
*/
int
sunGXDoBitblt(pSrc, pDst, alu, prgnDst, pptSrc, planemask)
    DrawablePtr	    pSrc, pDst;
    int		    alu;
    RegionPtr	    prgnDst;
    DDXPointPtr	    pptSrc;
    unsigned long   planemask;
{
    register sunGXPtr	gx = sunGXGetScreenPrivate (pSrc->pScreen);
    register long r;
    register BoxPtr pboxTmp;
    register DDXPointPtr pptTmp;
    register int nbox;
    BoxPtr pboxNext,pboxBase,pbox;

    /* setup GX ( need fg of 0xff for blits ) */
    GXBlitInit(gx,gx_blit_rop_table[alu],planemask);

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);

    /* need to blit rectangles in different orders, depending on the direction of copy
       so that an area isnt overwritten before it is blitted */
    if( (pptSrc->y < pbox->y1) && (nbox > 1) ){

	if( (pptSrc->x < pbox->x1) && (nbox > 1) ){

	    /* reverse order of bands and rects in each band */
	    pboxTmp=pbox+nbox;
	    pptTmp=pptSrc+nbox;
	    
	    while (nbox--){
		pboxTmp--;
		pptTmp--;	
		gx->x0=pptTmp->x;
		gx->y0=pptTmp->y;
		gx->x1=pptTmp->x+(pboxTmp->x2-pboxTmp->x1)-1;
		gx->y1=pptTmp->y+(pboxTmp->y2-pboxTmp->y1)-1;
		gx->x2=pboxTmp->x1;
		gx->y2=pboxTmp->y1;
		gx->x3=pboxTmp->x2-1;
		gx->y3=pboxTmp->y2-1;
		GXBlitDone(gx,r);
	    }
	}
	else{

	    /* keep ordering in each band, reverse order of bands */
	    pboxBase = pboxNext = pbox+nbox-1;

	    while (pboxBase >= pbox){ /* for each band */

		/* find first box in band */
		while ((pboxNext >= pbox) &&
		       (pboxBase->y1 == pboxNext->y1))
		    pboxNext--;
		
		pboxTmp = pboxNext+1;			/* first box in band */
		pptTmp = pptSrc + (pboxTmp - pbox);	/* first point in band */
		
		while (pboxTmp <= pboxBase){ /* for each box in band */
		    gx->x0=pptTmp->x;
		    gx->y0=pptTmp->y;
		    gx->x1=pptTmp->x+(pboxTmp->x2-pboxTmp->x1)-1;
		    gx->y1=pptTmp->y+(pboxTmp->y2-pboxTmp->y1)-1;
		    gx->x2=pboxTmp->x1;
		    gx->y2=pboxTmp->y1;
		    gx->x3=pboxTmp->x2-1;
		    gx->y3=pboxTmp->y2-1;
		    ++pboxTmp;
		    ++pptTmp;	
		    GXBlitDone(gx,r);
		}
		pboxBase = pboxNext;
	    }
	}
    }
    else{

	if( (pptSrc->x < pbox->x1) && (nbox > 1) ){
	
	    /* reverse order of rects in each band */
	    pboxBase = pboxNext = pbox;

	    while (pboxBase < pbox+nbox){ /* for each band */

		/* find last box in band */
		while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		    pboxNext++;
		
		pboxTmp = pboxNext;			/* last box in band */
		pptTmp = pptSrc + (pboxTmp - pbox);	/* last point in band */
		
		while (pboxTmp != pboxBase){ /* for each box in band */
		    --pboxTmp;
		    --pptTmp;	
		    gx->x0=pptTmp->x;
		    gx->y0=pptTmp->y;
		    gx->x1=pptTmp->x+(pboxTmp->x2-pboxTmp->x1)-1;
		    gx->y1=pptTmp->y+(pboxTmp->y2-pboxTmp->y1)-1;
		    gx->x2=pboxTmp->x1;
		    gx->y2=pboxTmp->y1;
		    gx->x3=pboxTmp->x2-1;
		    gx->y3=pboxTmp->y2-1;
		    GXBlitDone(gx,r);
		}
		pboxBase = pboxNext;
	    }
	}
	else{

	    /* dont need to change order of anything */
	    pptTmp=pptSrc;
	    pboxTmp=pbox;
	    
	    while(nbox--){
		gx->x0=pptTmp->x;
		gx->y0=pptTmp->y;
		gx->x1=pptTmp->x+(pboxTmp->x2-pboxTmp->x1)-1;
		gx->y1=pptTmp->y+(pboxTmp->y2-pboxTmp->y1)-1;
		gx->x2=pboxTmp->x1;
		gx->y2=pboxTmp->y1;
		gx->x3=pboxTmp->x2-1;
		gx->y3=pboxTmp->y2-1;
		pboxTmp++;
		pptTmp++;
		GXBlitDone(gx,r);
	    }
	}
    }
    GXWait(gx,r);
}

RegionPtr
sunGXCopyArea(pSrcDrawable, pDstDrawable,
            pGC, srcx, srcy, width, height, dstx, dsty)
    register DrawablePtr pSrcDrawable;
    register DrawablePtr pDstDrawable;
    GC *pGC;
    int srcx, srcy;
    int width, height;
    int dstx, dsty;
{
    extern RegionPtr	cfbBitBlt (), cfbCopyArea ();

    if (pSrcDrawable->type != DRAWABLE_WINDOW)
	return cfbCopyArea (pSrcDrawable, pDstDrawable,
            pGC, srcx, srcy, width, height, dstx, dsty);
    return cfbBitBlt (pSrcDrawable, pDstDrawable,
            pGC, srcx, srcy, width, height, dstx, dsty, sunGXDoBitblt, 0);
}

void
sunGXFillRectAll (pDrawable, pGC, nBox, pBox)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nBox;
    BoxPtr	    pBox;
{
    register sunGXPtr	gx = sunGXGetScreenPrivate (pDrawable->pScreen);
    register sunGXPrivGCPtr gxPriv = sunGXGetGCPrivate (pGC);
    register cfbPrivGCPtr   devPriv = cfbGetGCPrivate (pGC);
    register int	r;

    GXDrawInit(gx,pGC->fgPixel,gx_solid_rop_table[pGC->alu],pGC->planemask);
    if (gxPriv->stipple)
	GXStippleInit(gx,gxPriv->stipple);
    while (nBox--) {
	gx->arecty = pBox->y1;
	gx->arectx = pBox->x1;
	gx->arecty = pBox->y2 - 1;
	gx->arectx = pBox->x2 - 1;
	pBox++;
	GXDrawDone(gx,r);
    }
    GXWait(gx,r);
}

#define NUM_STACK_RECTS	1024

void
sunGXPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    register GCPtr pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    xRectangle	    *prect;
    RegionPtr	    prgnClip;
    register BoxPtr pbox;
    register BoxPtr pboxClipped;
    BoxPtr	    pboxClippedBase;
    BoxPtr	    pextent;
    BoxRec	    stackRects[NUM_STACK_RECTS];
    cfbPrivGC	    *priv;
    int		    numRects;
    void	    (*BoxFill)();
    int		    n;
    int		    xorg, yorg;

    priv = (cfbPrivGC *) pGC->devPrivates[cfbGCPrivateIndex].ptr;
    prgnClip = priv->pCompositeClip;
    prect = prectInit;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (xorg || yorg)
    {
	prect = prectInit;
	n = nrectFill;
	while(n--)
	{
	    prect->x += xorg;
	    prect->y += yorg;
	    prect++;
	}
    }

    prect = prectInit;

    numRects = REGION_NUM_RECTS(prgnClip) * nrectFill;
    if (numRects > NUM_STACK_RECTS)
    {
	pboxClippedBase = (BoxPtr)ALLOCATE_LOCAL(numRects * sizeof(BoxRec));
	if (!pboxClippedBase)
	    return;
    }
    else
	pboxClippedBase = stackRects;

    pboxClipped = pboxClippedBase;
	
    if (REGION_NUM_RECTS(prgnClip) == 1)
    {
	int x1, y1, x2, y2, bx2, by2;

	pextent = REGION_RECTS(prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
    	while (nrectFill--)
    	{
	    if ((pboxClipped->x1 = prect->x) < x1)
		pboxClipped->x1 = x1;
    
	    if ((pboxClipped->y1 = prect->y) < y1)
		pboxClipped->y1 = y1;
    
	    bx2 = (int) prect->x + (int) prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    pboxClipped->x2 = bx2;
    
	    by2 = (int) prect->y + (int) prect->height;
	    if (by2 > y2)
		by2 = y2;
	    pboxClipped->y2 = by2;

	    prect++;
	    if ((pboxClipped->x1 < pboxClipped->x2) &&
		(pboxClipped->y1 < pboxClipped->y2))
	    {
		pboxClipped++;
	    }
    	}
    }
    else
    {
	int x1, y1, x2, y2, bx2, by2;

	pextent = (*pGC->pScreen->RegionExtents)(prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
    	while (nrectFill--)
    	{
	    BoxRec box;
    
	    if ((box.x1 = prect->x) < x1)
		box.x1 = x1;
    
	    if ((box.y1 = prect->y) < y1)
		box.y1 = y1;
    
	    bx2 = (int) prect->x + (int) prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    box.x2 = bx2;
    
	    by2 = (int) prect->y + (int) prect->height;
	    if (by2 > y2)
		by2 = y2;
	    box.y2 = by2;
    
	    prect++;
    
	    if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    	continue;
    
	    n = REGION_NUM_RECTS (prgnClip);
	    pbox = REGION_RECTS(prgnClip);
    
	    /* clip the rectangle to each box in the clip region
	       this is logically equivalent to calling Intersect()
	    */
	    while(n--)
	    {
		pboxClipped->x1 = max(box.x1, pbox->x1);
		pboxClipped->y1 = max(box.y1, pbox->y1);
		pboxClipped->x2 = min(box.x2, pbox->x2);
		pboxClipped->y2 = min(box.y2, pbox->y2);
		pbox++;

		/* see if clipping left anything */
		if(pboxClipped->x1 < pboxClipped->x2 && 
		   pboxClipped->y1 < pboxClipped->y2)
		{
		    pboxClipped++;
		}
	    }
    	}
    }
    if (pboxClipped != pboxClippedBase)
	sunGXFillRectAll(pDrawable, pGC,
		    pboxClipped-pboxClippedBase, pboxClippedBase);
    if (pboxClippedBase != stackRects)
    	DEALLOCATE_LOCAL(pboxClippedBase);
}

void
sunGXFillSpans (pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
    int		    x, y;
				/* next three parameters are post-clip */
    int		    n;		/* number of spans to fill */
    DDXPointPtr	    ppt;	/* pointer to list of start points */
    int		    *pwidthFree;/* copies of the pointers to free */
    DDXPointPtr	    pptFree;
    int		    *pwidth;
    register sunGXPtr	gx = sunGXGetScreenPrivate (pDrawable->pScreen);
    cfbPrivGCPtr    devPriv = cfbGetGCPrivate(pGC);
    register sunGXPrivGCPtr gxPriv = sunGXGetGCPrivate (pGC);
    register int    r;

    GXDrawInit(gx,pGC->fgPixel,gx_solid_rop_table[pGC->alu],pGC->planemask)
    if (gxPriv->stipple)
	GXStippleInit(gx,gxPriv->stipple);
    n = nInit * miFindMaxBand(devPriv->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	if (pptFree) DEALLOCATE_LOCAL(pptFree);
	if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(devPriv->pCompositeClip,
		     pptInit, pwidthInit, nInit,
		     ppt, pwidth, fSorted);
    while (n--)
    {
	x = ppt->x;
	y = ppt->y;
	ppt++;
	sunGXFillSpan(gx,y,x,x + *pwidth++ - 1,r);
    }
    GXWait(gx,r);
}

#ifdef NOTDEF
/* cfb is faster for dots */
void
sunGXPolyPoint(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr pGC;
    int mode;
    int npt;
    xPoint *pptInit;
{
    register sunGXPtr	gx = sunGXGetScreenPrivate (pDrawable->pScreen);
    RegionPtr	    cclip;
    int		    nbox;
    register int    i;
    register BoxPtr pbox;
    cfbPrivGCPtr    devPriv;
    xPoint	    *ppt;
    int		    x, y;
    int		    r;
    int		    off;

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    if (devPriv->rop == GXnoop)
	return;
    mode -= CoordModePrevious;
    cclip = devPriv->pCompositeClip;
    GXDrawInit(gx,pGC->fgPixel,gx_solid_rop_table[pGC->alu],pGC->planemask);
    gx->offx = pDrawable->x;
    gx->offy = pDrawable->y;
    for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip);
	 --nbox >= 0;
	 pbox++)
    {
	gx->clipminx = pbox->x1;
	gx->clipminy = pbox->y1;
	gx->clipmaxx = pbox->x2 - 1;
	gx->clipmaxy = pbox->y2 - 1;
	if (!mode)
	{
	    x = 0;
	    y = 0;
	    for (ppt = pptInit, i = npt; --i >= 0;)
	    {
	    	gx->apointy = y += ppt->y;
	    	gx->apointx = x += ppt->x;
	    	++ppt;
	    	GXDrawDone(gx,r);
	    }
	}
	else
	{
	    for (ppt = pptInit, i = npt; --i >= 0;)
	    {
	    	gx->apointy = ppt->y;
	    	gx->apointx = ppt->x;
	    	++ppt;
	    	GXDrawDone(gx,r);
	    }
	}
    }
    GXWait(gx,r);
    gx->offx = 0;
    gx->offy = 0;
    gx->clipminx = 0;
    gx->clipminy = 0;
    gx->clipmaxx = pDrawable->pScreen->width-1;
    gx->clipmaxy = pDrawable->pScreen->height-1;
}
#endif

#include "mifillarc.h"

#define FILLSPAN(gx,y,x1,x2,r) {\
    if (x2 >= x1) {\
	sunGXFillSpan(gx,y,x1,x2,r) \
    } \
}

#define FILLSLICESPANS(flip,y) \
    if (!flip) \
    { \
	FILLSPAN(gx,y,xl,xr,r) \
    } \
    else \
    { \
	xc = xorg - x; \
	FILLSPAN(gx, y, xc, xr, r) \
	xc += slw - 1; \
	FILLSPAN(gx, y, xl, xc, r) \
    }

sunGXFillEllipse (pDraw, gx, arc)
    DrawablePtr	pDraw;
    sunGXPtr	gx;
    xArc	*arc;
{
    int x, y, e;
    int yk, xk, ym, xm, dx, dy, xorg, yorg;
    int	y_top, y_bot;
    miFillArcRec info;
    register int n;
    register int xpos;
    int	r;
    int	slw;

    miFillArcSetup(arc, &info);
    MIFILLARCSETUP();
    xorg += pDraw->x;
    yorg += pDraw->y;
    y_top = yorg - y;
    y_bot = yorg + y + dy;
    while (y)
    {
	y_top++;
	y_bot--;
	MIFILLARCSTEP(slw);
	if (!slw)
	    continue;
	xpos = xorg - x;
	sunGXFillSpan (gx,y_top,xpos,xpos+slw - 1,r);
	if (miFillArcLower(slw))
	    sunGXFillSpan (gx,y_bot,xpos,xpos+slw - 1,r);
    }
}


static void
sunGXFillArcSlice (pDraw, pGC, gx, arc)
    DrawablePtr pDraw;
    GCPtr	pGC;
    sunGXPtr	gx;
    xArc	*arc;
{
    int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
    register int x, y, e;
    miFillArcRec info;
    miArcSliceRec slice;
    int xl, xr, xc;
    int	y_top, y_bot;
    int	r;

    miFillArcSetup(arc, &info);
    miFillArcSliceSetup(arc, &slice, pGC);
    MIFILLARCSETUP();
    xorg += pDraw->x;
    yorg += pDraw->y;
    y_top = yorg - y;
    y_bot = yorg + y + dy;
    slice.edge1.x += pDraw->x;
    slice.edge2.x += pDraw->x;
    while (y > 0)
    {
	y_top++;
	y_bot--;
	MIFILLARCSTEP(slw);
	MIARCSLICESTEP(slice.edge1);
	MIARCSLICESTEP(slice.edge2);
	if (miFillSliceUpper(slice))
	{
	    MIARCSLICEUPPER(xl, xr, slice, slw);
	    FILLSLICESPANS(slice.flip_top, y_top);
	}
	if (miFillSliceLower(slice))
	{
	    MIARCSLICELOWER(xl, xr, slice, slw);
	    FILLSLICESPANS(slice.flip_bot, y_bot);
	}
    }
}

#ifdef NOTDEF
#define BIG_ENDIAN
#include    "circleset.h"
#define UNSET_CIRCLE	if (old_width) \
			{ \
			    gx->alu = gx_solid_rop_table[pGC->alu]; \
			    old_width = -old_width; \
			}

#else
#define UNSET_CIRCLE
#endif

void
sunGXPolyFillArc (pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc *arc;
    register int i;
    BoxRec box;
    RegionPtr cclip;
    register sunGXPtr	gx = sunGXGetScreenPrivate (pDraw->pScreen);
    sunGXPrivGCPtr	gxPriv = sunGXGetGCPrivate (pGC);
    register int	r;
    int			old_width;

    GXDrawInit(gx,pGC->fgPixel,gx_solid_rop_table[pGC->alu],pGC->planemask);
    if (gxPriv->stipple)
	GXStippleInit(gx,gxPriv->stipple);
    cclip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
    old_width = 0;
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miFillArcEmpty(arc))
	    continue;
	if (miCanFillArc(arc))
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->height + 1;
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN)
	    {
		if ((arc->angle2 >= FULLCIRCLE) ||
		    (arc->angle2 <= -FULLCIRCLE))
		{
#ifdef NOTDEF
/* who really needs fast filled circles? */
		    if (arc->width == arc->height && arc->width <= 16 &&
			!gxPriv->stipple)
		    {
			int offx, offy;
			if (arc->width != old_width)
			{
			    int	    i;
			    Uint    *sp, *dp;

			    if (old_width != -arc->width)
			    {
			    	sp = (Uint *) filled_arcs[arc->width-1];
			    	dp = gx->pattern;
			    	i = 8;
			    	while (i--)
				    dp[i] = sp[i];
			    }
			    gx->alu = gx_stipple_rop_table[pGC->alu];
			    old_width = arc->width;
			}
			offx = 16 - box.x1 & 0xf;
			offy = 16 - box.y1 & 0xf;
			gx->patalign = (offx << 16) | offy;
			gx->arecty = box.y1;
			gx->arectx = box.x1;
			gx->arecty = box.y2 - 1;
			gx->arectx = box.x2 - 1;
			GXDrawDone (gx, r);
		    }
		    else
#endif
		    {
			UNSET_CIRCLE
			sunGXFillEllipse (pDraw, gx, arc);
		    }
		}
		else
		{
		    UNSET_CIRCLE
		    sunGXFillArcSlice (pDraw, pGC, gx, arc);
		}
		continue;
	    }
	}
	UNSET_CIRCLE
	miPolyFillArc(pDraw, pGC, 1, arc);
    }
    GXWait (gx, r);
}

void
sunGXFillPoly1Rect (pDrawable, pGC, shape, mode, count, ptsIn)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		count;
    DDXPointPtr	ptsIn;
{
    cfbPrivGCPtr    devPriv;
    int		    maxy;
    int		    origin;
    register int    vertex1, vertex2;
    int		    c;
    BoxPtr	    extents;
    int		    clip;
    int		    y;
    int		    *vertex1p, *vertex2p;
    int		    *endp;
    int		    x1, x2;
    int		    dx1, dx2;
    int		    dy1, dy2;
    int		    e1, e2;
    int		    step1, step2;
    int		    sign1, sign2;
    int		    h;
    int		    l, r;
    sunGXPtr	    gx = sunGXGetScreenPrivate (pDrawable->pScreen);
    sunGXPrivGCPtr  gxPriv = sunGXGetGCPrivate (pGC);
    int		    gx_r;

    if (mode == CoordModePrevious || shape != Convex)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, count, ptsIn);
	return;
    }
    GXDrawInit(gx,pGC->fgPixel,gx_solid_rop_table[pGC->alu],pGC->planemask);
    if (gxPriv->stipple)
	GXStippleInit(gx,gxPriv->stipple);
    devPriv = cfbGetGCPrivate (pGC);
    origin = *((int *) &pDrawable->x);
    origin -= (origin & 0x8000) << 1;
    extents = &devPriv->pCompositeClip->extents;
    vertex1 = *((int *) &extents->x1) - origin;
    vertex2 = *((int *) &extents->x2) - origin - 0x00010001;
    clip = 0;
    y = 32767;
    maxy = 0;
    vertex2p = (int *) ptsIn;
    endp = vertex2p + count;
    while (count--)
    {
	c = *vertex2p;
	clip |= (c - vertex1) | (vertex2 - c);
	c = intToY(c);
	if (c < y) 
	{
	    y = c;
	    vertex1p = vertex2p;
	}
	vertex2p++;
	if (c > maxy)
	    maxy = c;
    }
    if (y == maxy)
	return;

    if (clip & 0x80008000)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, vertex2p - (int *) ptsIn, ptsIn);
	return;
    }

    gx->offx = pDrawable->x;
    gx->offy = pDrawable->y;
    vertex2p = vertex1p;
    vertex2 = vertex1 = *vertex2p++;
    if (vertex2p == endp)
	vertex2p = (int *) ptsIn;
#define Setup(c,x,vertex,dx,dy,e,sign,step) {\
    x = intToX(vertex); \
    if (dy = intToY(c) - y) { \
    	dx = intToX(c) - x; \
	step = 0; \
    	if (dx >= 0) \
    	{ \
	    e = 0; \
	    sign = 1; \
	    if (dx >= dy) {\
	    	step = dx / dy; \
	    	dx = dx % dy; \
	    } \
    	} \
    	else \
    	{ \
	    e = 1 - dy; \
	    sign = -1; \
	    dx = -dx; \
	    if (dx >= dy) { \
		step = - (dx / dy); \
		dx = dx % dy; \
	    } \
    	} \
    } \
    vertex = c; \
}

#define Step(x,dx,dy,e,sign,step) {\
    x += step; \
    if ((e += dx) > 0) \
    { \
	x += sign; \
	e -= dy; \
    } \
}
    for (;;)
    {
	if (y == intToY(vertex1))
	{
	    do
	    {
	    	if (vertex1p == (int *) ptsIn)
		    vertex1p = endp;
	    	c = *--vertex1p;
	    	Setup (c,x1,vertex1,dx1,dy1,e1,sign1,step1)
	    } while (y == intToY(vertex1));
	    h = dy1;
	}
	else
	{
	    Step(x1,dx1,dy1,e1,sign1,step1)
	    h = intToY(vertex1) - y;
	}
	if (y == intToY(vertex2))
	{
	    do
	    {
	    	c = *vertex2p++;
	    	if (vertex2p == endp)
		    vertex2p = (int *) ptsIn;
	    	Setup (c,x2,vertex2,dx2,dy2,e2,sign2,step2)
	    } while (y == intToY(vertex2));
	    if (dy2 < h)
		h = dy2;
	}
	else
	{
	    Step(x2,dx2,dy2,e2,sign2,step2)
	    if ((c = (intToY(vertex2) - y)) < h)
		h = c;
	}
	/* fill spans for this segment */
	for (;;)
	{
	    l = x1;
	    r = x2;
    	    if (r < l)
	    {
	    	l = x2;
	    	r = x1;
    	    }
	    if (l != r)
		sunGXFillSpan (gx,y,l,r-1,gx_r);
	    y++;
	    if (!--h)
		break;
	    Step(x1,dx1,dy1,e1,sign1,step1)
	    Step(x2,dx2,dy2,e2,sign2,step2)
	}
	if (y == maxy)
	    break;
    }
    GXWait(gx,r);
    gx->offx = 0;
    gx->offy = 0;
}

/* XXX Note that the GX does not allow CapNotLast, so the code
 * fakes it.  This is expensive to do legitimately as the GX is
 * technically asynchronous and should be synced with GXWait before
 * fetching and storing the final line point.  This code works, though,
 * but it might not work forever.  If only the hardware was designed
 * for X.
 */

/* hard code the screen width; otherwise we'd have to check or mul */

#define WIDTH_MUL(y)	(((y) << 10) + ((y) << 7))

void
sunGXPolySeg1Rect (pDrawable, pGC, nseg, pSeg)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nseg;
    xSegment	    *pSeg;
{
    sunGXPtr	    gx = sunGXGetScreenPrivate (pDrawable->pScreen);
    sunGXPrivGCPtr  gxPriv = sunGXGetGCPrivate (pGC);
    BoxPtr	    extents;
    cfbPrivGCPtr    devPriv;
    int		    x, y;
    int		    r;
    unsigned char   *baseAddr, *topAddr, *saveAddr = 0, save;

    GXDrawInit(gx,pGC->fgPixel,gx_solid_rop_table[pGC->alu],pGC->planemask);
    if (gxPriv->stipple)
	GXStippleInit(gx,gxPriv->stipple);
    x = pDrawable->x;
    y = pDrawable->y;
    gx->offx = pDrawable->x;
    gx->offy = pDrawable->y;
    
    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    extents = &devPriv->pCompositeClip->extents;
    gx->clipminx = extents->x1;
    gx->clipminy = extents->y1;
    gx->clipmaxx = extents->x2 - 1;
    gx->clipmaxy = extents->y2 - 1;
    if (pGC->capStyle == CapNotLast)
    {
	cfbGetWindowByteWidthAndPointer((WindowPtr)pDrawable,x,baseAddr);
	baseAddr = baseAddr + WIDTH_MUL(y) + x;
	topAddr = baseAddr + WIDTH_MUL(pDrawable->height) + pDrawable->width;
    	while (nseg--)
    	{
	    gx->aliney = pSeg->y1;
	    gx->alinex = pSeg->x1;
	    y = pSeg->y2;
	    x = pSeg->x2;
	    if ((saveAddr = baseAddr + WIDTH_MUL(y) + x) < baseAddr ||
		saveAddr >= topAddr)
		saveAddr = 0;
	    else
		save = *saveAddr;
	    gx->aliney = y;
	    gx->alinex = x;
	    pSeg++;
	    GXDrawDone (gx, r);
	    if (saveAddr)
		*saveAddr = save;
    	}
    }
    else
    {
    	while (nseg--)
    	{
	    gx->aliney = pSeg->y1;
	    gx->alinex = pSeg->x1;
	    gx->aliney = pSeg->y2;
	    gx->alinex = pSeg->x2;
	    pSeg++;
	    GXDrawDone (gx, r);
    	}
    }
    GXWait (gx, r);
    gx->offx = 0;
    gx->offy = 0;
    gx->clipminx = 0;
    gx->clipminy = 0;
    gx->clipmaxx = pDrawable->pScreen->width;
    gx->clipmaxy = pDrawable->pScreen->height;
}

void
sunGXPolylines1Rect (pDrawable, pGC, mode, npt, ppt)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    mode;
    int		    npt;
    DDXPointPtr	    ppt;
{
    sunGXPtr	    gx = sunGXGetScreenPrivate (pDrawable->pScreen);
    sunGXPrivGCPtr  gxPriv = sunGXGetGCPrivate (pGC);
    BoxPtr	    extents;
    cfbPrivGCPtr    devPriv;
    unsigned char   *baseAddr, *topAddr, *saveAddr = 0, save;
    int		    x, y;
    int		    r;
    Bool	    careful;
    Bool	    capNotLast;

    GXDrawInit(gx,pGC->fgPixel,gx_solid_rop_table[pGC->alu],pGC->planemask);
    if (gxPriv->stipple)
	GXStippleInit(gx,gxPriv->stipple);
    x = pDrawable->x;
    y = pDrawable->y;
    gx->offx = x;
    gx->offy = y;
    careful = (pGC->alu & 0xc == 0x8 || pGC->alu & 0x3 == 0x2);
    capNotLast = pGC->capStyle == CapNotLast;
    mode -= CoordModePrevious;

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    extents = &devPriv->pCompositeClip->extents;
    gx->clipminx = extents->x1;
    gx->clipminy = extents->y1;
    gx->clipmaxx = extents->x2 - 1;
    gx->clipmaxy = extents->y2 - 1;
    --npt;
    if (mode && !careful && !capNotLast)
    {
	y = ppt->y;
	x = ppt->x;
	ppt++;
    	while (npt--)
    	{
	    gx->aliney = y;
	    gx->alinex = x;
	    gx->aliney = y = ppt->y;
	    gx->alinex = x = ppt->x;
	    ++ppt;
	    GXDrawDone(gx,r);
	}
    }
    else
    {
	cfbGetWindowByteWidthAndPointer((WindowPtr)pDrawable,x,baseAddr);
	baseAddr = baseAddr + WIDTH_MUL(y) + x;
	topAddr = baseAddr + WIDTH_MUL(pDrawable->height) + pDrawable->width;
	y = ppt->y;
	x = ppt->x;
	ppt++;
    	while (npt--)
    	{
	    gx->aliney = y;
	    gx->alinex = x;
	    if (!mode)
	    {
	    	y += ppt->y;
	    	x += ppt->x;
	    }
	    else
	    {
	    	y = ppt->y;
	    	x = ppt->x;
	    }
	    if (careful || !npt && capNotLast)
	    {
	    	if ((saveAddr = baseAddr + WIDTH_MUL(y) + x) >= baseAddr &&
		    saveAddr < topAddr)
		    saveAddr = 0;
	    	else
		    save = *saveAddr;
	    }
	    gx->aliney = y;
	    gx->alinex = x;
	    ppt++;
	    GXDrawDone (gx, r);
	    if (saveAddr)
	    {
	    	*saveAddr = save;
	    	saveAddr = 0;
	    }
    	}
    }
    GXWait (gx, r);
    gx->offx = 0;
    gx->offy = 0;
    gx->clipminx = 0;
    gx->clipminy = 0;
    gx->clipmaxx = pDrawable->pScreen->width;
    gx->clipmaxy = pDrawable->pScreen->height;
}

void
sunGXPolyFillRect1Rect (pDrawable, pGC, nrect, prect)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nrect;
    xRectangle	*prect;
{
    sunGXPtr	    gx = sunGXGetScreenPrivate (pDrawable->pScreen);
    sunGXPrivGCPtr  gxPriv = sunGXGetGCPrivate (pGC);
    cfbPrivGCPtr    devPriv = cfbGetGCPrivate (pGC);
    BoxPtr	    extents = &devPriv->pCompositeClip->extents;
    int		    r;
    int		    x, y;

    GXDrawInit(gx,pGC->fgPixel,gx_solid_rop_table[pGC->alu],pGC->planemask);
    if (gxPriv->stipple)
	GXStippleInit(gx,gxPriv->stipple);
    gx->offx = pDrawable->x;
    gx->offy = pDrawable->y;
    gx->clipminx = extents->x1;
    gx->clipminy = extents->y1;
    gx->clipmaxx = extents->x2 - 1;
    gx->clipmaxy = extents->y2 - 1;
    while (nrect--)
    {
	gx->arecty = y = prect->y;
	gx->arectx = x = prect->x;
	gx->arecty = y + (int) prect->height - 1;
	gx->arectx = x + (int) prect->width - 1;
	prect++;
	GXDrawDone (gx, r);
    }
    GXWait (gx, r);
    gx->offx = 0;
    gx->offy = 0;
    gx->clipminx = 0;
    gx->clipminy = 0;
    gx->clipmaxx = pDrawable->pScreen->width-1;
    gx->clipmaxy = pDrawable->pScreen->height-1;
}

void
sunGXFillBoxSolid (pDrawable, nBox, pBox, pixel)
    DrawablePtr	    pDrawable;
    int		    nBox;
    BoxPtr	    pBox;
    unsigned long   pixel;
{
    register sunGXPtr	gx = sunGXGetScreenPrivate (pDrawable->pScreen);
    register int	r;

    GXDrawInit(gx,pixel,gx_solid_rop_table[GXcopy],~0);
    while (nBox--) {
	gx->arecty = pBox->y1;
	gx->arectx = pBox->x1;
	gx->arecty = pBox->y2 - 1;
	gx->arectx = pBox->x2 - 1;
	pBox++;
	GXDrawDone(gx,r);
    }
    GXWait(gx,r);
}

void
sunGXFillBoxStipple (pDrawable, nBox, pBox, stipple)
    DrawablePtr	    pDrawable;
    int		    nBox;
    BoxPtr	    pBox;
    sunGXStipplePtr stipple;
{
    register sunGXPtr	gx = sunGXGetScreenPrivate (pDrawable->pScreen);
    register int	r;
    int			patx, paty;

    patx = 16 - (pDrawable->x & 0xf);
    paty = 16 - (pDrawable->y & 0xf);
    stipple->patalign = (patx <<  16) | paty;
    GXDrawInit(gx,0,gx_solid_rop_table[GXcopy],~0);
    GXStippleInit(gx, stipple);
    while (nBox--) {
	gx->arecty = pBox->y1;
	gx->arectx = pBox->x1;
	gx->arecty = pBox->y2 - 1;
	gx->arectx = pBox->x2 - 1;
	pBox++;
	GXDrawDone(gx,r);
    }
    GXWait(gx,r);
}

sunGXCheckTile (pPixmap, stipple)
    PixmapPtr	    pPixmap;
    sunGXStipplePtr stipple;
{
    unsigned short  *sbits;
    unsigned int    fg = ~0, bg = ~0;
    unsigned char   *tilebitsLine, *tilebits, tilebit;
    unsigned short  sbit, mask;
    int		    nbwidth;
    int		    h, w;
    int		    x, y;
    int		    s_y, s_x;

    h = pPixmap->drawable.height;
    if (h > 16 || (h & (h - 1)))
	return FALSE;
    w = pPixmap->drawable.width;
    if (w > 16 || (w & (w - 1)))
	return FALSE;
    sbits = (unsigned short *) stipple->bits;
    tilebitsLine = (unsigned char *) pPixmap->devPrivate.ptr;
    nbwidth = pPixmap->devKind;
    for (y = 0; y < h; y++) {
	tilebits = tilebitsLine;
	tilebitsLine += nbwidth;
	sbit = 0;
	mask = 1 << 15;
	for (x = 0; x < w; x++)
	{
	    tilebit = *tilebits++;
	    if (tilebit == fg)
		sbit |=  mask;
	    else if (tilebit != bg)
	    {
		if (fg == ~0)
		{
		    fg = tilebit;
		    sbit |= mask;
		}
		else if (bg == ~0)
		{
		    bg = tilebit;
		}
		else
		{
		    return FALSE;
		}
	    }
	    mask >>= 1;
	}
	for (s_x = w; s_x < 16; s_x <<= 1)
	    sbit = sbit | (sbit >> s_x);
	for (s_y = y; s_y < 16; s_y += h)
	    sbits[s_y] = sbit;
    }
    stipple->fore = fg;
    stipple->back = bg;
    return TRUE;
}

sunGXCheckStipple (pPixmap, stipple)
    PixmapPtr	    pPixmap;
    sunGXStipplePtr stipple;
{
    unsigned short  *sbits;
    unsigned long   *stippleBits;
    unsigned long   sbit, mask;
    int		    h, w;
    int		    x, y;
    int		    s_y, s_x;

    h = pPixmap->drawable.height;
    if (h > 16 || (h & (h - 1)))
	return FALSE;
    w = pPixmap->drawable.width;
    if (w > 16 || (w & (w - 1)))
	return FALSE;
    sbits = (unsigned short *) stipple->bits;
    stippleBits = (unsigned long *) pPixmap->devPrivate.ptr;
    mask = ((1 << w) - 1) << (16 - w);
    for (y = 0; y < h; y++) {
	sbit = (*stippleBits++ >> 16) & mask;
	for (s_x = w; s_x < 16; s_x <<= 1)
	    sbit = sbit | (sbit >> s_x);
	for (s_y = y; s_y < 16; s_y += h)
	    sbits[s_y] = sbit;
    }
    return TRUE;
}

static  sunGXStipplePtr tmpStipple;

sunGXCheckFill (pGC, pDrawable)
    GCPtr	pGC;
    DrawablePtr	pDrawable;
{
    sunGXPrivGCPtr	    gxPriv = sunGXGetGCPrivate (pGC);
    sunGXStipplePtr	    stipple;
    Uint		    alu;
    int			    xrot, yrot;

    if (pGC->fillStyle == FillSolid)
    {
	if (gxPriv->stipple)
	{
	    xfree (gxPriv->stipple);
	    gxPriv->stipple = 0;
	}
	return TRUE;
    }
    if (!(stipple = gxPriv->stipple))
    {
	if (!tmpStipple)
	{
	    tmpStipple = (sunGXStipplePtr) xalloc (sizeof *tmpStipple);
	    if (!tmpStipple)
		return FALSE;
	}
	stipple = tmpStipple;
    }
    alu =  gx_opaque_stipple_rop_table[pGC->alu];
    switch (pGC->fillStyle) {
    case FillTiled:
	if (!sunGXCheckTile (pGC->tile.pixmap, stipple))
	    return FALSE;
	break;
    case FillStippled:
	alu = gx_stipple_rop_table[pGC->alu];
    case FillOpaqueStippled:
	if (!sunGXCheckStipple (pGC->stipple, stipple))
	    return FALSE;
	stipple->fore = pGC->fgPixel;
	stipple->back = pGC->bgPixel;
	break;
    }
    xrot = (pGC->patOrg.x + pDrawable->x) & 0xf;
    yrot = (pGC->patOrg.y + pDrawable->y) & 0xf;
/*
    stipple->patalign = ((16 - (xrot & 0xf)) << 16) | (16 - (yrot & 0xf));
*/
    xrot = 16 - xrot;
    yrot = 16 - yrot;
    stipple->patalign = (xrot << 16) | yrot;
    stipple->alu = alu;
    gxPriv->stipple = stipple;
    if (stipple == tmpStipple)
	tmpStipple = 0;
    return TRUE;
}

void	sunGXValidateGC ();
void	cfbChangeGC ();
void	cfbCopyGC ();
void	sunGXDestroyGC ();
void	cfbChangeClip ();
void	cfbDestroyClip();
void	cfbCopyClip();

GCFuncs	sunGXGCFuncs = {
    sunGXValidateGC,
    cfbChangeGC,
    cfbCopyGC,
    sunGXDestroyGC,
    cfbChangeClip,
    cfbDestroyClip,
    cfbCopyClip,
};

GCOps	sunGXTEOps1Rect = {
    sunGXFillSpans,
    cfbSetSpans,
    cfbPutImage,
    sunGXCopyArea,
    cfbCopyPlane,
    cfbPolyPoint,
    sunGXPolylines1Rect,
    sunGXPolySeg1Rect,
    miPolyRectangle,
    cfbZeroPolyArcSS8Copy,
    sunGXFillPoly1Rect,
    sunGXPolyFillRect1Rect,
    sunGXPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    cfbTEGlyphBlt8,
    cfbPolyGlyphBlt8,
    cfbPushPixels8,
    NULL,
};

GCOps	sunGXTEOps = {
    sunGXFillSpans,
    cfbSetSpans,
    cfbPutImage,
    sunGXCopyArea,
    cfbCopyPlane,
    cfbPolyPoint,
    cfbLineSS,
    cfbSegmentSS,
    miPolyRectangle,
    cfbZeroPolyArcSS8Copy,
    miFillPolygon,
    sunGXPolyFillRect,
    sunGXPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    cfbTEGlyphBlt8,
    cfbPolyGlyphBlt8,
    cfbPushPixels8,
    NULL,
};

GCOps	sunGXNonTEOps1Rect = {
    sunGXFillSpans,
    cfbSetSpans,
    cfbPutImage,
    sunGXCopyArea,
    cfbCopyPlane,
    cfbPolyPoint,
    sunGXPolylines1Rect,
    sunGXPolySeg1Rect,
    miPolyRectangle,
    cfbZeroPolyArcSS8Copy,
    sunGXFillPoly1Rect,
    sunGXPolyFillRect1Rect,
    sunGXPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    cfbImageGlyphBlt8,
    cfbPolyGlyphBlt8,
    cfbPushPixels8,
    NULL,
};

GCOps	sunGXNonTEOps = {
    sunGXFillSpans,
    cfbSetSpans,
    cfbPutImage,
    sunGXCopyArea,
    cfbCopyPlane,
    cfbPolyPoint,
    cfbLineSS,
    cfbSegmentSS,
    miPolyRectangle,
    cfbZeroPolyArcSS8Copy,
    miFillPolygon,
    sunGXPolyFillRect,
    sunGXPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    cfbImageGlyphBlt8,
    cfbPolyGlyphBlt8,
    cfbPushPixels8,
    NULL,
};

#define PPW 4

GCOps *
sunGXMatchCommon (pGC, devPriv)
    GCPtr	    pGC;
    cfbPrivGCPtr    devPriv;
{
    if (pGC->lineWidth != 0)
	return 0;
    if (pGC->lineStyle != LineSolid)
	return 0;
    if (pGC->fillStyle != FillSolid)
	return 0;
    if (devPriv->rop != GXcopy)
	return 0;
    if (pGC->font &&
	FONTMAXBOUNDS(pGC->font,rightSideBearing) -
        FONTMINBOUNDS(pGC->font,leftSideBearing) <= 32 &&
	FONTMINBOUNDS(pGC->font,characterWidth) >= 0)
    {
	if (TERMINALFONT(pGC->font)
#if PPW == 4
	    && FONTMAXBOUNDS(pGC->font,characterWidth) >= 4
#endif
	)
	    if (devPriv->oneRect)
		return &sunGXTEOps1Rect;
	    else
		return &sunGXTEOps;
	else
	    if (devPriv->oneRect)
		return &sunGXNonTEOps1Rect;
	    else
		return &sunGXNonTEOps;
    }
    return 0;
}

void
sunGXValidateGC (pGC, changes, pDrawable)
    GCPtr	pGC;
    Mask	changes;
    DrawablePtr	pDrawable;
{
    int         mask;		/* stateChanges */
    int         index;		/* used for stepping through bitfields */
    int		new_rrop;
    int         new_line, new_text, new_fillspans, new_fillarea;
    int		new_rotate;
    int		xrot, yrot;
    /* flags for changing the proc vector */
    cfbPrivGCPtr devPriv;
    sunGXPrivGCPtr  gxPriv;
    int		oneRect;
    int		canGX;

    gxPriv = sunGXGetGCPrivate (pGC);
    if (pDrawable->type != DRAWABLE_WINDOW)
    {
	if (gxPriv->type == DRAWABLE_WINDOW)
	{
	    extern GCOps    cfbNonTEOps;

	    cfbDestroyOps (pGC->ops);
	    pGC->ops = &cfbNonTEOps;
	    changes = (1 << GCLastBit+1) - 1;
	    pGC->stateChanges = changes;
	    gxPriv->type = pDrawable->type;
	}
	cfbValidateGC (pGC, changes, pDrawable);
	return;
    }
    gxPriv->type = DRAWABLE_WINDOW;

    new_rotate = pGC->lastWinOrg.x != pDrawable->x ||
		 pGC->lastWinOrg.y != pDrawable->y;

    pGC->lastWinOrg.x = pDrawable->x;
    pGC->lastWinOrg.y = pDrawable->y;

    devPriv = ((cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr));

    new_rrop = FALSE;
    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;
    new_fillarea = FALSE;

    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */

    if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
	)
    {
	ScreenPtr pScreen = pGC->pScreen;
	RegionPtr   pregWin;
	Bool        freeTmpClip, freeCompClip;

	if (pGC->subWindowMode == IncludeInferiors) {
	    pregWin = NotClippedByChildren((WindowPtr) pDrawable);
	    freeTmpClip = TRUE;
	}
	else {
	    pregWin = &((WindowPtr) pDrawable)->clipList;
	    freeTmpClip = FALSE;
	}
	freeCompClip = devPriv->freeCompClip;

	/*
	 * if there is no client clip, we can get by with just keeping
	 * the pointer we got, and remembering whether or not should
	 * destroy (or maybe re-use) it later.  this way, we avoid
	 * unnecessary copying of regions.  (this wins especially if
	 * many clients clip by children and have no client clip.) 
	 */
	if (pGC->clientClipType == CT_NONE) {
	    if (freeCompClip)
		(*pScreen->RegionDestroy) (devPriv->pCompositeClip);
	    devPriv->pCompositeClip = pregWin;
	    devPriv->freeCompClip = freeTmpClip;
	}
	else {
	    /*
	     * we need one 'real' region to put into the composite
	     * clip. if pregWin the current composite clip are real,
	     * we can get rid of one. if pregWin is real and the
	     * current composite clip isn't, use pregWin for the
	     * composite clip. if the current composite clip is real
	     * and pregWin isn't, use the current composite clip. if
	     * neither is real, create a new region. 
	     */

	    (*pScreen->TranslateRegion)(pGC->clientClip,
					pDrawable->x + pGC->clipOrg.x,
					pDrawable->y + pGC->clipOrg.y);
					      
	    if (freeCompClip)
	    {
		(*pGC->pScreen->Intersect)(devPriv->pCompositeClip,
					   pregWin, pGC->clientClip);
		if (freeTmpClip)
		    (*pScreen->RegionDestroy)(pregWin);
	    }
	    else if (freeTmpClip)
	    {
		(*pScreen->Intersect)(pregWin, pregWin, pGC->clientClip);
		devPriv->pCompositeClip = pregWin;
	    }
	    else
	    {
		devPriv->pCompositeClip = (*pScreen->RegionCreate)(NullBox,
								   0);
		(*pScreen->Intersect)(devPriv->pCompositeClip,
				      pregWin, pGC->clientClip);
	    }
	    devPriv->freeCompClip = TRUE;
	    (*pScreen->TranslateRegion)(pGC->clientClip,
					-(pDrawable->x + pGC->clipOrg.x),
					-(pDrawable->y + pGC->clipOrg.y));
					      
	}
	oneRect = REGION_NUM_RECTS(devPriv->pCompositeClip) == 1;
	if (oneRect != devPriv->oneRect)
	{
	    new_line = TRUE;
	    new_fillarea = TRUE;
	}
	devPriv->oneRect = oneRect;
    }

    mask = changes;
    while (mask) {
	index = lowbit (mask);
	mask &= ~index;

	/*
	 * this switch acculmulates a list of which procedures might have
	 * to change due to changes in the GC.  in some cases (e.g.
	 * changing one 16 bit tile for another) we might not really need
	 * a change, but the code is being paranoid. this sort of batching
	 * wins if, for example, the alu and the font have been changed,
	 * or any other pair of items that both change the same thing. 
	 */
	switch (index) {
	case GCFunction:
	case GCForeground:
	    new_rrop = TRUE;
	    break;
	case GCPlaneMask:
	    new_rrop = TRUE;
	    new_text = TRUE;
	    break;
	case GCBackground:
	    break;
	case GCLineStyle:
	case GCLineWidth:
	    new_line = TRUE;
	    break;
	case GCCapStyle:
	    break;
	case GCJoinStyle:
	    break;
	case GCFillStyle:
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_line = TRUE;
	    new_fillarea = TRUE;
	    break;
	case GCFillRule:
	    break;
	case GCTile:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;

	case GCStipple:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;

	case GCTileStipXOrigin:
	    new_rotate = TRUE;
	    break;

	case GCTileStipYOrigin:
	    new_rotate = TRUE;
	    break;

	case GCFont:
	    new_text = TRUE;
	    break;
	case GCSubwindowMode:
	    break;
	case GCGraphicsExposures:
	    break;
	case GCClipXOrigin:
	    break;
	case GCClipYOrigin:
	    break;
	case GCClipMask:
	    break;
	case GCDashOffset:
	    break;
	case GCDashList:
	    break;
	case GCArcMode:
	    break;
	default:
	    break;
	}
    }

    /*
     * If the drawable has changed,  check its depth & ensure suitable
     * entries are in the proc vector. 
     */
    if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
	new_fillspans = TRUE;	/* deal with FillSpans later */
    }

    if ((new_rotate || new_fillspans))
    {
	Bool new_pix = FALSE;
	xrot = pGC->patOrg.x + pDrawable->x;
	yrot = pGC->patOrg.y + pDrawable->y;

	if (!sunGXCheckFill (pGC, pDrawable))
	{
	    switch (pGC->fillStyle)
	    {
	    case FillTiled:
	    	if (!pGC->tileIsPixel)
	    	{
		    int width = pGC->tile.pixmap->drawable.width * PSZ;
    
		    if ((width <= 32) && !(width & (width - 1)))
		    {
		    	cfbCopyRotatePixmap(pGC->tile.pixmap,
					    &devPriv->pRotatedPixmap,
					    xrot, yrot);
		    	new_pix = TRUE;
		    }
	    	}
	    	break;
	    case FillStippled:
	    case FillOpaqueStippled:
	    	{
		    int width = pGC->stipple->drawable.width;
    
		    if ((width <= 32) && !(width & (width - 1)))
		    {
		    	mfbCopyRotatePixmap(pGC->stipple,
					    &devPriv->pRotatedPixmap, xrot, yrot);
		    	new_pix = TRUE;
		    }
	    	}
	    	break;
	    }
	}
	if (!new_pix && devPriv->pRotatedPixmap)
	{
	    cfbDestroyPixmap(devPriv->pRotatedPixmap);
	    devPriv->pRotatedPixmap = (PixmapPtr) NULL;
	}
    }

    if (new_rrop)
    {
	int old_rrop;

	if (gxPriv->stipple)
	{
	    if (pGC->fillStyle == FillStippled)
		gxPriv->stipple->alu = gx_stipple_rop_table[pGC->alu];
	    else
		gxPriv->stipple->alu = gx_opaque_stipple_rop_table[pGC->alu];
	    if (pGC->fillStyle != FillTiled)
	    {
		gxPriv->stipple->fore = pGC->fgPixel;
		gxPriv->stipple->back = pGC->bgPixel;
	    }
	}
	old_rrop = devPriv->rop;
	devPriv->rop = cfbReduceRasterOp (pGC->alu, pGC->fgPixel,
					   pGC->planemask,
					   &devPriv->and, &devPriv->xor);
	if (old_rrop == devPriv->rop)
	    new_rrop = FALSE;
	else
	{
	    new_line = TRUE;
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	}
    }

    if (new_rrop || new_fillspans || new_text || new_fillarea || new_line)
    {
	GCOps	*newops;

	if (newops = sunGXMatchCommon (pGC, devPriv))
 	{
	    if (pGC->ops->devPrivate.val)
		cfbDestroyOps (pGC->ops);
	    pGC->ops = newops;
	    new_rrop = new_line = new_fillspans = new_text = new_fillarea = 0;
	}
 	else
 	{
	    if (!pGC->ops->devPrivate.val)
	    {
		extern GCOps	*cfbCreateOps();
		pGC->ops = cfbCreateOps (pGC->ops);
		pGC->ops->devPrivate.val = 1;
	    }
	}
    }

    canGX = pGC->fillStyle == FillSolid || gxPriv->stipple;

    /* deal with the changes we've collected */
    if (new_line)
    {
	pGC->ops->FillPolygon = miFillPolygon;
	if (devPriv->oneRect && canGX)
	    pGC->ops->FillPolygon = sunGXFillPoly1Rect;
	if (pGC->lineWidth == 0)
	{
	    if ((pGC->lineStyle == LineSolid) && (pGC->fillStyle == FillSolid))
	    {
		switch (devPriv->rop)
		{
		case GXxor:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8Xor;
		    break;
		case GXcopy:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8Copy;
		    break;
		default:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8General;
		    break;
		}
	    }
	    else
		pGC->ops->PolyArc = miZeroPolyArc;
	}
	else
	    pGC->ops->PolyArc = miPolyArc;
	pGC->ops->PolySegment = miPolySegment;
	switch (pGC->lineStyle)
	{
	case LineSolid:
	    if(pGC->lineWidth == 0)
	    {
		if (devPriv->oneRect && canGX)
		{
		    pGC->ops->PolySegment = sunGXPolySeg1Rect;
		    pGC->ops->Polylines = sunGXPolylines1Rect;
		}
		else if (pGC->fillStyle == FillSolid)
		{
		    if (devPriv->oneRect)
		    {
			pGC->ops->Polylines = cfb8LineSS1Rect;
			pGC->ops->PolySegment = cfb8SegmentSS1Rect;
		    }
		    else
		    {
		    	pGC->ops->Polylines = cfbLineSS;
		    	pGC->ops->PolySegment = cfbSegmentSS;
		    }
		}
	    }
	    else
		pGC->ops->Polylines = miWideLine;
	    break;
	case LineOnOffDash:
	case LineDoubleDash:
	    if (pGC->lineWidth == 0 && pGC->fillStyle == FillSolid)
	    {
		pGC->ops->Polylines = cfbLineSD;
		pGC->ops->PolySegment = cfbSegmentSD;
	    } else
		pGC->ops->Polylines = miWideDash;
	    break;
	}
    }

    if (new_text && (pGC->font))
    {
        if (FONTMAXBOUNDS(pGC->font,rightSideBearing) -
            FONTMINBOUNDS(pGC->font,leftSideBearing) > 32 ||
	    FONTMINBOUNDS(pGC->font,characterWidth) < 0)
        {
            pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
        else
        {
#if PPW == 4
	    if (pGC->fillStyle == FillSolid)
	    {
		if (devPriv->rop == GXcopy)
		    pGC->ops->PolyGlyphBlt = cfbPolyGlyphBlt8;
		else
		    pGC->ops->PolyGlyphBlt = cfbPolyGlyphRop8;
	    }
	    else
#endif
		pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            /* special case ImageGlyphBlt for terminal emulator fonts */
            if (TERMINALFONT(pGC->font) &&
		(pGC->planemask & PMSK) == PMSK
#if PPW == 4
		&& FONTMAXBOUNDS(pGC->font,characterWidth) >= 4
#endif
		)
	    {
#if PPW == 4
                pGC->ops->ImageGlyphBlt = cfbTEGlyphBlt8;
#else
                pGC->ops->ImageGlyphBlt = cfbTEGlyphBlt;
#endif
	    }
            else
	    {
#if PPW == 4
		pGC->ops->ImageGlyphBlt = cfbImageGlyphBlt8;
#else
                pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
#endif
	    }
        }
    }    


    if (new_fillspans) {
	if (canGX)
	    pGC->ops->FillSpans = sunGXFillSpans;
	else switch (pGC->fillStyle) {
	case FillTiled:
	    if (devPriv->pRotatedPixmap)
	    {
		if (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK)
		    pGC->ops->FillSpans = cfbTile32FSCopy;
		else
		    pGC->ops->FillSpans = cfbTile32FSGeneral;
	    }
	    else
		pGC->ops->FillSpans = cfbUnnaturalTileFS;
	    break;
	case FillStippled:
#if PPW == 4
	    if (devPriv->pRotatedPixmap)
		pGC->ops->FillSpans = cfb8Stipple32FS;
	    else
#endif
		pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	    break;
	case FillOpaqueStippled:
#if PPW == 4
	    if (devPriv->pRotatedPixmap)
		pGC->ops->FillSpans = cfb8OpaqueStipple32FS;
	    else
#endif
		pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	    break;
	default:
	    FatalError("cfbValidateGC: illegal fillStyle\n");
	}
    } /* end of new_fillspans */

    if (new_fillarea) {
	pGC->ops->PolyFillRect = cfbPolyFillRect;
	pGC->ops->PolyFillArc = miPolyFillArc;
	if (canGX)
	{
	    pGC->ops->PolyFillArc = sunGXPolyFillArc;
	    pGC->ops->PolyFillRect = sunGXPolyFillRect;
	    if (oneRect)
		pGC->ops->PolyFillRect = sunGXPolyFillRect1Rect;
	}
	pGC->ops->PushPixels = mfbPushPixels;
	if (pGC->fillStyle == FillSolid && devPriv->rop == GXcopy)
	    pGC->ops->PushPixels = cfbPushPixels8;
    }
}

void
sunGXDestroyGC (pGC)
    GCPtr   pGC;
{
    sunGXPrivGCPtr	    gxPriv = sunGXGetGCPrivate (pGC);

    if (gxPriv->stipple)
	xfree (gxPriv->stipple);
    cfbDestroyGC (pGC);
}

sunGXCreateGC (pGC)
    GCPtr   pGC;
{
    sunGXPrivGCPtr  gxPriv;
    if (pGC->depth == 1)
	return mfbCreateGC (pGC);
    if (!cfbCreateGC (pGC))
	return FALSE;
    pGC->ops = &sunGXNonTEOps;
    pGC->funcs = &sunGXGCFuncs;
    gxPriv = sunGXGetGCPrivate(pGC);
    gxPriv->type = DRAWABLE_WINDOW;
    gxPriv->stipple = 0;
    return TRUE;
}

Bool
sunGXCreateWindow (pWin)
    WindowPtr	pWin;
{
    if (!cfbCreateWindow (pWin))
	return FALSE;
    pWin->devPrivates[sunGXWindowPrivateIndex].ptr = 0;
    return TRUE;
}

Bool
sunGXDestroyWindow (pWin)
    WindowPtr	pWin;
{
    sunGXStipplePtr stipple;
    if (stipple = sunGXGetWindowPrivate(pWin))
	xfree (stipple);
    return cfbDestroyWindow (pWin);
}

sunGXChangeWindowAttributes (pWin, mask)
    WindowPtr	pWin;
    Mask	mask;
{
    sunGXStipplePtr stipple;
    Mask	    index;
    register cfbPrivWin *pPrivWin;
    int		    width;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);
    while (mask)
    {
	index = lowbit(mask);
	mask &= ~index;
	switch (index)
	{
	case CWBackPixmap:
	    stipple = sunGXGetWindowPrivate(pWin);
	    if (!stipple)
	    {
		tmpStipple = (sunGXStipplePtr) xalloc (sizeof *tmpStipple);
		stipple = tmpStipple;
	    }
	    if (pWin->backgroundState == None ||
		pWin->backgroundState == ParentRelative)
	    {
		pPrivWin->fastBackground = FALSE;
		if (stipple = sunGXGetWindowPrivate(pWin))
		{
		    xfree (stipple);
		    sunGXSetWindowPrivate(pWin,0);
		}
	    }
 	    else if (stipple && sunGXCheckTile (pWin->background.pixmap, stipple))
	    {
		stipple->alu = gx_opaque_stipple_rop_table[GXcopy];
		pPrivWin->fastBackground = FALSE;
		sunGXSetWindowPrivate(pWin, stipple);
		if (stipple == tmpStipple)
		    tmpStipple = 0;
	    }
 	    else if (((width = (pWin->background.pixmap->drawable.width * PSZ)) <= 32) &&
		       !(width & (width - 1)))
	    {
		if (stipple = sunGXGetWindowPrivate(pWin))
		{
		    xfree (stipple);
		    sunGXSetWindowPrivate(pWin,0);
		}
		cfbCopyRotatePixmap(pWin->background.pixmap,
				  &pPrivWin->pRotatedBackground,
				  pWin->drawable.x,
				  pWin->drawable.y);
		if (pPrivWin->pRotatedBackground)
		{
    	    	    pPrivWin->fastBackground = TRUE;
    	    	    pPrivWin->oldRotate.x = pWin->drawable.x;
    	    	    pPrivWin->oldRotate.y = pWin->drawable.y;
		}
		else
		{
		    pPrivWin->fastBackground = FALSE;
		}
	    }
	    else
	    {
		if (stipple = sunGXGetWindowPrivate(pWin))
		{
		    xfree (stipple);
		    sunGXSetWindowPrivate(pWin,0);
		}
		pPrivWin->fastBackground = FALSE;
	    }
	    break;
	case CWBackPixel:
	    pPrivWin->fastBackground = FALSE;
	    break;

	case CWBorderPixmap:
	    if (((width = (pWin->border.pixmap->drawable.width * PSZ)) <= 32) &&
		!(width & (width - 1)))
	    {
		cfbCopyRotatePixmap(pWin->border.pixmap,
				    &pPrivWin->pRotatedBorder,
				    pWin->drawable.x,
				    pWin->drawable.y);
		if (pPrivWin->pRotatedBorder)
		{
		    pPrivWin->fastBorder = TRUE;
		    pPrivWin->oldRotate.x = pWin->drawable.x;
		    pPrivWin->oldRotate.y = pWin->drawable.y;
		}
		else
		{
		    pPrivWin->fastBorder = TRUE;
		}
	    }
	    else
	    {
		pPrivWin->fastBorder = FALSE;
	    }
	    break;
	case CWBorderPixel:
	    pPrivWin->fastBorder = FALSE;
	    break;
	}
    }
    return (TRUE);
}

void
sunGXPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    register cfbPrivWin	*pPrivWin;
    sunGXStipplePtr stipple;
    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);

    switch (what) {
    case PW_BACKGROUND:
	stipple = sunGXGetWindowPrivate(pWin);
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
	    if (stipple)
	    {
		sunGXFillBoxStipple (pWin,
				  (int)REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion),
				  stipple);
		return;
	    }
	    else if (pPrivWin->fastBackground)
	    {
		cfbFillBoxTile32 ((DrawablePtr)pWin,
				  (int)REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion),
				  pPrivWin->pRotatedBackground);
		return;
	    }
	    else
	    {
		cfbFillBoxTileOdd ((DrawablePtr)pWin,
				   (int)REGION_NUM_RECTS(pRegion),
				   REGION_RECTS(pRegion),
				   pWin->background.pixmap,
				   (int) pWin->drawable.x, (int) pWin->drawable.y);
		return;
	    }
	    break;
	case BackgroundPixel:
	    sunGXFillBoxSolid(pWin,
			     (int)REGION_NUM_RECTS(pRegion),
			     REGION_RECTS(pRegion),
			     pWin->background.pixel);
	    return;
    	}
    	break;
    case PW_BORDER:
	if (pWin->borderIsPixel)
	{
	    sunGXFillBoxSolid(pWin,
			     (int)REGION_NUM_RECTS(pRegion),
			     REGION_RECTS(pRegion),
			     pWin->border.pixel);
	    return;
	}
	else if (pPrivWin->fastBorder)
	{
	    cfbFillBoxTile32 ((DrawablePtr)pWin,
			      (int)REGION_NUM_RECTS(pRegion),
			      REGION_RECTS(pRegion),
			      pPrivWin->pRotatedBorder);
	    return;
	}
	else if (pWin->border.pixmap->drawable.width >= PPW/2)
	{
	    cfbFillBoxTileOdd ((DrawablePtr)pWin,
			       (int)REGION_NUM_RECTS(pRegion),
			       REGION_RECTS(pRegion),
			       pWin->border.pixmap,
			       (int) pWin->drawable.x, (int) pWin->drawable.y);
	    return;
	}
	break;
    }
    miPaintWindow (pWin, pRegion, what);
}

void 
sunGXCopyWindow(pWin, ptOldOrg, prgnSrc)
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
    extern WindowPtr *WindowTable;

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

    sunGXDoBitblt ((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
		    GXcopy, prgnDst, pptSrc, ~0L);
    DEALLOCATE_LOCAL(pptSrc);
    (* pWin->drawable.pScreen->RegionDestroy)(prgnDst);
}

sunGXInit (pScreen, fb)
    ScreenPtr	pScreen;
    fbFd	*fb;
{
    sunGXPtr	    gx;
    Uint	    mode;
    register long   r;

    if (serverGeneration != sunGXGeneration)
    {
	sunGXScreenPrivateIndex = AllocateScreenPrivateIndex();
	if (sunGXScreenPrivateIndex == -1)
	    return FALSE;
	sunGXGCPrivateIndex = AllocateGCPrivateIndex ();
	sunGXWindowPrivateIndex = AllocateWindowPrivateIndex ();
	sunGXGeneration = serverGeneration;
    }
    if (!AllocateGCPrivate(pScreen, sunGXGCPrivateIndex, sizeof (sunGXPrivGCRec)))
	return FALSE;
    if (!AllocateWindowPrivate(pScreen, sunGXWindowPrivateIndex, 0))
	return FALSE;
    gx = (sunGXPtr) fb->fb;
    mode = gx->mode;
    GXWait(gx,r);
    mode &= ~(	GX_BLIT_ALL |
		GX_MODE_ALL | 
		GX_DRAW_ALL |
 		GX_BWRITE0_ALL |
		GX_BWRITE1_ALL |
 		GX_BREAD_ALL |
 		GX_BDISP_ALL);
    mode |=	GX_BLIT_SRC |
		GX_MODE_COLOR8 |
		GX_DRAW_RENDER |
		GX_BWRITE0_ENABLE |
		GX_BWRITE1_DISABLE |
		GX_BREAD_0 |
		GX_BDISP_0;
    gx->mode = mode;
    gx->clip = 0;
    gx->offx = 0;
    gx->offy = 0;
    gx->clipminx = 0;
    gx->clipminy = 0;
    gx->clipmaxx = fb->info.fb_width;
    gx->clipmaxy = fb->info.fb_height;
    pScreen->devPrivates[sunGXScreenPrivateIndex].ptr = (pointer) gx;
    /*
     * Replace various screen functions
     */
    pScreen->CreateGC = sunGXCreateGC;
    pScreen->CreateWindow = sunGXCreateWindow;
    pScreen->ChangeWindowAttributes = sunGXChangeWindowAttributes;
    pScreen->DestroyWindow = sunGXDestroyWindow;
    pScreen->PaintWindowBackground = sunGXPaintWindow;
    pScreen->PaintWindowBorder = sunGXPaintWindow;
    pScreen->CopyWindow = sunGXCopyWindow;
}

