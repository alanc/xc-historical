/* $XConsortium: spfont.c,v 1.5 91/05/29 15:28:16 keith Exp $ */
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
 *
 * Author: Dave Lemke, Network Computing Devices Inc
 *
 */

/*
 * Speedo font loading
 */

#include	"FSproto.h"
#include	"spint.h"
#include	<server/include/servermd.h>

#ifndef DEFAULT_BIT_ORDER

#ifdef BITMAP_BIT_ORDER
#define DEFAULT_BIT_ORDER BITMAP_BIT_ORDER
#else
#define DEFAULT_BIT_ORDER UNKNOWN_BIT_ORDER
#endif

#endif

extern void SpeedoCloseFont();
static int  get_sp_glyphs(),
            get_sp_bitmaps(),
            get_sp_metrics(),
            get_sp_extents();
static int load_sp_font();

static CharInfoRec junkDefault;

static void
CopyCharInfo(ci, dst)
    CharInfoPtr ci;
    fsCharInfo *dst;
{
    xCharInfo  *src = &ci->metrics;

    dst->ascent = src->ascent;
    dst->descent = src->descent;
    dst->left = src->leftSideBearing;
    dst->right = src->rightSideBearing;
    dst->width = src->characterWidth;
    dst->attributes = 0;
}

/* XXX -- may have to add in some work for setting default metrics values */

/* ARGSUSED */
static int
get_sp_extents(client, pfont, flags, num_ranges, range, num_extents, data)
    pointer     client;
    FontPtr     pfont;
    Mask        flags;
    unsigned long num_ranges;
    fsRange    *range;
    unsigned long *num_extents;
    fsCharInfo **data;
{
    int         start,
                end,
                i,
                j;
    unsigned long size;
    fsCharInfo *ci,
               *pci;
    fsRange    *rp;
    FontInfoPtr pinfo;
    SpeedoFontPtr spf;
    SpeedoMasterFontPtr master;
    CharInfoRec src;
    int		firstChar;

    assert(pfont);
    pinfo = &pfont->info;
    spf = (SpeedoFontPtr) pfont->fontPrivate;
    master = spf->master;

    firstChar = master->first_char_id;
    if (flags & LoadAll) {
	start = master->first_char_id;
	end = master->max_id;

	*num_extents = end - start + 1;
	size = sizeof(fsCharInfo) * (*num_extents);
	pci = ci = (fsCharInfo *) xalloc(size);
	if (!ci)
	    return AllocError;

	/* copy all the extents */
	for (i = start; i <= end; i++) {
	    src = spf->encoding[i - firstChar];
	    CopyCharInfo(&src, pci);
	    pci++;
	}

	/* make sure it didn't go off the end */
	assert(pci == (fsCharInfo *) ((char *) ci + size));
	assert(pci == (ci + (end - start + 1)));

	*data = ci;
	return Successful;
    }
    /* normal case */
    /* figure out how big everything has to be */
    *num_extents = 0;
    for (i = 0, rp = range; i < num_ranges; i++, rp++) {
	start = (rp->min_char.high << 8) + rp->min_char.low;
	end = (rp->max_char.high << 8) + rp->max_char.low;

	/* range check */
	if (end < start ||
		(end > (pinfo->lastRow << 8) + pinfo->lastCol)
		|| (end < (pinfo->firstRow << 8) + pinfo->firstCol)
		|| (start > (pinfo->lastRow << 8) + pinfo->lastCol)
		|| (start < (pinfo->firstRow << 8) + pinfo->firstCol))
	    return BadCharRange;

	*num_extents += end - start + 1;
    }

    size = sizeof(fsCharInfo) * (*num_extents);
    pci = ci = (fsCharInfo *) xalloc(size);
    if (!ci)
	return AllocError;

    /* copy all the extents */
    for (i = 0, rp = range; i < num_ranges; i++, rp++) {
	start = (rp->min_char.high << 8) + rp->min_char.low;
	end = (rp->max_char.high << 8) + rp->max_char.low;

	/* copy all the extents */
	for (j = start; j <= end; j++) {
	    src = spf->encoding[j - firstChar];
	    CopyCharInfo(&src, pci);
	    pci++;
	}

	/* make sure it didn't go off the end */
	assert(pci == (fsCharInfo *) ((char *) ci + size));
	assert(pci == (ci + (end - start + 1)));
    }

    *data = ci;

    return Successful;
}

