#ifndef lint
static char rcsid[] = "$Header: Viewport.c,v 1.7 88/01/19 15:27:18 swick Locked $";
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


#include <X/Xlib.h>
#include <X/Intrinsic.h>
#include <X/Misc.h>
#include <X/Scroll.h>
#include <X/Atoms.h>
#include <X/Viewport.h>
#include "ViewportP.h"


static void ScrollUpDownProc(), ThumbProc();

static Boolean defFalse = False;

#define offset(field) XtOffset(ViewportWidget,viewport.field)
static XtResource resources[] = {
    {XtNforceBars, XtCBoolean, XrmRBoolean, sizeof(Boolean),
	 offset(forcebars), XrmRBoolean, (caddr_t)&defFalse },
    {XtNallowHoriz, XtCBoolean, XrmRBoolean, sizeof(Boolean),
	 offset(allowhoriz), XrmRBoolean, (caddr_t)&defFalse },
    {XtNallowVert, XtCBoolean, XrmRBoolean, sizeof(Boolean),
	 offset(allowvert), XrmRBoolean, (caddr_t)&defFalse },
    {XtNuseBottom, XtCBoolean, XrmRBoolean, sizeof(Boolean),
	 offset(usebottom), XrmRBoolean, (caddr_t)&defFalse },
    {XtNuseRight, XtCBoolean, XrmRBoolean, sizeof(Boolean),
	 offset(useright), XrmRBoolean, (caddr_t)&defFalse },
};
#undef offset

static void ClassInitialize(), Initialize(), Realize(), Resize(),
    ChangeManaged();
static Boolean SetValues(), DoLayout();
static XtGeometryResult GeometryManager();

ViewportClassRec viewportClassRec = {
  { /* core_class fields */
    /* superclass	  */	(WidgetClass) &formClassRec,
    /* class_name	  */	"Viewport",
    /* widget_size	  */	sizeof(ViewportRec),
    /* class_initialize	  */	ClassInitialize,
    /* class_inited	  */	FALSE,
    /* initialize	  */	Initialize,
    /* realize		  */	Realize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	Resize,
    /* expose		  */	NULL, /* inherited */
    /* set_values	  */	SetValues,
    /* accept_focus	  */	NULL,
    /* callback_private	  */	NULL,
    /* reserved_private	  */	NULL
  },
  { /* composite_class fields */
    /* geometry_manager	  */	GeometryManager,
    /* change_managed	  */	ChangeManaged,
    /* insert_child	  */	NULL, /* inherited */
    /* delete_child	  */	NULL, /* inherited */
    /* move_focus_to_next */	NULL,
    /* move_focus_to_prev */	NULL
  },
  { /* constraint_class fields */
    /* subresourses	  */	NULL,
    /* subresource_count  */	0,
    /* constraint_size	  */	sizeof(ViewportConstraintsRec),
    /* initialize	  */	NULL,
    /* destroy		  */	NULL,
    /* set_values	  */	NULL
  },
  { /* form_class fields */
    /* empty		  */	0
  },
  { /* viewport_class fields */
    /* empty		  */	0
  }
};


WidgetClass viewportWidgetClass = (WidgetClass)&viewportClassRec;

static void ClassInitialize()
{
    register WidgetClass superclass = viewportClassRec.core_class.superclass;

#define Inherit(class, method) \
    viewportClassRec.method = ((class)superclass)->method

    Inherit(WidgetClass, core_class.expose);
    Inherit(CompositeWidgetClass, composite_class.insert_child);
    Inherit(CompositeWidgetClass, composite_class.delete_child);
}

