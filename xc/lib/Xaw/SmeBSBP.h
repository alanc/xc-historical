/*
* $XConsortium: BSBMenuEnP.h,v 1.2 89/09/29 19:03:55 kit Exp $
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
 * MenuEntryP.h - Private definitions for MenuEntry object
 * 
 */

#ifndef _XawBSBMenuEntryP_h
#define _XawBSBMenuEntryP_h

/***********************************************************************
 *
 * MenuEntry Object Private Data
 *
 ***********************************************************************/

#include <X11/Xaw/MenuEntryP.h>
#include <X11/Xaw/BSBMenuEnt.h>

/************************************************************
 *
 * New fields for the MenuEntry Object class record.
 *
 ************************************************************/

typedef struct _BSBMenuEntryClassPart {
  XtPointer extension;
} BSBMenuEntryClassPart;

/* Full class record declaration */
typedef struct _BSBMenuEntryClassRec {
    RectObjClassPart       rect_class;
    MenuEntryClassPart     menu_entry_class;
    BSBMenuEntryClassPart  bsb_entry_class;
} BSBMenuEntryClassRec;

extern BSBMenuEntryClassRec bSBMenuEntryClassRec;

/* New fields for the MenuEntry Object record */
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

} BSBMenuEntryPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _BSBMenuEntryRec {
  ObjectPart         object;
  RectObjPart        rectangle;
  MenuEntryPart	     menu_entry;
  BSBMenuEntryPart   bsb_entry;
} BSBMenuEntryRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

#endif /* _XawBSBMenuEntryP_h */
