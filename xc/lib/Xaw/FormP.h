/* $XConsortium: FormP.h,v 1.14 89/05/11 01:05:20 kit Exp $ */
/* Copyright	Massachusetts Institute of Technology	1987 */


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

/* Form widget private definitions */

#ifndef _XawFormP_h
#define _XawFormP_h

#include <X11/Xaw/Form.h>
#include <X11/ConstrainP.h>

#define XtREdgeType		"EdgeType"

typedef struct {int empty;} FormClassPart;

typedef struct _FormClassRec {
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    FormClassPart	form_class;
} FormClassRec;

extern FormClassRec formClassRec;

typedef struct _FormPart {
    Dimension	old_width, old_height; /* last known dimensions		 */
    int		default_spacing;    /* default distance between children */
    int		no_refigure;	    /* no re-layout while > 0		 */
    Boolean	needs_relayout;	    /* next time no_refigure == 0	 */
} FormPart;

typedef struct _FormRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    FormPart		form;
} FormRec;

typedef struct _FormConstraintsPart {
/*
 * Constraint Resources.
 */
    XtEdgeType	top, bottom,	/* where to drag edge on resize		*/
		left, right;
    int		dx;		/* desired horiz offset			*/
    Widget	horiz_base;	/* measure dx from here if non-null	*/
    int		dy;		/* desired vertical offset		*/
    Widget	vert_base;	/* measure dy from here if non-null	*/
    Boolean	allow_resize;	/* TRUE if child may request resize	*/

/*
 * Private contstraint resources.
 */

    int         virtual_width, virtual_height; 

/*
 * What the size of this child would be if we did not impose the 
 * constraint the width and height must be greater than zero (0).
 */

} FormConstraintsPart;

typedef struct _FormConstraintsRec {
    FormConstraintsPart	form;
} FormConstraintsRec, *FormConstraints;

#endif /* _XawFormP_h */