static Widget CreateScrollbar(w, horizontal)
    ViewportWidget w;
    Boolean horizontal;
{
    Widget clip = w->viewport.clip;
    ViewportConstraints constraints =
	(ViewportConstraints)clip->core.constraints;
    static XtCallbackRec scrollCallback[] = { {NULL, NULL}, {NULL, NULL} };
    static XtCallbackRec thumbCallback[] = { {NULL, NULL}, {NULL, NULL} };
    static Arg barArgs[] = {
	{XtNorientation, NULL},
	{XtNlength, NULL},
	{XtNleft, NULL},
	{XtNright, NULL},
	{XtNtop, NULL},
	{XtNbottom, NULL},
	{XtNscrollProc, (XtArgVal)scrollCallback},
	{XtNthumbProc, (XtArgVal)thumbCallback},
    };
    Widget bar;

    XtSetCallback( scrollCallback[0], ScrollUpDownProc, w );
    XtSetCallback( thumbCallback[0], ThumbProc, w );
    XtSetArg(barArgs[0], XtNorientation,
	      horizontal ? XtorientHorizontal : XtorientVertical );
    XtSetArg(barArgs[1], XtNlength,
	     horizontal ? clip->core.width : clip->core.height);
    XtSetArg(barArgs[2], XtNleft,
	     (!horizontal && w->viewport.useright) ? XtRubber : XtChainLeft);
    XtSetArg(barArgs[3], XtNright,
	     (!horizontal && !w->viewport.useright) ? XtRubber : XtChainRight);
    XtSetArg(barArgs[4], XtNtop,
	     (horizontal && w->viewport.usebottom) ? XtRubber: XtChainTop);
    XtSetArg(barArgs[5], XtNbottom,
	     (horizontal && !w->viewport.usebottom) ? XtRubber: XtChainBottom);

    bar = XtCreateWidget( (horizontal ? "horizontal" : "vertical"),
			  scrollbarWidgetClass, (Widget)w,
			  barArgs, XtNumber(barArgs) );

    if (horizontal) {
	Dimension bw = bar->core.border_width;
	w->viewport.horiz_bar = bar;
	constraints->form.vert_base = bar;
	XtResizeWidget( clip, clip->core.width,	/* %%% wrong, but... */
		        w->core.height - bar->core.height - bw,
		        clip->core.border_width );
    }
    else {
	Dimension bw = bar->core.border_width;
	w->viewport.vert_bar = bar;
	constraints->form.horiz_base = bar;
	XtResizeWidget( clip, w->core.width - bar->core.width - bw, /* %%% */
		        bar->core.height, clip->core.border_width );
    }

    XtSetMappedWhenManaged( bar, False );
    XtManageChild( bar );

    return bar;
}

/* ARGSUSED */
static void Initialize(request, new, args, num_args)
    Widget request, new;
    ArgList args;
    Cardinal *num_args;
{
    ViewportWidget w = (ViewportWidget)new;
    static Arg clip_args[] = {
	{XtNfromHoriz, NULL},
	{XtNfromVert, NULL},
	{XtNwidth, NULL},
	{XtNheight, NULL},
	{XtNborderWidth, 0},
	{XtNleft, (XtArgVal)XtChainLeft},
	{XtNright, (XtArgVal)XtChainRight},
	{XtNtop, (XtArgVal)XtChainTop},
	{XtNbottom, (XtArgVal)XtChainBottom}
    };

    w->viewport.clip = w->viewport.child
	= w->viewport.horiz_bar = w->viewport.vert_bar
	= (Widget)NULL;

    w->form.default_spacing = 0;

    if (w->viewport.forcebars) {
	if (w->viewport.allowhoriz)
	    clip_args[1].value = (XtArgVal)CreateScrollbar(w, True);

	if (w->viewport.allowvert)
	    clip_args[0].value = (XtArgVal)CreateScrollbar(w, True);
    }

    clip_args[2].value = (XtArgVal)Max(w->core.width,1);
    clip_args[3].value = (XtArgVal)Max(w->core.height,1);
    w->viewport.clip = XtCreateWidget( "clip", widgetClass, new,
				       clip_args, XtNumber(clip_args) );
    XtManageChild( w->viewport.clip );	/* see ChangeManaged() */
}

static void Realize(widget, value_mask, attributes)
    Widget widget;
    XtValueMask *value_mask;
    XSetWindowAttributes *attributes;
{
    ViewportWidget w = (ViewportWidget)widget;
    register Widget child = w->viewport.child;
    register Widget clip = w->viewport.clip;

    *value_mask |= CWBitGravity;
    attributes->bit_gravity = NorthWestGravity;
    (*w->core.widget_class->core_class.superclass
     ->core_class.realize)(widget, value_mask, attributes);

    (*w->core.widget_class->core_class.resize)(widget);	/* turn on bars */

    if (child) {
	XtMoveWidget( child, (Position)0, (Position)0 );
	XtRealizeWidget( clip );
	XtRealizeWidget( child );
	XReparentWindow( XtDisplay(w), XtWindow(child), XtWindow(clip),
			 (Position)0, (Position)0 );
	XtMapWidget( child );
    }
}

