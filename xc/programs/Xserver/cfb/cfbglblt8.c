/*
 * Poly glyph blt for 8 bit displays.  Accepts
 * an arbitrary font <= 29 bits wide, in Copy mode only.
 */

/*
Copyright 1989 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission.  M.I.T. makes no
representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
*/
/* $XConsortium: Exp $ */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"

#if PPW == 4

#if (BITMAP_BIT_ORDER == MSBFirst)
#define GetFourBits(x)		(((x) >> 28) & 0xf)
#define ToXoff(x,xoff)		((x) >> (xoff))
#define NextFourBits(x)		((x) <<= 4)
#else
#define GetFourBits(x)		((x) & 0xf)
#define ToXoff(x,xoff)		((x) << (xoff))
#define NextFourBits(x)		((x) >>= 4)
#endif

#ifndef mips
#define SLOW_CPU
#endif

#ifdef SLOW_CPU
static unsigned long	masks[16] = {
    0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
    0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
    0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
    0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff,
};
#define WriteBits(dst,pixel,bits) \
    {						    \
    register unsigned long mask = masks[(bits)];    \
    *(dst) = (*(dst) & ~mask) | ((pixel) & mask);   \
    }
      
#else

#if (BITMAP_BIT_ORDER == MSBFirst)
#define WriteBits(dst,pixel,bits) \
	switch (bits) {			\
	case 0:				\
	    break;			\
	case 1:				\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 2:				\
	    ((char *) (dst))[2] = (pixel);	\
	    break;			\
	case 3:				\
	    ((short *) (dst))[1] = (pixel);	\
	    break;			\
	case 4:				\
	    ((char *) (dst))[1] = (pixel);	\
	    break;			\
	case 5:				\
	    ((char *) (dst))[3] = (pixel);	\
	    ((char *) (dst))[1] = (pixel);	\
	    break;			\
	case 6:				\
	    ((char *) (dst))[2] = (pixel);	\
	    ((char *) (dst))[1] = (pixel);	\
	    break;			\
	case 7:				\
	    ((short *) (dst))[1] = (pixel);	\
	    ((char *) (dst))[1] = (pixel);	\
	    break;			\
	case 8:				\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 9:				\
	    ((char *) (dst))[3] = (pixel);	\
	    ((char *) (dst))[0] = (pixel);	\
	    break;			\
	case 10:			\
	    ((char *) (dst))[2] = (pixel);	\
	    ((char *) (dst))[0] = (pixel);	\
	    break;			\
	case 11:			\
	    ((short *) (dst))[1] = (pixel);	\
	    ((char *) (dst))[0] = (pixel);	\
	    break;			\
	case 12:			\
	    ((short *) (dst))[0] = (pixel);	\
	    break;			\
	case 13:			\
	    ((char *) (dst))[3] = (pixel);	\
	    ((short *) (dst))[0] = (pixel);	\
	    break;			\
	case 14:			\
	    ((char *) (dst))[2] = (pixel);	\
	    ((short *) (dst))[0] = (pixel);	\
	    break;			\
	case 15:			\
	    ((long *) (dst))[0] = (pixel);	\
	    break;			\
	}
#else
#define WriteBits(dst,pixel,bits) \
	switch (bits) {			\
	case 0:				\
	    break;			\
	case 1:				\
	    ((char *) (dst))[0] = (pixel);	\
	    break;			\
	case 2:				\
	    ((char *) (dst))[1] = (pixel);	\
	    break;			\
	case 3:				\
	    ((short *) (dst))[0] = (pixel);	\
	    break;			\
	case 4:				\
	    ((char *) (dst))[2] = (pixel);	\
	    break;			\
	case 5:				\
	    ((char *) (dst))[0] = (pixel);	\
	    ((char *) (dst))[2] = (pixel);	\
	    break;			\
	case 6:				\
	    ((char *) (dst))[1] = (pixel);	\
	    ((char *) (dst))[2] = (pixel);	\
	    break;			\
	case 7:				\
	    ((short *) (dst))[0] = (pixel);	\
	    ((char *) (dst))[2] = (pixel);	\
	    break;			\
	case 8:				\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 9:				\
	    ((char *) (dst))[0] = (pixel);	\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 10:			\
	    ((char *) (dst))[1] = (pixel);	\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 11:			\
	    ((short *) (dst))[0] = (pixel);	\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 12:			\
	    ((short *) (dst))[1] = (pixel);	\
	    break;			\
	case 13:			\
	    ((char *) (dst))[0] = (pixel);	\
	    ((short *) (dst))[1] = (pixel);	\
	    break;			\
	case 14:			\
	    ((char *) (dst))[1] = (pixel);	\
	    ((short *) (dst))[1] = (pixel);	\
	    break;			\
	case 15:			\
	    ((long *) (dst))[0] = (pixel);	\
	    break;			\
	}
