#ifndef lint
static char rcsid[] = "$Header: Box.c,v 1.24 88/02/05 21:09:40 swick Exp $";
#endif lint

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
 * Box.c - Box composite widget
 * 
 */

#include	"IntrinsicP.h"
#include	<X/Atoms.h>
#include	<X/Misc.h>
#include	"BoxP.h"

/****************************************************************
 *
 * Box Resources
 *
 ****************************************************************/

static int defFour = 4;

static XtResource resources[] = {
    {XtNhSpace, XtCHSpace, XtRInt, sizeof(int),
	 XtOffset(BoxWidget, box.h_space), XtRInt, (caddr_t)&defFour},
    {XtNvSpace, XtCVSpace, XtRInt, sizeof(int),
	 XtOffset(BoxWidget, box.v_space), XtRInt, (caddr_t)&defFour},
};

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

static void Initialize();
static void Realize();
static void Resize();
static Boolean SetValues();
static XtGeometryResult GeometryManager();
static void ChangeManaged();
static void ClassInitialize();

#define superclass	(&compositeClassRec)
BoxClassRec boxClassRec = {
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) superclass,
    /* class_name         */    "Box",
    /* widget_size        */    sizeof(BoxRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_init    */	NULL,
    /* class_inited       */	FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    Realize,
    /* actions            */    NULL,
    /* num_actions	  */	0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    Resize,
    /* expose             */    NULL,
    /* set_values         */    SetValues,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus       */    NULL,
    /* version            */	XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
  },{
/* composite_class fields */
    /* geometry_manager   */    GeometryManager,
    /* change_managed     */    ChangeManaged,
    /* insert_child	  */	NULL,	/* Inherit from superclass */
    /* delete_child	  */	NULL,	/* Inherit from superclass */
    /* move_focus_to_next */    NULL,
    /* move_focus_to_prev */    NULL
  },{
    /* mumble		  */	0	/* Make C compiler happy   */
  }
};

WidgetClass boxWidgetClass = (WidgetClass)&boxClassRec;


/****************************************************************
 *
 * Private Routines
 *
 ****************************************************************/

static void ClassInitialize()
{
#define Inherit(method) \
    boxClassRec.method = superclass->method;

    Inherit(composite_class.insert_child);
    Inherit(composite_class.delete_child);
}

/*
 *
 * Do a layout, either actually assigning positions, or just calculating size.
 * Returns minimum width and height that will preserve the same layout.
 *
 */

/* ARGSUSED */
static DoLayout(bbw, width, height, replyWidth, replyHeight, position)
    BoxWidget	bbw;
    Dimension	width, height;
    Dimension	*replyWidth, *replyHeight;	/* RETURN */
    Boolean	position;	/* actually reposition the windows? */
{
    Cardinal  i;
    Dimension w, h;	/* Width and height needed for box 		*/
    Dimension lw, lh;	/* Width and height needed for current line 	*/
    Dimension bw, bh;	/* Width and height needed for current widget 	*/
    Dimension h_space;  /* Local copy of bbw->box.h_space 		*/
    register Widget widget;	/* Current widget 			*/
    Mask valuemask;
    XSetWindowAttributes attributes;
 
    /* Box width and height */
    h_space = bbw->box.h_space;
    w = h_space;
    h = bbw->box.v_space;
   
    /* Line width and height */
    lh = 0;
    lw = h_space;
  
    valuemask = CWWinGravity;
    attributes.win_gravity = NorthWestGravity;
    for (i = 0; i < bbw->composite.num_children; i++) {
	widget = bbw->composite.children[i];
	if (widget->core.managed) {
	    /* Compute widget width */
	    bw = widget->core.width + 2*widget->core.border_width + h_space;
	    if ((lw + bw > width) && (lw > h_space)) {
		/* At least one widget on this line, and can't fit any more.
		   Start new line */
		attributes.win_gravity = UnmapGravity;
		AssignMax(w, lw);
		h += lh + bbw->box.v_space;
		lh = 0;
		lw = h_space;
	    }
	    if (position && (lw != widget->core.x || h != widget->core.y)) {
		if (widget->core.y == bbw->box.v_space && XtIsRealized(widget))
		    /* %%% dunno why, but on our server we get cruft */
		    XUnmapWindow( XtDisplay(widget), XtWindow(widget) );
		XtMoveWidget(bbw->composite.children[i], (int)lw, (int)h);
		if (XtIsRealized(widget)) {
		    XChangeWindowAttributes( XtDisplay(widget),
					     XtWindow(widget),
					     valuemask, &attributes );
		}
	    }
	    lw += bw;
	    bh = widget->core.height + 2*widget->core.border_width;
	    AssignMax(lh, bh);
	} /* if managed */
    } /* for */

    if (position && XtIsRealized((Widget)bbw))
	XMapSubwindows( XtDisplay((Widget)bbw), XtWindow((Widget)bbw) );

    /* Finish off last line */
    if (lw > h_space) {
	AssignMax(w, lw);
        h += lh + bbw->box.v_space;
    }

    *replyWidth = Max(w, 1);
    *replyHeight = Max(h, 1);
}

