/*
* $XConsortium: ToggleP.h,v 1.3 89/05/01 13:19:48 kit Exp $
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
 * ToggleP.h - Private definitions for Toggle widget
 * 
 * Author: Chris D. Peterson
 *         MIT X Consortium
 *         kit@expo.lcs.mit.edu
 *  
 * Date:   January 12, 1989
 *
 */

#ifndef _XawToggleP_h
#define _XawToggleP_h

#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/CommandP.h>

/***********************************************************************
 *
 * Toggle Widget Private Data
 *
 ***********************************************************************/

#define streq(a,b) ( strcmp( (a), (b) ) == 0)

typedef struct _RadioGroup {
  struct _RadioGroup *prev, *next; /* Pointers to other elements in group. */
  Widget widget;		  /* Widget corrosponding to this element. */
} RadioGroup;

/************************************
 *
 *  Class structure
 *
 ***********************************/

   /* New fields for the Toggle widget class record */
typedef struct _ToggleClass 
  {
    int makes_compiler_happy;  /* not used */
  } ToggleClassPart;

   /* Full class record declaration */
typedef struct _ToggleClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    LabelClassPart	label_class;
    CommandClassPart	command_class;
    ToggleClassPart     toggle_class;
} ToggleClassRec;

extern ToggleClassRec toggleClassRec;

/***************************************
 *
 *  Instance (widget) structure 
 *
 **************************************/

    /* New fields for the Toggle widget record */
typedef struct {
    /* resources */
    Widget      widget;
    caddr_t     radio_data;

    /* private data */
    RadioGroup * radio_group;
} TogglePart;

   /* Full widget declaration */
typedef struct _ToggleRec {
    CorePart         core;
    SimplePart	     simple;
    LabelPart	     label;
    CommandPart	     command;
    TogglePart       toggle;
} ToggleRec;

#endif /* _XawToggleP_h */


