/*
* $XConsortium: MenuEntryP.h,v 1.1 89/09/28 16:44:51 kit Exp $
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
 * MenuEntryP.h - Private definitions for MenuEntry widget
 * 
 */

#ifndef _XawMenuEntryP_h
#define _XawMenuEntryP_h

/***********************************************************************
 *
 * MenuEntry Widget Private Data
 *
 ***********************************************************************/

#include <X11/RectObjP.h>
#include <X11/Xaw/MenuEntry.h>

/************************************************************
 *
 * New fields for the MenuEntry widget class record.
 *
 ************************************************************/

typedef struct _MenuEntryClassPart {
  void (*highlight)();
  void (*unhighlight)();
  void (*notify)();	
  XtPointer extension;
} MenuEntryClassPart;

/* Full class record declaration */
typedef struct _MenuEntryClassRec {
    RectObjClassPart    rect_class;
    MenuEntryClassPart	menu_entry_class;
} MenuEntryClassRec;

extern MenuEntryClassRec menuEntryClassRec;

/* New fields for the MenuEntry widget record */
typedef struct {
    /* resources */
    XtCallbackList callbacks;	/* The callback list */

} MenuEntryPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _MenuEntryRec {
  ObjectPart     object;
  RectObjPart    rectangle;
  MenuEntryPart	 menu_entry;
} MenuEntryRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

typedef void (*_XawEntryVoidFunc)();

#define XtInheritHighlight   ((_XawEntryVoidFunc) _XtInherit)
#define XtInheritUnhighlight XtInheritHighlight
#define XtInheritNotify      XtInheritHighlight

#endif /* _XawMenuEntryP_h */
