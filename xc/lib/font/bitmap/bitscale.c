/*
 * $XConsortium: bitscale.c,v 1.22 94/02/08 12:29:57 gildea Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * M.I.T.  BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include "fntfilst.h"
#include "bitmap.h"
#include <math.h>

extern Atom MakeAtom();

void bitmapUnloadScalable();

enum scaleType {
    atom, truncate_atom, pixel_size, point_size, resolution_x,
    resolution_y, average_width, scaledX, scaledY, unscaled, fontname,
    raw_ascent, raw_descent, raw_pixelsize, raw_pointsize,
    raw_average_width, uncomputed
};

typedef struct _fontProp {
    char       *name;
    Atom        atom;
    enum scaleType type;
} fontProp;

static unsigned long fontGeneration = 0;	/* initialization flag */

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
    "FONT", 0, fontname,
    "RAW_ASCENT", 0, raw_ascent,
    "RAW_DESCENT", 0, raw_descent,
    "RAW_PIXELSIZE", 0, raw_pixelsize,
    "RAW_POINTSIZE", 0, raw_pointsize,
    "RAW_AVERAGE_WIDTH", 0, raw_average_width
};

#define TRANSFORM_POINT(matrix, x, y, dest) \
	((dest)[0] = (matrix)[0] * (x) + (matrix)[2] * (y), \
	 (dest)[1] = (matrix)[1] * (x) + (matrix)[3] * (y))

#define CHECK_EXTENT(lsb, rsb, desc, asc, data) \
	((lsb) > (data)[0] ? (lsb) = (data)[0] : 0 , \
	 (rsb) < (data)[0] ? (rsb) = (data)[0] : 0, \
	 (-desc) > (data)[1] ? (desc) = -(data)[1] : 0 , \
	 (asc) < (data)[1] ? (asc) = (data)[1] : 0)

#define NPROPS (sizeof(fontNamePropTable) / sizeof(fontProp))

/* Warning: order of the next two tables is critically interdependent.
   Location of "unscaled" properties at the end of fontPropTable[]
   is important. */

static fontProp fontPropTable[] = {
    "MIN_SPACE", 0, scaledX,
    "NORM_SPACE", 0, scaledX,
    "MAX_SPACE", 0, scaledX,
    "END_SPACE", 0, scaledX,
    "AVG_CAPITAL_WIDTH", 0, scaledX,
    "AVG_LOWERCASE_WIDTH", 0, scaledX,
    "QUAD_WIDTH", 0, scaledX,
    "FIGURE_WIDTH", 0, scaledX,
    "SUPERSCRIPT_X", 0, scaledX,
    "SUPERSCRIPT_Y", 0, scaledY,
    "SUBSCRIPT_X", 0, scaledX,
    "SUBSCRIPT_Y", 0, scaledY,
    "SUPERSCRIPT_SIZE", 0, scaledY,
    "SUBSCRIPT_SIZE", 0, scaledY,
    "SMALL_CAP_SIZE", 0, scaledY,
    "UNDERLINE_POSITION", 0, scaledY,
    "UNDERLINE_THICKNESS", 0, scaledY,
    "STRIKEOUT_ASCENT", 0, scaledY,
    "STRIKEOUT_DESCENT", 0, scaledY,
    "CAP_HEIGHT", 0, scaledY,
    "X_HEIGHT", 0, scaledY,
    "ITALIC_ANGLE", 0, unscaled,
    "RELATIVE_SETWIDTH", 0, unscaled,
    "RELATIVE_WEIGHT", 0, unscaled,
    "WEIGHT", 0, unscaled,
    "DESTINATION", 0, unscaled
};

static fontProp rawFontPropTable[] = {
    "RAW_MIN_SPACE", 0, 0,
    "RAW_NORM_SPACE", 0, 0,
    "RAW_MAX_SPACE", 0, 0,
    "RAW_END_SPACE", 0, 0,
    "RAW_AVG_CAPITAL_WIDTH", 0, 0,
    "RAW_AVG_LOWERCASE_WIDTH", 0, 0,
    "RAW_QUAD_WIDTH", 0, 0,
    "RAW_FIGURE_WIDTH", 0, 0,
    "RAW_SUPERSCRIPT_X", 0, 0,
    "RAW_SUPERSCRIPT_Y", 0, 0,
    "RAW_SUBSCRIPT_X", 0, 0,
    "RAW_SUBSCRIPT_Y", 0, 0,
    "RAW_SUPERSCRIPT_SIZE", 0, 0,
    "RAW_SUBSCRIPT_SIZE", 0, 0,
    "RAW_SMALL_CAP_SIZE", 0, 0,
    "RAW_UNDERLINE_POSITION", 0, 0,
    "RAW_UNDERLINE_THICKNESS", 0, 0,
    "RAW_STRIKEOUT_ASCENT", 0, 0,
    "RAW_STRIKEOUT_DESCENT", 0, 0,
    "RAW_CAP_HEIGHT", 0, 0,
    "RAW_X_HEIGHT", 0, 0,
};

static void
initFontPropTable()
{
    int         i;
    fontProp   *t;

    i = sizeof(fontNamePropTable) / sizeof(fontProp);
    for (t = fontNamePropTable; i; i--, t++)
	t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);

    i = sizeof(fontPropTable) / sizeof(fontProp);
    for (t = fontPropTable; i; i--, t++)
	t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);

    i = sizeof(rawFontPropTable) / sizeof(fontProp);
    for (t = rawFontPropTable; i; i--, t++)
	t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
}

static FontEntryPtr
GetScalableEntry (fpe, name)
    FontPathElementPtr	fpe;
    FontNamePtr		name;
{
    FontDirectoryPtr	dir;

    dir = (FontDirectoryPtr) fpe->private;
    return FontFileFindNameInDir (&dir->scalable, name);
}

static double
get_matrix_horizontal_component(matrix)
    double *matrix;
{
    return hypot(matrix[0], matrix[1]);
}

static double
get_matrix_vertical_component(matrix)
    double *matrix;
{
    return hypot(matrix[2], matrix[3]);
}


