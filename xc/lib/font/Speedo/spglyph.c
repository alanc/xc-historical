/* $XConsortium: spglyph.c,v 1.3 91/05/11 09:58:37 rws Exp $ */
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

#include	"spint.h"

#undef	CLIP_BBOX_NOISE

static CurrentFontValuesRec current_font_values;
static CurrentFontValuesPtr cfv = &current_font_values;
static int  bit, byte, scan;

unsigned long
compute_sp_data_size(pfont, mappad, scanlinepad, start, end)
    FontPtr     pfont;
    int         mappad,
                scanlinepad;
    unsigned long start,
                end;
{
    unsigned long ch;
    unsigned long size = 0;
    int         bpr;
    SpeedoFontPtr spf = (SpeedoFontPtr) pfont->fontPrivate;
    FontInfoPtr pinfo = &pfont->info;
    int	    firstChar;

    firstChar = spf->master->first_char_id;

    /* allocate the space */
    switch (mappad) {
	int         charsize;
	CharInfoPtr ci;
	xCharInfo  *cim;

    case BitmapFormatImageRectMin:
	cfv->bpr = 0;
	for (ch = start; ch <= end; ch++) {
	    ci = &spf->encoding[ch - firstChar];
	    if (!ci)
		ci = spf->pDefault;
	    cim = &ci->metrics;
	    charsize = GLYPH_SIZE(ci, scanlinepad);
	    charsize *= cim->ascent + cim->descent;
	    size += charsize;
	}
	break;
    case BitmapFormatImageRectMaxWidth:
	bpr = GLWIDTHBYTESPADDED(pinfo->ink_maxbounds.rightSideBearing
			- pinfo->ink_minbounds.leftSideBearing, scanlinepad);
	cfv->bpr = bpr;
	for (ch = start; ch <= end; ch++) {
	    ci = &spf->encoding[ch - firstChar];
	    if (!ci)
		ci = spf->pDefault;
	    cim = &ci->metrics;
	    charsize = bpr * (cim->ascent + cim->descent);
	    size += charsize;
	}
	break;
    case BitmapFormatImageRectMax:
	bpr = GLWIDTHBYTESPADDED(pinfo->ink_maxbounds.rightSideBearing
			- pinfo->ink_minbounds.leftSideBearing, scanlinepad);
	cfv->bpr = bpr;
	size = (end - start + 1) * bpr
	    * (pinfo->ink_maxbounds.ascent +
	       pinfo->ink_maxbounds.descent);
	break;
    default:
	assert(0);
    }

    return size;
}

static void
finish_line(spf)
    SpeedoFontPtr spf;
{
    int         bpr = cfv->bpr;
    CharInfoPtr ci = &spf->encoding[cfv->char_id - spf->master->first_char_id];

    if (bpr == 0) {
	bpr = GLYPH_SIZE(ci, cfv->scanpad);
    }
    if (bpr) {			/* char may not have any metrics... */
	cfv->bp += bpr;
    }
    assert(cfv->bp - cur_spf->bitmaps <= cur_spf->bitmap_size);
}


void
sp_set_bitmap_bits(y, xbit1, xbit2)
    fix15       y;
    fix15       xbit1,
                xbit2;
{
    fix15       i;
    int		nmiddle;
    CARD32	startmask, endmask;
    CARD32	*dst;

    if (xbit1 > cfv->bit_width) {

#ifdef CLIP_BBOX_NOISE
	SpeedoErr("Run wider than bitmap width -- truncated\n");
#endif

	xbit1 = cfv->bit_width;
    }
    if (xbit2 > cfv->bit_width) {

#ifdef CLIP_BBOX_NOISE
	SpeedoErr("Run wider than bitmap width -- truncated\n");
#endif

	xbit2 = cfv->bit_width;
    }
    while (cfv->cur_y != y) {
	finish_line(cur_spf);
	cfv->cur_y++;
    }

    cfv->last_y = y;
    if (y >= cfv->bit_height) {

#ifdef CLIP_BBOX_NOISE
	SpeedoErr("Y larger than bitmap height -- truncated\n");
#endif

	cfv->trunc = 1;
	return;
    }
/*
 * XXX
 *
 * needs testing
 */

    if (xbit1 < 0)	/* XXX this is more than a little bit rude... */
	xbit1 = 0;

    dst = ((CARD32 *) cfv->bp) + (xbit1 >> 5);
    nmiddle = (xbit2 >> 5) - (xbit1 >> 5);
    xbit1 &= 31;
    xbit2 &= 31;
    if (bit == MSBFirst)
    {
	startmask =   ((CARD32) ~0) >> xbit1;
	endmask   = ~(((CARD32) ~0) >> xbit2);
    }
    else
    {
	startmask =   ((CARD32) ~0) << xbit1;
	endmask   = ~(((CARD32) ~0) << xbit2);
    }
    if (nmiddle == 0)
	*dst |= endmask & startmask;
    else {
	*dst++ |= startmask;
	while (--nmiddle)
	    *dst++ = ~0;
	*dst |= endmask;
    }
}