/*
 *
 * Calculate preferred size, given constraining box
 * Returns TRUE if everything will fit in the constraints, FALSE otherwise.
 * Also returns minimum width and height that will preserve the same layout.
 *
 */

static Boolean PreferredSize(bbw, width, height, replyWidth, replyHeight)
    BoxWidget	bbw;
    Dimension	width, height;
    Dimension	*replyWidth, *replyHeight;
{
    DoLayout(bbw, width, height, replyWidth, replyHeight, FALSE);
    return ((*replyWidth <= width) && (*replyHeight <= height));
}

/*
 *
 * Actually layout the box
 *
 */

static void Resize(w)
    Widget	w;
{
    Dimension junk;

    if (XtIsRealized(w))
	XClearWindow( XtDisplay(w), XtWindow(w) );

    DoLayout((BoxWidget)w, w->core.width, w->core.height,
	     &junk, &junk, TRUE);
} /* Resize */

/*
 *
 * Try to do a new layout within the current width and height;
 * if that fails try to do it within the box returned by PreferredSize.
 *
 * TryNewLayout just says if it's possible, and doesn't actually move the kids
 */

static Boolean TryNewLayout(bbw)
    BoxWidget	bbw;
{
    Dimension	width, height, proposed_width, proposed_height;
    int		iterations;

    if (!PreferredSize(bbw, bbw->core.width, bbw->core.height, &width, &height))
	(void) PreferredSize(bbw, width, height, &width, &height);

    if ((bbw->core.width == width) && (bbw->core.height == height)) {
        /* Same size */
	return (TRUE);
    }

    /* let's see if our parent will go for a new size. */
    iterations = 0;
    do {
	switch (XtMakeResizeRequest(
			    (Widget)bbw, width, height,
			    &proposed_width, &proposed_height))
	{
	    case XtGeometryYes:
		return (TRUE);

	    case XtGeometryNo:
		if ((width <= bbw->core.width) && (height <= bbw->core.height))
		    return (TRUE);
		else
		    return (FALSE);

	    case XtGeometryAlmost:
		if (proposed_height != height && proposed_width != width) {
		    /* punt; parent has its own ideas */
		    width = proposed_width;
		    height = proposed_height;
		}
		else if (proposed_width != width) {
		    /* recalc bounding box; height might change */
		    DoLayout(bbw, proposed_width, 0, &width, &height, FALSE);
		    width = proposed_width;
		}
		else { /* proposed_height != height */
		    Boolean fits;
		    Dimension last_width, last_height;

		    /* find minimum width for this height */
		    DoLayout(bbw, 0, proposed_height, &width, &height, FALSE);
		    if (height <= proposed_height) {
			height = proposed_height;
			continue;
		    }
		    do { /* find some width big enough */
			last_height = height;
			width *= 2;
			fits = DoLayout(bbw, width, proposed_height,
					&width, &height, FALSE);
		    } while (!fits && height > last_height);
		    do { /* find minimum width */
			last_width = width--;
			fits = DoLayout(bbw, width, proposed_height,
					&width, &height, FALSE);
		    } while (fits);
		    width = last_width;
		    height = proposed_height;
		}
	}
	iterations++;
    } while (iterations < 10);
    return (FALSE);
}

