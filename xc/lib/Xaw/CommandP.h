/*
* $Header: CommandP.h,v 1.14 88/02/14 13:58:59 rws Exp $
*/

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/* 
 * CommandP.h - Private definitions for Command widget
 * 
 * Author:	Mark S. Ackerman
 *              MIT/Project Athena
 * Date:	Thu Aug 27 1987
 */

#ifndef _XtCommandP_h
#define _XtCommandP_h

#include <X11/Command.h>
#include <X11/LabelP.h>

/***********************************************************************
 *
 * Command Widget Private Data
 *
 ***********************************************************************/

/************************************
 *
 *  Class structure
 *
 ***********************************/


   /* New fields for the Command widget class record */
typedef struct _CommandClass 
  {
    int makes_compiler_happy;  /* not used */
  } CommandClassPart;

   /* Full class record declaration */
typedef struct _CommandClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    LabelClassPart	label_class;
    CommandClassPart    command_class;
} CommandClassRec;

extern CommandClassRec commandClassRec;

/***************************************
 *
 *  Instance (widget) structure 
 *
 **************************************/

    /* New fields for the Command widget record */
typedef struct {
    /* resources */
    Dimension   highlight_thickness;
    XtCallbackList callbacks;

    /* private state */
    Pixmap      gray_pixmap;
    GC          highlight_GC;
    GC          inverse_GC;
    GC          inverse_text_GC;
    Boolean     set;
    Boolean     highlighted;
    Boolean     display_set;
    Boolean     display_highlighted;
} CommandPart;


/*    XtEventsPtr eventTable;*/


   /* Full widget declaration */
typedef struct _CommandRec {
    CorePart         core;
    SimplePart	     simple;
    LabelPart	     label;
    CommandPart      command;
} CommandRec;

#endif _XtCommandP_h