/* ARGSUSED */
static Boolean SetValues(current, request, new, last)
    Widget current, request, new;
    Boolean last;
{
    ViewportWidget w = (ViewportWidget)new;
    ViewportWidget cw = (ViewportWidget)current;

    if (w->viewport.forcebars != cw->viewport.forcebars) {
	if (w->viewport.forcebars) {
	    if (w->viewport.allowhoriz && !w->viewport.horiz_bar)
		CreateScrollbar( w, True );
	    if (w->viewport.allowvert && !w->viewport.vert_bar)
		CreateScrollbar( w, False );
	}
    }

    /* take care of bars, &tc. */
    (*w->core.widget_class->core_class.resize)( new );
    return False;
}


static void ChangeManaged(widget)
    Widget widget;
{
    ViewportWidget w = (ViewportWidget)widget;
    register int num_children = w->composite.num_children;
    register Widget child, *childP;
    register int i;

    child = (Widget)NULL;
    for (childP=w->composite.children, i=0; i < num_children; childP++, i++) {
	if (XtIsManaged(*childP)
	    && *childP != w->viewport.clip
	    && *childP != w->viewport.horiz_bar
	    && *childP != w->viewport.vert_bar)
	{
	    child = *childP;
	    break;
	}
    }

    if (child != w->viewport.child) {
	w->viewport.child = child;
	if (child) {
	    XtResizeWidget( child, child->core.width,
			    child->core.height, (Dimension)0 );
	    if (XtIsRealized(widget) && !XtIsRealized(child)) {
		XtMoveWidget( child, (Position)0, (Position)0 );
		XtRealizeWidget( child );
		XReparentWindow( XtDisplay(w), XtWindow(child),
				 XtWindow(w->viewport.clip),
				 (Position)0, (Position)0 );
		XtMapWidget( child );
	    }
	    /* %%% DoLayout should be FormClass method */
	    if (DoLayout( widget, child->core.width, child->core.height ))
		(*widget->core.widget_class->core_class.resize)( widget );
	    /* %%% do we need to hide this child from Form?  */
	}
    }

#ifdef notdef
    (*((CompositeWidgetClass)w->core.widget_class->core_class.superclass)
     ->composite_class.change_managed)( widget );
#endif
}


static void SetBar(w, top, length, total)
    Widget w;
    Position top;
    Dimension length, total;
{
    XtScrollBarSetThumb(w, (float) top / total, (float) length / total );
}

static void RedrawThumbs(w)
  ViewportWidget w;
{
    register Widget child = w->viewport.child;
    register Widget clip = w->viewport.clip;

    if (w->viewport.horiz_bar)
	SetBar( w->viewport.horiz_bar, -(child->core.x),
	        clip->core.width, child->core.width );

    if (w->viewport.vert_bar)
	SetBar( w->viewport.vert_bar, -(child->core.y),
	        clip->core.height, child->core.height );
}



static void MoveChild(w, x, y)
    ViewportWidget w;
    Position x, y;
{
    register Widget child = w->viewport.child;
    register Widget clip = w->viewport.clip;

    /* make sure we never move past right/bottom borders */
    if (-x + clip->core.width > child->core.width)
	x = -(child->core.width - clip->core.width);

    if (-y + clip->core.height > child->core.height)
	y = -(child->core.height - clip->core.height);

    /* make sure we never move past left/top borders */
    if (x >= 0) x = 0;
    if (y >= 0) y = 0;

    XtMoveWidget(child, x, y);

    RedrawThumbs(w);
}

