/* $XConsortium: Geometry.c,v 1.60 94/01/14 17:56:12 kaleb Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital, MIT, or Sun not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

#include "IntrinsicI.h"
#include "ShellP.h"

static void ClearRectObjAreas(r, old)
    RectObj r;
    XWindowChanges* old;
{
    Widget pw = _XtWindowedAncestor((Widget)r);
    int bw2;

    bw2 = old->border_width << 1;
    XClearArea( XtDisplay(pw), XtWindow(pw),
		old->x, old->y,
		old->width + bw2, old->height + bw2,
		TRUE );

    bw2 = r->rectangle.border_width << 1;
    XClearArea( XtDisplay(pw), XtWindow(pw),
		(int)r->rectangle.x, (int)r->rectangle.y,
		(unsigned int)(r->rectangle.width + bw2),
	        (unsigned int)(r->rectangle.height + bw2),
		TRUE );
}

/*
 * Internal function used by XtMakeGeometryRequest and XtSetValues.
 * Returns more data than the public interface.  Does not convert
 * XtGeometryDone to XtGeometryYes.
 *
 * clear_rect_obj - *** RETURNED ***  
 *		    TRUE if the rect obj has been cleared, false otherwise.
 */

XtGeometryResult 
_XtMakeGeometryRequest (widget, request, reply, clear_rect_obj)
    Widget widget;
    XtWidgetGeometry *request, *reply;
    Boolean * clear_rect_obj;
{
    XtWidgetGeometry    junk;
    XtGeometryHandler manager;
    XtGeometryResult returnCode;
    Widget parent = widget->core.parent;
    XtGeometryMask	changeMask;
    Boolean managed, parentRealized, rgm = False;
    XWindowChanges changes;

    *clear_rect_obj = FALSE;

    if (XtIsShell(widget)) {
	ShellClassExtension ext;
	LOCK_PROCESS;
	for (ext = (ShellClassExtension)((ShellWidgetClass)XtClass(widget))
		   ->shell_class.extension;
	     ext != NULL && ext->record_type != NULLQUARK;
	     ext = (ShellClassExtension)ext->next_extension);

	if (ext != NULL) {
	    if (  ext->version == XtShellExtensionVersion
		  && ext->record_size == sizeof(ShellClassExtensionRec)) {
		manager = ext->root_geometry_manager;
		rgm = True;
	    } else {
		String params[1];
		Cardinal num_params = 1;
		params[0] = XtClass(widget)->core_class.class_name;
		XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		     "invalidExtension", "xtMakeGeometryRequest",
		     XtCXtToolkitError,
		     "widget class %s has invalid ShellClassExtension record",
		     params, &num_params);
	    }
	} else {
	    XtAppErrorMsg(XtWidgetToApplicationContext(widget),
			  "internalError", "xtMakeGeometryRequest",
			  XtCXtToolkitError,
			  "internal error; ShellClassExtension is NULL",
			  NULL, NULL);
	}
	managed = True;
	parentRealized = TRUE;
	UNLOCK_PROCESS;
    } else if (parent == NULL) {
	XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		      "invalidParent","xtMakeGeometryRequest",XtCXtToolkitError,
		      "non-shell has no parent in XtMakeGeometryRequest",
		      (String *)NULL, (Cardinal *)NULL);
    } else /* not shell */ {
	managed = XtIsManaged(widget);

	if (XtIsComposite(parent)) {
	    parentRealized = XtIsRealized(parent);
	    LOCK_PROCESS;
	    manager = ((CompositeWidgetClass) (parent->core.widget_class))
		    ->composite_class.geometry_manager;
	    UNLOCK_PROCESS;
	} else if (managed) {
	    /* Should never happen - XtManageChildren should have checked */
	    XtAppErrorMsg(XtWidgetToApplicationContext(widget),
			  "invalidParent", "xtMakeGeometryRequest",
			  XtCXtToolkitError,
			  "XtMakeGeometryRequest - parent not composite",
			  (String *)NULL, (Cardinal *)NULL);
	} else {
	    /* no need to waste time checking if parent is actually realized
	     * at this point; since the child is unmanaged we need to perform
	     * the configure iff the child is realized, so we dummy the
	     * parentRealized checks below.
	     */
	    parentRealized = TRUE;
	}
    }

    if (managed && manager == (XtGeometryHandler) NULL) {
	XtErrorMsg("invalidGeometryManager","xtMakeGeometryRequest",
                 XtCXtToolkitError,
                 "XtMakeGeometryRequest - parent has no geometry manager",
                  (String *)NULL, (Cardinal *)NULL);
    }

    if (widget->core.being_destroyed) return XtGeometryNo;

    /* see if requesting anything to change */
    changeMask = 0;
    if (request->request_mode & CWStackMode
	&& request->stack_mode != XtSMDontChange) {
	    changeMask |= CWStackMode;
	    if (request->request_mode & CWSibling) {
		XtCheckSubclass(request->sibling, rectObjClass,
				"XtMakeGeometryRequest");
		changeMask |= CWSibling;
	    }
    }
    if (request->request_mode & CWX
	&& widget->core.x != request->x) changeMask |= CWX;
    if (request->request_mode & CWY
	&& widget->core.y != request->y) changeMask |= CWY;
    if (request->request_mode & CWWidth
	&& widget->core.width != request->width) changeMask |= CWWidth;
    if (request->request_mode & CWHeight
	&& widget->core.height != request->height) changeMask |= CWHeight;
    if (request->request_mode & CWBorderWidth
	&& widget->core.border_width != request->border_width)
	changeMask |= CWBorderWidth;
    if (! changeMask) return XtGeometryYes;
    changeMask |= (request->request_mode & XtCWQueryOnly);

    if ( !(changeMask & XtCWQueryOnly) && XtIsRealized(widget) ) {
	/* keep record of the current geometry so we know what's changed */
	changes.x = widget->core.x ;
	changes.y = widget->core.y ;
	changes.width = widget->core.width ;
	changes.height = widget->core.height ;
	changes.border_width = widget->core.border_width ;
    }

    if (!managed || !parentRealized) {
	/* Don't get parent's manager involved--assume the answer is yes */
	if (changeMask & XtCWQueryOnly) {
	    /* He was just asking, don't change anything, just tell him yes */
	    return XtGeometryYes;
	} else {
	    /* copy values from request to widget */
	    if (request->request_mode & CWX)
		widget->core.x = request->x;
	    if (request->request_mode & CWY)
		widget->core.y = request->y;
	    if (request->request_mode & CWWidth)
		widget->core.width = request->width;
	    if (request->request_mode & CWHeight)
		widget->core.height = request->height;
	    if (request->request_mode & CWBorderWidth)
		widget->core.border_width = request->border_width;
	    if (!parentRealized) return XtGeometryYes;
	    else returnCode = XtGeometryYes;
	}
    } else {
	/* go ask the widget's geometry manager */
	if (reply == (XtWidgetGeometry *) NULL) {
	    returnCode = (*manager)(widget, request, &junk);
	} else {
	    returnCode = (*manager)(widget, request, reply);
	}
    }

    /*
     * If Unrealized, not a XtGeometryYes, or a query-only then we are done.
     */

    if ((returnCode != XtGeometryYes) || 
	(changeMask & XtCWQueryOnly) || !XtIsRealized(widget)) {

	return returnCode;
    }

    if (XtIsWidget(widget)) {	/* reconfigure the window (if needed) */

	if (rgm) return returnCode;

	if (changes.x != widget->core.x) {
 	    changeMask |= CWX;
 	    changes.x = widget->core.x;
 	}
 	if (changes.y != widget->core.y) {
 	    changeMask |= CWY;
 	    changes.y = widget->core.y;
 	}
 	if (changes.width != widget->core.width) {
 	    changeMask |= CWWidth;
 	    changes.width = widget->core.width;
 	}
 	if (changes.height != widget->core.height) {
 	    changeMask |= CWHeight;
 	    changes.height = widget->core.height;
 	}
 	if (changes.border_width != widget->core.border_width) {
 	    changeMask |= CWBorderWidth;
 	    changes.border_width = widget->core.border_width;
 	}
	if (changeMask & CWStackMode) {
	    changes.stack_mode = request->stack_mode;
	    if (changeMask & CWSibling)
		if (XtIsWidget(request->sibling))
		    changes.sibling = XtWindow(request->sibling);
		else
		    changeMask &= ~(CWStackMode | CWSibling);
	}

	XConfigureWindow(XtDisplay(widget), XtWindow(widget),
			 changeMask, &changes);
    }
    else {			/* RectObj child of realized Widget */
	*clear_rect_obj = TRUE;
	ClearRectObjAreas((RectObj)widget, &changes);
    }

    return returnCode;
} /* _XtMakeGeometryRequest */