#endif
#endif

void
cfbPolyGlyphBlt8 (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    unsigned char *pglyphBase;	/* start of array of glyphs */
{
    unsigned long   c;
    unsigned long   pixel;
    unsigned long   *dst;
    int		    ewTmp;
    int		    xoff;
    unsigned long   *glyphBits;

    CharInfoPtr		pci;
    FontInfoPtr		pfi = pGC->font->pFI;
    unsigned long	*dstLine;
    unsigned long	*pdstBase;
    int			s;
    int			hTmp;
    int			widthDst;
    int			widthGlyph;
    int			h;
    int			ew;
    int			xG, yG;
    BoxRec		bbox;		/* for clipping */
    int			lshift;
    int			widthDiff;
    int			nw;
    int			w;

    pixel = PFILL (pGC->fgPixel);
    
    /* compute an approximate (but covering) bounding box */
    if ((ppci[0]->metrics.leftSideBearing < 0))
	bbox.x1 = ppci[0]->metrics.leftSideBearing;
    else
	bbox.x1 = 0;
    w = 0;
    for (h = 0; h < nglyph; h++)
	w += ppci[h]->metrics.characterWidth;
    bbox.x2 = w - ppci[nglyph-1]->metrics.characterWidth;
    bbox.x2 += ppci[nglyph-1]->metrics.rightSideBearing;
    bbox.y1 = -pfi->maxbounds.metrics.ascent;
    bbox.y2 = pfi->maxbounds.metrics.descent;

    bbox.x1 += x + pDrawable->x;
    bbox.x2 += x + pDrawable->x;
    bbox.y1 += y + pDrawable->y;
    bbox.y2 += y + pDrawable->y;

    switch ((*pGC->pScreen->RectIn)(
                ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip, &bbox))
    {
      case rgnPART:
	cfbPolyGlyphBlt8Clipped(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
      case rgnOUT:
	return;
    }

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pdstBase = (unsigned long *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	widthDst = (int)
		  (((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	pdstBase = (unsigned long *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	widthDst = (int)(((PixmapPtr)pDrawable)->devKind) >> 2;
    }
    x += pDrawable->x;
    y += pDrawable->y;
    while (nglyph--)
    {
	pci = *ppci++;
	glyphBits = (unsigned long *) (pglyphBase + pci->byteOffset);
	w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	xG = x + pci->metrics.leftSideBearing;
	yG = y - pci->metrics.ascent;
	dstLine = pdstBase + yG * widthDst + (xG >> 2);
	xoff = xG & 0x3;
	ew = (w + xoff + 3) >> 2;
	hTmp = pci->metrics.descent + pci->metrics.ascent;
	dst = dstLine;
	switch (ew) {
	case 0:
	    break;
	case 1:
	    while (hTmp--)
	    {
	    	c = ToXoff (*glyphBits++, xoff);
		WriteBits (dst, pixel, GetFourBits(c));
	    	dst += widthDst;
	    }
	    break;
	case 2:
	    while (hTmp--)
	    {
	    	c = ToXoff (*glyphBits++, xoff);
	    	if (c)
	    	{
		    WriteBits (dst, pixel, GetFourBits(c));
		    NextFourBits(c);
		    WriteBits (dst+1, pixel, GetFourBits(c));
	    	}
	    	dst += widthDst;
	    }
	    break;
	case 3:
	    while (hTmp--)
	    {
	    	c = ToXoff (*glyphBits++, xoff);
	    	if (c)
	    	{
		    WriteBits (dst, pixel, GetFourBits(c));
		    NextFourBits(c);
		    WriteBits (dst+1, pixel, GetFourBits(c));
		    NextFourBits(c);
		    WriteBits (dst+2, pixel, GetFourBits(c));
	    	}
	    	dst += widthDst;
	    }
	    break;
	default:
	    widthDiff = widthDst - ew;
	    dst -= widthDiff;
	    while (hTmp--)
	    {
	    	c = ToXoff (*glyphBits++, xoff);
	    	dst += widthDiff;
	    	if (!c)
	    	{
		    dst += ew;
		    continue;
	    	}
	    	ewTmp = ew;
	    	while (ewTmp--) {
		    WriteBits(dst, pixel, GetFourBits(c))
		    dst++;
		    NextFourBits(c);
	    	}
	    }
	}
	x += pci->metrics.characterWidth;
    }
}

unsigned long	lowBits[32] = {
    0xffffffff, 0x7fffffff, 0x3fffffff, 0x1fffffff,
    0x0fffffff, 0x07ffffff, 0x03ffffff, 0x01ffffff,
    0x00ffffff, 0x007fffff, 0x003fffff, 0x001fffff,
    0x000fffff, 0x0007ffff, 0x0003ffff, 0x0001ffff,
    0x0000ffff, 0x00007fff, 0x00003fff, 0x00001fff,
    0x00000fff, 0x000007ff, 0x000003ff, 0x000001ff,
    0x000000ff, 0x0000007f, 0x0000003f, 0x0000001f,
    0x0000000f, 0x00000007, 0x00000003, 0x00000001,
};

unsigned long highBits[32] = {
    0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8,
    0xfffffff0, 0xffffffe0, 0xffffffc0, 0xffffff80,
    0xffffff00, 0xfffffe00, 0xfffffc00, 0xfffff800,
    0xfffff000, 0xffffe000, 0xffffc000, 0xffff8000,
    0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000,
    0xfff00000, 0xffe00000, 0xffc00000, 0xff800000,
    0xff000000, 0xfe000000, 0xfc000000, 0xf8000000,
    0xf0000000, 0xe0000000, 0xc0000000, 0x80000000,
};

#if (BITMAP_BIT_ORDER == MSBFirst)
#define rightMask   lowBits
#define leftMask    lowBits
#else
#define rightMask   highBits
#define leftMask    highBits
#endif

/*
 * a grungy little routine.  This computes clip masks
 * for partial character blts.  Returns rgnOUT if the
 * entire character is clipped; returns rgnIN if the entire
 * character is unclipped; returns rgnPART if a portion of
 * the character is visible.  Computes clip masks for each
 * longword of the character -- and those with the
 * contents of the glyph to compute the visible bits.
 */

int
clipBlt (pBox, numRects, x, y, w, h, clips)
    BoxPtr	pBox;
    int		numRects;
    int		x, y, w, h;
    unsigned long   *clips;
{
    int	    yBand, yBandBot;
    int	    ch;
    unsigned long	    clip;
    int	    partIN = FALSE, partOUT = FALSE;
    int	    result;

    if (numRects == 0)
	return rgnOUT;
    while (numRects && pBox->y2 <= y)
    {
	--numRects;
	++pBox;
    }
    if (!numRects || pBox->y1 >= y + h)
	return rgnOUT;
    yBand = pBox->y1;
    while (numRects && pBox->y1 == yBand && pBox->x2 <= x)
    {
	--numRects;
	++pBox;
    }
    if (!numRects || pBox->y1 >= y + h)
	return rgnOUT;
    if (numRects &&
	x >= pBox->x1 &&
	x + w <= pBox->x2 &&
	y >= pBox->y1 &&
	y + h <= pBox->y2)
    {
	return rgnIN;
    }
    ch = 0;
    while (ch < h && y + ch < pBox->y1)
    {
	partOUT = TRUE;
	clips[ch++] = 0;
    }
    while (numRects && pBox->y1 < y + h)
    {
	yBand = pBox->y1;
	yBandBot = pBox->y2;
    	while (numRects && pBox->y1 == yBand && pBox->x2 <= x)
    	{
	    --numRects;
	    ++pBox;
    	}
    	if (!numRects)
	    break;
	clip = 0;
    	while (numRects && pBox->y1 == yBand && pBox->x1 < x + w)
    	{
	    if (x < pBox->x1)
		if (pBox->x2 < x + w)
		    clip |= leftMask[pBox->x1 - x] & ~rightMask[pBox->x2 - x];
		else
		    clip |= leftMask[pBox->x1 - x];
 	    else
		if (pBox->x2 < x + w)
		    clip |= ~rightMask[pBox->x2 - x];
		else
		    clip = ~0;
	    --numRects;
	    ++pBox;
    	}
	if (clip != 0)
		partIN = TRUE;
	if (clip != ~0)
		partOUT = TRUE;
	while (ch < h && y + ch < yBandBot)
	    clips[ch++] = clip;
	while (numRects && pBox->y1 == yBand)
	{
	    --numRects;
	    ++pBox;
	}
    }
    while (ch < h)
    {
	partOUT = TRUE;
	clips[ch++] = 0;
    }
    result = rgnOUT;
    if (partIN)
    {
	if (partOUT)
	    result = rgnPART;
	else
	    result = rgnIN;
    }
    return result;
}
#undef leftMask
#undef rightMask

cfbPolyGlyphBlt8Clipped (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    unsigned char *pglyphBase;	/* start of array of glyphs */
{
    unsigned long   c;
    unsigned long   pixel;
    unsigned long   *dst;
    unsigned long   mask;
    int		    ewTmp;
    int		    xoff;
    unsigned long   *rightChar;
    unsigned long   leftMask, rightMask;

    CharInfoPtr		pci;
    FontInfoPtr		pfi = pGC->font->pFI;
    unsigned long	*dstLine;
    unsigned long	*pdstBase;
    unsigned long	*cTmp, *clips;
    int			maxAscent, maxDescent;
    int			s;
    int			hTmp;
    int			widthDst;
    int			widthGlyph;
    int			h;
    int			ew;
    int			xG, yG;
    BoxPtr		pBox;
    int			numRects;
    int			lshift;
    int			widthDiff;
    int			nw;
    int			w;
    RegionPtr		pRegion;
    int			yBand;

    pixel = PFILL (pGC->fgPixel);
    
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pdstBase = (unsigned long *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	widthDst = (int)
		  (((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	pdstBase = (unsigned long *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	widthDst = (int)(((PixmapPtr)pDrawable)->devKind) >> 2;
    }
    x += pDrawable->x;
    y += pDrawable->y;
    maxAscent = pfi->maxbounds.metrics.ascent;
    maxDescent = pfi->maxbounds.metrics.descent;

    pRegion = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;

    pBox = REGION_RECTS(pRegion);
    numRects = REGION_NUM_RECTS (pRegion);
    while (numRects && pBox->y2 <= y - maxAscent)
    {
	++pBox;
	--numRects;
    }
    if (!numRects || pBox->y1 >= y + maxDescent)
	return;
    yBand = pBox->y1;
    while (numRects && pBox->y1 == yBand && pBox->x2 <= x + pfi->minbounds.metrics.leftSideBearing)
    {
	++pBox;
	--numRects;
    }
    if (!numRects)
	return;
    clips = ALLOCATE_LOCAL ((maxAscent + maxDescent) * sizeof (unsigned long));
    while (nglyph--)
    {
	pci = *ppci++;
	rightChar = (unsigned long *) (pglyphBase + pci->byteOffset);
	w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	xG = x + pci->metrics.leftSideBearing;
	yG = y - pci->metrics.ascent;
	dstLine = pdstBase + yG * widthDst + (xG >> 2);
	xoff = xG & 0x3;
	ew = (w + xoff + 3) >> 2;
	hTmp = pci->metrics.descent + pci->metrics.ascent;
	widthDiff = widthDst - ew;
	dst = dstLine - widthDiff;
	switch (clipBlt (pBox, numRects, xG, yG, w, hTmp, clips)) {
	case rgnIN:
	    while (hTmp--)
	    {
	    	c = ToXoff (*rightChar++, xoff);
	    	dst += widthDiff;
	    	if (!c)
	    	{
		    dst += ew;
		    continue;
	    	}
	    	ewTmp = ew;
	    	while (ewTmp--) {
		    WriteBits (dst, pixel, GetFourBits(c));
		    dst++;
		    NextFourBits(c);
	    	}
	    }
	    break;
	case rgnPART:
	    cTmp = clips;
	    while (hTmp--)
	    {
	    	c = ToXoff (*rightChar++ & *cTmp++, xoff);
	    	dst += widthDiff;
	    	if (!c)
	    	{
		    dst += ew;
		    continue;
	    	}
	    	ewTmp = ew;
	    	while (ewTmp--) {
		    WriteBits (dst, pixel, GetFourBits(c));
		    dst++;
		    NextFourBits(c);
	    	}
	    }
	    break;
	}
	x += pci->metrics.characterWidth;
    }
    DEALLOCATE_LOCAL (clips);
}

#endif /* PPW == 4 */