/*
 * packs up the glyphs as requested by the format
 */

static int
pack_sp_glyphs(pfont, format, flags, num_ranges, range, tsize, num_glyphs,
	       offsets, data)
    FontPtr     pfont;
    int         format;
    Mask        flags;
    unsigned long num_ranges;
    fsRange    *range;
    unsigned long *tsize;
    unsigned long *num_glyphs;
    fsOffset  **offsets;
    pointer    *data;
{
    unsigned long start,
                end;
    int         i;
    fsOffset   *lengths = (fsOffset *) 0,
               *l;
    unsigned long size = 0;
    pointer     gdata,
                gd;
    unsigned long ch;
    int         bitorder,
                byteorder,
                scanlinepad,
                scanlineunit,
                mappad;
    int         bpr,
                skiprows = 0;
    fsRange    *rp;
    FontInfoPtr pinfo = &pfont->info;
    SpeedoFontPtr spf = (SpeedoFontPtr) pfont->fontPrivate;
    SpeedoMasterFontPtr master = spf->master;
    int         min_left = pinfo->minbounds.leftSideBearing;
    int         err;
    int         src_glyph_pad;
    int         src_bit_order;
    int         src_byte_order;
    int		firstChar;

    err = CheckFSFormat(format, (fsBitmapFormatMask) ~ 0,
		&bitorder, &byteorder, &scanlineunit, &scanlinepad, &mappad);
    if (err != Successful)
	return err;
    (void) CheckFSFormat(pfont->format, (fsBitmapFormatMask) ~ 0,
		&src_bit_order, &src_byte_order, &err, &src_glyph_pad, &err);

    if (!spf->pDefault)
	spf->pDefault = &junkDefault;

    firstChar = master->first_char_id;
    /* special case for all glyphs first */
    if (flags & LoadAll) {
	start = master->first_char_id;
	end = master->max_id;
	*num_glyphs = end - start + 1;
	size = compute_sp_data_size(pfont, mappad, scanlinepad, start, end);;
	num_ranges = 1;
    } else {
	*num_glyphs = 0;
	for (i = 0, rp = range; i < num_ranges; i++, rp++) {
	    start = (rp->min_char.high << 8) + rp->min_char.low;
	    end = (rp->max_char.high << 8) + rp->max_char.low;

	    /* range check */
	    if (end < start ||
		    (end > (pinfo->lastRow << 8) + pinfo->lastCol)
		    || (end < (pinfo->firstRow << 8) + pinfo->firstCol)
		    || (start > (pinfo->lastRow << 8) + pinfo->lastCol)
		    || (start < (pinfo->firstRow << 8) + pinfo->firstCol))
		return BadCharRange;

	    *num_glyphs += end - start + 1;
	    size += compute_sp_data_size(pfont, mappad, scanlinepad, start, end);
	}
    }

    gd = gdata = (pointer) xalloc(size);
    if (!gdata)
	return AllocError;
    bzero((char *) gdata, size);

    /* get space for glyph offsets */
    l = lengths = (fsOffset *) xalloc(sizeof(fsOffset) *
				      *num_glyphs);
    if (!lengths) {
	xfree((char *) gdata);
	return AllocError;
    }
    /* finally do the work */
    for (i = 0, rp = range; i < num_ranges; i++, rp++) {
	/*
	 * compute start & end.  if all_glyphs is set, we still have them
	 * laying around
	 */
	if (!(flags & LoadAll)) {
	    start = (rp->min_char.high << 8) + rp->min_char.low;
	    end = (rp->max_char.high << 8) + rp->max_char.low;
	}
	for (ch = start; ch <= end; ch++) {
	    CharInfoPtr ci = &spf->encoding[ch - firstChar];
	    xCharInfo  *cim;
	    int         src_bitoffset;
	    int         srcbpr;
	    pointer     gstart;
	    int         r,
	                shift = 0;

	    /* save the offset */
	    (*l).position = (gd - gdata);
	    gstart = gd;

	    if (!ci)		/* ignore missing chars */
		continue;

	    src_bitoffset = (int) ci->bits;
	    cim = &ci->metrics;

	    /* sanity check */
	    assert((cim->rightSideBearing - cim->leftSideBearing) <= (pinfo->maxbounds.rightSideBearing - pinfo->minbounds.leftSideBearing));

	    srcbpr = GLWIDTHBYTESPADDED(cim->rightSideBearing -
					cim->leftSideBearing, src_glyph_pad);
	    /*
	     * caculate bytes-per-row for PadNone (others done in allocaton
	     * phase), what (if anything) to ignore or add as padding
	     */
	    switch (mappad) {
	    case BitmapFormatImageRectMin:
		bpr = GLYPH_SIZE(ci, scanlinepad);
		break;
	    case BitmapFormatImageRectMax:
		/* leave the first padded rows blank */
		gd += bpr *
		    (pinfo->maxbounds.ascent - cim->ascent);
		skiprows = bpr *
		    (pinfo->maxbounds.descent - cim->descent);
		/* fall thru */
	    case BitmapFormatImageRectMaxWidth:
		shift = cim->leftSideBearing - min_left;
		break;
	    default:
		assert(0);
	    }

#define	access_type	unsigned char *
/* XXX
 * this would best be a larger size (long), but that causes unaligned
 * refs on (at least) SPARC
 *
 * this whole mess should be rewritten by someone who understands bitblt...
 */

	    for (r = 0; r < (cim->ascent + cim->descent); r++) {
		access_type row = (access_type) (src_bitoffset + (r * srcbpr));
		access_type r2 = (access_type) (gd + (r * bpr));
		int         db;

		for (db = 0;
			db <= ((cim->rightSideBearing - min_left) /
			       (sizeof(*row) * 8));
			db++) {

#if (DEFAULT_BIT_ORDER == MSBFirst)
		    r2[db] = row[db] >> shift;
		    if (db > 0 && shift)	/* get the leftovers from
						 * above */
			r2[db] |= (row[db - 1] << ((sizeof(*row) * 8) - shift));
#else
		    r2[db] = row[db] << shift;
		    if (db > 0 && shift)	/* get the leftovers from
						 * above */
			r2[db] |= (row[db - 1] >> ((sizeof(*row) * 8) - shift));
#endif
		}
	    }
	    /* skip the amount we just filled in */
	    gd += (cim->descent + cim->ascent) * bpr
		+ skiprows;	/* leave the last rows blank */

	    (*l).length = gd - gstart;
	    l++;
	}
    }


    bitorder = (bitorder == BitmapFormatBitOrderLSB) ?
	LSBFirst : MSBFirst;
    byteorder = (byteorder == BitmapFormatByteOrderLSB) ?
	LSBFirst : MSBFirst;

    /* now do the bit, byte, word swapping */
    if (bitorder != src_bit_order)
	BitOrderInvert(gdata, size);
    if (byteorder != src_byte_order) {
	if (scanlineunit == 2)
	    TwoByteSwap(gdata, size);
	else if (scanlineunit == 4)
	    FourByteSwap(gdata, size);
    }
    *data = gdata;
    *tsize = size;
    *offsets = lengths;

    return Successful;
}

