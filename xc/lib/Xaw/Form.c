#ifndef lint
static char rcsid[] = "$Header: Form.c,v 1.7 88/01/22 20:26:07 swick Locked $";
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
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include <X/Xlib.h>
#include <X/Intrinsic.h>
#include <X/Atoms.h>
#include <X/Form.h>
#include "FormP.h"

/* Private Definitions */

#define DEFAULTVALUE	-9999


#define Offset(field) XtOffset(FormWidget, form.field)
static XtResource resources[] = {
    {XtNdefaultDistance, XtCThickness, XrmRInt, sizeof(int),
	Offset(default_spacing), XrmRString, "4"},
};
#undef Offset

static XtEdgeType defEdge = XtRubber;

#define Offset(field) XtOffset(FormConstraints, form.field)
static XtResource formConstraintResources[] = {
    {XtNtop, XtCEdge, XtREdgeType, sizeof(XtEdgeType),
	Offset(top), XtREdgeType, (caddr_t)&defEdge},
    {XtNbottom, XtCEdge, XtREdgeType, sizeof(XtEdgeType),
	Offset(bottom), XtREdgeType, (caddr_t)&defEdge},
    {XtNleft, XtCEdge, XtREdgeType, sizeof(XtEdgeType),
	Offset(left), XtREdgeType, (caddr_t)&defEdge},
    {XtNright, XtCEdge, XtREdgeType, sizeof(XtEdgeType),
	Offset(right), XtREdgeType, (caddr_t)&defEdge},
    {XtNhorizDistance, XtCThickness, XrmRInt, sizeof(int),
	Offset(dx), XrmRString, "-9999" /*DEFAULTVALUE*/ },
    {XtNfromHoriz, XtCWidget, XtRWidget, sizeof(Widget),
	Offset(horiz_base), XtRWidget, (caddr_t)NULL},
    {XtNvertDistance, XtCThickness, XrmRInt, sizeof(int),
	Offset(dy), XrmRString, "-9999" /*DEFAULTVALUE*/ },
    {XtNfromVert, XtCWidget, XtRWidget, sizeof(Widget),
	Offset(vert_base), XtRWidget, (caddr_t)NULL},
    {XtNresizable, XtCBoolean, XrmRBoolean, sizeof(Boolean),
	Offset(allow_resize), XrmRString, "FALSE"},
};
#undef Offset

static void Initialize(), Resize();
static void ConstraintInitialize();
static Boolean SetValues(), ConstraintSetValues();
static XtGeometryResult GeometryManager();
static void ChangeManaged();

FormClassRec formClassRec = {
  { /* core_class fields */
    /* superclass         */    (WidgetClass) &constraintClassRec,
    /* class_name         */    "Form",
    /* widget_size        */    sizeof(FormRec),
    /* class_initialize   */    NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    Initialize,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    Resize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    SetValues,
    /* accept_focus       */    NULL,
    /* callback_private   */    NULL,
    /* reserved_private   */    NULL
  },
  { /* composite_class fields */
    /* geometry_manager   */   GeometryManager,
    /* change_managed     */   ChangeManaged,
    /* insert_child       */   XtInheritInsertChild,
    /* delete_child       */   XtInheritDeleteChild,
    /* move_focus_to_next */   NULL,
    /* move_focus_to_prev */   NULL
  },
  { /* constraint_class fields */
    /* subresourses       */   formConstraintResources,
    /* subresource_count  */   XtNumber(formConstraintResources),
    /* constraint_size    */   sizeof(FormConstraintsRec),
    /* initialize         */   ConstraintInitialize,
    /* destroy            */   NULL,
    /* set_values         */   ConstraintSetValues
  },
  { /* form_class fields */
    /* empty              */   0
  }
};

WidgetClass formWidgetClass = (WidgetClass)&formClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/


/* ARGSUSED */
static void Initialize(request, new, args, num_args)
    Widget request, new;
    ArgList args;
    Cardinal *num_args;
{
    FormWidget fw = (FormWidget)new;

    fw->form.old_width = fw->core.width;
    fw->form.old_height = fw->core.height;
    fw->form.no_refigure = 0;
    fw->form.needs_relayout = FALSE;
}


static void RefigureLocations(w)
    Widget w;
{
    FormWidget fw = (FormWidget)w;
    int num_children = fw->composite.num_children;
    WidgetList children = fw->composite.children;
    Widget *childP;
    Position x, y, maxx, maxy;

    if (fw->form.no_refigure) {
	fw->form.needs_relayout = TRUE;
	return;
    }

    maxx = maxy = 1;
    for (childP = children; childP - children < num_children; childP++) {
	FormConstraints form = (FormConstraints)(*childP)->core.constraints;
	if (!XtIsManaged(*childP)) continue;
	x = form->form.dx;
	y = form->form.dy;
	if (form->form.horiz_base)
	    x += form->form.horiz_base->core.x
	         + form->form.horiz_base->core.width
	         + (form->form.horiz_base->core.border_width << 1);
	if (form->form.vert_base)
	    y += form->form.vert_base->core.y
	         + form->form.vert_base->core.height
	         + (form->form.vert_base->core.border_width << 1);
	XtMoveWidget( *childP, x, y );
	x += (*childP)->core.width  + ((*childP)->core.border_width << 1);
	y += (*childP)->core.height + ((*childP)->core.border_width << 1);
	if (maxx < x) maxx = x;
	if (maxy < y) maxy = y;
    }

    maxx += fw->form.default_spacing;
    maxy += fw->form.default_spacing;
    if (maxx != fw->core.width || maxy != fw->core.height) {
	XtGeometryResult result;
	result = XtMakeResizeRequest( w, (Dimension)maxx, (Dimension)maxy,
				      (Dimension*)&maxx, (Dimension*)&maxy );
	if (result == XtGeometryAlmost)
	    XtMakeResizeRequest( w, (Dimension)maxx, (Dimension)maxy,
				 NULL, NULL );
	fw->form.old_width  = fw->core.width;
	fw->form.old_height = fw->core.height;
    }

    fw->form.needs_relayout = FALSE;
}