static Bool
ComputeScaleFactors(from, to, dx, dy, sdx, sdy, rescale_x)
    FontScalablePtr from,
                to;
    double     *dx, *sdx,
               *dy, *sdy,
	       *rescale_x;
{
    double srcpixelset, destpixelset, srcpixel, destpixel;

    srcpixelset = get_matrix_horizontal_component(from->pixel_matrix);
    destpixelset = get_matrix_horizontal_component(to->pixel_matrix);
    srcpixel = get_matrix_vertical_component(from->pixel_matrix);
    destpixel = get_matrix_vertical_component(to->pixel_matrix);

    if (srcpixelset >= EPS)
    {
	*dx = destpixelset / srcpixelset;
	*sdx = 1000.0 / srcpixelset;
    }
    else
	*sdx = *dx = 0;

    *rescale_x = 1.0;

    /* If client specified a width, it overrides setsize; in this
       context, we interpret width as applying to the font before any
       rotation, even though that's not what is ultimately returned in
       the width field. */
    if (from->width > 0 && to->width > 0 && fabs(*dx) > EPS)
    {
	double rescale = (double)to->width / (double)from->width;

	/* If the client specified a transformation matrix, the rescaling
	   for width does *not* override the setsize.  Instead, just check
	   for consistency between the setsize from the matrix and the
	   setsize that would result from rescaling according to the width.
	   This assumes (perhaps naively) that the width is correctly
	   reported in the name.  As an interesting side effect, this test
	   may result in choosing a different source bitmap (one that
	   scales consistently between the setsize *and* the width) than it
	   would choose if a width were not specified.  Sort of a hidden
	   multiple-master functionality. */
	if ((to->values_supplied & PIXELSIZE_MASK) == PIXELSIZE_ARRAY ||
	    (to->values_supplied & POINTSIZE_MASK) == POINTSIZE_ARRAY)
	{
	    /* Reject if resulting width difference is >= 1 pixel */
	    if (fabs(rescale * from->width - *dx * from->width) >= 10)
		return FALSE;
	}
	else
	{
	    *rescale_x = rescale/(*dx);
	    *dx = rescale;
	}
    }

    if (srcpixel >= EPS)
    {
	*dy = destpixel / srcpixel;
	*sdy = 1000.0 / srcpixel;
    }
    else
	*sdy = *dy = 0;

    return TRUE;
}

/* favor enlargement over reduction because of aliasing resulting
   from reduction */
#define SCORE(m,s) \
if (m >= 1.0) { \
    if (m == 1.0) \
        score += (16 * s); \
    else if (m == 2.0) \
        score += (4 * s); \
    else \
        score += (3 * s) / m; \
} else { \
        score += (2 * s) * m; \
}

/* don't need to favor enlargement when looking for bitmap that can
   be used unscalable */
#define SCORE2(m,s) \
if (m >= 1.0) \
    score += (8 * s) / m; \
else \
    score += (8 * s) * m;

FontEntryPtr
FindBestToScale(fpe, entry, vals, best, dxp, dyp, sdxp, sdyp, fpep)
    FontPathElementPtr	fpe;
    FontEntryPtr	entry;
    FontScalablePtr	vals,
			best;
    double		*dxp, *sdxp,
			*dyp, *sdyp;
    FontPathElementPtr	*fpep;
{
    FontScalableRec temp;
    int		    source, i;
    int		    best_score, best_unscaled_score,
		    score;
    double	    dx, sdx, dx_amount,
		    dy, sdy, dy_amount,
		    best_dx, best_sdx, best_dx_amount,
		    best_dy, best_sdy, best_dy_amount,
		    best_unscaled_sdx, best_unscaled_sdy,
		    rescale_x, best_rescale_x,
		    best_unscaled_rescale_x;
    FontEntryPtr    zero;
    FontNameRec	    zeroName;
    char	    zeroChars[MAXFONTNAMELEN];
    FontDirectoryPtr	dir;
    FontScaledPtr   scaled;
    FontScalableExtraPtr   extra;
    FontScaledPtr   best_scaled, best_unscaled;
    FontPathElementPtr	best_fpe, best_unscaled_fpe;
    FontEntryPtr    bitmap = NULL;
    FontEntryPtr    result;
    int		    aliascount = 20;
    FontPathElementPtr	bitmap_fpe;
    FontNameRec	    xlfdName;

    /* find the best match */
    best_scaled = 0;
    best_score = 0;
    best_unscaled = 0;
    best_unscaled_score = -1;
    memcpy (zeroChars, entry->name.name, entry->name.length);
    zeroChars[entry->name.length] = '\0';
    zeroName.name = zeroChars;
    FontParseXLFDName (zeroChars, &temp, FONT_XLFD_REPLACE_ZERO);
    zeroName.length = strlen (zeroChars);
    zeroName.ndashes = entry->name.ndashes;
    xlfdName.name = vals->xlfdName;
    xlfdName.length = strlen(xlfdName.name);
    xlfdName.ndashes = FontFileCountDashes(xlfdName.name, xlfdName.length);
    restart_bestscale_loop: ;
    /*
     * Look through all the registered bitmap sources for
     * the same zero name as ours; entries along that one
     * can be scaled as desired.
     */
    for (source = 0; source < FontFileBitmapSources.count; source++)
    {
	/* There might already be a bitmap that satisfies the request
	   but didn't have a zero name that was found by the scalable
	   font matching logic.  Keep track if there is.  */
	if (bitmap == NULL && vals->xlfdName != NULL)
	{
	    bitmap_fpe = FontFileBitmapSources.fpe[source];
	    dir = (FontDirectoryPtr) bitmap_fpe->private;
	    bitmap = FontFileFindNameInDir (&dir->nonScalable, &xlfdName);
	    if (bitmap && bitmap->type != FONT_ENTRY_BITMAP)
	    {
		if (bitmap->type == FONT_ENTRY_ALIAS && aliascount > 0)
		{
		    aliascount--;
		    xlfdName.name = bitmap->u.alias.resolved;
		    xlfdName.length = strlen(xlfdName.name);
		    xlfdName.ndashes = FontFileCountDashes(xlfdName.name,
							   xlfdName.length);
		    bitmap = NULL;
		    goto restart_bestscale_loop;
		}
		else
		    bitmap = NULL;
	    }
	}

	if (FontFileBitmapSources.fpe[source] == fpe)
	    zero = entry;
	else
	{
	    dir = (FontDirectoryPtr) FontFileBitmapSources.fpe[source]->private;
	    zero = FontFileFindNameInDir (&dir->scalable, &zeroName);
	    if (!zero)
		continue;
	}
	extra = zero->u.scalable.extra;
	for (i = 0; i < extra->numScaled; i++)
	{
	    FontScalableRec tmpvals;
	    scaled = &extra->scaled[i];
	    if (!scaled->bitmap)
		continue;
	    if (!ComputeScaleFactors(&scaled->vals, vals, &dx, &dy, &sdx, &sdy,
				     &rescale_x))
		continue;
	    score = 0;
	    dx_amount = dx;
	    dy_amount = dy;
	    SCORE(dy_amount, 10);
	    SCORE(dx_amount, 1);
	    if ((score > best_score) ||
		    ((score == best_score) &&
		     ((dy_amount < best_dy_amount) ||
 		      ((dy_amount == best_dy_amount) &&
 		       (dx_amount < best_dx_amount))))) 
	    {
		best_fpe = FontFileBitmapSources.fpe[source];
	    	best_scaled = scaled;
	    	best_score = score;
	    	best_dx = dx;
	    	best_dy = dy;
	    	best_sdx = sdx;
	    	best_sdy = sdy;
	    	best_dx_amount = dx_amount;
	    	best_dy_amount = dy_amount;
		best_rescale_x = rescale_x;
	    }
	    /* Is this font a candidate for use without ugly rescaling? */
	    if (fabs(dx) > EPS && fabs(dy) > EPS &&
		fabs(vals->pixel_matrix[0] * rescale_x -
		     scaled->vals.pixel_matrix[0]) < 1 &&
		fabs(vals->pixel_matrix[1] * rescale_x -
		     scaled->vals.pixel_matrix[1]) < EPS &&
		fabs(vals->pixel_matrix[2] -
		     scaled->vals.pixel_matrix[2]) < EPS &&
		fabs(vals->pixel_matrix[3] -
		     scaled->vals.pixel_matrix[3]) < 1)
	    {
		/* Yes.  The pixel sizes are close on the diagonal and
		   extremely close off the diagonal. */
		score = 0;
		SCORE2(vals->pixel_matrix[3] /
		       scaled->vals.pixel_matrix[3], 10);
		SCORE2(vals->pixel_matrix[0] * rescale_x /
		       scaled->vals.pixel_matrix[0], 1);
		if (score > best_unscaled_score)
		{
		    best_unscaled_fpe = FontFileBitmapSources.fpe[source];
	    	    best_unscaled = scaled;
	    	    best_unscaled_sdx = sdx / dx;
	    	    best_unscaled_sdy = sdy / dy;
		    best_unscaled_score = score;
		    best_unscaled_rescale_x = rescale_x;
		}
	    }
	}
    }
    if (best_unscaled)
    {
	*best = best_unscaled->vals;
	*fpep = best_unscaled_fpe;
	*dxp = 1.0;
	*dyp = 1.0;
	*sdxp = best_unscaled_sdx;
	*sdyp = best_unscaled_sdy;
	rescale_x = best_unscaled_rescale_x;
	result = best_unscaled->bitmap;
    }
    else if (best_scaled)
    {
	*best = best_scaled->vals;
	*fpep = best_fpe;
	*dxp = best_dx;
	*dyp = best_dy;
	*sdxp = best_sdx;
	*sdyp = best_sdy;
	rescale_x = best_rescale_x;
	result = best_scaled->bitmap;
    }
    else
	result = NULL;

    if (bitmap != NULL && (result == NULL || *dxp != 1.0 || *dyp != 1.0))
    {
	*fpep = bitmap_fpe;
	FontParseXLFDName (bitmap->name.name, best, FONT_XLFD_REPLACE_NONE);
	if (ComputeScaleFactors(best, best, dxp, dyp, sdxp, sdyp, &rescale_x))
	    result = bitmap;
	else
	    result = NULL;
    }

    if (rescale_x != 1.0)
    {
	/* We have rescaled horizontally due to an XLFD width field.  Change
	   the matrix appropriately */
	vals->pixel_matrix[0] *= rescale_x;
	vals->pixel_matrix[1] *= rescale_x;
#ifdef NOTDEF
	/* This would force the pointsize and pixelsize fields in the
	   FONT property to display as matrices to more accurately
	   report the font being supplied.  It might also break existing
	   applications that expect a single number in that field. */
	vals->values_supplied =
	    vals->values_supplied & ~(PIXELSIZE_MASK | POINTSIZE_MASK) |
	    PIXELSIZE_ARRAY;
#else /* NOTDEF */
	vals->values_supplied = vals->values_supplied & ~POINTSIZE_MASK;
#endif /* NOTDEF */
	/* Recompute and reround the FontScalablePtr values after
	   rescaling for the new width. */
	FontFileCompleteXLFD(vals, vals);
    }

    return result;
}