/*
 *
 * Geometry Manager
 *
 */

/*ARGSUSED*/
static XtGeometryResult GeometryManager(w, request, reply)
    Widget		w;
    XtWidgetGeometry	*request;
    XtWidgetGeometry	*reply;	/* RETURN */

{
    Dimension	width, height, borderWidth;
    BoxWidget bbw;

    /* Position request always denied */
    if ((request->request_mode & CWX && request->x != w->core.x) ||
	(request->request_mode & CWY && request->y != w->core.y))
        return (XtGeometryNo);

    /* Size changes must see if the new size can be accomodated */
    if (request->request_mode & (CWWidth | CWHeight | CWBorderWidth)) {

	/* Make all three fields in the request valid */
	if ((request->request_mode & CWWidth) == 0)
	    request->width = w->core.width;
	if ((request->request_mode & CWHeight) == 0)
	    request->height = w->core.height;
        if ((request->request_mode & CWBorderWidth) == 0)
	    request->border_width = w->core.border_width;

	/* Save current size and set to new size */
	width = w->core.width;
	height = w->core.height;
	borderWidth = w->core.border_width;
	w->core.width = request->width;
	w->core.height = request->height;
	w->core.border_width = request->border_width;

	/* Decide if new layout works: (1) new widget is smaller,
	   (2) new widget fits in existing Box, (3) Box can be
	   expanded to allow new widget to fit */

	bbw = (BoxWidget) w->core.parent;

/* whenever a child changes his geometry, we attempt to
 * change ours to be the minimum enclosing size...
	if (((request->width + request->border_width <= width + borderWidth) &&
	    (request->height + request->border_width <= height + borderWidth))
	|| PreferredSize(bbw, bbw->core.width, bbw->core.height, &junk, &junk)
	|| TryNewLayout(bbw)) {
 */
	if (TryNewLayout(bbw)) {
	    /* Fits in existing or new space, relayout */
	    Resize((Widget)bbw);
	    return (XtGeometryYes);
	} else {
	    /* Cannot satisfy request, change back to original geometry */
	    w->core.width = width;
	    w->core.height = height;
	    w->core.border_width = borderWidth;
	    return (XtGeometryNo);
	}
    }; /* if any size changes requested */

    /* Any stacking changes don't make a difference, so allow if that's all */
    return (XtGeometryYes);
}

static void ChangeManaged(w)
    Widget w;
{
    /* Reconfigure the box */
    (void) TryNewLayout((BoxWidget)w);
    Resize(w);
}

/* ARGSUSED */
static void Initialize(request, new)
    Widget request, new;
{
    BoxWidget newbbw = (BoxWidget)new;

/* ||| What are consequences of letting height, width be 0? If okay, then
       Initialize can be NULL */

    if (newbbw->core.width == 0)
        newbbw->core.width = ((newbbw->box.h_space != 0)
			      ? newbbw->box.h_space : defFour);
    if (newbbw->core.height == 0)
	newbbw->core.height = ((newbbw->box.v_space != 0)
			       ? newbbw->box.v_space : defFour);
} /* Initialize */

static void Realize(w, valueMask, attributes)
    register Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    attributes->bit_gravity = NorthWestGravity;
    *valueMask |= CWBitGravity;

    XtCreateWindow( w, (unsigned)InputOutput, (Visual *)CopyFromParent,
		    *valueMask, attributes);
} /* Realize */

/* ARGSUSED */
static Boolean SetValues(current, request, new)
    Widget current, request, new;
{
   /* need to relayout if h_space or v_space change */

    return False;
}

    