static Position TransformCoord(loc, old, new, type)
Position loc;
Dimension old, new;
XtEdgeType type;
{
    if (type == XtRubber)
        if (old > 0)
	    return loc * new / old;
        else
	    return loc;
    if (type == XtChainTop || type == XtChainLeft)
	return loc;
    return loc + new - old;
}


static void Resize(w)
    Widget w;
{
    FormWidget fw = (FormWidget)w;
    WidgetList children = fw->composite.children;
    int num_children = fw->composite.num_children;
    Widget *childP;
    Position x, y;
    Dimension width, height;

    for (childP = children; childP - children < num_children; childP++) {
	FormConstraints form = (FormConstraints)(*childP)->core.constraints;
	if (!XtIsManaged(*childP)) continue;
	x = TransformCoord( (*childP)->core.x, fw->form.old_width,
			    fw->core.width, form->form.left );
	y = TransformCoord( (*childP)->core.y, fw->form.old_height,
			    fw->core.height, form->form.top );
	width =
	  TransformCoord((Position)((*childP)->core.x
				    + (*childP)->core.width
				    + (*childP)->core.border_width),
			 fw->form.old_width, fw->core.width,
			 form->form.right ) -x - (*childP)->core.border_width;
	height =
	  TransformCoord((Position)((*childP)->core.y
				    + (*childP)->core.height
				    + (*childP)->core.border_width),
			 fw->form.old_height, fw->core.height,
			 form->form.bottom ) -y - (*childP)->core.border_width;
	if (width < 1) width = 1;
	if (height < 1) height = 1;
	XtMoveWidget( (*childP), x, y );
	XtResizeWidget((*childP), width, height, (*childP)->core.border_width);
    }

    fw->form.old_width = fw->core.width;
    fw->form.old_height = fw->core.height;
}


/* ARGSUSED */
static XtGeometryResult GeometryManager(w, request, reply)
    Widget w;
    XtWidgetGeometry *request;
    XtWidgetGeometry *reply;	/* RETURN */
{
    FormConstraints form = (FormConstraints)w->core.constraints;
    XtWidgetGeometry allowed;

    if ((request->request_mode & ~(CWWidth | CWHeight)) ||
	!form->form.allow_resize)
	return XtGeometryNo;

    if (request->request_mode & CWWidth)
	allowed.width = request->width;
    else
	allowed.width = w->core.width;

    if (request->request_mode & CWHeight)
	allowed.height = request->height;
    else
	allowed.height = w->core.height;

    if (allowed.width == w->core.width && allowed.height == w->core.height)
	return XtGeometryNo;

    w->core.width = allowed.width;
    w->core.height = allowed.height;
    RefigureLocations( w->core.parent );
    return XtGeometryYes;
}



/* ARGSUSED */
static Boolean SetValues(current, request, new, last)
    Widget current, request, new;
    Boolean last;
{
    return( FALSE );
}


/* ARGSUSED */
static void ConstraintInitialize(request, new, args, num_args)
    Widget request, new;
    ArgList args;
    Cardinal *num_args;
{
    FormConstraints form = (FormConstraints)new->core.constraints;
    FormWidget fw = (FormWidget)new->core.parent;

    if (form->form.dx == DEFAULTVALUE)
        form->form.dx = fw->form.default_spacing;

    if (form->form.dy == DEFAULTVALUE)
        form->form.dy = fw->form.default_spacing;
}


/* ARGSUSED */
static Boolean ConstraintSetValues(current, request, new, last)
    Widget current, request, new;
    Boolean last;
{
    return( FALSE );
}

static void ChangeManaged(w)
    Widget w;
{
#ifdef notdef
    FormWidget fw = (FormWidget)w;
    WidgetList children = fw->composite.children;
    int num_children = fw->composite.num_children;
    Widget child, *childP, *unmanagedP;

    unmanagedP = NULL;
    for (childP = children; childP - children < num_children; childP++) {
	if (XtIsManaged(*childP)) {
	    if (unmanagedP) {
		child = *unmanagedP;
		*unmanagedP = *childP;
		*childP = child;
		childP = unmanagedP;	/* simplest to just backtrack */
	    }
	}
	else {
	    if (!unmanagedP)
		unmanagedP = childP;
	}
    }
#endif
    RefigureLocations( w );
}
    


/**********************************************************************
 *
 * Public routines
 *
 **********************************************************************/


/* 
 * Set or reset figuring (ignored if realized)
 */

void XtFormDoLayout(w, doit)
Widget w;
Boolean doit;
{
    register FormWidget fw = (FormWidget)w;

    if (doit && fw->form.no_refigure > 0)
	fw->form.no_refigure--;
    else
	if (!XtIsRealized(w))
	    fw->form.no_refigure++;

    if (fw->form.needs_relayout)
	RefigureLocations( w );
}