static long
doround(x)
double x;
{
    return (x >= 0) ? (long)(x + .5) : (long)(x - .5);
}

static int
computeProps(pf, wasStringProp, npf, isStringProp, nprops, xfactor, yfactor,
	     sXfactor, sYfactor)
    FontPropPtr pf;
    char	*wasStringProp;
    FontPropPtr npf;
    char	*isStringProp;
    unsigned int nprops;
    double      xfactor, sXfactor,
                yfactor, sYfactor;
{
    int         n;
    int         count;
    fontProp   *t;
    double      rawfactor;

    for (count = 0; nprops > 0; nprops--, pf++, wasStringProp++) {
	n = sizeof(fontPropTable) / sizeof(fontProp);
	for (t = fontPropTable; n && (t->atom != pf->name); n--, t++);
	if (!n)
	    continue;

	switch (t->type) {
	case scaledX:
	    npf->value = doround(xfactor * (double)pf->value);
	    rawfactor = sXfactor;
	    break;
	case scaledY:
	    npf->value = doround(yfactor * (double)pf->value);
	    rawfactor = sYfactor;
	    break;
	case unscaled:
	    npf->value = pf->value;
	    npf->name = pf->name;
	    npf++;
	    count++;
	    *isStringProp++ = *wasStringProp;
	    break;
	}
	if (t->type != unscaled)
	{
	    npf->name = pf->name;
	    npf++;
	    count++;
	    npf->value = doround(rawfactor * (double)pf->value);
	    npf->name = rawFontPropTable[t - fontPropTable].atom;
	    npf++;
	    count++;
	    *isStringProp++ = *wasStringProp;
	    *isStringProp++ = *wasStringProp;
	}
    }
    return count;
}


static int
ComputeScaledProperties(sourceFontInfo, name, vals, dx, dy, sdx, sdy,
			sWidth, pProps, pIsStringProp)
    FontInfoPtr     sourceFontInfo;	/* the font to be scaled */
    char	    *name;		/* name of resulting font */
    FontScalablePtr vals;
    double	    dx, sdx,
		    dy, sdy;	/* scale factors in x and y directions */
    long	    sWidth;	/* 1000-pixel average width */
    FontPropPtr	    *pProps;	/* returns properties; preallocated */
    char	    **pIsStringProp;  /* return booleans; preallocated */
{
    int         n;
    char       *ptr1,
               *ptr2;
    char       *ptr3;
    FontPropPtr fp;
    fontProp   *fpt;
    extern int  serverGeneration;
    char	*isStringProp;
    int		nProps;

    if (fontGeneration != serverGeneration) {
	initFontPropTable();
	fontGeneration = serverGeneration;
    }
    nProps = NPROPS + 1 + sizeof(fontPropTable) / sizeof(fontProp) +
			  sizeof(rawFontPropTable) / sizeof(fontProp);
    fp = (FontPropPtr) xalloc(sizeof(FontPropRec) * nProps);
    *pProps = fp;
    if (!fp)
	return 1;
    isStringProp = (char *) xalloc (nProps);
    *pIsStringProp = isStringProp;
    if (!isStringProp)
    {
	xfree (fp);
	return 1;
    }
    ptr2 = name;
    for (fpt = fontNamePropTable, n = NPROPS;
	 n;
 	 fp++, fpt++, n--, isStringProp++)
    {

	if (*ptr2)
	{
	    ptr1 = ptr2 + 1;
	    if (!(ptr2 = strchr(ptr1, '-'))) ptr2 = strchr(ptr1, '\0');
	}

	*isStringProp = 0;
	switch (fpt->type) {
	case atom:
	    fp->value = MakeAtom(ptr1, ptr2 - ptr1, TRUE);
	    *isStringProp = 1;
	    break;
	case truncate_atom:
	    for (ptr3 = ptr1; *ptr3; ptr3++)
		if (*ptr3 == '=' || *ptr3 == '-' ||
		    *ptr3 == '+' || *ptr3 == '~')
		    break;
	    if (!*ptr3) ptr3 = ptr2;
	    fp->value = MakeAtom(ptr1, ptr3 - ptr1, TRUE);
	    *isStringProp = 1;
	    break;
	case pixel_size:
	    fp->value = doround(vals->pixel_matrix[3]);
	    break;
	case point_size:
	    fp->value = doround(vals->point_matrix[3] * 10.0);
	    break;
	case resolution_x:
	    fp->value = vals->x;
	    break;
	case resolution_y:
	    fp->value = vals->y;
	    break;
	case average_width:
	    fp->value = vals->width;
	    break;
	case fontname:
	    fp->value = MakeAtom(name, strlen(name), TRUE);
	    *isStringProp = 1;
	    break;
	case raw_ascent:
	    fp->value = sourceFontInfo->fontAscent * sdy;
	    break;
	case raw_descent:
	    fp->value = sourceFontInfo->fontDescent * sdy;
	    break;
	case raw_pointsize:
	    fp->value = (long)(72270.0 / (double)vals->y + .5);
	    break;
	case raw_pixelsize:
	    fp->value = 1000;
	    break;
	case raw_average_width:
	    fp->value = sWidth;
	    break;
	}
	fp->name = fpt->atom;
    }
    n = NPROPS;
    n += computeProps(sourceFontInfo->props, sourceFontInfo->isStringProp,
		      fp, isStringProp, sourceFontInfo->nprops, dx, dy,
		      sdx, sdy);
    return n;
}