/* Public routines */

XtGeometryResult XtMakeGeometryRequest (widget, request, reply)
    Widget         widget;
    XtWidgetGeometry *request, *reply;
{
    Boolean junk;
    XtGeometryResult r;
    XtGeometryHookDataRec call_data;
    Widget hookobj = XtHooksOfDisplay(XtDisplayOfObject(widget));
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    if (XtHasCallbacks(hookobj, XtNgeometryHook) == XtCallbackHasSome) {
	call_data.type = XtHpreGeometry;
	call_data.widget = widget;
	call_data.request = request;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.geometryhook_callbacks, 
		(XtPointer)&call_data);
	call_data.result = r = 
	    _XtMakeGeometryRequest(widget, request, reply, &junk);
	call_data.type = XtHpostGeometry;
	call_data.reply = reply;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.geometryhook_callbacks, 
		(XtPointer)&call_data);
    } else {
	r = _XtMakeGeometryRequest(widget, request, reply, &junk);
    }
    UNLOCK_APP(app);

    return ((r == XtGeometryDone) ? XtGeometryYes : r);
}

#if NeedFunctionPrototypes
XtGeometryResult 
XtMakeResizeRequest(
    Widget	widget,
    _XtDimension width,
    _XtDimension height,
    Dimension	*replyWidth,
    Dimension	*replyHeight)
