/* $XConsortium: spinfo.c,v 1.9 93/08/24 18:48:51 gildea Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
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
 * Author: Dave Lemke, Network Computing Devices, Inc
 */

#include "fntfilst.h"
#include "spint.h"
#include <math.h>

/* percentage of pointsize used to specify ascent & descent */
#define	STRETCH_FACTOR	120

enum scaleType {
    atom, truncate_atom, pixel_size, point_size, resolution_x,
    resolution_y, average_width
};

typedef struct _fontProp {
    char       *name;
    long        atom;
    enum scaleType type;
}           fontProp;

static fontProp fontNamePropTable[] = {
    "FOUNDRY", 0, atom,
    "FAMILY_NAME", 0, atom,
    "WEIGHT_NAME", 0, truncate_atom,
    "SLANT", 0, atom,
    "SETWIDTH_NAME", 0, atom,
    "ADD_STYLE_NAME", 0, atom,
    "PIXEL_SIZE", 0, pixel_size,
    "POINT_SIZE", 0, point_size,
    "RESOLUTION_X", 0, resolution_x,
    "RESOLUTION_Y", 0, resolution_y,
    "SPACING", 0, atom,
    "AVERAGE_WIDTH", 0, average_width,
    "CHARSET_REGISTRY", 0, atom,
    "CHARSET_ENCODING", 0, truncate_atom,
};

/* Warning: following array is closely related to the sequence of
   defines after it. */

static fontProp extraProps[] = {
    "FONT", 0, 0,
    "COPYRIGHT", 0, 0,
    "RAW_PIXELSIZE", 0, 0,
    "RAW_POINTSIZE", 0, 0,
    "RAW_ASCENT", 0, 0,
    "RAW_DESCENT", 0, 0,
    "RAW_AVERAGE_WIDTH", 0, 0,
};

/* this is a bit kludgy */
#define	FONTPROP	0
#define	COPYRIGHTPROP	1
#define RAWPIXELPROP	2
#define RAWPOINTPROP	3
#define RAWASCENTPROP	4
#define RAWDESCENTPROP	5
#define RAWWIDTHPROP	6


#define NNAMEPROPS (sizeof(fontNamePropTable) / sizeof(fontProp))
#define NEXTRAPROPS (sizeof(extraProps) / sizeof(fontProp))

#define	NPROPS	(NNAMEPROPS + NEXTRAPROPS)

void
sp_make_standard_props()
{
    int         i;
    fontProp   *t;

    i = sizeof(fontNamePropTable) / sizeof(fontProp);
    for (t = fontNamePropTable; i; i--, t++)
	t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
    i = sizeof(extraProps) / sizeof(fontProp);
    for (t = extraProps; i; i--, t++)
	t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
}

void
sp_make_header(spf, pinfo)
    SpeedoFontPtr spf;
    FontInfoPtr pinfo;
{
    int         pixel_size;
    SpeedoMasterFontPtr spmf = spf->master;

    pinfo->firstCol = spmf->first_char_id & 0xff;
    pinfo->firstRow = spmf->first_char_id >> 8;
    pinfo->lastCol = spmf->max_id & 0xff;
    pinfo->lastRow = spmf->max_id >> 8;

    /* XXX -- hackery here */
    pinfo->defaultCh = 0;
/* computed by FontComputeInfoAccelerators:
 *  noOverlap
 *  constantMetrics
 *  terminalFont
 *  constantWidth
 *  inkInside
 */
    pinfo->inkMetrics = 0;
    pinfo->allExist = 0;
    pinfo->drawDirection = LeftToRight;
    pinfo->cachable = 1;
    if (spf->specs.xxmult != spf->specs.yymult)
	pinfo->anamorphic = TRUE;
    else
	pinfo->anamorphic = FALSE;
/* computed by sp_compute_bounds:
 *  maxOverlap
 *  maxbounds
 *  minbounds
 *  ink_maxbounds
 *  ink_minbounds
 */
    pixel_size = spf->vals.pixel_matrix[3] * STRETCH_FACTOR / 100;
    pinfo->fontAscent = pixel_size * 764 / 1000;	/* 764 == EM_TOP */
    pinfo->fontDescent = pixel_size - pinfo->fontAscent;
}

