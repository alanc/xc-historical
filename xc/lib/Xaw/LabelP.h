/* $Header: LabelPrivate.h,v 1.1 87/08/27 16:55:13 haynes Exp $ */

/*
 *	sccsid:	%W%	%G%
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
 * LabelPrivate.h - Private definitions for Label widget
 * 
 * Author:	Charles Haynes
 * 		Digital Equipment Corporation
 * 		Western Software Laboratory
 * Date:	Thu Aug 27 1987
 */

#ifndef _XtLabelPrivate_h
#define _XtLabelPrivate_h

/***********************************************************************
 *
 * Label Widget Private Data
 *
 ***********************************************************************/

/* New fields for the Label widget class record */

typedef struct {int foo;} LabelClass;

/* Full class record declaration */
typedef struct _LabelWidgetClassData {
    CoreClass	core_class;
    LabelClass	label_class;
} *LabelWidgetClass;

/* New fields for the Label widget record */
typedef struct {
    Pixel	foreground;
    XFontStruct	*font;
    char	*label;
    XtJustify	justify;
    Dimension	internalWidth;
    Dimension	internalHeight;

    GC		gc;
    Position	labelX;
    Position	labelY;
    Dimension	labelWidth;
    Dimension	labelHeight;
    unsigned int labelLen;
} Label;

/* $Log:	LabelPrivate.h,v $
 * Revision 1.1  87/08/27  16:55:13  haynes
 * Initial revision
 *  */


#endif _XtLabelPrivate_h
/* DON'T ADD STUFF AFTER THIS #endif */