/* ARGSUSED */
static int
get_sp_bitmaps(client, pfont, format, flags, num_ranges, range,
	       size, num_glyphs, offsets, data)
    pointer     client;
    FontPtr     pfont;
    fsBitmapFormat format;
    Mask        flags;
    unsigned long num_ranges;
    fsRange    *range;
    unsigned long *size;
    unsigned long *num_glyphs;
    fsOffset  **offsets;
    pointer    *data;
{
    *size = 0;
    *data = (pointer) 0;
    *num_glyphs = 0;
    return pack_sp_glyphs(pfont, format, flags,
			  num_ranges, range, size, num_glyphs, offsets, data);
}

static int
get_sp_glyphs(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    CharInfoPtr *glyphs;	/* RETURN */
{
    SpeedoFontPtr spf;
    unsigned int firstCol;
    register unsigned int numCols;
    unsigned int firstRow;
    unsigned int numRows;
    CharInfoPtr *glyphsBase;
    register unsigned int c;
    register CharInfoPtr pci;
    unsigned int r;
    CharInfoPtr encoding;
    CharInfoPtr pDefault;
    int         itemSize;
    int         err = Successful;

    spf = (SpeedoFontPtr) pFont->fontPrivate;
    encoding = spf->encoding;
    pDefault = spf->pDefault;
    firstCol = pFont->info.firstCol;
    numCols = pFont->info.lastCol - firstCol + 1;
    glyphsBase = glyphs;


    /* XXX - this should be much smarter */
    /* make sure the glyphs are there */
    if (charEncoding == Linear8Bit || charEncoding == TwoD8Bit)
	itemSize = 1;
    else
	itemSize = 2;

#ifdef notyet
    if (!fsd->complete)
	err = fs_load_glyphs(NULL, pFont, count, itemSize, chars);
#endif

    if (err != Successful)
	return err;

    switch (charEncoding) {

    case Linear8Bit:
    case TwoD8Bit:
	if (pFont->info.firstRow > 0)
	    break;
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = &encoding[c];
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols && (pci = &encoding[c])->bits)
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;
    case Linear16Bit:
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = &encoding[c];
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols && (pci = &encoding[c])->bits)
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;

    case TwoD16Bit:
	firstRow = pFont->info.firstRow;
	numRows = pFont->info.lastRow - firstRow + 1;
	while (count--) {
	    r = (*chars++) - firstRow;
	    c = (*chars++) - firstCol;
	    if (r < numRows && c < numCols &&
		    (pci = &encoding[r * numCols + c])->bits)
		*glyphs++ = pci;
	    else if (pDefault)
		*glyphs++ = pDefault;
	}
	break;
    }
    *glyphCount = glyphs - glyphsBase;
    return Successful;
}

