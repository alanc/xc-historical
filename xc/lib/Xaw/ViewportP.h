/*
 * $Header: ViewportP.h,v 1.1 88/01/12 14:50:17 swick Locked $
 * Private declarations for ViewportWidgetClass
 */

#ifndef _ViewportP_h
#define _ViewportP_h

#include <X/Viewport.h>
#include "FormP.h"

typedef struct {int empty;} ViewportClassPart;

typedef struct _ViewportClassRec {
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    FormClassPart	form_class;
    ViewportClassPart	viewport_class;
} ViewportClassRec;

extern ViewportClassRec viewportClassRec;

typedef struct _ViewportPart {
    /* resources */
    Boolean forcebars;		/* Whether we should always display */
				/* the selected scrollbars. */
    Boolean allowhoriz;		/* Whether we allow horizontal scrollbars. */
    Boolean allowvert;		/* Whether we allow vertical scrollbars. */
    Boolean usebottom;		/* True iff horiz bars appear at bottom. */
    Boolean useright;		/* True iff vert bars appear at right. */
    /* private state */
    Widget clip, child;		/* The clipping and (scrolled) child widgets */
    Widget  horiz_bar, vert_bar;/* What scrollbars we currently have. */
} ViewportPart;

typedef struct _ViewportRec {
    CorePart	core;
    CompositePart	composite;
    ConstraintPart	constraint;
    FormPart		form;
    ViewportPart	viewport;
} ViewportRec;

typedef struct {
    /* resources */

    /* private state */
    Boolean		reparented; /* True if child has been re-parented */
} ViewportConstraintsPart;

typedef struct _ViewportConstraintsRec {
    FormConstraintsPart		form;
    ViewportConstraintsPart	viewport;
} ViewportConstraintsRec, *ViewportConstraints;

#endif _ViewportP_h