static void
adjust_min_max(minc, maxc, tmp)
    xCharInfo  *minc,
               *maxc,
               *tmp;
{
#define MINMAX(field,ci) \
	if (minc->field > (ci)->field) \
	     minc->field = (ci)->field; \
	if (maxc->field < (ci)->field) \
	    maxc->field = (ci)->field;

    MINMAX(ascent, tmp);
    MINMAX(descent, tmp);
    MINMAX(leftSideBearing, tmp);
    MINMAX(rightSideBearing, tmp);
    MINMAX(characterWidth, tmp);

    if ((INT16)minc->attributes > (INT16)tmp->attributes)
	minc->attributes = tmp->attributes;
    if ((INT16)maxc->attributes < (INT16)tmp->attributes)
	maxc->attributes = tmp->attributes;

#undef	MINMAX
}


void
sp_compute_bounds(spf, pinfo, flags, sWidth)
    SpeedoFontPtr spf;
    FontInfoPtr pinfo;
    unsigned long flags;
    long *sWidth;
{
    int         i,
                id,
                index,
		maxOverlap,
		overlap;
    xCharInfo   minchar,
                maxchar,
                tmpchar;
    bbox_t      bbox;
    fix31       width;
    double      pix_width,
                total_width = 0.0;
    SpeedoMasterFontPtr spmf = spf->master;
    int	firstChar;
    int num_chars = 0;

    firstChar = spmf->first_char_id;
    minchar.ascent = minchar.descent =
	minchar.leftSideBearing = minchar.rightSideBearing =
	minchar.characterWidth = minchar.attributes = 32767;
    maxchar.ascent = maxchar.descent =
	maxchar.leftSideBearing = maxchar.rightSideBearing =
	maxchar.characterWidth = maxchar.attributes = -32767;
    maxOverlap = -32767;
    *sWidth = 0;
    for (i = 0; i < spmf->num_chars; i++) {
	int j;
	int char_id;

	index = spmf->enc[i * 2 + 1];
	char_id = spmf->enc[i * 2];
      /*
       * See if this character is in the list of ranges specified in the
       * XLFD name
       */
	for (j = 0; j < spf->vals.nranges; j++)
	    if (char_id >= mincharno(spf->vals.ranges[j]) &&
		    char_id <= maxcharno(spf->vals.ranges[j]))
		break;
	if (spf->vals.nranges && j == spf->vals.nranges)
	    continue;
	num_chars++;

	if (!(flags & ComputeBoundsOnly)) {

	    width = sp_get_char_width(index);

	    /* convert to pixel coords */
	    pix_width = (int)width * (spf->specs.xxmult / 65536L) +
		((int) width * (spf->specs.xxmult % 65536L))
		/ 65536L;
	    pix_width /= 65536L;

	    (void) sp_get_char_bbox(index, &bbox);
	    bbox.ymax = (bbox.ymax + 32768L) >> 16;
	    bbox.ymin = (bbox.ymin + 32768L) >> 16;
	    bbox.xmin = (bbox.xmin + 32768L) >> 16;
	    bbox.xmax = (bbox.xmax + 32768L) >> 16;
	    tmpchar.ascent = bbox.ymax;
	    tmpchar.descent = -bbox.ymin;
	    tmpchar.characterWidth = (int)(pix_width +		/* round */
					   (pix_width > 0 ? 0.5 : -0.5));
	    tmpchar.rightSideBearing = bbox.xmax;
	    tmpchar.leftSideBearing = bbox.xmin;

	    if (!tmpchar.characterWidth &&
		tmpchar.ascent == -tmpchar.descent &&
		tmpchar.rightSideBearing == tmpchar.leftSideBearing)
	    {
		/* Character appears non-existent, probably as a result
		   of the transformation.  Let's give it one pixel in
		   the universe so it's not mistaken for non-existent. */
		tmpchar.leftSideBearing = tmpchar.descent = 0;
		tmpchar.rightSideBearing = tmpchar.ascent = 1;
	    }

	    tmpchar.attributes = (int)((double)(int)width / 65.536 + .5);
	}
	else
	    tmpchar = spf->encoding[char_id - firstChar].metrics;

	adjust_min_max(&minchar, &maxchar, &tmpchar);
	overlap = tmpchar.rightSideBearing - tmpchar.characterWidth;
	if (maxOverlap < overlap)
	    maxOverlap = overlap;

	total_width += fabs(pix_width);
	*sWidth += abs((INT16)tmpchar.attributes);

	if (flags & SaveMetrics) {
	    id = spmf->enc[i * 2] - firstChar;
	    assert(id <= spmf->max_id - firstChar);
	    spf->encoding[id].metrics = tmpchar;
	}
    }


    if (num_chars > 0)
    {
	double n2 = (double)num_chars / 2.0;
	spf->vals.width = (total_width * 10.0 + n2) / num_chars;
	*sWidth = (int)((double)*sWidth * 10.0 + n2) / num_chars;
    }
    else
    {
	spf->vals.width = 0;
    }
    pinfo->maxbounds = maxchar;
    pinfo->minbounds = minchar;
    pinfo->ink_maxbounds = maxchar;
    pinfo->ink_minbounds = minchar;
    pinfo->maxOverlap = maxOverlap;
}