static void ScaleBitmap();

static int
compute_xform_matrix(vals, dx, dy, xform, inv_xform, xmult, ymult)
    FontScalablePtr	vals;
    double		dx, dy, *inv_xform, *xmult, *ymult;
    register double	*xform;
{
    double det, sintheta, costheta, tanphi;
    double pixel = get_matrix_vertical_component(vals->pixel_matrix);
    double pixelset = get_matrix_horizontal_component(vals->pixel_matrix);

    if (pixel < EPS || pixelset < EPS) return 0;

    /* Initialize the transformation matrix to the scaling factors */
    xform[0] = dx / pixelset;
    xform[1] = xform[2] = 0.0;
    xform[3] = dy / pixel;

/* Inline matrix multiply -- somewhat ugly to minimize register usage */
#define MULTIPLY_XFORM(a,b,c,d) \
{ \
  register double aa = (a), bb = (b), cc = (c), dd = (d); \
  register double temp; \
  temp =     aa * xform[0] + cc * xform[1]; \
  aa =       aa * xform[2] + cc * xform[3]; \
  xform[1] = bb * xform[0] + dd * xform[1]; \
  xform[3] = bb * xform[2] + dd * xform[3]; \
  xform[0] = temp; \
  xform[2] = aa; \
}

    /* Rescale the transformation matrix for size of source font */
    MULTIPLY_XFORM(vals->pixel_matrix[0],
		   vals->pixel_matrix[1],
		   vals->pixel_matrix[2],
		   vals->pixel_matrix[3]);

    *xmult = xform[0];
    *ymult = xform[3];


    if (inv_xform == NULL) return 1;

    /* Compute the determinant for use in inverting the matrix. */
    det = xform[0] * xform[3] - xform[1] * xform[2];

    /* If the determinant is tiny or zero, give up */
    if (fabs(det) < EPS) return 0;

    /* Compute the inverse */
    inv_xform[0] = xform[3] / det;
    inv_xform[1] = -xform[1] / det;
    inv_xform[2] = -xform[2] / det;
    inv_xform[3] = xform[0] / det;

    return 1;
}

/*
 *  ScaleFont
 *  returns a pointer to the new scaled font, or NULL (due to AllocError).
 */
FontPtr
ScaleFont(opf, widthMult, heightMult, sWidthMult, sHeightMult, vals,
	  newWidthMult, newHeightMult, sWidth)
    FontPtr     opf;		/* originating font */
    double      widthMult;	/* glyphs width scale factor */
    double      heightMult;	/* glyphs height scale factor */
    double      sWidthMult;	/* scalable glyphs width scale factor */
    double      sHeightMult;	/* scalable glyphs height scale factor */
    FontScalablePtr	vals;
    double      *newWidthMult;	/* return: X component of glyphs width
				   scale factor */
    double      *newHeightMult;	/* return: Y component of glyphs height
				   scale factor */
    long	*sWidth;	/* return: average 1000-pixel width */
{
    FontPtr     pf;
    FontInfoPtr pfi,
                opfi;
    BitmapFontPtr  bitmapFont,
                obitmapFont;
    CharInfoPtr pci,
                opci;
    int         nchars;		/* how many characters in the font */
    char       *glyphBytes;
    unsigned    bytestoalloc;
    int        *scratch;
    int         i;
    int         glyph;
    int		firstCol, lastCol, firstRow, lastRow;
    double	xform[4], inv_xform[4];
    double	xmult, ymult;
    int		totalwidth = 0, totalchars = 0;
    int		inkindex1, inkindex2;
#define OLDINDEX(i) (((i)/(lastCol - firstCol + 1) + \
		      firstRow - opf->info.firstRow) * \
		     (opf->info.lastCol - opf->info.firstCol + 1) + \
		     (i)%(lastCol - firstCol + 1) + \
		     firstCol - opf->info.firstCol)

    extern int  bitmapGetBitmaps();
    extern int  bitmapGetExtents();
    extern int  bitmapGetGlyphs();
    extern int  bitmapGetMetrics();

    *sWidth = 0;

    opfi = &opf->info;
    glyph = opf->glyph;
    obitmapFont = (BitmapFontPtr) opf->fontPrivate;

    bitmapFont = 0;
    pf = (FontPtr) xalloc(sizeof(FontRec));
    if (!pf)
	goto bail;
    pf->refcnt = 0;
    pf->maxPrivate = -1;
    pf->devPrivates = (pointer *) 0;
    pf->bit = opf->bit;
    pf->byte = opf->byte;
    pf->glyph = opf->glyph;
    pf->scan = opf->scan;

    pf->get_glyphs = bitmapGetGlyphs;
    pf->get_metrics = bitmapGetMetrics;
    pf->unload_font = bitmapUnloadScalable;
    pf->unload_glyphs = NULL;

    pfi = &pf->info;
    *pfi = *opfi;
    /* If charset subsetting specified in vals, determine what our range
       needs to be for the output font */
    if (vals->nranges)
    {
	int i;

	pfi->allExist = 0;
	firstCol = 255;
	lastCol = 0;
	firstRow = 255;
	lastRow = 0;

	for (i = 0; i < vals->nranges; i++)
	{
	    if (vals->ranges[i].min_char_high != vals->ranges[i].max_char_high)
	    {
		firstCol = opfi->firstCol;
		lastCol = opfi->lastCol;
	    }
	    if (firstCol > vals->ranges[i].min_char_low)
		firstCol = vals->ranges[i].min_char_low;
	    if (lastCol < vals->ranges[i].max_char_low)
		lastCol = vals->ranges[i].max_char_low;
	    if (firstRow > vals->ranges[i].min_char_high)
		firstRow = vals->ranges[i].min_char_high;
	    if (lastRow < vals->ranges[i].max_char_high)
		lastRow = vals->ranges[i].max_char_high;
	}

	if (firstCol > lastCol || firstRow > lastRow)
	    goto bail;

	if (firstCol < opfi->firstCol)
	    firstCol = opfi->firstCol;
	if (lastCol > opfi->lastCol)
	    lastCol = opfi->lastCol;
	if (firstRow < opfi->firstRow)
	    firstRow = opfi->firstRow;
	if (lastRow > opfi->lastRow)
	    lastRow = opfi->lastRow;
    }
    else
    {
	firstCol = opfi->firstCol;
	lastCol = opfi->lastCol;
	firstRow = opfi->firstRow;
	lastRow = opfi->lastRow;
    }

    bitmapFont = (BitmapFontPtr) xalloc(sizeof(BitmapFontRec));
    if (!bitmapFont)
	goto bail;
    nchars = (lastRow - firstRow + 1) * (lastCol - firstCol + 1);
    pfi->firstRow = firstRow;
    pfi->lastRow = lastRow;
    pfi->firstCol = firstCol;
    pfi->lastCol = lastCol;
    pf->fontPrivate = (pointer) bitmapFont;
    bitmapFont->version_num = obitmapFont->version_num;
    bitmapFont->num_chars = nchars;
    bitmapFont->num_tables = obitmapFont->num_tables;
    bitmapFont->metrics = 0;
    bitmapFont->ink_metrics = 0;
    bitmapFont->bitmaps = 0;
    bitmapFont->encoding = 0;
    bitmapFont->bitmapExtra = 0;
    bitmapFont->pDefault = 0;
    bitmapFont->metrics = (CharInfoPtr) xalloc(nchars * sizeof(CharInfoRec));
    if (!bitmapFont->metrics)
	goto bail;
    bitmapFont->encoding = (CharInfoPtr *) xalloc(nchars * sizeof(CharInfoPtr));
    if (!bitmapFont->encoding)
	goto bail;

    bytestoalloc = 0;