#else
XtGeometryResult 
XtMakeResizeRequest (widget, width, height, replyWidth, replyHeight)
    Widget	widget;
    Dimension	width, height;
    Dimension	*replyWidth, *replyHeight;
#endif
{
    XtWidgetGeometry request, reply;
    XtGeometryResult r;
    XtGeometryHookDataRec call_data;
    Boolean junk;
    Widget hookobj = XtHooksOfDisplay(XtDisplayOfObject(widget));
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    request.request_mode = CWWidth | CWHeight;
    request.width = width;
    request.height = height;

    if (XtHasCallbacks(hookobj, XtNgeometryHook) == XtCallbackHasSome) {
	call_data.type = XtHpreGeometry;
	call_data.widget = widget;
	call_data.request = &request;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.geometryhook_callbacks, 
		(XtPointer)&call_data);
	call_data.result = r = 
	    _XtMakeGeometryRequest(widget, &request, &reply, &junk);
	call_data.type = XtHpostGeometry;
	call_data.reply = &reply;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.geometryhook_callbacks, 
		(XtPointer)&call_data);
    } else {
	r = _XtMakeGeometryRequest(widget, &request, &reply, &junk);
    }
    if (replyWidth != NULL)
	if (r == XtGeometryAlmost && reply.request_mode & CWWidth)
	    *replyWidth = reply.width;
	else
	    *replyWidth = width;
    if (replyHeight != NULL)
	if (r == XtGeometryAlmost && reply.request_mode & CWHeight)
	    *replyHeight = reply.height;
	else
	    *replyHeight = height;
    UNLOCK_APP(app);
    return r;
} /* XtMakeResizeRequest */

void XtResizeWindow(w)
    Widget w;
{
    WIDGET_TO_APPCON(w);

    LOCK_APP(app);
    if (XtIsRealized(w)) {
	XWindowChanges changes;
	changes.width = w->core.width;
	changes.height = w->core.height;
	changes.border_width = w->core.border_width;
	XConfigureWindow(XtDisplay(w), XtWindow(w),
	    (unsigned) CWWidth | CWHeight | CWBorderWidth, &changes);
    }
    UNLOCK_APP(app);
} /* XtResizeWindow */

static void
ResizeWidget(call_data, old)
    XtConfigureHookData call_data;
    XWindowChanges* old;
{
    XtWidgetProc resize;
    Widget w = call_data->widget;

    if (XtIsRealized(w)) {
	if (XtIsWidget(w))
	    XConfigureWindow(XtDisplay(w), XtWindow(w), 
			     call_data->changeMask, &call_data->changes);
	else {
	    Widget pw = _XtWindowedAncestor(w);
	    XWindowChanges* new = &call_data->changes;
	    old->width += (old->border_width << 1);
	    old->height += (old->border_width << 1);
	    if ((Dimension)(new->width + (new->border_width << 1)) > old->width)
		old->width = new->width + (new->border_width << 1);
	    if ((Dimension)(new->height + (new->border_width << 1)) > old->height)
		old->height = new->height + (new->border_width << 1);
	    XClearArea(XtDisplay(pw), XtWindow(pw),
		       (int)w->core.x, (int)w->core.y,
		       (unsigned int)old->width, (unsigned int)old->height,
		       TRUE);
	}
    }

    LOCK_PROCESS;
    resize = XtClass(w)->core_class.resize;
    UNLOCK_PROCESS;
    if ((call_data->changeMask & (CWWidth | CWHeight)) && 
	resize != (XtWidgetProc) NULL)
	(*resize)(w);
}

