/* $Header$ */
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

/* types for Drawable */
#define DRAWABLE_WINDOW 0
#define DRAWABLE_PIXMAP 1
#define UNDRAWABLE_WINDOW -1

/* flags to PaintWindow() */
#define PW_BACKGROUND 0
#define PW_BORDER 1

#define NullPixmap ((PixmapPtr)0)
#define IS_VALID_PIXMAP(pPixmap) (\
    ((pPixmap != (PixmapPtr)None) && (pPixmap != (PixmapPtr)CopyFromParent) \
	&& (pPixmap != (PixmapPtr)USE_BORDER_PIXEL) \
	&& (pPixmap != (PixmapPtr)ParentRelative))? TRUE : FALSE)

typedef struct _DrawInfo *DrawablePtr;	
typedef struct _Pixmap *PixmapPtr;
#endif /* PIXMAP_H */
