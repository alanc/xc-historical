/* $XConsortium: Manage.c,v 1.26 93/08/27 16:29:23 kaleb Exp $ */

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

static String XtNinvalidChild = "invalidChild";
static String XtNxtUnmanageChildren = "xtUnmanageChildren";
static String XtNxtManageChildren = "xtManageChildren";

void XtUnmanageChildren(children, num_children)
    WidgetList children;
    Cardinal num_children;
{
    CompositeWidget	parent;
    Widget		child;
    Cardinal		num_unique_children, i;
    XtWidgetProc	change_managed;
    Bool		parent_realized;
    XtAppContext	app;

    if (num_children == 0) return;
    if (children[0] == NULL) {
	XtWarningMsg(XtNinvalidChild,XtNxtUnmanageChildren,XtCXtToolkitError,
                  "Null child passed to XtUnmanageChildren",
		  (String *)NULL, (Cardinal *)NULL);
	return;
    }
    app = XtWidgetToApplicationContext(children[0]);
    LOCK_APP(app);
    parent = (CompositeWidget) children[0]->core.parent;
    if (parent->core.being_destroyed) {
	UNLOCK_APP(app);
	return;
    }

    if (XtIsComposite((Widget) parent)) {
	LOCK_PROCESS;
        change_managed = ((CompositeWidgetClass) parent->core.widget_class)
		    ->composite_class.change_managed;
	UNLOCK_PROCESS;
	parent_realized = XtIsRealized((Widget)parent);
    }

    num_unique_children = 0;
    for (i = 0; i < num_children; i++) {
	child = children[i];
	if (child == NULL) {
	    XtAppWarningMsg(app,
		  XtNinvalidChild,XtNxtUnmanageChildren,XtCXtToolkitError,
                  "Null child passed to XtUnmanageChildren",
		  (String *)NULL, (Cardinal *)NULL);
	    UNLOCK_APP(app);
	    return;
	}
        if ((CompositeWidget) child->core.parent != parent) {
	   XtAppWarningMsg(app,
		   "ambiguousParent",XtNxtUnmanageChildren,XtCXtToolkitError,
           "Not all children have same parent in XtUnmanageChildren",
             (String *)NULL, (Cardinal *)NULL);
	} else
        if (child->core.managed) {
            num_unique_children++;
	    child->core.managed = FALSE;
            if (XtIsWidget(child)
		&& XtIsRealized(child)
		&& child->core.mapped_when_managed)
                    XtUnmapWidget(child);
            else
	    { /* RectObj child */
		Widget pw = child->core.parent;
		RectObj r = (RectObj) child;
		while ((pw!=NULL) && (!XtIsWidget(pw))) pw = pw->core.parent;
		if ((pw!=NULL) && XtIsRealized (pw))
		    XClearArea (XtDisplay (pw), XtWindow (pw),
			r->rectangle.x, r->rectangle.y,
			r->rectangle.width + (r->rectangle.border_width << 1),
			r->rectangle.height + (r->rectangle.border_width << 1),
			TRUE);
	    }

        }
    }
    if (num_unique_children != 0 && change_managed != NULL && parent_realized) {
	(*change_managed) ((Widget)parent);
    }
    UNLOCK_APP(app);
} /* XtUnmanageChildren */


void XtUnmanageChild(child)
    Widget child;
{
    XtUnmanageChildren(&child, (Cardinal)1);
} /* XtUnmanageChild */


