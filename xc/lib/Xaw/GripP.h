/*
* $Header: GripP.h,v 1.3 87/12/17 16:43:12 swick Exp $
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
 *  GripP.h - Private definitions for Grip widget (Used by VPane Widget)
 *
 *  Author:       Jeanne M. Rich
 *                Digital Equipment Corporation
 *                Western Software Laboratory
 *  Date:         Wednesday September 9, 1987
 */

#ifndef _XtGripP_h
#define _XtGripP_h

/*******************************************************************************
 *
 * Grip Widget Private Date (Used by VPane Widget)
 *
 ******************************************************************************/

/* New fields for the Grip widget class record */
typedef struct {int empty;} GripClassPart;

/* Full Class record declaration */
typedef struct _GripClassRec {
    CoreClassPart    core_class;
    GripClassPart    grip_class;
} GripClassRec;

extern GripClassRec gripClassRec;

/* New fields for the Grip widget record */
typedef struct {
  XtCallbackList grip_action;
} GripPart;

/*****************************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************************/

typedef struct _GripRec {
   CorePart    core;
   GripPart    grip;
} GripRec;

#endif _XtGripP_h

