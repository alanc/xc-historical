/* $XConsortium: pixmap.h,v 5.5 93/07/12 09:44:49 dpw Exp $ */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#ifndef PIXMAP_H
#define PIXMAP_H

#include "misc.h"
#include "screenint.h"

/* types for Drawable */
#define DRAWABLE_WINDOW 0
#define DRAWABLE_PIXMAP 1
#define UNDRAWABLE_WINDOW 2
#define DRAWABLE_BUFFER 3

/* flags to PaintWindow() */
#define PW_BACKGROUND 0
#define PW_BORDER 1

#define NullPixmap ((PixmapPtr)0)

typedef struct _Drawable *DrawablePtr;	
typedef struct _Pixmap *PixmapPtr;

typedef union _PixUnion {
    PixmapPtr		pixmap;
    unsigned long	pixel;
} PixUnion;

#define SamePixUnion(a,b,isPixel)\
    ((isPixel) ? (a).pixel == (b).pixel : (a).pixmap == (b).pixmap)

#define EqualPixUnion(as, a, bs, b)				\
    ((as) == (bs) && (SamePixUnion (a, b, as)))

#define OnScreenDrawable(type) \
	((type == DRAWABLE_WINDOW) || (type == DRAWABLE_BUFFER))

#define WindowDrawable(type) \
	((type == DRAWABLE_WINDOW) || (type == UNDRAWABLE_WINDOW))

extern PixmapPtr GetScratchPixmapHeader(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    int /*width*/,
    int /*height*/,
    int /*depth*/,
    int /*bitsPerPixel*/,
    int /*devKind*/,
    pointer /*pPixData*/
#endif
);

extern void FreeScratchPixmapHeader(
#if NeedFunctionPrototypes
    PixmapPtr /*pPixmap*/
#endif
);

extern Bool CreateScratchPixmapsForScreen(
#if NeedFunctionPrototypes
    int /*scrnum*/
#endif
);

extern void FreeScratchPixmapsForScreen(
#if NeedFunctionPrototypes
    int /*scrnum*/
#endif
);

extern PixmapPtr AllocatePixmap(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    int /*pixDataSize*/
#endif
);

#endif /* PIXMAP_H */
