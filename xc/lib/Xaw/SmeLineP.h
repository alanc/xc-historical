/*
* $XConsortium: SmeLineP.h,v 1.1 89/09/29 19:04:02 kit Exp $
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
 * SmeLineP.h - Private definitions for SmeLine widget
 * 
 */

#ifndef _XawSmeLineP_h
#define _XawSmeLineP_h

/***********************************************************************
 *
 * SmeLine Widget Private Data
 *
 ***********************************************************************/

#include <X11/Xaw/SmeP.h>
#include <X11/Xaw/SmeLine.h>

/************************************************************
 *
 * New fields for the SmeLine widget class record.
 *
 ************************************************************/

typedef struct _SmeLineClassPart {
  XtPointer extension;
} SmeLineClassPart;

/* Full class record declaration */
typedef struct _SmeLineClassRec {
    RectObjClassPart    rect_class;
    SmeClassPart	sme_class;
    SmeLineClassPart	sme_line_class;
} SmeLineClassRec;

extern SmeLineClassRec smeLineClassRec;

/* New fields for the SmeLine widget record */
typedef struct {
    /* resources */
    Pixel foreground;		/* Foreground color. */
    Pixmap stipple;		/* Line Stipple. */
    Dimension line_width;	/* Width of the line. */

    /* private data.  */

    GC gc;			/* Graphics context for drawing line. */
} SmeLinePart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _SmeLineRec {
  ObjectPart     object;
  RectObjPart    rectangle;
  SmePart	 sme;
  SmeLinePart	 sme_line;
} SmeLineRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

#endif /* _XawSmeLineP_h */