#undef MAXSHORT
#define MAXSHORT    32767
#undef MINSHORT
#define MINSHORT    -32768

    pfi->anamorphic = FALSE;
    if (heightMult != widthMult)
	pfi->anamorphic = TRUE;
    pfi->cachable = TRUE;

    if (!compute_xform_matrix(vals, widthMult, heightMult, xform,
			      inv_xform, &xmult, &ymult))
	goto bail;

    pfi->fontAscent = opfi->fontAscent * ymult;
    pfi->fontDescent = opfi->fontDescent * ymult;

    pfi->minbounds.leftSideBearing = MAXSHORT;
    pfi->minbounds.rightSideBearing = MAXSHORT;
    pfi->minbounds.ascent = MAXSHORT;
    pfi->minbounds.descent = MAXSHORT;
    pfi->minbounds.characterWidth = MAXSHORT;
    pfi->minbounds.attributes = MAXSHORT;

    pfi->maxbounds.leftSideBearing = MINSHORT;
    pfi->maxbounds.rightSideBearing = MINSHORT;
    pfi->maxbounds.ascent = MINSHORT;
    pfi->maxbounds.descent = MINSHORT;
    pfi->maxbounds.characterWidth = MINSHORT;
    pfi->maxbounds.attributes = MINSHORT;

    /* Compute the transformation and inverse transformation matrices.
       Can fail if the determinant is zero. */

    inkindex1 = 0;
    pci = bitmapFont->metrics;
    for (i = 0; i < nchars; i++)
    {
	if (opci = obitmapFont->encoding[inkindex2 = OLDINDEX(i)])
	{
	    double newlsb, newrsb, newdesc, newasc, point[2];

#define minchar(p) ((p).min_char_low + ((p).min_char_high << 8))
#define maxchar(p) ((p).max_char_low + ((p).max_char_high << 8))

	    if (vals->nranges)
	    {
		int row = i / (lastCol - firstCol + 1) + firstRow;
		int col = i % (lastCol - firstCol + 1) + firstCol;
		int ch = (row << 8) + col;
		int j;
		for (j = 0; j < vals->nranges; j++)
		    if (ch >= minchar(vals->ranges[j]) &&
			ch <= maxchar(vals->ranges[j]))
			break;
		if (j == vals->nranges)
		{
		    bitmapFont->encoding[i] = 0;
		    continue;
		}
	    }

	    if (opci->metrics.leftSideBearing == 0 &&
		opci->metrics.rightSideBearing == 0 &&
		opci->metrics.ascent == 0 &&
		opci->metrics.descent == 0 &&
		opci->metrics.characterWidth == 0)
	    {
		bitmapFont->encoding[i] = 0;
		continue;
	    }

	    bitmapFont->encoding[i] = pci;

	    /* Compute new extents for this glyph */
	    TRANSFORM_POINT(xform,
			    opci->metrics.leftSideBearing,
			    -opci->metrics.descent,
			    point);
	    newlsb = point[0];
	    newrsb = newlsb;
	    newdesc = -point[1];
	    newasc = -newdesc;
	    TRANSFORM_POINT(xform,
			    opci->metrics.leftSideBearing,
			    opci->metrics.ascent,
			    point);
	    CHECK_EXTENT(newlsb, newrsb, newdesc, newasc, point);
	    TRANSFORM_POINT(xform,
			    opci->metrics.rightSideBearing,
			    -opci->metrics.descent,
			    point);
	    CHECK_EXTENT(newlsb, newrsb, newdesc, newasc, point);
	    TRANSFORM_POINT(xform,
			    opci->metrics.rightSideBearing,
			    opci->metrics.ascent,
			    point);
	    CHECK_EXTENT(newlsb, newrsb, newdesc, newasc, point);

	    pci->metrics.leftSideBearing = (int)floor(newlsb);
	    pci->metrics.rightSideBearing = (int)floor(newrsb + .5);
	    pci->metrics.descent = (int)ceil(newdesc);
	    pci->metrics.ascent = (int)floor(newasc + .5);
	    /* Accumulate total width of characters before transformation,
	       to ascertain predominant direction of font. */
	    totalwidth += opci->metrics.characterWidth;
	    pci->metrics.characterWidth =
		doround((double)opci->metrics.characterWidth * xmult);
	    pci->metrics.attributes =
		doround((double)opci->metrics.characterWidth * sWidthMult);
	    if (!pci->metrics.characterWidth)
	    {
		/* Since transformation may shrink width, height, and
		   escapement to zero, make sure existing characters
		   are not mistaken for undefined characters. */

		if (pci->metrics.rightSideBearing ==
		    pci->metrics.leftSideBearing)
		    pci->metrics.rightSideBearing++;
		if (pci->metrics.ascent == -pci->metrics.descent)
		    pci->metrics.ascent++;
	    }
    
    
	    bytestoalloc += BYTES_FOR_GLYPH(pci, glyph);
	    pci++;
	}
	else
	    bitmapFont->encoding[i] = 0;
    }
    bitmapFont->bitmaps = (char *) xalloc(bytestoalloc);
    if (!bitmapFont->bitmaps)
	goto bail;
    bzero(bitmapFont->bitmaps, bytestoalloc);


    /*
     * For each character, set the per-character metrics, scale the glyph, and
     * check per-font minbounds and maxbounds character information.
     */

    glyphBytes = bitmapFont->bitmaps;
    pci = bitmapFont->metrics;
    for (i = 0; i < nchars; i++)
    {
	CharInfoRec temppci;
	if ((pci = bitmapFont->encoding[i]) &&
	    (opci = obitmapFont->encoding[OLDINDEX(i)]))
	{
	    pci = bitmapFont->encoding[i];
	    pci->bits = glyphBytes;
	    ScaleBitmap (pf, opci, pci, inv_xform,
			 widthMult, heightMult);
	    totalchars++;
	    *sWidth += abs((int)(INT16)pci->metrics.attributes);
	    glyphBytes += BYTES_FOR_GLYPH(pci, glyph);
#define MINMAX(field) \
	    if (pfi->minbounds.field > pci->metrics.field) \
	    	pfi->minbounds.field = pci->metrics.field; \
	    if (pfi->maxbounds.field < pci->metrics.field) \
	    	pfi->maxbounds.field = pci->metrics.field
    
	    MINMAX(leftSideBearing);
	    MINMAX(rightSideBearing);
	    MINMAX(ascent);
	    MINMAX(descent);
	    MINMAX(characterWidth);

	    /* Hack: Cast attributes into a signed quantity.  Tread lightly
	       for now and don't go changing the global Xproto.h file */
	    if ((INT16)pfi->minbounds.attributes >
		(INT16)pci->metrics.attributes)
	    	pfi->minbounds.attributes = pci->metrics.attributes;
	    if ((INT16)pfi->maxbounds.attributes <
		(INT16)pci->metrics.attributes)
	    	pfi->maxbounds.attributes = pci->metrics.attributes;
#undef MINMAX
	}
    }
    pfi->ink_minbounds = pfi->minbounds;
    pfi->ink_maxbounds = pfi->maxbounds;
    if (totalchars)
    {
	*sWidth = (*sWidth * 10 + totalchars / 2) / totalchars;
	if (totalwidth < 0)
	{
	    /* Dominant direction is R->L */
	    *sWidth = -*sWidth;
	}

	vals->width = doround((double)*sWidth * vals->pixel_matrix[0] / 1000.0);
    }
    else
    {
	vals->width = 0;
	*sWidth = 0;
    }
    FontComputeInfoAccelerators (pfi);

    if (pfi->defaultCh != (unsigned short) NO_SUCH_CHAR) {
	unsigned int r,
	            c,
	            cols;

	r = pfi->defaultCh >> 8;
	c = pfi->defaultCh & 0xFF;
	if (pfi->firstRow <= r && r <= pfi->lastRow &&
		pfi->firstCol <= c && c <= pfi->lastCol) {
	    cols = pfi->lastCol - pfi->firstCol + 1;
	    r = r - pfi->firstRow;
	    c = c - pfi->firstCol;
	    bitmapFont->pDefault = bitmapFont->encoding[r * cols + c];
	}
    }

    *newWidthMult = xmult;
    *newHeightMult = ymult;
    return pf;
