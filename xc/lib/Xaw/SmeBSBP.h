/*
* $XConsortium: SmeBSBP.h,v 1.3 89/10/05 17:50:35 kit Exp $
*/


/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/* 
 * SmeP.h - Private definitions for Sme object
 * 
 */

#ifndef _XawSmeBSBP_h
#define _XawSmeBSBP_h

/***********************************************************************
 *
 * Sme Object Private Data
 *
 ***********************************************************************/

#include <X11/Xaw/SmeP.h>
#include <X11/Xaw/SmeBSB.h>

/************************************************************
 *
 * New fields for the Sme Object class record.
 *
 ************************************************************/

typedef struct _SmeBSBClassPart {
  XtPointer extension;
} SmeBSBClassPart;

/* Full class record declaration */
typedef struct _SmeBSBClassRec {
    RectObjClassPart       rect_class;
    SmeClassPart     sme_class;
    SmeBSBClassPart  sme_bsb_class;
} SmeBSBClassRec;

extern SmeBSBClassRec smeBSBClassRec;

/* New fields for the Sme Object record */
typedef struct {
    /* resources */
    String label;		/* The entry label. */
    int vert_space;		/* extra vert space to leave, as a percentage
				   of the font height of the label. */
    Pixmap left_bitmap, right_bitmap; /* bitmaps to show. */
    Dimension left_margin, right_margin; /* left and right margins. */
    Pixel foreground;		/* foreground color. */
    XFontStruct * font;		/* The font to show label in. */
    XtJustify justify;		/* Justification for the label. */

/* private resources. */

    GC norm_gc;			/* noral color gc. */
    GC rev_gc;			/* reverse color gc. */
    GC norm_gray_gc;		/* Normal color (grayed out) gc. */
    GC invert_gc;		/* gc for flipping colors. */

    Dimension left_bitmap_width; /* size of each bitmap. */
    Dimension left_bitmap_height;
    Dimension right_bitmap_width;
    Dimension right_bitmap_height;

} SmeBSBPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _SmeBSBRec {
  ObjectPart         object;
  RectObjPart        rectangle;
  SmePart	     sme;
  SmeBSBPart   sme_bsb;
} SmeBSBRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

#endif /* _XawSmeBSBP_h */