static CharInfoRec nonExistantChar;

static int
get_sp_metrics(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    xCharInfo **glyphs;		/* RETURN */
{
    int         ret;
    SpeedoFontPtr spf;
    int         i;
    CharInfoPtr	oldDefault;

    spf = (SpeedoFontPtr) pFont->fontPrivate;
    oldDefault = spf->pDefault;
    spf->pDefault = &nonExistantChar;
    ret = get_sp_glyphs(pFont, count, chars, charEncoding,
			glyphCount, (CharInfoPtr *) glyphs);

    spf->pDefault = oldDefault;
    return ret;
}

int
open_sp_font(fontname, filename, entry, format, fmask, flags, spfont)
    char       *fontname,
               *filename;
    FontEntryPtr entry;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    Mask        flags;
    SpeedoFontPtr *spfont;
{
    SpeedoFontPtr spf;
    SpeedoMasterFontPtr spmf;
    FontPtr     mpfont;
    int         ret;
    fsResolution *res;
    int         num_res;
    char        tmpname[MAXFONTNAMELEN];
    specs_t     specs;
    FontScalableRec vals;
    double      pointsize;
    int         x_res,
                y_res;

    /* make a master if we don't have one */
    if (entry) {
	mpfont = (FontPtr) entry->u.scalable.extra->private;
	if (!mpfont) {
	    mpfont = (FontPtr) xalloc(sizeof(FontRec));
	    if (!mpfont)
		return AllocError;
	    flags |= FontLoadBitmaps;	/* make sure a master is all there */
	    ret = load_sp_font(entry->name.name, filename, (FontEntryPtr) 0,
			       format, fmask, mpfont, flags);
	    if (ret != Successful) {
		xfree(mpfont);
		return ret;
	    }
	    entry->u.scalable.extra->private = (pointer) mpfont;
	}
	spf = (SpeedoFontPtr) mpfont->fontPrivate;
	spmf = spf->master;
    } else {
	ret = open_master(filename, &spmf);
	if (ret != Successful)
	    return ret;
    }

    spf = (SpeedoFontPtr) xalloc(sizeof(SpeedoFontRec));
    if (!spf)
	return AllocError;
    bzero((char *) spf, sizeof(SpeedoFontRec));

    spf->master = spmf;
    spmf->refcount++;
    sp_reset_master(spmf);

    /* set default values */
    res = (fsResolution *) GetClientResolutions(&num_res);
    if (num_res) {
	x_res = res->x_resolution;
	y_res = res->y_resolution;
	pointsize = res->point_size;
    } else {
	x_res = 75;
	y_res = 75;
	pointsize = 12;
    }

    /* tear apart name to get sizes */
    strcpy(tmpname, fontname);
    if (!FontParseXLFDName(tmpname, &vals, FONT_XLFD_REPLACE_NONE))
	return BadFontName;

    if (vals.x > 0)
	x_res = vals.x;
    if (vals.y > 0)
	y_res = vals.y;
    if (vals.point > 0)
	pointsize = vals.point;
    else if (vals.pixel > 0) {
	/* make sure we don't caculate it to 0 */
	pointsize = (vals.pixel * 722.70)/y_res;
    }
    spf->vals.point = pointsize;
    spf->vals.x = x_res;
    spf->vals.y = y_res;
    spf->vals.pixel = ((pointsize * y_res) / 722.7) + 0.5;	/* round it */

    /* set up specs */

    specs.pfont = &spmf->font;
    /* XXX beware of overflow */
    /* Note that point size is in decipoints */
    specs.xxmult = (int) (pointsize * x_res / 720 * (1 << 16));
    specs.xymult = 0L << 16;
    specs.xoffset = 0L << 16;
    specs.yxmult = 0L << 16;
    specs.yymult = (int) (pointsize * y_res / 720 * (1 << 16));
    specs.yoffset = 0L << 16;
    specs.flags = MODE_SCREEN;
    specs.out_info = NULL;

    if (!sp_set_specs(&specs))
	return BadFontName;

    spf->specs = specs;

    *spfont = spf;

    return Successful;
}

static int
load_sp_font(fontname, filename, entry, format, fmask, pfont, flags)
    char       *fontname,
               *filename;
    FontEntryPtr    entry;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    FontPtr     pfont;
    Mask        flags;
{
    SpeedoFontPtr spf;
    SpeedoMasterFontPtr spmf;
    int         esize;
    int         ret;

    ret = open_sp_font(fontname, filename, entry, format, fmask, flags, &spf);

    spmf = spf->master;
    esize = sizeof(CharInfoRec) * (spmf->max_id - spmf->first_char_id + 1);

    spf->encoding = (CharInfoPtr) xalloc(esize);
    bzero((char *) spf->encoding, esize);
    if (!spf->encoding) {
	close_sp_font(spf);
	return AllocError;
    }
    cur_spf = spf;

    make_sp_header(spf, &pfont->info);

    compute_sp_bounds(spf, &pfont->info, SaveMetrics);

    compute_sp_props(spf, fontname, &pfont->info);

    pfont->fontPrivate = (pointer) spf;

/* XXX */
    flags |= FontLoadBitmaps;

    if (flags & FontLoadBitmaps) {
	cur_spf = spf;
	ret = build_all_sp_bitmaps(pfont, format, fmask);
    }
    if (ret != Successful)
	return ret;

    pfont->format = format;

    pfont->get_bitmaps = get_sp_bitmaps;
    pfont->get_metrics = get_sp_metrics;
    pfont->get_glyphs = get_sp_glyphs;
    pfont->get_extents = get_sp_extents;
    pfont->unload_font = SpeedoCloseFont;
    pfont->refcnt = 0;
    pfont->maxPrivate = -1;
    pfont->devPrivates = (pointer *) 0;

    return ret;
}

int
SpeedoFontLoad(ppfont, fontname, filename, entry, format, fmask, flags)
    FontPtr    *ppfont;
    char       *fontname;
    char       *filename;
    FontEntryPtr    entry;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    Mask        flags;
{
    FontPtr     pfont;
    int         ret;

    pfont = (FontPtr) xalloc(sizeof(FontRec));
    if (!pfont) {
	return AllocError;
    }
    ret = load_sp_font(fontname, filename, entry, format, fmask, pfont, flags);

    if (ret == Successful)
	*ppfont = pfont;

    return ret;
}

void
close_sp_font(spf)
    SpeedoFontPtr spf;
{
    SpeedoMasterFontPtr spmf;

    spmf = spf->master;
    if (--spmf->refcount == 0) {
	if (spmf->state & MasterFileOpen) {
	    (void) fclose(spmf->fp);
	    xfree(spmf->f_buffer);
	    xfree(spmf->c_buffer);
	}
    }
    xfree(spf->encoding);
    xfree(spf->bitmaps);
    xfree(spf);
}

void
SpeedoCloseFont(pfont)
    FontPtr     pfont;
{
    SpeedoFontPtr spf;

    spf = (SpeedoFontPtr) pfont->fontPrivate;
    close_sp_font(spf);
    xfree(pfont->info.isStringProp);
    xfree(pfont->info.props);
}
