#ifndef lint
static char Xrcsid[] = "$XConsortium: Form.c,v 1.24 89/05/11 01:05:15 kit Exp $";
#endif /* lint */


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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Xmu.h>
#include <X11/Xaw/FormP.h>

/* Private Definitions */

static int def0 = 0;
static int def4 = 4;
static int DEFAULTVALUE = -99999;

#define Offset(field) XtOffset(FormWidget, form.field)
static XtResource resources[] = {
    {XtNdefaultDistance, XtCThickness, XtRInt, sizeof(int),
	Offset(default_spacing), XtRInt, (caddr_t)&def4}
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
    {XtNhorizDistance, XtCThickness, XtRInt, sizeof(int),
	Offset(dx), XtRInt, (caddr_t)&DEFAULTVALUE},
    {XtNfromHoriz, XtCWidget, XtRWidget, sizeof(Widget),
	Offset(horiz_base), XtRWidget, (caddr_t)NULL},
    {XtNvertDistance, XtCThickness, XtRInt, sizeof(int),
	Offset(dy), XtRInt, (caddr_t)&DEFAULTVALUE},
    {XtNfromVert, XtCWidget, XtRWidget, sizeof(Widget),
	Offset(vert_base), XtRWidget, (caddr_t)NULL},
    {XtNresizable, XtCBoolean, XtRBoolean, sizeof(Boolean),
	Offset(allow_resize), XtRInt, (caddr_t)&def0},
};
#undef Offset

static void ClassInitialize(), Initialize(), Resize();
static void ConstraintInitialize();
static Boolean SetValues(), ConstraintSetValues();
static XtGeometryResult GeometryManager();
static void ChangeManaged();

