/* $XConsortium: charinfo.c,v 1.2 92/03/17 20:31:29 eswu Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Defines the routines GetExtents and GetBitmaps,
 * called from routines in fontinfo.c.
 * Was once on the other side of the font library interface as util/fsfuncs.c.
 */

#include <X11/Xos.h>
#include "misc.h"
#include "fontstruct.h"
#include "clientstr.h"
#define FSMD_H
#include "FSproto.h"

#define GLWIDTHBYTESPADDED(bits,nbytes) \
	((nbytes) == 1 ? (((bits)+7)>>3)        /* pad to 1 byte */ \
	:(nbytes) == 2 ? ((((bits)+15)>>3)&~1)  /* pad to 2 bytes */ \
	:(nbytes) == 4 ? ((((bits)+31)>>3)&~3)  /* pad to 4 bytes */ \
	:(nbytes) == 8 ? ((((bits)+63)>>3)&~7)  /* pad to 8 bytes */ \
	: 0)

#define GLYPH_SIZE(ch, nbytes)          \
	GLWIDTHBYTESPADDED((ch)->metrics.rightSideBearing - \
			(ch)->metrics.leftSideBearing, (nbytes))

#define n2dChars(pfi)   (((pfi)->lastRow - (pfi)->firstRow + 1) * \
                         ((pfi)->lastCol - (pfi)->firstCol + 1))

static CharInfoRec  junkDefault;

static int
getCharInfos (pfont, num_ranges, range, ink_metrics, nump, retp)
    FontPtr	pfont;
    int		num_ranges;
    fsRange	*range;
    Bool	ink_metrics;
    int		*nump;		/* return */
    CharInfoPtr	**retp;		/* return */
{
    CharInfoPtr	*xchars, *xci;
    int		nchars;
    FontInfoPtr pinfo = &pfont->info;
    int		r, c;
    unsigned char   ch[2];
    int         firstCol = pinfo->firstCol;
    int         firstRow = pinfo->firstRow;
    int         lastRow = pinfo->lastRow;
    int         lastCol = pinfo->lastCol;
    int		minCol, maxCol;
    int         num_cols = lastCol - firstCol + 1;
    fsRange	local_range, *rp;
    int		i;
    FontEncoding    encoding;
    int		err;
    unsigned long   glyphCount;
    unsigned short  defaultCh;
    CharInfoPtr	    defaultPtr;
    int (*metrics_func) ();
    
    /*
     * compute nchars
     */
    if (num_ranges == 0) {
	if (lastRow)
	    nchars = n2dChars(pinfo);
	else
	    nchars = lastCol - firstCol + 1;
	local_range.min_char.low = firstCol;
	local_range.min_char.high = firstRow;
	local_range.max_char.low = lastCol;
	local_range.max_char.high = lastRow;
	range = &local_range;
	num_ranges = 1;
    } else {
	nchars = 0;
	for (i = 0, rp = range; i < num_ranges; i++, rp++) {
	    if (rp->min_char.high > rp->max_char.high)
		return BadCharRange;
	    if (rp->min_char.high == rp->max_char.high)
	    {
		if (rp->min_char.low > rp->max_char.low)
		    return BadCharRange;
		nchars += rp->max_char.low - rp->min_char.low + 1;
	    }
	    else
	    {
		nchars += lastRow - rp->min_char.low + 1;
		nchars += (rp->max_char.high - rp->min_char.high - 1) * num_cols;
		nchars += rp->max_char.low - firstRow + 1;
	    }
	}
    }

    xchars = (CharInfoPtr *) fsalloc (sizeof (CharInfoPtr) * nchars);
    if (!xchars)
	return AllocError;

    if (ink_metrics)
	metrics_func = pfont->get_metrics;
    else
	metrics_func = pfont->get_glyphs;

    xci = xchars;
    encoding = Linear16Bit;
    if (lastRow)
	encoding = TwoD16Bit;
    defaultCh = pinfo->defaultCh;
    ch[0] = defaultCh >> 8;
    ch[1] = defaultCh & 0xff;
    /* get the default character */
    (*metrics_func) (pfont, 1, ch, encoding,
			  &glyphCount, &defaultPtr);
    if (glyphCount != 1)
	defaultPtr = 0;
    
    /* for each range, get each character individually, undoing the
     default character substitution so we get zero metrics for
     non-existent characters. */
    for (i = 0, rp = range; i < num_ranges; i++, rp++) {
	for (r = rp->min_char.high; r <= rp->max_char.high; r++)
	{
	    minCol = firstCol;
	    if (r == rp->min_char.high)
		minCol = rp->min_char.low;
	    maxCol = lastCol;
	    if (r == rp->max_char.high)
		maxCol = rp->max_char.low;
	    for (c = minCol; c <= maxCol; c++) {
		ch[0] = r;
		ch[1] = c;
		err = (*metrics_func) (pfont, 1, ch, encoding,
					    &glyphCount, xci);
		if (err != Successful)
		{
		    fsfree (xchars);
		    return err;
		}
		if (glyphCount != 1 || 
		    *xci == defaultPtr && defaultCh != ((r<<8)+c))
		    *xci = &junkDefault;
		xci++;
	    }
	}
    }
    *retp = xchars;
    *nump = nchars;
    return Successful;
}

