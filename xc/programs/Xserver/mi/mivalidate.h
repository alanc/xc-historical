/* $XConsortium: mivalidate.h,v 1.1 93/09/28 17:55:26 dpw Exp $ */

/*
 * Copyright 1993 Massachusetts Institute of Technology
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
 */

#ifndef MIVALIDATE_H
#define MIVALIDATE_H

#include "miscstruct.h"
#include "regionstr.h"

typedef union _Validate {
    struct BeforeValidate {
	DDXPointRec	oldAbsCorner;	/* old window position */
	RegionPtr	borderVisible;	/* visible region of border, */
					/* non-null when size changes */
	Bool		resized;	/* unclipped winSize has changed - */
					/* don't call SaveDoomedAreas */
    } before;
    struct AfterValidate {
	RegionRec	exposed;	/* exposed regions, absolute pos */
	RegionRec	borderExposed;
    } after;
} ValidateRec;

#endif /* MIVALIDATE_H */
