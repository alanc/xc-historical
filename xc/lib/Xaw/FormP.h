/* $Header: FormP.h,v 1.7 88/02/14 13:59:26 rws Exp $ */
/* Copyright	Massachusetts Institute of Technology	1987 */

/* Form widget private definitions */

#ifndef _FormP_h
#define _FormP_h

#include <X11/Form.h>
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
    XtEdgeType	top, bottom,	/* where to drag edge on resize		*/
		left, right;
    int		dx;		/* desired horiz offset			*/
    Widget	horiz_base;	/* measure dx from here if non-null	*/
    int		dy;		/* desired vertical offset		*/
    Widget	vert_base;	/* measure dy from here if non-null	*/
    Boolean	allow_resize;	/* TRUE if child may request resize	*/
} FormConstraintsPart;

typedef struct _FormConstraintsRec {
    FormConstraintsPart	form;
} FormConstraintsRec, *FormConstraints;

#endif _FormP_h
