/*
* $XConsortium: LineMenuEntryP.h,v 1.1 89/09/28 16:44:51 kit Exp $
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
 * LineMenuEntryP.h - Private definitions for LineMenuEntry widget
 * 
 */

#ifndef _XawLineMenuEntryP_h
#define _XawLineMenuEntryP_h

/***********************************************************************
 *
 * LineMenuEntry Widget Private Data
 *
 ***********************************************************************/

#include <X11/Xaw/MenuEntryP.h>
#include <X11/Xaw/LineMenuEn.h>

/************************************************************
 *
 * New fields for the LineMenuEntry widget class record.
 *
 ************************************************************/

typedef struct _LineMenuEntryClassPart {
  XtPointer extension;
} LineMenuEntryClassPart;

/* Full class record declaration */
typedef struct _LineMenuEntryClassRec {
    RectObjClassPart            rect_class;
    MenuEntryClassPart	        menu_entry_class;
    LineMenuEntryClassPart	line_entry_class;
} LineMenuEntryClassRec;

extern LineMenuEntryClassRec lineMenuEntryClassRec;

/* New fields for the LineMenuEntry widget record */
typedef struct {
    /* resources */
    Pixel foreground;		/* Foreground color. */
    Pixmap stipple;		/* Line Stipple. */
    Dimension line_width;	/* Width of the line. */

    /* private data.  */

    GC gc;			/* Graphics context for drawing line. */
} LineMenuEntryPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _LineMenuEntryRec {
  ObjectPart             object;
  RectObjPart            rectangle;
  MenuEntryPart	         menu_entry;
  LineMenuEntryPart	 line_entry;
} LineMenuEntryRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

#endif /* _XawLineMenuEntryP_h */