int
GetExtents(client, pfont, flags, num_ranges, range, num_extents, data)
    ClientPtr     client;
    FontPtr     pfont;
    Mask        flags;
    unsigned long num_ranges;
    fsRange    *range;
    unsigned long *num_extents;	/* return */
    fsCharInfo **data;		/* return */
{
    unsigned long size;
    fsCharInfo *ci,
    *pci;
    fsRange    *rp;
    CharInfoPtr	*xchars, *xcharsFree, xci;
    int		nchars;
    int		err;
    
    if (flags & LoadAll)
	num_ranges = 0;
    err = getCharInfos (pfont, num_ranges, range,
			client->major_version > 1 ? TRUE : FALSE,
			&nchars, &xchars);
    if (err != Successful)
	return err;
    
    size = sizeof(fsCharInfo) * nchars;
    pci = ci = (fsCharInfo *) fsalloc(size);
    if (!ci) {
	fsfree (xchars);
	return AllocError;
    }
    
    *num_extents = nchars;
    xcharsFree = xchars;
    
    while (nchars--) {
	xci = *xchars++;
	pci->ascent = xci->metrics.ascent;
	pci->descent = xci->metrics.descent;
	pci->left = xci->metrics.leftSideBearing;
	pci->right = xci->metrics.rightSideBearing;
	pci->width = xci->metrics.characterWidth;
	pci->attributes = 0;
	pci++;
    }
    
    fsfree (xcharsFree);
    
    *data = ci;
    
    return Successful;
}

