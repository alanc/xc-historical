/*
 * Poly glyph blt for 8 bit displays.  Accepts
 * an arbitrary font <= 32 bits wide, in Copy mode only.
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
/* $XConsortium: cfbglblt8.c,v 5.10 90/11/30 14:54:26 keith Exp $ */

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
#include	"cfb8bit.h"

#define BOX_OVERLAP(box1, box2, xoffset, yoffset) \
 	((box1)->x1 <= ((int) (box2)->x2 + (xoffset)) && \
 	 ((int) (box2)->x1 + (xoffset)) <= (box1)->x2 && \
	 (box1)->y1 <= ((int) (box2)->y2 + (yoffset)) && \
 	 ((int) (box2)->y1 + (yoffset)) <= (box1)->y2)

#if PPW == 4

#if GLYPHPADBYTES != 4
#define USE_LEFTBITS
#endif

#ifdef USE_LEFTBITS
typedef	unsigned char	*glyphPointer;
extern long endtab[];

#define GlyphBits(bits,width,dst)	getleftbits(bits,width,dst); \
					(dst) &= widthMask; \
					(bits) += widthGlyph;
#define GlyphBitsS(bits,width,dst,off)	GlyphBits(bits,width,dst); \
					dst = BitRight (dst, off);
#else
typedef unsigned long	*glyphPointer;

#define GlyphBits(bits,width,dst)	dst = *bits++;
#define GlyphBitsS(bits,width,dst,off)	dst = BitRight(*bits++, off);
#endif

#ifdef GLYPHROP
#define cfbPolyGlyphBlt8	cfbPolyGlyphRop8
#define cfbPolyGlyphBlt8Clipped	cfbPolyGlyphRop8Clipped

#undef WriteFourBits
#define WriteFourBits(dst,pixel,bits)	RRopFourBits(dst,bits)

#endif

static void cfbPolyGlyphBlt8Clipped();

#if defined(HAS_STIPPLE_CODE) && !defined(GLYPHROP) && !defined(USE_LEFTBITS)
#define USE_STIPPLE_CODE
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
    register unsigned long  c;
#ifndef GLYPHROP
    register unsigned long  pixel;
#endif
    register unsigned long  *dst;
    register glyphPointer   glyphBits;
    register int	    xoff;
    register int	    ewTmp;

    CharInfoPtr		pci;
    FontInfoPtr		pfi = pGC->font->pFI;
    unsigned long	*dstLine;
    unsigned long	*pdstBase;
    int			hTmp;
    int			bwidthDst;
    int			widthDst;
    int			h;
    int			ew;
    BoxRec		bbox;		/* for clipping */
    int			widthDiff;
    int			w;
    RegionPtr		clip;
    BoxPtr		extents;
#ifdef USE_LEFTBITS
    int			widthGlyph;
    unsigned long	widthMask;
#endif
#ifdef USE_STIPPLE_CODE
    void		(*stipple)();
    extern void		stipplestack (), stipplestackte ();

    stipple = stipplestack;
    if (pfi->inkMetrics)
	stipple = stipplestackte;
