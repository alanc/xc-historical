/*
* $XConsortium: ToggleP.h,v 1.21 88/09/27 11:19:53 swick Exp $
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
 *         MIT X Consortium / Project Athena
 *         kit@athena.mit.edu
 *  
 * Date:   January 12, 1988
 *
 */

#ifndef _XtToggleP_h
#define _XtToggleP_h

#include <X11/Toggle.h>
#include <X11/CommandP.h>

/***********************************************************************
 *
 * Toggle Widget Private Data
 *
 ***********************************************************************/

#define streq(a,b) ( strcmp( (a), (b) ) == 0)

typedef struct _RadioList {
  struct _RadioList *prev, *next; /* Pointers to other elements in list. */
  Widget widget;		  /* Widget corrosponding to this element. */
} RadioList;

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
    Boolean     state;
    Widget      widget;
    caddr_t     radio_data;

    /* private data */
    RadioList * radio_list;
} TogglePart;

   /* Full widget declaration */
typedef struct _ToggleRec {
    CorePart         core;
    SimplePart	     simple;
    LabelPart	     label;
    CommandPart	     command;
    TogglePart       toggle;
} ToggleRec;

#endif _XtToggleP_h