/* ARGSUSED */
void
sp_open_bitmap(x_set_width, y_set_width, xorg, yorg, xsize, ysize)
    fix31       x_set_width;
    fix31       y_set_width;
    fix31       xorg;
    fix31       yorg;
    fix15       xsize;
    fix15       ysize;
{
    CharInfoPtr ci = &cur_spf->encoding[cfv->char_id - cur_spf->master->first_char_id];
    int	    off_horz;
    int	    off_vert;

    off_horz = (fix15) ((xorg + 32768L) / 65536);
    off_vert = (fix15) ((yorg + 32768L) / 65536);
    ci->metrics.leftSideBearing = off_horz;
    ci->metrics.descent = -off_vert;
    ci->metrics.rightSideBearing = xsize + off_horz;
    ci->metrics.ascent = ysize + off_vert;

    cfv->bit_width = xsize;
    cfv->bit_height = ysize;

    assert(cfv->bp - cur_spf->bitmaps <= cur_spf->bitmap_size);
    ci->bits = (char *) cfv->bp;

    cfv->cur_y = 0;
}

void
sp_close_bitmap()
{
    CharInfoPtr ci = &cur_spf->encoding[cfv->char_id - cur_spf->master->first_char_id];
    int bpr = cfv->bpr;

    if (bpr == 0)
	bpr = GLYPH_SIZE (ci, cfv->scanpad);
    if (!cfv->trunc)
	finish_line(cur_spf);
    cfv->trunc = 0;
    cfv->last_y++;
    while (cfv->last_y < cfv->bit_height) {
	finish_line(cur_spf);
	cfv->last_y++;
    }
    if (byte != bit)
    {
	switch (scan) {
	case 1:
	    break;
	case 2:
	    TwoByteSwap (cfv->bp, bpr * cfv->bit_height);
	    break;
	case 4:
	    FourByteSwap (cfv->bp, bpr * cfv->bit_height);
	    break;
	}
    }
}

int
build_all_sp_bitmaps(pfont, format, fmask)
    FontPtr     pfont;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
{
    int         ret,
                glyph = 1,
                image = BitmapFormatImageRectMin;
    unsigned long glyph_size;
    SpeedoFontPtr spf = (SpeedoFontPtr) pfont->fontPrivate;
    SpeedoMasterFontPtr spmf = spf->master;
    pointer     bitmaps;
    int         start,
                end,
                i;

    scan = 1;
    ret = CheckFSFormat(format, fmask,
			&bit, &byte, &scan, &glyph, &image);

    if (ret != Successful)
	return BadFontFormat;

    start = spmf->first_char_id;
    end = spmf->max_id;
    glyph_size = compute_sp_data_size(pfont, image, glyph, start, end);

    /* XXX -- MONDO KLUDGE -- add some slop */
    /*
     * not sure why this is wanted, but it keeps the packer from going off the
     * end and toasting us down the line
     */
    glyph_size += 20;

#ifdef DEBUG
    spf->bitmap_size = glyph_size;
#endif

    bitmaps = (pointer) xalloc(glyph_size);
    if (!bitmaps)
	return AllocError;
    bzero((char *) bitmaps, glyph_size);

    /* set up some state */
    cur_spf = spf;
    spf->bitmaps = bitmaps;
    cfv->format = format;
    cfv->scanpad = glyph;
    cfv->bp = bitmaps;

    for (i = 0; i < spmf->num_chars; i++) {
	cfv->char_index = spmf->enc[i * 2 + 1];
	cfv->char_id = spmf->enc[i * 2];
	if (!cfv->char_id)
	    continue;
	if (!sp_make_char(cfv->char_index)) {
#ifdef DEBUG
	    SpeedoErr("Can't make char %x\n", cfv->char_index);
#endif
	}
    }

    return Successful;
}