#endif
    
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

    clip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
    extents = &clip->extents;

    x += pDrawable->x;
    y += pDrawable->y;

    /* check to make sure some of the text appears on the screen */
    if (!BOX_OVERLAP (extents, &bbox, x, y))
	return;

    bbox.x1 += x;
    bbox.x2 += x;
    bbox.y1 += y;
    bbox.y2 += y;

    switch ((*pGC->pScreen->RectIn)(clip, &bbox))
    {
      case rgnPART:
	cfbPolyGlyphBlt8Clipped(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
      case rgnOUT:
	return;
    }

#ifdef GLYPHROP
    cfb8CheckStipple (pGC->alu, pGC->fgPixel, pGC->planemask);
#else
    pixel = ((cfbPrivGCPtr) pGC->devPrivates[cfbGCPrivateIndex].ptr)->xor;
#endif

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pdstBase = (unsigned long *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	bwidthDst = (int)
		  (((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind);
    }
    else
    {
	pdstBase = (unsigned long *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	bwidthDst = (int)(((PixmapPtr)pDrawable)->devKind);
    }
    widthDst = bwidthDst >> 2;
    while (nglyph--)
    {
	pci = *ppci++;
	glyphBits = (glyphPointer) (pglyphBase + pci->byteOffset);
	w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	xoff = x + pci->metrics.leftSideBearing;
	dstLine = pdstBase +
	          (y - pci->metrics.ascent) * widthDst + (xoff >> 2);
	xoff &= 0x3;
	hTmp = pci->metrics.descent + pci->metrics.ascent;
#ifdef USE_LEFTBITS
	widthGlyph = GLYPHWIDTHBYTESPADDED (pci);
	widthMask = endtab[w];
#endif
#ifdef USE_STIPPLE_CODE
	(*stipple)(dstLine,glyphBits,pixel,bwidthDst,hTmp,xoff);
#else
	ew = (w + xoff + 3) >> 2;
	dst = dstLine;
	switch (ew) {
	case 0:
	    break;
	case 1:
	    while (hTmp--)
	    {
		GlyphBitsS(glyphBits, w, c, xoff)
		WriteFourBits (dst, pixel, GetFourBits(c));
	    	dst += widthDst;
	    }
	    break;
	case 2:
	    while (hTmp--)
	    {
		GlyphBitsS(glyphBits, w, c, xoff)
	    	if (c)
	    	{
		    WriteFourBits (dst, pixel, GetFourBits(c));
		    NextFourBits(c);
		    WriteFourBits (dst+1, pixel, GetFourBits(c));
	    	}
	    	dst += widthDst;
	    }
	    break;
	case 3:
	    while (hTmp--)
	    {
		GlyphBitsS(glyphBits, w, c, xoff)
	    	if (c)
	    	{
		    WriteFourBits (dst, pixel, GetFourBits(c));
		    NextFourBits(c);
		    WriteFourBits (dst+1, pixel, GetFourBits(c));
		    NextFourBits(c);
		    WriteFourBits (dst+2, pixel, GetFourBits(c));
	    	}
	    	dst += widthDst;
	    }
	    break;
	case 9:
	    widthDiff = widthDst - ew;
	    dst -= widthDiff;
	    while (hTmp--)
	    {
#if defined(__GNUC__) && defined(mc68020)
		volatile unsigned long c1; /* XXX 1.35 is really stupid */
#else
		unsigned long c1;
#endif

		GlyphBits(glyphBits, w, c)
	    	dst += widthDiff;
	    	if (!c)
	    	{
		    dst += ew;
		    continue;
	    	}
		c1 = BitLeft (c, 32 - xoff);
	    	c = BitRight (c, xoff);
	    	ewTmp = ew - 1;
	    	while (ewTmp--) {
		    WriteFourBits(dst, pixel, GetFourBits(c))
		    dst++;
		    NextFourBits(c);
	    	}
		WriteFourBits(dst, pixel, GetFourBits(c1));
		dst++;
	    }
	    break;
	default:
	    widthDiff = widthDst - ew;
	    dst -= widthDiff;
	    while (hTmp--)
	    {
		GlyphBitsS(glyphBits, w, c, xoff)
	    	dst += widthDiff;
	    	if (!c)
	    	{
		    dst += ew;
		    continue;
	    	}
	    	ewTmp = ew;
	    	while (ewTmp--) {
		    WriteFourBits(dst, pixel, GetFourBits(c))
		    dst++;
		    NextFourBits(c);
	    	}
	    }
	}
#endif
	x += pci->metrics.characterWidth;
    }
}

static void
cfbPolyGlyphBlt8Clipped (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    unsigned char *pglyphBase;	/* start of array of glyphs */
{
    register unsigned long  c;
#ifndef GLYPHROP
    register unsigned long  pixel;
#endif
    register unsigned long  *dst;
    register glyphPointer   glyphBits;
    register int	    xoff;
    register int	    ewTmp;
    unsigned long	    c1;

    CharInfoPtr		pci;
    FontInfoPtr		pfi = pGC->font->pFI;
    unsigned long	*dstLine;
    unsigned long	*pdstBase;
    unsigned long	*cTmp, *clips;
    int			maxAscent, maxDescent;
    int			hTmp;
    int			widthDst;
    int			bwidthDst;
    int			ew;
    int			xG, yG;
    BoxPtr		pBox;
    int			numRects;
    int			widthDiff;
    int			w;
    RegionPtr		pRegion;
    int			yBand;
#ifdef USE_LEFTBITS
    int			widthGlyph;
    unsigned long	widthMask;
#endif

#ifdef GLYPHROP
    cfb8CheckStipple (pGC->alu, pGC->fgPixel, pGC->planemask);
#else
    pixel = ((cfbPrivGCPtr) pGC->devPrivates[cfbGCPrivateIndex].ptr)->xor;
#endif
    
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pdstBase = (unsigned long *)
		(((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	bwidthDst = (int)
		  (((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind);
    }
    else
    {
	pdstBase = (unsigned long *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	bwidthDst = (int)(((PixmapPtr)pDrawable)->devKind);
    }
    widthDst = bwidthDst >> 2;
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
    clips = (unsigned long *)ALLOCATE_LOCAL ((maxAscent + maxDescent) *
					     sizeof (unsigned long));
    while (nglyph--)
    {
	pci = *ppci++;
	glyphBits = (glyphPointer) (pglyphBase + pci->byteOffset);
	w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	xG = x + pci->metrics.leftSideBearing;
	yG = y - pci->metrics.ascent;
	dstLine = pdstBase + yG * widthDst + (xG >> 2);
	hTmp = pci->metrics.descent + pci->metrics.ascent;
	xoff = xG & 0x3;
#ifndef USE_STIPPLE_CODE
	ew = (w + xoff + 3) >> 2;
	widthDiff = widthDst - ew;
	dst = dstLine - widthDiff;
#endif
#ifdef USE_LEFTBITS
	widthGlyph = GLYPHWIDTHBYTESPADDED (pci);
	widthMask = endtab[w];
#endif
	switch (cfb8ComputeClipMasks32 (pBox, numRects, xG, yG, w, hTmp, clips))
 	{
#ifdef USE_STIPPLE_CODE
	case rgnPART:
	    {
		int h;

		h = hTmp;
		while (h--)
		    clips[h] = clips[h] & glyphBits[h];
	    }
	    glyphBits = clips;
	    /* fall through */
	case rgnIN:
	    stipplestackte(dstLine,glyphBits,pixel,bwidthDst,hTmp,xoff);
	    break;
#else
	case rgnIN:
	    if (ew <= 8)
	    {
		while (hTmp--)
		{
		    GlyphBitsS(glyphBits, w, c, xoff)
	    	    dst += widthDiff;
	    	    if (!c)
	    	    {
		    	dst += ew;
		    	continue;
	    	    }
	    	    ewTmp = ew;
	    	    while (ewTmp--) {
		    	WriteFourBits (dst, pixel, GetFourBits(c));
		    	dst++;
		    	NextFourBits(c);
	    	    }
		}
	    }
	    else
	    {
		while (hTmp--)
		{
		    GlyphBits(glyphBits, w, c)
	    	    dst += widthDiff;
	    	    if (!c)
	    	    {
		    	dst += ew;
		    	continue;
	    	    }
		    c1 = BitLeft (c, 32 - xoff);
	    	    c = BitRight (c, xoff);
	    	    ewTmp = ew-1;
	    	    while (ewTmp--) {
		    	WriteFourBits (dst, pixel, GetFourBits(c));
		    	dst++;
		    	NextFourBits(c);
	    	    }
		    WriteFourBits(dst, pixel, GetFourBits(c1));
		    dst++;
		}
	    }
	    break;
	case rgnPART:
	    cTmp = clips;
	    if (ew <= 8)
	    {
	    	while (hTmp--)
	    	{
		    GlyphBits(glyphBits, w, c)
		    c = BitRight (c & *cTmp++, xoff);
		    dst += widthDiff;
		    if (!c)
		    {
		    	dst += ew;
		    	continue;
		    }
		    ewTmp = ew;
		    while (ewTmp--) {
			c1 = GetFourBits(c);
			if (c1)
			{
			    WriteFourBits (dst, pixel, c1);
			}
		    	dst++;
		    	NextFourBits(c);
		    }
	    	}
	    }
	    else
	    {
	    	while (hTmp--)
	    	{
		    GlyphBits(glyphBits, w, c)
		    c &= *cTmp++;
		    dst += widthDiff;
		    if (!c)
		    {
		    	dst += ew;
		    	continue;
		    }
		    c1 = BitLeft (c, 32 - xoff);
		    c = BitRight (c, xoff);
		    ewTmp = ew - 1;
		    while (ewTmp--) {
			if (GetFourBits(c))
			{
			    WriteFourBits (dst, pixel, GetFourBits(c));
			}
		    	dst++;
		    	NextFourBits(c);
		    }
		    if (GetFourBits(c1))
		    {
		    	WriteFourBits (dst, pixel, GetFourBits(c1));
		    }
		    dst++;
	    	}
	    }
	    break;
#endif
	}
	x += pci->metrics.characterWidth;
    }
    DEALLOCATE_LOCAL (clips);
}

#endif /* PPW == 4 */