FormClassRec formClassRec = {
  { /* core_class fields */
    /* superclass         */    (WidgetClass) &constraintClassRec,
    /* class_name         */    "Form",
    /* widget_size        */    sizeof(FormRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_init    */    NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */    NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    Resize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    SetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */    NULL,
    /* accept_focus       */    NULL,
    /* version            */    XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry     */	XtInheritQueryGeometry,	/* %%% fix this! */
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension          */	NULL
  },
  { /* composite_class fields */
    /* geometry_manager   */   GeometryManager,
    /* change_managed     */   ChangeManaged,
    /* insert_child       */   XtInheritInsertChild,
    /* delete_child       */   XtInheritDeleteChild,
    /* extension          */   NULL
  },
  { /* constraint_class fields */
    /* subresourses       */   formConstraintResources,
    /* subresource_count  */   XtNumber(formConstraintResources),
    /* constraint_size    */   sizeof(FormConstraintsRec),
    /* initialize         */   ConstraintInitialize,
    /* destroy            */   NULL,
    /* set_values         */   ConstraintSetValues,
    /* extension          */   NULL
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


static XrmQuark	XtQChainLeft, XtQChainRight, XtQChainTop,
		XtQChainBottom, XtQRubber;

#define	done(address, type) \
	{ toVal->size = sizeof(type); \
	  toVal->addr = (caddr_t) address; \
	  return; \
	}

/* ARGSUSED */
static void _CvtStringToEdgeType(args, num_args, fromVal, toVal)
    XrmValuePtr args;		/* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    static XtEdgeType edgeType;
    XrmQuark q;
    char lowerName[1000];

    LowerCase((char*)fromVal->addr, lowerName);
    q = XrmStringToQuark(lowerName);
    if (q == XtQChainLeft) {
	edgeType = XtChainLeft;
	done(&edgeType, XtEdgeType);
    }
    if (q == XtQChainRight) {
	edgeType = XtChainRight;
	done(&edgeType, XtEdgeType);
    }
    if (q == XtQChainTop) {
	edgeType = XtChainTop;
	done(&edgeType, XtEdgeType);
    }
    if (q == XtQChainBottom) {
	edgeType = XtChainBottom;
	done(&edgeType, XtEdgeType);
    }
    if (q == XtQRubber) {
	edgeType = XtRubber;
	done(&edgeType, XtEdgeType);
    }
    XtStringConversionWarning(fromVal->addr, "edgeType");
    toVal->addr = NULL;
    toVal->size = 0;
}

static void ClassInitialize()
{
    static XtConvertArgRec parentCvtArgs[] = {
	{XtBaseOffset, (caddr_t)XtOffset(Widget, core.parent), sizeof(Widget)}
    };
    XtQChainLeft   = XrmStringToQuark("chainleft");
    XtQChainRight  = XrmStringToQuark("chainright");
    XtQChainTop    = XrmStringToQuark("chaintop");
    XtQChainBottom = XrmStringToQuark("chainbottom");
    XtQRubber      = XrmStringToQuark("rubber");

    XtAddConverter( XtRString, XtREdgeType, _CvtStringToEdgeType, NULL, 0 );
    XtAddConverter( XtRString, XtRWidget, XmuCvtStringToWidget,
		    parentCvtArgs, XtNumber(parentCvtArgs) );
}


/* ARGSUSED */
static void Initialize(request, new)
    Widget request, new;
{
    FormWidget fw = (FormWidget)new;

    fw->form.old_width = fw->core.width;
    fw->form.old_height = fw->core.height;
    fw->form.no_refigure = FALSE;
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
    register Position loc;
    Dimension old, new;
    XtEdgeType type;
{
    if (type == XtRubber) {
        if ( ((int) old) > 0)
	    loc = (loc * new) / old;
    }
    else if (type == XtChainBottom || type == XtChainRight)
      loc += (Position)new - (Position)old;

    /* I don't see any problem with returning values less than zero. */

    return (loc);
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

	form->form.virtual_width =
	  TransformCoord((Position)((*childP)->core.x
				    + form->form.virtual_width
				    + 2 * (*childP)->core.border_width),
			 fw->form.old_width, fw->core.width,
			 form->form.right )
	    - (x + 2 * (*childP)->core.border_width);

	form->form.virtual_height =
	  TransformCoord((Position)((*childP)->core.y
				    + form->form.virtual_height
				    + 2 * (*childP)->core.border_width),
			 fw->form.old_height, fw->core.height,
			 form->form.bottom ) 
	    - ( y + 2 * (*childP)->core.border_width);
	
	width = (Dimension) 
	       (form->form.virtual_width < 1) ? 1 : form->form.virtual_width;
	height = (Dimension)
	       (form->form.virtual_height < 1) ? 1 : form->form.virtual_height;

	XtMoveWidget( (*childP), x, y );
	XtResizeWidget( (*childP), width, height,
		        (*childP)->core.border_width );
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

    if ((request->request_mode & ~(XtCWQueryOnly | CWWidth | CWHeight)) ||
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

    if (!(request->request_mode & XtCWQueryOnly)) {
        /* reset virtual width and height. */
      	form->form.virtual_width = w->core.width = allowed.width;
	form->form.virtual_height = w->core.height = allowed.height;
	RefigureLocations( w->core.parent );
    }
    return XtGeometryYes;
}



/* ARGSUSED */
static Boolean SetValues(current, request, new)
    Widget current, request, new;
{
    return( FALSE );
}


/* ARGSUSED */
static void ConstraintInitialize(request, new)
    Widget request, new;
{
    FormConstraints form = (FormConstraints)new->core.constraints;
    FormWidget fw = (FormWidget)new->core.parent;

    if (form->form.dx == DEFAULTVALUE)
        form->form.dx = fw->form.default_spacing;

    if (form->form.dy == DEFAULTVALUE)
        form->form.dy = fw->form.default_spacing;
}

/* ARGSUSED */
static Boolean ConstraintSetValues(current, request, new)
    Widget current, request, new;
{
    return( FALSE );
}

static void ChangeManaged(w)
    Widget w;
{ 
  FormWidget fw = (FormWidget)w;
  FormConstraints form;
  WidgetList children, childP;
  int num_children = fw->composite.num_children;
  Widget child;
  
  /*
   * Reset virtual width and height for all children.
   */
  
  for (children = childP = fw->composite.children ;
       childP - children < num_children; childP++) {
    child = *childP;
    if (XtIsManaged(child)) {
      form = (FormConstraints)child->core.constraints;

/*
 * If the size is one (1) then we must not change the virtual sizes, as
 * they contain useful information.  If someone actually wants a widget of
 * width or height one (1) in a form widget he will lose, can't win them all.
 *
 * Chris D. Peterson 2/9/89.
 */
	 

      if ( child->core.width != 1)
	form->form.virtual_width = (int) child->core.width;
      if ( child->core.height != 1)
	form->form.virtual_height = (int) child->core.height;
    }
  }
  RefigureLocations( w );
}
    
/**********************************************************************
 *
 * Public routines
 *
 **********************************************************************/

/* 
 * Set or reset figuring (ignored if not realized)
 */

void XawFormDoLayout(w, doit)
Widget w;
Boolean doit;
{
    register FormWidget fw = (FormWidget)w;

    fw->form.no_refigure = !doit;

    if ( XtIsRealized(w) && fw->form.needs_relayout )
        RefigureLocations( w );
}