static int
packGlyphs (client, pfont, format, flags, num_ranges, range, tsize, num_glyphs,
		offsets, data, freeData)
    ClientPtr   client;
    FontPtr     pfont;
    int         format;
    Mask        flags;
    unsigned long num_ranges;
    fsRange    *range;
    int        *tsize;
    unsigned long *num_glyphs;
    fsOffset  **offsets;
    pointer    *data;
    int		*freeData;
{
    unsigned long start,
    end;
    int         i;
    fsOffset	*lengths, *l;
    unsigned long size = 0;
    pointer     gdata,
    gd;
    long        ch;
    int         bitorder, byteorder, scanlinepad, scanlineunit, mappad;
    int		height, dstbpr, charsize;
    int		dst_off, src_off;
    Bool	contiguous, reformat;
    fsRange    *rp = range;
    int		nchars;
    int         src_glyph_pad = pfont->glyph;
    int         src_bit_order = pfont->bit;
    int         src_byte_order = pfont->byte;
    int         err;
    int		max_ascent, max_descent;
    int		min_left, max_right;
    int		srcbpr;
    int		lshift = 0, rshift = 0, dst_left_bytes = 0, src_left_bytes = 0;
    unsigned char   *src;
    unsigned char   *dst;
    unsigned char   bits1, bits2;
    int		    width;
    int		    src_extra;
    int		    dst_extra;
    int		    r, w;
    fsRange	allRange;
    CharInfoPtr	*bitChars, *bitCharsFree, bitc;
    CharInfoPtr	*inkChars, *inkCharsFree = 0, inkc;
    FontInfoPtr	pinfo = &pfont->info;
    xCharInfo	*bitm, *inkm;
    
    err = CheckFSFormat(format, (fsBitmapFormatMask) ~ 0,
			&bitorder, &byteorder, &scanlineunit, &scanlinepad, &mappad);
    
    if (err != Successful)
	return err;
    
    if (flags & LoadAll)
	num_ranges = 0;
    
    err = getCharInfos (pfont, num_ranges, range, FALSE, &nchars, &bitCharsFree);
    
    if (err != Successful)
	return err;
    
    /* compute dstbpr for padded out fonts */
    reformat = bitorder != src_bit_order || byteorder != src_byte_order;

    /* we need the ink metrics when shrink-wrapping a TE font (sigh) */
    if (mappad != BitmapFormatImageRectMax && pinfo->inkMetrics)
    {
	err = getCharInfos (pfont, num_ranges, range, TRUE, &nchars, &inkCharsFree);
	if (err != Successful)
	{
	    fsfree (bitCharsFree);
	    return err;
	}
	reformat = TRUE;
    }

    /* get space for glyph offsets */
    lengths = (fsOffset *) fsalloc(sizeof(fsOffset) * nchars);
    if (!lengths) {
	fsfree (bitCharsFree);
	return AllocError;
    }
    
    switch (mappad)
    {
    case BitmapFormatImageRectMax:
	max_ascent = FONT_MAX_ASCENT(pinfo);
	max_descent = FONT_MAX_DESCENT(pinfo);
	height = max_ascent + max_descent;
	/* do font ascent and font descent match bitmap bounds ? */
	if (height != pinfo->minbounds.ascent + pinfo->minbounds.descent)
	    reformat = TRUE;
	/* fall through */
    case BitmapFormatImageRectMaxWidth:
	min_left = FONT_MIN_LEFT(pinfo);
	max_right = FONT_MAX_RIGHT(pinfo);
	if (min_left != pinfo->maxbounds.leftSideBearing)
	    reformat = TRUE;
	if (max_right != pinfo->maxbounds.rightSideBearing)
	    reformat = TRUE;
	dstbpr = GLWIDTHBYTESPADDED(max_right - min_left, scanlinepad);
	break;
    case BitmapFormatImageRectMin:
	break;
    }
    if (mappad == BitmapFormatImageRectMax)
	charsize = dstbpr * height;
    size = 0;
    gdata = 0;
    contiguous = TRUE;
    l = lengths;
    inkChars = inkCharsFree;
    bitChars = bitCharsFree;
    for (i = 0; i < nchars; i++)
    {
    	inkc = bitc = *bitChars++;
	/* when ink metrics != bitmap metrics, use ink metrics */
	if (inkChars)
	    inkc = *inkChars++;
    	l->position = size;
    	if (bitc && bitc->bits) {
	    if (!gdata)
		gdata = (pointer) bitc->bits;
	    if ((char *) gdata + size != bitc->bits)
		contiguous = FALSE;
	    if (mappad == BitmapFormatImageRectMin)
		dstbpr = GLYPH_SIZE(inkc, scanlinepad);
	    if (mappad != BitmapFormatImageRectMax)
	    {
		height = inkc->metrics.ascent + inkc->metrics.descent;
		charsize = height * dstbpr;
	    }
	    l->length = charsize;
	    size += charsize;
	}
	else
	    l->length = 0;
	l++;
    }
    if (contiguous && !reformat)
    {
	*num_glyphs = nchars;
	*freeData = FALSE;
	*data = gdata;
	*tsize = size;
	*offsets = lengths;
	fsfree (bitCharsFree);
	return Successful;
    }
    if (size)
    {
	gdata = (pointer) fsalloc(size);
	if (!gdata) {
	    fsfree (bitCharsFree);
	    fsfree (lengths);
	    return AllocError;
	}
	bzero ((char *) gdata, size);
    }
    else
	gdata = NULL;
    
    *freeData = TRUE;
    l = lengths;
    gd = gdata;
    
    /* finally do the work */
    bitChars = bitCharsFree;
    inkChars = inkCharsFree;
    for (i = 0; i < nchars; i++, l++) 
    {
	inkc = bitc = *bitChars++;
	if (inkChars)
	    inkc = *inkChars++;

	/* ignore missing chars */
	if (l->length == 0)
	    continue;
	
	bitm = &bitc->metrics;
	inkm = &inkc->metrics;

	/* start address for the destination of bits for this char */

	dst = gd;

	/* adjust destination and calculate shift offsets */
	switch (mappad) {
	case BitmapFormatImageRectMax:
	    /* leave the first padded rows blank */
	    dst += dstbpr * (max_ascent - inkm->ascent);
	    /* fall thru */
	case BitmapFormatImageRectMaxWidth:
	    dst_off = inkm->leftSideBearing - min_left;
	    break;
	case BitmapFormatImageRectMin:
	    dst_off = 0;
	    dstbpr = GLYPH_SIZE(inkc, scanlinepad);
	    break;
	}

	srcbpr = GLYPH_SIZE (bitc, src_glyph_pad);
	src = (unsigned char *) bitc->bits;

	/* adjust source */
	src_off = 0;
	if (inkm != bitm)
	{
	    src += (bitm->ascent - inkm->ascent) * srcbpr;
	    src_off = inkm->leftSideBearing - bitm->leftSideBearing;
	}

	dst_left_bytes = dst_off >> 3;
	dst_off &= 7;
	src_left_bytes = src_off >> 3;
	src_off &= 7;

	/* minimum of source/dest bytes per row */
	width = srcbpr;
	if (srcbpr > dstbpr)
	    width = dstbpr;
	/* extra bytes in source and dest for padding */
	src_extra = srcbpr - width - src_left_bytes;
	dst_extra = dstbpr - width - dst_left_bytes;
	
#define MSBBitLeft(b,c)	((b) << (c))
#define MSBBitRight(b,c)	((b) >> (c))
#define LSBBitLeft(b,c)	((b) >> (c))
#define LSBBitRight(b,c)	((b) << (c))

	if (dst_off == src_off)
	{
	    if (srcbpr == dstbpr && src_left_bytes == dst_left_bytes)
	    {
		r = (bitm->ascent + bitm->descent) * width;
		bcopy (src, dst, r);
		dst += r;
	    }
	    else
	    {
		for (r = bitm->ascent + bitm->descent; r; r--)
		{
		    dst += dst_left_bytes;
		    src += src_left_bytes;
		    for (w = width; w; w--)
			*dst++ = *src++;
		    dst += dst_extra;
		    src += src_extra;
		}
	    }
	}
	else
	{
	    if (dst_off > src_off)
	    {
	    	rshift = dst_off - src_off;
	    	lshift = 8 - rshift;
	    }
	    else
	    {
	    	lshift = src_off - dst_off;
	    	rshift = 8 - lshift;
		/* run the loop one fewer time if necessary */
		if (src_extra <= dst_extra)
		{
		    dst_extra++;
		    width--;
		}
		else
		    src_extra--;
	    }
	    
	    for (r = bitm->ascent + bitm->descent; r; r--)
	    {
		dst += dst_left_bytes;
		src += src_left_bytes;
		bits2 = 0;
		/* fetch first part of source when necessary */
		if (dst_off < src_off)
		    bits2 = *src++;
		/*
 		 * XXX I bet this does not work when
		 * src_bit_order != src_byte_order && scanlineunit > 1
		 */
		for (w = width; w; w--)
		{
		    bits1 = *src++;
		    if (src_bit_order == MSBFirst)
		    {
			*dst++ = MSBBitRight(bits1, rshift) |
				 MSBBitLeft (bits2, lshift);
		    }
		    else
		    {
			*dst++ = LSBBitRight(bits1, rshift) |
				 LSBBitLeft (bits2, lshift);
		    }
		    bits2 = bits1;
		}
		/* get the last few bits if we have a place to store them */
		if (dst_extra > 0)
		{
		    if (src_bit_order == MSBFirst)
			*dst = MSBBitLeft (bits2, lshift);
		    else
			*dst = LSBBitLeft (bits2, lshift);
		}
		dst += dst_extra;
		src += src_extra;
	    }
	}
	/* skip the amount we just filled in */
	gd += l->length;
    }
    
    
    /* now do the bit, byte, word swapping */
    if (bitorder != src_bit_order)
	BitOrderInvert(gdata, size);
    if (byteorder != src_byte_order) 
    {
	if (scanlineunit == 2)
	    TwoByteSwap(gdata, size);
	else if (scanlineunit == 4)
	    FourByteSwap(gdata, size);
    }
    fsfree (bitCharsFree);
    *num_glyphs = nchars;
    *data = gdata;
    *tsize = size;
    *offsets = lengths;
    
    return Successful;
}

/* ARGSUSED */
int
GetBitmaps(client, pfont, format, flags, num_ranges, range,
		 size, num_glyphs, offsets, data, freeData)
    ClientPtr     client;
    FontPtr     pfont;
    fsBitmapFormat format;
    Mask        flags;
    unsigned long num_ranges;
    fsRange    *range;
    int        *size;
    unsigned long *num_glyphs;
    fsOffset  **offsets;
    pointer    *data;
    int		*freeData;
{
    assert(pfont);

    *size = 0;
    *data = (pointer) 0;
    return packGlyphs (client, pfont, format, flags,
			      num_ranges, range, size, num_glyphs,
			      offsets, data, freeData);
}
