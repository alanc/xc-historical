/* Form widget private definitions */

#ifndef _FormP_h
#define _FormP_h

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

typedef struct _FormConstraintsRec {
    XtEdgeType	top, bottom,	/* where to drag edge on resize		*/
		left, right;
    int		dx;		/* desired horiz offset			*/
    Widget	horiz_base;	/* measure dx from here if non-null	*/
    int		dy;		/* desired vertical offset		*/
    Widget	vert_base;	/* measure dy from here if non-null	*/
    Boolean	allow_resize;	/* TRUE if child may request resize	*/
} FormConstraintsRec, *FormConstraints;

extern XtResource formConstraintResources[];
extern Cardinal formConstraintResourceCount;

#endif _FormP_h
/* DON'T ADD STUFF AFTER THIS #endif */
