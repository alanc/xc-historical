/*
 * $XConsortium$
 *
 * Copyright 1989 Massachusetts Institute of Technology
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
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifndef _XawPanner_h
#define _XawPanner_h

/* #include <X11/Xaw/Panner.h> */
#include "Panner.h"

typedef struct {			/* new fields in widget class */
    int dummy;
} PannerClassPart;

typedef struct _PannerClassRec {	/* Panner widget class */
    CoreClassPart core_class;
    ClassPart simple_class;
    PannerClassPart panner_class;
} PannerClassRec;

typedef struct {			/* new fields in widget */
    /* resources... */
    XtCallbackList callbacks;		/* callback/Callback */
    Pixel foreground;			/* foreground/Foreground */
    Dimension line_width;		/* lineWidth/LineWidth */
    Dimension canvas_width;		/* canvasWidth/CanvasWidth */
    Dimension canvas_height;		/* canvasHeight/CanvasHeight */
    Position slider_x;			/* sliderX/SliderX */
    Position slider_y;			/* sliderY/SliderY */
    Dimension slider_width;		/* sliderWidth/SliderWidth */
    Dimension slider_height;		/* sliderHeight/SliderHeight */
    Boolean stretch;			/* stretch/Stretch */
    GC draw_gc;				/* drawGC/DrawGC */
    GC erase_gc;			/* eraseGC/EraseGC */
    /* private data... */
    GC default_draw_gc;			/* if draw_gc is NULL */
    GC default_erase_gc;		/* if erase_gc is NULL */
    Position back_x, back_y;		/* real upper left of canvas image */
    Dimension back_width, back_height;	/* real size of canvas image */
} PannerPart;

typedef struct _PannerRec {
    CorePart core;
    SimplePart simple;
    PannerPart panner;
} PannerRec;


/*
 * external declarations
 */
extern PannerClassRec pannerClassRec;

#endif /* _XawPanner_h */