static void Resize(widget)
    Widget widget;
{
    ViewportWidget w = (ViewportWidget)widget;
    register Widget child = w->viewport.child;
    register Widget clip = w->viewport.clip;
    ViewportConstraints constraints
	= (ViewportConstraints)clip->core.constraints;
    int   lw, lh;
    Boolean needshoriz, needsvert;
    int clip_width, clip_height;
    int child_width, child_height;

    if (child) {
	child_width = child->core.width;
	child_height = child->core.height;
    }
    else {
	child_width = 0;
	child_height = 0;
    }

    clip_width = w->core.width;
    clip_height = w->core.height;
    if (w->viewport.forcebars) {
	if (needsvert = w->viewport.allowvert)
	    clip_width -= w->viewport.vert_bar->core.width +
			  w->viewport.vert_bar->core.border_width;
	if (needshoriz = w->viewport.allowhoriz)
	    clip_height -= w->viewport.horiz_bar->core.height +
			   w->viewport.horiz_bar->core.border_width;
	AssignMax( clip_width, 1 );
	AssignMax( clip_height, 1 );
    }
    else {
	do {
	    lw = clip_width;
	    lh = clip_height;
	    if (w->viewport.allowhoriz)
		needshoriz = (Boolean)(child_width > clip_width);
	    else {
		child_width = clip_width;
		needshoriz = FALSE;
	    }
	    if (w->viewport.allowvert)
		needsvert = (Boolean)(child_height > clip_height);
	    else {
		child_height = clip_height;
		needsvert = FALSE;
	    }
	    if (! XtIsRealized(widget))
		needsvert = needshoriz = FALSE;
	    if (needshoriz && !w->viewport.horiz_bar)
		CreateScrollbar(w, True);
	    if (needsvert && !w->viewport.vert_bar)
		CreateScrollbar(w, False);
	    clip_width = w->core.width -
		(needsvert ? w->viewport.vert_bar->core.width
			     + w->viewport.vert_bar->core.border_width : 0);
	    clip_height = w->core.height -
		(needshoriz ? w->viewport.horiz_bar->core.height
			      + w->viewport.horiz_bar->core.border_width : 0);
	    AssignMax( clip_width, 1 );
	    AssignMax( clip_height, 1 );
	} while (lw != clip_width || lh != clip_height);
    }

    if (XtIsRealized(clip))
	XRaiseWindow( XtDisplay(clip), XtWindow(clip) );

    XtMoveWidget( clip,
		  needsvert ? (w->viewport.useright ? 0 :
			       w->viewport.vert_bar->core.width +
			       w->viewport.vert_bar->core.border_width) : 0,
		  needshoriz ? (w->viewport.usebottom ? 0 :
				w->viewport.horiz_bar->core.height +
			        w->viewport.horiz_bar->core.border_width) : 0);
    XtResizeWidget( clip, clip_width, clip_height, 0 );
	
    if (w->viewport.horiz_bar) {
	register Widget bar = w->viewport.horiz_bar;
	if (!needshoriz) {
	    constraints->form.vert_base = (Widget)NULL;
	    XtDestroyWidget( w->viewport.horiz_bar );
	    w->viewport.horiz_bar = (Widget)NULL;
	}
	else {
	    register int bw = bar->core.border_width;
	    XtResizeWidget( bar, clip_width, bar->core.height, bw );
	    XtMoveWidget( bar,
			  ((needsvert && !w->viewport.useright)
			   ? w->viewport.vert_bar->core.width
			   : -bw),
			  (w->viewport.usebottom
			    ? w->core.height - bar->core.height - bw
			    : -bw) );
	    XtSetMappedWhenManaged( bar, True );
	}
    }

    if (w->viewport.vert_bar) {
	register Widget bar = w->viewport.vert_bar;
	if (!needsvert) {
	    constraints->form.horiz_base = (Widget)NULL;
	    XtDestroyWidget( bar );
	    w->viewport.vert_bar = (Widget)NULL;
	}
	else {
	    register int bw = bar->core.border_width;
	    XtResizeWidget( bar, bar->core.width, clip_height, bw );
	    XtMoveWidget( bar,
			  (w->viewport.useright
			   ? w->core.width - bar->core.width - bw 
			   : -bw),
			  ((needshoriz && !w->viewport.usebottom)
			    ? w->viewport.horiz_bar->core.height
			    : -bw) );
	    XtSetMappedWhenManaged( bar, True );
	}
    }

    if (child) {
	XtResizeWidget( child, (Dimension)child_width,
		        (Dimension)child_height, 0 );
	MoveChild(w, child->core.x, child->core.y);
    }
}



/* Semi-public routines */


static void ScrollUpDownProc(widget, closure, call_data)
    Widget widget;
    caddr_t closure;
    caddr_t call_data;
{
    ViewportWidget w = (ViewportWidget)closure;
    register Widget child = w->viewport.child;
    int pix = (int)call_data;
    Position x, y;

    x = child->core.x - ((widget == w->viewport.horiz_bar) ? pix : 0);
    y = child->core.y - ((widget == w->viewport.vert_bar) ? pix : 0);
    MoveChild(w, x, y);
}