void XtManageChildren(children, num_children)
    WidgetList  children;
    Cardinal    num_children;
{
#define MAXCHILDREN 100
    CompositeWidget	parent;
    Widget		child;
    Cardinal		num_unique_children, i;
    XtWidgetProc	change_managed;
    WidgetList		unique_children;
    Widget		cache[MAXCHILDREN];
    Bool		parent_realized;
    XtAppContext	app;

    if (num_children == 0) return;
    if (children[0] == NULL) {
	XtWarningMsg(XtNinvalidChild,XtNxtManageChildren,XtCXtToolkitError,
         "null child passed to XtManageChildren",
	 (String *)NULL, (Cardinal *)NULL);
	return;
    } 
    app = XtWidgetToApplicationContext(children[0]);
    LOCK_APP(app);
    parent = (CompositeWidget) children[0]->core.parent;
    if (XtIsComposite((Widget) parent)) {
	LOCK_PROCESS;
        change_managed = ((CompositeWidgetClass) parent->core.widget_class)
		    ->composite_class.change_managed;
	UNLOCK_PROCESS;
	parent_realized = XtIsRealized((Widget)parent);

    } else {
	XtAppErrorMsg(app,
		"invalidParent",XtNxtManageChildren, XtCXtToolkitError,
	    "Attempt to manage a child when parent is not Composite",
	    (String *) NULL, (Cardinal *) NULL);
    }
    if (parent->core.being_destroyed) {
	UNLOCK_APP(app);
	return;
    }

    /* Construct new list of children that really need to be operated upon. */
    if (num_children <= MAXCHILDREN) {
	unique_children = cache;
    } else {
	unique_children = (WidgetList) XtMalloc(num_children * sizeof(Widget));
    }
    num_unique_children = 0;
    for (i = 0; i < num_children; i++) {
	child = children[i];
	if (child == NULL) {
	    XtAppWarningMsg(app,
		XtNinvalidChild,XtNxtManageChildren,XtCXtToolkitError,
		"null child passed to XtManageChildren",
		(String *)NULL, (Cardinal *)NULL);
	    if (unique_children != cache) XtFree((char *) unique_children);
	    UNLOCK_APP(app);
	    return;
	}
#ifdef DEBUG
	if (!XtIsRectObj(child)) {
	    String params[2];
	    Cardinal num_params = 2;
	    params[0] = XtName(child);
	    params[1] = child->core.widget_class->core_class.class_name;
	    XtAppWarningMsg(app,
			    "notRectObj",XtNxtManageChildren,XtCXtToolkitError,
			    "child \"%s\", class %s is not a RectObj",
			    params, &num_params);
	    continue;
	}
#endif /*DEBUG*/
        if ((CompositeWidget) child->core.parent != parent) {
	    XtAppWarningMsg(app,
		    "ambiguousParent",XtNxtManageChildren,XtCXtToolkitError,
		"Not all children have same parent in XtManageChildren",
		(String *)NULL, (Cardinal *)NULL);
	} else if (! child->core.managed && !child->core.being_destroyed) {
	    unique_children[num_unique_children++] = child;
	    child->core.managed = TRUE;
	}
    }

    if (num_unique_children != 0 && parent_realized) {
	/* Compute geometry of new managed set of children. */
	if (change_managed != NULL) (*change_managed) ((Widget)parent);

	/* Realize each child if necessary, then map if necessary */
	for (i = 0; i < num_unique_children; i++) {
	    child = unique_children[i];
	    if (XtIsWidget(child)) {
		if (! XtIsRealized(child)) XtRealizeWidget(child);
		if (child->core.mapped_when_managed) XtMapWidget(child);
	    } else { /* RectObj child */
		Widget pw = child->core.parent;
		RectObj r = (RectObj) child;
		while ((pw!=NULL) && (!XtIsWidget(pw)))
		    pw = pw->core.parent;
		if (pw != NULL)
		    XClearArea (XtDisplay (pw), XtWindow (pw),
		    r->rectangle.x, r->rectangle.y,
		    r->rectangle.width + (r->rectangle.border_width << 1),
		    r->rectangle.height + (r->rectangle.border_width << 1),
		    TRUE);
            }
        }
    }

    if (unique_children != cache) XtFree((char *) unique_children);
    UNLOCK_APP(app);
} /* XtManageChildren */


void XtManageChild(child)
    Widget child;
{
    XtManageChildren(&child, (Cardinal) 1);
} /* XtManageChild */


#if NeedFunctionPrototypes
void XtSetMappedWhenManaged(
    register Widget widget,
    _XtBoolean	    mapped_when_managed
    )
#else
void XtSetMappedWhenManaged(widget, mapped_when_managed)
    register Widget widget;
    Boolean	    mapped_when_managed;
#endif
{
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    if (widget->core.mapped_when_managed == mapped_when_managed) {
	UNLOCK_APP(app);
	return;
    }
    widget->core.mapped_when_managed = mapped_when_managed;
    if (! XtIsManaged(widget)) {
	UNLOCK_APP(app);
	return;
    }

    if (mapped_when_managed) {
	/* Didn't used to be mapped when managed.		*/
	if (XtIsRealized(widget)) XtMapWidget(widget);
    } else {
	/* Used to be mapped when managed.			*/
	if (XtIsRealized(widget)) XtUnmapWidget(widget);
    }
    UNLOCK_APP(app);
} /* XtSetMappedWhenManaged */
