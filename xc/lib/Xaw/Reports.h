/*
 * $XConsortium: Reports.h,v 1.1 90/02/13 14:13:01 jim Exp $
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
 */

#ifndef _Xaw_Reports_h
#define _Xaw_Reports_h

/*
 * XawPannerReport - this structure is used by the reportCallback of the
 * Panner, Porthole, Viewport, and Scrollbar widgets to report its position.
 * All fields must be filled in, although the changed field may be used as
 * a hint as to which fields have been altered since the last report.
 */
typedef struct {
    unsigned int changed;		/* mask, see below */
    Position inner_x, inner_y;		/* location of inner within outer */
    Dimension inner_width, inner_height;  /* size of inner */
    Dimension outer_width, outer_height;  /* size of outer */
} XawPannerReport;

#define XawPRInnerX		(1)
#define XawPRInnerY		(2)
#define XawPRInnerWidth		(4)
#define XawPRInnerHeight	(8)
#define XawPROuterWidth		(16)
#define XawPROuterHeight	(32)
#define XawPRAll		(63)	/* union */

#define XtNreportCallback "reportCallback"
#define XtCReportCallback "reportCallback"

#endif /* _Xaw_Reports_h */