void
sp_compute_props(spf, fontname, pinfo, sWidth)
    SpeedoFontPtr spf;
    char       *fontname;
    FontInfoPtr pinfo;
    long	sWidth;
{
    FontPropPtr pp;
    int         i,
                nprops;
    fontProp   *fpt;
    char       *is_str;
    char       *ptr1,
               *ptr2;
    char       *ptr3;

    nprops = pinfo->nprops = NPROPS;
    pinfo->isStringProp = (char *) xalloc(sizeof(char) * nprops);
    pinfo->props = (FontPropPtr) xalloc(sizeof(FontPropRec) * nprops);
    if (!pinfo->isStringProp || !pinfo->props) {
	xfree(pinfo->isStringProp);
	pinfo->isStringProp = (char *) 0;
	xfree(pinfo->props);
	pinfo->props = (FontPropPtr) 0;
	return;
    }
    bzero(pinfo->isStringProp, (sizeof(char) * nprops));

    ptr2 = fontname;
    for (i = NNAMEPROPS, pp = pinfo->props, fpt = fontNamePropTable,
	    is_str = pinfo->isStringProp;
	    i;
	    i--, pp++, fpt++, is_str++) {

        if (*ptr2)
        {
            ptr1 = ptr2 + 1;
            if (!(ptr2 = index(ptr1, '-'))) ptr2 = index(ptr1, '\0');
        }

	pp->name = fpt->atom;
	switch (fpt->type) {
	case atom:
	    *is_str = TRUE;
	    pp->value = MakeAtom(ptr1, ptr2 - ptr1, TRUE);
	    break;
	case truncate_atom:
	    *is_str = TRUE;
	    for (ptr3 = ptr1; *ptr3; ptr3++)
		if (*ptr3 == '=' || *ptr3 == '-' ||
		    *ptr3 == '+' || *ptr3 == '~')
		    break;
	    pp->value = MakeAtom(ptr1, ptr3 - ptr1, TRUE);
	    break;
	case pixel_size:
	    pp->value = spf->vals.pixel_matrix[3];
	    break;
	case point_size:
	    pp->value = (int)(spf->vals.point_matrix[3] * 10.0 +
			      (spf->vals.point_matrix[3] > 0 ? .5 : -.5));
	    break;
	case resolution_x:
	    pp->value = spf->vals.x;
	    break;
	case resolution_y:
	    pp->value = spf->vals.y;
	    break;
	case average_width:
	    pp->value = spf->vals.width;
	    break;
	}
    }

    for (i = 0, fpt = extraProps; i < NEXTRAPROPS; i++, is_str++, pp++, fpt++) {
	pp->name = fpt->atom;
	switch (i) {
	case FONTPROP:
	    *is_str = TRUE;
	    pp->value = MakeAtom(fontname, strlen(fontname), TRUE);
	    break;
	case COPYRIGHTPROP:
	    *is_str = TRUE;
	    pp->value = MakeAtom(spf->master->copyright,
				 strlen(spf->master->copyright), TRUE);
	    break;
         case RAWPIXELPROP:
            *is_str = FALSE;
            pp->value = 1000;
	    break;
         case RAWPOINTPROP:
            *is_str = FALSE;
            pp->value = (long)(72270.0 / (double)spf->vals.y + .5);
	    break;
         case RAWASCENTPROP:
            *is_str = FALSE;
            pp->value = STRETCH_FACTOR * 764 / 100;
	    break;
         case RAWDESCENTPROP:
            *is_str = FALSE;
            pp->value = STRETCH_FACTOR * 236 / 100;
	    break;
         case RAWWIDTHPROP:
            *is_str = FALSE;
            pp->value = sWidth;
	    break;
	}
    }
}