#if NeedFunctionPrototypes
void XtResizeWidget(
    Widget w,
    _XtDimension width,
    _XtDimension height,
    _XtDimension borderWidth
    )
#else
void XtResizeWidget(w, width, height, borderWidth)
    Widget w;
    Dimension width, height, borderWidth;
#endif
{
    XtConfigureHookDataRec call_data;
    XWindowChanges old;
    WIDGET_TO_APPCON(w);

    LOCK_APP(app);
    call_data.widget = w;
    call_data.changeMask = (XtGeometryMask) 0;
    if ((old.width = w->core.width) != width) {
	call_data.changes.width = w->core.width = width;
	call_data.changeMask |= CWWidth;
    }

    if ((old.height = w->core.height) != height) {
	call_data.changes.height = w->core.height = height;
	call_data.changeMask |= CWHeight;
    }

    if ((old.border_width = w->core.border_width) != borderWidth) {
	call_data.changes.border_width = w->core.border_width = borderWidth;
	call_data.changeMask |= CWBorderWidth;
    }

    if (call_data.changeMask != 0) {
	Widget hookobj = XtHooksOfDisplay(XtDisplayOfObject(w));;
	if (XtHasCallbacks(hookobj, XtNconfigureHook) == XtCallbackHasSome) {
	    call_data.type = XtHpreConfigure;
	    XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.confighook_callbacks, 
		(XtPointer)&call_data);
	    ResizeWidget(&call_data, &old);
	    call_data.type = XtHpostConfigure;
	    XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.confighook_callbacks, 
		(XtPointer)&call_data);
	} else {
	    ResizeWidget(&call_data, &old);
	}
    }
    UNLOCK_APP(app);
} /* XtResizeWidget */

static void
ConfigureWidget(call_data, old)
    XtConfigureHookData call_data;
    XWindowChanges* old;
{
    XtWidgetProc resize;
    Widget w = call_data->widget;

    if (XtIsRealized(w)) {
	if (XtIsWidget(w))
	    XConfigureWindow(XtDisplay(w), XtWindow(w), 
			     call_data->changeMask, &call_data->changes);
	else
	    ClearRectObjAreas((RectObj)w, old);
    }

    LOCK_PROCESS;
    resize = XtClass(w)->core_class.resize;
    UNLOCK_PROCESS;
    if ((call_data->changeMask & (CWWidth | CWHeight)) && 
	resize != (XtWidgetProc) NULL)
	(*resize)(w);
}

#if NeedFunctionPrototypes
void XtConfigureWidget(
    Widget w,
    _XtPosition x,
    _XtPosition y,
    _XtDimension width,
    _XtDimension height,
    _XtDimension borderWidth
    )
#else
void XtConfigureWidget(w, x, y, width, height, borderWidth)
    Widget w;
    Position x, y;
    Dimension width, height, borderWidth;
#endif
{
    XWindowChanges old;
    XtConfigureHookDataRec call_data;
    WIDGET_TO_APPCON(w);

    LOCK_APP(app);
    call_data.widget = w;
    call_data.changeMask = (XtGeometryMask) 0;
    if ((old.x = w->core.x) != x) {
	call_data.changes.x = w->core.x = x;
	call_data.changeMask |= CWX;
    }

    if ((old.y = w->core.y) != y) {
	call_data.changes.y = w->core.y = y;
	call_data.changeMask |= CWY;
    }

    if ((old.width = w->core.width) != width) {
	call_data.changes.width = w->core.width = width;
	call_data.changeMask |= CWWidth;
    }

    if ((old.height = w->core.height) != height) {
	call_data.changes.height = w->core.height = height;
	call_data.changeMask |= CWHeight;
    }

    if ((old.border_width = w->core.border_width) != borderWidth) {
	call_data.changes.border_width = w->core.border_width = borderWidth;
	call_data.changeMask |= CWBorderWidth;
    }

    if (call_data.changeMask != 0) {
	Widget hookobj = XtHooksOfDisplay(XtDisplayOfObject(w));
	if (XtHasCallbacks(hookobj, XtNconfigureHook) == XtCallbackHasSome) {
	    call_data.type = XtHpreConfigure;
	    XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.confighook_callbacks, 
		(XtPointer)&call_data);
	    ConfigureWidget(&call_data, &old);
	    call_data.type = XtHpostConfigure;
	    XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.confighook_callbacks, 
		(XtPointer)&call_data);
	} else {
	    ConfigureWidget(&call_data, &old);
	}
    }
    UNLOCK_APP(app);
} /* XtConfigureWidget */

