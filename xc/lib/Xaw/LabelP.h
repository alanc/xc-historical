/*
* $Header: LabelP.h,v 1.8 87/12/04 10:56:40 swick Locked $
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
 * LabelP.h - Private definitions for Label widget
 * 
 */

#ifndef _XtLabelP_h
#define _XtLabelP_h

/***********************************************************************
 *
 * Label Widget Private Data
 *
 ***********************************************************************/

/* New fields for the Label widget class record */

typedef struct {int foo;} LabelClassPart;

/* Full class record declaration */
typedef struct _LabelClassRec {
    CoreClassPart	core_class;
    LabelClassPart	label_class;
} LabelClassRec;

extern LabelClassRec labelClassRec;

/* New fields for the Label widget record */
typedef struct {
    Pixel	foreground;
    XFontStruct	*font;
    char	*label;
    XtJustify	justify;
    Dimension	internal_width;
    Dimension	internal_height;

    GC		normal_GC;
    GC          gray_GC;
    Pixmap      gray_pixmap;
    Position	label_x;
    Position	label_y;
    Dimension	label_width;
    Dimension	label_height;
    unsigned int label_len;
} LabelPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _LabelRec {
    CorePart	core;
    LabelPart	label;
} LabelRec;

#endif _XtLabelP_h