/* ARGSUSED */
static void ThumbProc(widget, closure, percent)
    Widget widget;
    caddr_t closure;
    float percent;
{
    ViewportWidget w = (ViewportWidget)closure;
    register Widget child = w->viewport.child;
    Position x, y;

    if (widget == w->viewport.horiz_bar)
	x = -(int)(percent * child->core.width);
    else
	x = child->core.x;

    if (widget == w->viewport.vert_bar)
	y = -(int)(percent * child->core.height);
    else
	y = child->core.y;

    MoveChild(w, x, y);
}


static XtGeometryResult GeometryManager(child, request, reply)
    Widget child;
    XtWidgetGeometry *request, *reply;
{
    ViewportWidget w = (ViewportWidget)child->core.parent;
    Boolean rWidth = request->request_mode & CWWidth;
    Boolean rHeight = request->request_mode & CWHeight;
    XtWidgetGeometry allowed, myrequest;
    XtGeometryResult result;
    Boolean resized;

    if (child != w->viewport.child
	|| request->request_mode & ~(CWWidth | CWHeight | CWBorderWidth)
	|| ((request->request_mode & CWBorderWidth)
	    && request->border_width > 0))
	return XtGeometryNo;

    allowed = *request;
    result = XtGeometryYes;

    /* %%% DoLayout should be a FormClass method */
    resized = DoLayout( (Widget)w,
		        (rWidth ? request->width : w->core.width),
		        (rHeight ? request->height : w->core.height) );

    if (rWidth && w->core.width != request->width) {
	if (!w->viewport.allowhoriz) {
	    allowed.width = w->core.width;
	    result = XtGeometryAlmost;
	}
    }
    if (rHeight && w->core.height != request->height) {
	if (!w->viewport.allowvert) {
	    allowed.height = w->core.height;
	    result = XtGeometryAlmost;
	}
    }
    *reply = allowed;
    if (result == XtGeometryYes) {
	Boolean needs_horiz = False, needs_vert = False;
	if (rWidth)  child->core.width = request->width;
	if (rHeight) child->core.height = request->height;
	myrequest.request_mode = 0;
	if (child->core.width > w->core.width) needs_horiz = True;
	if (child->core.height > w->core.height) needs_vert = True;
	if (needs_horiz && !w->viewport.horiz_bar && XtIsRealized((Widget)w)) {
	    CreateScrollbar( w, True );
	    if ((myrequest.height = w->viewport.horiz_bar->core.height << 1)
		> w->core.height)
		myrequest.request_mode |= CWHeight;
	}
	if (needs_vert && !w->viewport.vert_bar && XtIsRealized((Widget)w)) {
	    CreateScrollbar( w, False );
	    if ((myrequest.width = w->viewport.vert_bar->core.width << 1)
		> w->core.width)
		myrequest.request_mode |= CWWidth;
	}
	if (myrequest.request_mode) {
	    XtGeometryResult ans =
		XtMakeGeometryRequest( (Widget)w, &myrequest, &myrequest );
	    if (ans == XtGeometryAlmost)
		ans = XtMakeGeometryRequest( (Widget)w, &myrequest, NULL );
	    if (ans == XtGeometryYes)
		resized = True;
	}
    }

    if (resized) (*w->core.widget_class->core_class.resize)( (Widget)w );
    return result;
}


/* %%% DoLayout should be a FormClass method */
static Boolean DoLayout(w, width, height)
    Widget w;
    Dimension width, height;
{
    XtWidgetGeometry geometry;
    XtGeometryResult result;

    geometry.request_mode = CWWidth | CWHeight;
    geometry.width = width;
    geometry.height = height;

    if (XtIsRealized(w)) {
	if (((ViewportWidget)w)->viewport.allowhoriz)
	    geometry.width = Min(w->core.width, width);
	if (((ViewportWidget)w)->viewport.allowvert)
	    geometry.height = Min(w->core.height, height);
    }
    if ((result = XtMakeGeometryRequest(w, &geometry, &geometry))
	== XtGeometryAlmost)
	result = XtMakeGeometryRequest(w, &geometry, NULL);

    return (result == XtGeometryYes);
}
