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

#include "cfb.h"
#include "cfbmskbits.h"

static void cfbPaintArea32(), cfbPaintAreaSolid(), cfbPaintAreaOther();

static void cfbTileOddWin();
extern void miPaintWindow();

void
cfbPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    register cfbPrivWin	*pPrivWin;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);

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
		cfbFillBoxTile32 (pWin, REGION_NUM_RECTS(pRegion), REGION_RECTS(pRegion),
				  pPrivWin->pRotatedBackground);
		return;
	    }
	    break;
	case BackgroundPixel:
	    cfbFillBoxSolid (pWin, REGION_NUM_RECTS(pRegion), REGION_RECTS(pRegion),
		pWin->background.pixel, TRUE);
	    return;
    	}
    	break;
    case PW_BORDER:
	if (pWin->borderIsPixel)
	{
	    cfbFillBoxSolid (pWin, REGION_NUM_RECTS(pRegion), REGION_RECTS(pRegion),
		pWin->border.pixel, TRUE);
	    return;
	}
	else if (pPrivWin->fastBorder)
	{
	    cfbFillBoxTile32 (pWin, REGION_NUM_RECTS(pRegion), REGION_RECTS(pRegion),
				  pPrivWin->pRotatedBorder);
	    return;
	}
	break;
    }
    miPaintWindow (pWin, pRegion, what);
}

static void
cfbPaintAreaOther(pWin, pRegion, what)
    WindowPtr pWin;
    RegionPtr pRegion;
    int what;	

{
    int nbox;		/* number of boxes to fill */
    register BoxPtr pbox;	/* pointer to list of boxes to fill */
    int tileHeight;	/* height of the tile */
    int tileWidth;	/* width of the tile */

    PixmapPtr pPixmap;
    int w;		/* width of current box */
    register int h;	/* height of current box */
    int x, y;		/* current scan line */
    int	height, width;

    if ( pWin->drawable.depth != PSZ )
	FatalError( "cfbPaintAreaOther: invalid depth\n" );

    if (what == PW_BACKGROUND)
    {
	pPixmap = pWin->background.pixmap;
    }
    else
    {
	pPixmap = pWin->border.pixmap;
    } 
    tileHeight = pPixmap->drawable.height;
    tileWidth = pPixmap->drawable.width;

    nbox = REGION_NUM_RECTS(pRegion);
    pbox = REGION_RECTS(pRegion);

    while (nbox--)
    {
	w = pbox->x2;
	h = pbox->y2;

	if (  w - pbox->x1 <= PPW)
	{
	    y = pbox->y1;
	    x = pbox->x1;
	    width = min(tileWidth, w - x);
	    while ((height = h - y) > 0)
	    {
		height = min(height, tileHeight);
		cfbTileOddWin(pPixmap, pWin, width, height, x, y);
		y += tileHeight;
	    }
	}
	else
	{
	    y = pbox->y1;
	    while((height = h - y) > 0)
	    {
		height = min(height, tileHeight);
		x = pbox->x1;
		while ((width = w - x) > 0)
		{
		    width = min(tileWidth, width);
		    cfbTileOddWin(pPixmap, pWin, width, height, x, y);
		    x += tileWidth;
		}
		y += tileHeight;
	    }
	}
        pbox++;
    }
}

static void
cfbTileOddWin(pSrc, pDstWin, tileWidth, tileHeight, x, y)
    PixmapPtr	pSrc;		/* pointer to src tile */
    WindowPtr	pDstWin;	/* pointer to dest window */
    int	   	tileWidth;	/* width of tile */
    int		tileHeight;	/* height of tile */
    int 	x;		/* destination x */
    int 	y;		/* destination y */
    
{
    int 		*psrcLine, *pdstLine;
    register int	*pdst, *psrc;
    register int	nl;
    register int	tmpSrc;
    int			widthSrc, widthDst, nlMiddle, startmask, endmask;
    PixmapPtr		pDstPixmap;


    psrcLine = (int *)pSrc->devPrivate.ptr;

    pDstPixmap = (PixmapPtr)pDstWin->drawable.pScreen->devPrivate;
    widthDst = (int)pDstPixmap->devKind >> 2;
    pdstLine = (int *)pDstPixmap->devPrivate.ptr + (y * widthDst);
    widthSrc = (int)pSrc->devKind >> 2;

    if(tileWidth <= PPW)
    {
        int dstBit;

        psrc = psrcLine;
	pdst = pdstLine + (x / PPW);
	dstBit = x & PIM;

	while(tileHeight--)
	{
	    getbits(psrc, 0, tileWidth, tmpSrc);
/*XXX*/	    putbits(tmpSrc, dstBit, tileWidth, pdst, -1);
	    pdst += widthDst;
	    psrc += widthSrc;
	}

    }
    else
    {
	register int xoffSrc;	/* offset (>= 0, < 32) from which to
			         * fetch whole longwords fetched in src */
	int nstart;		/* number of ragged bits at start of dst */
	int nend;		/* number of regged bits at end of dst */
#ifdef notdef
	int srcStartOver;	/* pulling nstart bits from src overflows
			         * into the next word? */
#endif

	maskbits(x, tileWidth, startmask, endmask, nlMiddle);
	if (startmask)
	    nstart = PPW - (x & PIM);
	else
	    nstart = 0;
	if (endmask)
	    nend = (x + tileWidth) & PIM;

	xoffSrc = nstart & PIM;
#ifdef notdef
	srcStartOver = nstart > PLST;
#endif

	pdstLine += (x >> PWSH);

	while (tileHeight--)
	{
	    psrc = psrcLine;
	    pdst = pdstLine;

	    if (startmask)
	    {
		getbits(psrc, 0, nstart, tmpSrc);
/*XXX*/		putbits(tmpSrc, (x & PIM), nstart, pdst, -1);
		pdst++;
#ifdef	notdef
/* XXX - not sure if this is right or not DSHR */
		if (srcStartOver)
		    psrc++;
#endif
	    }

	    nl = nlMiddle;
	    while (nl--)
	    {
		getbits(psrc, xoffSrc, PPW, tmpSrc);
		*pdst++ = tmpSrc;
		psrc++;
	    }

	    if (endmask)
	    {
		getbits(psrc, xoffSrc, nend, tmpSrc);
/*XXX*/		putbits(tmpSrc, 0, nend, pdst, -1);
	    }

	    pdstLine += widthDst;
	    psrcLine += widthSrc;
	}
    }
}
