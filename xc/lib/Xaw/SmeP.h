/*
* $XConsortium: SmeP.h,v 1.2 89/09/29 19:03:52 kit Exp $
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
 * SmeP.h - Private definitions for Sme widget
 * 
 */

#ifndef _XawSmeP_h
#define _XawSmeP_h

/***********************************************************************
 *
 * Sme Widget Private Data
 *
 ***********************************************************************/

#include <X11/RectObjP.h>
#include <X11/Xaw/Sme.h>

/************************************************************
 *
 * New fields for the Sme widget class record.
 *
 ************************************************************/

typedef struct _SmeClassPart {
  void (*highlight)();
  void (*unhighlight)();
  void (*notify)();	
  XtPointer extension;
} SmeClassPart;

/* Full class record declaration */
typedef struct _SmeClassRec {
    RectObjClassPart    rect_class;
    SmeClassPart	sme_class;
} SmeClassRec;

extern SmeClassRec smeClassRec;

/* New fields for the Sme widget record */
typedef struct {
    /* resources */
    XtCallbackList callbacks;	/* The callback list */

} SmePart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _SmeRec {
  ObjectPart     object;
  RectObjPart    rectangle;
  SmePart	 sme;
} SmeRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

typedef void (*_XawEntryVoidFunc)();

#define XtInheritHighlight   ((_XawEntryVoidFunc) _XtInherit)
#define XtInheritUnhighlight XtInheritHighlight
#define XtInheritNotify      XtInheritHighlight

#endif /* _XawSmeP_h */
