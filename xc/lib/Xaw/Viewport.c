#ifndef lint
static char rcsid[] = "$Header: Viewport.c,v 1.1 87/09/11 07:59:43 toddb Exp $";
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

static void Initialize(), Realize(), Resize(), ChangeManaged();
static Boolean SetValues();
static XtGeometryResult GeometryManager();

ViewportClassRec viewportClassRec = {
  { /* core_class fields */
    /* superclass	  */	(WidgetClass) &formClassRec,
    /* class_name	  */	"Viewport",
    /* widget_size	  */	sizeof(ViewportRec),
    /* class_initialize	  */	NULL,
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
    /* expose		  */	XtInheritExpose,
    /* set_values	  */	SetValues,
    /* accept_focus	  */	NULL,
    /* callback_private	  */	NULL,
    /* reserved_private	  */	NULL
  },
  { /* composite_class fields */
    /* geometry_manager	  */	GeometryManager,
    /* change_managed	  */	ChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
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
	     horizontal ? w->core.width : w->core.height);
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
		        w->core.height - bar->core.height - bw, bw );
    }
    else {
	Dimension bw = bar->core.border_width;
	w->viewport.vert_bar = bar;
	constraints->form.horiz_base = bar;
	XtResizeWidget( clip, w->core.width - bar->core.width - bw, /* %%% */
		        bar->core.height, bw );
    }
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

    (*w->core.widget_class->core_class.superclass
     ->core_class.realize)(widget, value_mask, attributes);

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
	    /* %%% do we need to hide this child from Form?  */
	}
	w->viewport.child = child;
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
	if (w->viewport.allowvert)
	    clip_width -= w->viewport.vert_bar->core.width +
			  w->viewport.vert_bar->core.border_width;
	if (w->viewport.allowhoriz)
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
	    if (needshoriz && !w->viewport.horiz_bar) {
		CreateScrollbar(w, True);
	    }
	    if (needsvert && !w->viewport.vert_bar) {
		CreateScrollbar(w, False);
	    }
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
    resized = False;

    if ((!w->viewport.allowhoriz && rWidth) ||
	(!w->viewport.allowvert && rHeight)) {
	myrequest.request_mode = CWWidth | CWHeight;
	myrequest.width = rWidth ?
	      (w->viewport.allowhoriz ?
		  Min(w->core.width, request->width) : request->width)
	    : w->core.width;
	myrequest.height = rHeight ?
	      (w->viewport.allowvert ?
		  Min(w->core.height, request->height) : request->height)
	    : w->core.height;
	if (w->core.width != myrequest.width ||
	    w->core.height != myrequest.height) {
	    result = XtMakeGeometryRequest((Widget)w, &myrequest, &allowed);
	    if (result == XtGeometryYes)
		resized = True;
	    else {
		if (result == XtGeometryNo) {
		    if (w->viewport.allowhoriz)
			allowed.width = Max(w->core.width, request->width);
		    else
			allowed.width = w->core.width;
		    if (w->viewport.allowvert)
			allowed.height = Max(w->core.height, request->height);
		    else
			allowed.height = w->core.height;
		}
		if ((rWidth && allowed.width != request->width) ||
		    (rHeight && allowed.height != request->height)) {
		    *reply = allowed;
		    result = XtGeometryAlmost;
		}
	    }
	}
    }

    if (result == XtGeometryYes) {
	Boolean needs_horiz = False, needs_vert = False;
	if (rWidth)  child->core.width = request->width;
	if (rHeight) child->core.height = request->height;
	myrequest.request_mode = 0;
	if (child->core.width > w->core.width) needs_horiz = True;
	if (child->core.height > w->core.height) needs_vert = True;
	if (needs_horiz && !w->viewport.horiz_bar) {
	    CreateScrollbar( w, True );
	    if ((myrequest.height = w->viewport.horiz_bar->core.height << 1)
		> w->core.height)
		myrequest.request_mode |= CWHeight;
	}
	if (needs_vert && !w->viewport.vert_bar) {
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
	if (resized) (*w->core.widget_class->core_class.resize)( (Widget)w );
    }

    return result;
}