static void
MoveWidget(call_data, old)
    XtConfigureHookData call_data;
    XWindowChanges* old;
{
    Widget w = call_data->widget;

    if (XtIsRealized(w)) {
	if (XtIsWidget(w))
	    XMoveWindow(XtDisplay(w), XtWindow(w), w->core.x, w->core.y);
	else {
	    old->width = w->core.width;
	    old->height = w->core.height;
	    old->border_width = w->core.border_width;
	    ClearRectObjAreas((RectObj)w, old);
	}
    }

}

#if NeedFunctionPrototypes
void XtMoveWidget(
    Widget w,
    _XtPosition x,
    _XtPosition y
    )
#else
void XtMoveWidget(w, x, y)
    Widget w;
    Position x, y;
#endif
{
    XtConfigureHookDataRec call_data;
    XWindowChanges old;
    WIDGET_TO_APPCON(w);

    LOCK_APP(app);
    call_data.widget = w;
    call_data.changeMask = (XtGeometryMask) 0;
    if ((old.x = w->core.x) != x) {
	call_data.changes.x = w->core.x = x;
	call_data.changeMask |= CWX;
    }

    if ((old.y = w->core.y) != y) {
	call_data.changes.y = w->core.y = y;
	call_data.changeMask |= CWY;
    }

    if (call_data.changeMask != 0) {
	Widget hookobj = XtHooksOfDisplay(XtDisplayOfObject(w));
	if (XtHasCallbacks(hookobj, XtNconfigureHook) == XtCallbackHasSome) {
	    call_data.type = XtHpreConfigure;
	    XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.confighook_callbacks, 
		(XtPointer)&call_data);
	    MoveWidget(&call_data, &old);
	    call_data.type = XtHpostConfigure;
	    XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.confighook_callbacks, 
		(XtPointer)&call_data);
	} else {
	    MoveWidget(&call_data, &old);
	}
    }
    UNLOCK_APP(app);
} /* XtMoveWidget */

#if NeedFunctionPrototypes
void XtTranslateCoords(
    register Widget w,
    _XtPosition x,
    _XtPosition y,
    register Position *rootx,	/* return */
    register Position *rooty	/* return */
    )
#else
void XtTranslateCoords(w, x, y, rootx, rooty)
    register Widget w;
    Position x, y;
    register Position *rootx, *rooty;	/* return */
#endif
{
    Position garbagex, garbagey;
    XtAppContext app = XtWidgetToApplicationContext(w);

    LOCK_APP(app);
    if (rootx == NULL) rootx = &garbagex;
    if (rooty == NULL) rooty = &garbagey;

    *rootx = x;
    *rooty = y;

    for (; w != NULL && ! XtIsShell(w); w = w->core.parent) {
	*rootx += w->core.x + w->core.border_width;
	*rooty += w->core.y + w->core.border_width;
    }

    if (w == NULL)
        XtAppWarningMsg(app,
		"invalidShell","xtTranslateCoords",XtCXtToolkitError,
                "Widget has no shell ancestor",
		(String *)NULL, (Cardinal *)NULL);
    else {
	Position x, y;
	extern void _XtShellGetCoordinates();
	_XtShellGetCoordinates( w, &x, &y );
	*rootx += x + w->core.border_width;
	*rooty += y + w->core.border_width;
    }
    UNLOCK_APP(app);
}

XtGeometryResult XtQueryGeometry(widget, intended, reply)
    Widget widget;
    register XtWidgetGeometry *intended; /* parent's changes; may be NULL */
    XtWidgetGeometry *reply;	/* child's preferred geometry; never NULL */
{
    XtWidgetGeometry null_intended;
    XtGeometryHandler query;
    XtGeometryResult result;
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    LOCK_PROCESS;
    query = XtClass(widget)->core_class.query_geometry;
    UNLOCK_PROCESS;
    reply->request_mode = 0;
    if (query != NULL) {
	if (intended == NULL) {
	    null_intended.request_mode = 0;
	    intended = &null_intended;
	}
	result = (*query) (widget, intended, reply);
    }
    else {
	result = XtGeometryYes;
    }

#define FillIn(mask, field) \
	if (!(reply->request_mode & mask)) reply->field = widget->core.field;

    FillIn(CWX, x);
    FillIn(CWY, y);
    FillIn(CWWidth, width);
    FillIn(CWHeight, height);
    FillIn(CWBorderWidth, border_width);
#undef FillIn

    if (!reply->request_mode & CWStackMode) 
	reply->stack_mode = XtSMDontChange;
    UNLOCK_APP(app);
    return result;
}
