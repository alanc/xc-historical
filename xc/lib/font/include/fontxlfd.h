/*
 * $XConsortium: fontxlfd.h,v 1.3 93/09/04 09:45:05 gildea Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
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

#ifndef _FONTXLFD_H_
#define _FONTXLFD_H_

#include "FSproto.h"

/* Constants for values_supplied bitmap */

#define SIZE_SPECIFY_MASK		0xf

#define PIXELSIZE_MASK			0x3
#define PIXELSIZE_UNDEFINED		0
#define PIXELSIZE_SCALAR		0x1
#define PIXELSIZE_ARRAY			0x2
#define PIXELSIZE_SCALAR_NORMALIZED	0x3	/* Adjusted for resolution */

#define POINTSIZE_MASK			0xc
#define POINTSIZE_UNDEFINED		0
#define POINTSIZE_SCALAR		0x4
#define POINTSIZE_ARRAY			0x8

#define PIXELSIZE_WILDCARD		0x10
#define POINTSIZE_WILDCARD		0x20

#define ENHANCEMENT_SPECIFY_MASK	0xc0

#define EMBOLDENING_SPECIFIED		0x40
#define CHARSUBSET_SPECIFIED		0x80

#define EPS		1.0e-20
#define XLFD_NDIGITS	3		/* Round numbers in pixel and
					   point arrays to this many
					   digits for repeatability */
double xlfd_round_double();

typedef struct _FontScalable {
    int		values_supplied;	/* Bitmap identifying what advanced
					   capabilities or enhancements
					   were specified in the font name */
    double	pixel_matrix[4];
    double	point_matrix[4];

    /* Pixel and point fields are deprecated in favor of the
       transformation matrices.  They are provided and filled in for the
       benefit of rasterizers that do not handle the matrices.  */

    int		pixel,
		point;

    int         x,
                y,
                width;
    /* Elements for HP enhancements to XLFD string */
    int		horiz_weight;
    int		vert_weight;
    char	*xlfdName;
    int		nranges;
    fsRange	*ranges;
}           FontScalableRec, *FontScalablePtr;

extern Bool FontParseXLFDName();
extern fsRange *FontParseRanges();

#define FONT_XLFD_REPLACE_NONE	0
#define FONT_XLFD_REPLACE_STAR	1
#define FONT_XLFD_REPLACE_ZERO	2
#define FONT_XLFD_REPLACE_VALUE	3

#endif				/* _FONTXLFD_H_ */