bail:
    if (pf)
	xfree(pf);
    if (bitmapFont) {
	xfree(bitmapFont->metrics);
	xfree(bitmapFont->ink_metrics);
	xfree(bitmapFont->bitmaps);
	xfree(bitmapFont->encoding);
    }
    return NULL;
}

static int
lcm(a, b)			/* least common multiple */
    int         a,
                b;
{
    register int m;
    register int larger,
                smaller;

    if (a > b) {
	m = larger = a;
	smaller = b;
    } else {
	m = larger = b;
	smaller = a;
    }

    while (m % smaller)
	m += larger;
    return m;
}

static void
ScaleBitmap(pFont, opci, pci, inv_xform, widthMult, heightMult)
    FontPtr     pFont;
    CharInfoPtr opci;
    CharInfoPtr pci;
    double     *inv_xform;
    double	widthMult;
    double	heightMult;
{
    register char  *bitmap,		/* The bits */
               *newBitmap;
    register int   bpr,			/* Padding information */
		newBpr;
    int         width,			/* Extents information */
                height,
                newWidth,
                newHeight;
    register int row,			/* Loop variables */
		col;
    INT32	deltaX,			/* Increments for resampling loop */
		deltaY;
    INT32	xValue,			/* Subscripts for resampling loop */
		yValue;
    double	point[2];
    unsigned char *char_grayscale = 0;
    INT32	*diffusion_workspace, *thisrow, *nextrow, pixmult;
    int		box_x, box_y;

    static unsigned char masklsb[] =
	{ 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
    static unsigned char maskmsb[] =
	{ 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1 };
    unsigned char	*mask = (pFont->bit == LSBFirst ? masklsb : maskmsb);


    bitmap = opci->bits;
    newBitmap = pci->bits;
    width = GLYPHWIDTHPIXELS(opci);
    height = GLYPHHEIGHTPIXELS(opci);
    newWidth = GLYPHWIDTHPIXELS(pci);
    newHeight = GLYPHHEIGHTPIXELS(pci);
    if (!newWidth || !newHeight || !width || !height)
	return;

    bpr = BYTES_PER_ROW(width, pFont->glyph);
    newBpr = BYTES_PER_ROW(newWidth, pFont->glyph);

    if (widthMult > 0.0 && heightMult > 0.0 &&
	(widthMult < 1.0 || heightMult < 1.0))
    {
	/* We are reducing in one or both dimensions.  In an attempt to
	   reduce aliasing, we'll antialias by passing the original
	   glyph through a low-pass box filter (which results in a
	   grayscale image), then use error diffusion to create bitonal
	   output in the resampling loop.  */

	/* First compute the sizes of the box filter */
	widthMult = ceil(1.0 / widthMult);
	heightMult = ceil(1.0 / heightMult);
	box_x = width / 2;
	box_y = height / 2;
	if (widthMult < (double)box_x) box_x = (int)widthMult;
	if (heightMult < (double)box_y) box_y = (int)heightMult;
	/* The pixmult value (below) is used to darken the image before
	   we perform error diffusion: a necessary concession to the
	   fact that it's very difficult to generate readable halftoned
	   glyphs.  The degree of darkening is proportional to the size
	   of the blurring filter, hence inversely proportional to the
	   darkness of the lightest gray that results from antialiasing.
	   The result is that characters that exercise this logic (those
	   generated by reducing from a larger source font) tend to err
	   on the side of being too bold instead of being too light to
	   be readable. */
	pixmult = box_x * box_y * 192;

	if (box_x > 1 || box_y > 1)
	{
	    /* Looks like we need to anti-alias.  Create a workspace to
	       contain the grayscale character plus an additional row and
	       column for scratch */
	    char_grayscale =
		(unsigned char *)xalloc((width + 1) * (height + 1));
	    if (char_grayscale)
	    {
		diffusion_workspace =
		    (INT32 *)xalloc((newWidth + 2) * 2 * sizeof(int));
		if (!diffusion_workspace)
		{
		    xfree(char_grayscale);
		    char_grayscale = (unsigned char *)0;
		}
		/* Initialize our error diffusion workspace for later use */
		bzero((char *)diffusion_workspace + sizeof(INT32),
		      (newWidth + 3) * sizeof(int));
		thisrow = diffusion_workspace + 1;
		nextrow = diffusion_workspace + newWidth + 3;
	    }
	}
    }

    if (char_grayscale)
    {
	/* We will be doing antialiasing.  First copy the bitmap into
	   our buffer, mapping input range [0,1] to output range
	   [0,255].  */
	register unsigned char *srcptr, *dstptr;
	srcptr = (unsigned char *)bitmap;
	dstptr = char_grayscale;
	for (row = 0; row < height; row++)
	{
	    for (col = 0; col < width; col++)
		*dstptr++ = (srcptr[col >> 3] & mask[col & 0x7]) ? 255 : 0;
	    srcptr += bpr;	/* On to next row of source */
	    dstptr++;		/* Skip scratch column in dest */
	}
	if (box_x > 1)
	{
	    /* Our box filter has a width > 1... let's filter the rows */

	    int right_width = box_x / 2;
	    int left_width = box_x - right_width - 1;

	    for (row = 0; row < height; row++)
	    {
		int sum = 0;
		int left_size = 0, right_size = 0;

		srcptr = char_grayscale + (width + 1) * row;
		dstptr = char_grayscale + (width + 1) * height; /* scratch */

		/* We've computed the shape of our full box filter.  Now
		   compute the right-hand part of the moving sum */
		for (right_size = 0; right_size < right_width; right_size++)
		    sum += srcptr[right_size];

		/* Now start moving the sum, growing the box filter, and
		   dropping averages into our scratch buffer */
		for (left_size = 0; left_size < left_width; left_size++)
		{
		    sum += srcptr[right_width];
		    *dstptr++ = sum / (left_size + right_width + 1);
		    srcptr++;
		}

		/* The box filter has reached full width... continue
		   computation of moving average until the right side
		   hits the wall. */
		for (col = left_size; col + right_size < width; col++)
		{
		    sum += srcptr[right_width];
		    *dstptr++ = sum / box_x;
		    sum -= srcptr[-left_width];
		    srcptr++;
		}

		/* Collapse the right side of the box filter */
		for (; right_size > 0; right_size--)
		{
		    *dstptr++ = sum / (left_width + right_size);
		    sum -= srcptr[-left_width];
		    srcptr++;
		}

		/* Done with the row... copy dest back over source */
		memmove(char_grayscale + (width + 1) * row,
			char_grayscale + (width + 1) * height,
			width);
	    }
	}
	if (box_y > 1)
	{
	    /* Our box filter has a height > 1... let's filter the columns */

	    int bottom_height = box_y / 2;
	    int top_height = box_y - bottom_height - 1;

	    for (col = 0; col < width; col++)
	    {
		int sum = 0;
		int top_size = 0, bottom_size = 0;

		srcptr = char_grayscale + col;
		dstptr = char_grayscale + width;	 /* scratch */

		/* We've computed the shape of our full box filter.  Now
		   compute the bottom part of the moving sum */
		for (bottom_size = 0;
		     bottom_size < bottom_height;
		     bottom_size++)
		    sum += srcptr[bottom_size * (width + 1)];

		/* Now start moving the sum, growing the box filter, and
		   dropping averages into our scratch buffer */
		for (top_size = 0; top_size < top_height; top_size++)
		{
		    sum += srcptr[bottom_height * (width + 1)];
		    *dstptr = sum / (top_size + bottom_height + 1);
		    dstptr += width + 1;
		    srcptr += width + 1;
		}

		/* The box filter has reached full height... continue
		   computation of moving average until the bottom
		   hits the wall. */
		for (row = top_size; row + bottom_size < height; row++)
		{
		    sum += srcptr[bottom_height * (width + 1)];
		    *dstptr = sum / box_y;
		    dstptr += width + 1;
		    sum -= srcptr[-top_height * (width + 1)];
		    srcptr += width + 1;
		}

		/* Collapse the bottom of the box filter */
		for (; bottom_size > 0; bottom_size--)
		{
		    *dstptr = sum / (top_height + bottom_size);
		    dstptr += width + 1;
		    sum -= srcptr[-top_height * (width + 1)];
		    srcptr += width + 1;
		}

		/* Done with the column... copy dest back over source */

		dstptr = char_grayscale + col;
		srcptr = char_grayscale + width;	 /* scratch */
		for (row = 0; row < height; row++)
		{
		    *dstptr = *srcptr;
		    dstptr += width + 1;
		    srcptr += width + 1;
		}
	    }
	}

	/* Increase the grayvalue to increase ink a bit */
	srcptr = char_grayscale;
	for (row = 0; row < height; row++)
	{
	    for (col = 0; col < width; col++)
	    {
		register int pixvalue = (int)*srcptr * pixmult / 256;
		if (pixvalue > 255) pixvalue = 255;
		*srcptr = pixvalue;
		srcptr++;
	    }
	    srcptr++;
	}
    }

    /* Compute the increment values for the resampling loop */
    TRANSFORM_POINT(inv_xform, 1, 0, point);
    deltaX = (INT32)(point[0] * 65536.0);
    deltaY = (INT32)(-point[1] * 65536.0);

    /* Resampling loop:  resamples original glyph for generation of new
       glyph in transformed coordinate system. */

    for (row = 0; row < newHeight; row++)
    {
	/* Compute inverse transformation for start of this row */
	TRANSFORM_POINT(inv_xform,
			(double)(pci->metrics.leftSideBearing) + .5,
			(double)(pci->metrics.ascent - row) - .5,
			point);

	/* Adjust for coordinate system to get resampling point */
	point[0] -= opci->metrics.leftSideBearing;
	point[1] = opci->metrics.ascent - point[1];

	/* Convert to integer coordinates */
	xValue = (INT32)(point[0] * 65536.0);
	yValue = (INT32)(point[1] * 65536.0);

	if (char_grayscale)
	{
	    INT32 *temp;
	    for (col = 0; col < newWidth; col++)
	    {
		register int x = xValue >> 16, y = yValue >> 16;
		int pixvalue, error;
    
		pixvalue = ((x >= 0 && x < width && y >= 0 && y < height) ?
			    char_grayscale[x + y * (width + 1)] : 0) +
			   thisrow[col] / 16;
		if (pixvalue > 255) pixvalue = 255;
		else if (pixvalue < 0) pixvalue = 0;

		/* Choose the bit value and set resulting error value */
		if (pixvalue >= 128)
		{
		    newBitmap[(col >> 3) + row * newBpr] |= mask[col & 0x7];
		    error = pixvalue - 255;
		}
		else
		    error = -pixvalue;

		/* Diffuse the error */
		thisrow[col + 1] += error * 7;
		nextrow[col - 1] += error * 3;
		nextrow[col] += error * 5;
		nextrow[col + 1] = error;

		xValue += deltaX;
		yValue += deltaY;
	    }

	    /* Add in error values that fell off either end */
	    nextrow[0] += nextrow[-1];
	    nextrow[newWidth - 2] += thisrow[newWidth];
	    nextrow[newWidth - 1] += nextrow[newWidth];
	    nextrow[newWidth] = 0;

	    temp = nextrow;
	    nextrow = thisrow;
	    thisrow = temp;
	    nextrow[-1] = nextrow[0] = 0;
	}
	else
	{
	    for (col = 0; col < newWidth; col++)
	    {
		register int x = xValue >> 16, y = yValue >> 16;
    
		if (x >= 0 && x < width && y >= 0 && y < height)
		{
		    /* Use point-sampling for rescaling. */

		    if (bitmap[(x >> 3) + y * bpr] & mask[x & 0x7])
			newBitmap[(col >> 3) + row * newBpr] |= mask[col & 0x7];
		}

		xValue += deltaX;
		yValue += deltaY;
	    }
	}
    }


    if (char_grayscale)
    {
	xfree(char_grayscale);
	xfree(diffusion_workspace);
    }
}

#ifdef NOTDEF
/*
 *	exported interfaces
 */

FontFileLoadName(dirs, ndirs, name, pfont, format, fmask)
    FontFileDirPtr *dirs;
    int         ndirs;
    char       *name;
    FontPtr    *pfont;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
{
    FontFileNamePtr fname;
    char        full_name[1024];
    int         ret = BadFontName;
    int         i;

    i = 0;
    while (i < ndirs) {
	if (fname = FontFileFindNameInDir(dirs[i], name)) {
	    if (!fname->alias) {
		if (!fname->font) {
		    strcpy(full_name, dirs[i]->dir);
		    strcat(full_name, fname->file);
		    ret = FontFileLoad(pfont, full_name, format, fmask);
		    if (ret == Successful) {
			fname->font = *pfont;
			(*pfont)->fpePrivate = (pointer) fname;
		    }
		    return ret;
		}
		*pfont = fname->font;
		return Successful;
	    }
	    name = fname->file;
	    i = 0;
	} else
	    i++;
    }
    return BadFontName;
}
#endif

/* ARGSUSED */
BitmapOpenScalable (fpe, pFont, flags, entry, fileName, vals, format, fmask,
		    non_cachable_font)
    FontPathElementPtr	fpe;
    FontPtr		*pFont;
    int			flags;
    FontEntryPtr	entry;
    char		*fileName;  /* unused */
    FontScalablePtr	vals;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
    FontPtr		non_cachable_font;	/* We don't do licensing */
{
    FontScalableRec	best;
    FontPtr		font = NullFont;
    double		dx, sdx,
			dy, sdy;
    FontPropPtr		props;
    char		*isStringProp;
    int			propCount;
    int			status;
    long		sWidth;

    FontEntryPtr	scaleFrom;
    FontPathElementPtr	scaleFPE;
    FontPtr		sourceFont;
    char		fontName[MAXFONTNAMELEN];

    /* Can't deal with mix-endian fonts yet */

#ifdef NOTDEF /* XXX need better test */
    if ((format & BitmapFormatByteOrderMask) !=
	    (format & BitmapFormatBitOrderMask))
	return NullFontFileName;
#endif

    /* Reject outrageously small font sizes to keep the math from
       blowing up. */
    if (get_matrix_vertical_component(vals->pixel_matrix) < 1.0 ||
	get_matrix_horizontal_component(vals->pixel_matrix) < 1.0)
	return BadFontName;

    scaleFrom = FindBestToScale (fpe, entry, vals, &best, &dx, &dy,
				 &sdx, &sdy, &scaleFPE);
    if (!scaleFrom)
	return BadFontName;

    status = FontFileOpenBitmap(scaleFPE, &sourceFont, LoadAll, scaleFrom,
				format, fmask);

    if (status != Successful)
	return BadFontName;

    if (!vals->width)
	vals->width = best.width * dx;

    /* Compute the scaled font */

    font = ScaleFont(sourceFont, dx, dy, sdx, sdy, vals, &dx, &dy, &sWidth);

    if (!font)
    {
	if (!sourceFont->refcnt)
	    FontFileCloseFont((FontPathElementPtr) 0, sourceFont);
	return AllocError;
    }

    /* Prepare font properties for the new font */

    strcpy (fontName, scaleFrom->name.name);
    FontParseXLFDName (fontName, vals, FONT_XLFD_REPLACE_VALUE);

    propCount = ComputeScaledProperties(&sourceFont->info, fontName, vals,
					dx, dy, sdx, sdy, sWidth, &props,
					&isStringProp);

    if (!sourceFont->refcnt)
	FontFileCloseFont((FontPathElementPtr) 0, sourceFont);

    if (propCount && (!props || !isStringProp))
    {
	font->info.nprops = 0;
	font->info.props = (FontPropPtr)0;
	font->info.isStringProp = (char *)0;
	bitmapUnloadScalable(font);
	return AllocError;
    }

    font->info.props = props;
    font->info.nprops = propCount;
    font->info.isStringProp = isStringProp;

    *pFont = font;
    return Successful;
}

BitmapGetInfoScalable (fpe, pFontInfo, entry, fontName, fileName, vals)
    FontPathElementPtr	fpe;
    FontInfoPtr		pFontInfo;
    FontEntryPtr	entry;
    FontNamePtr		fontName;
    char		*fileName;
    FontScalablePtr	vals;
{
    FontPtr pfont;
    int flags = 0;
    long format = 0;  /* It doesn't matter what format for just info */
    long fmask = 0;
    int ret;

    ret = BitmapOpenScalable(fpe, &pfont, flags, entry, fileName, vals,
			     format, fmask, NULL);
    if (ret != Successful)
        return ret;
    *pFontInfo = pfont->info;

    pfont->info.props = NULL;
    pfont->info.isStringProp = NULL;

    (*pfont->unload_font)(pfont);
    return Successful;
}

void
bitmapUnloadScalable (pFont)
    FontPtr	    pFont;
{
    BitmapFontPtr   bitmapFont;
    FontInfoPtr	    pfi;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    pfi = &pFont->info;
    xfree (pfi->props);
    xfree (pfi->isStringProp);
    xfree (bitmapFont->encoding);
    xfree (bitmapFont->bitmaps);
    xfree (bitmapFont->ink_metrics);
    xfree (bitmapFont->metrics);
    xfree (pFont->fontPrivate);
    xfree (pFont->devPrivates);
    xfree (pFont);
}
