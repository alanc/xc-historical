/* $XConsortium: Manage.c,v 1.28 94/01/06 18:15:34 kaleb Exp $ */

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
static String XtNxtChangeManagedSet = "xtChangeManagedSet";

static void UnmanageChildren(children, num_children, parent, num_unique_children, call_change_managed, caller_func)
    WidgetList children;
    Cardinal num_children;
    Widget parent;
    Cardinal* num_unique_children;
    Boolean call_change_managed;
    String caller_func;
{
    Widget		child;
    Cardinal		i;
    XtWidgetProc	change_managed;
    Bool		parent_realized;

    *num_unique_children = 0;

    if (XtIsComposite((Widget) parent)) {
	LOCK_PROCESS;
        change_managed = ((CompositeWidgetClass) parent->core.widget_class)
		    ->composite_class.change_managed;
	UNLOCK_PROCESS;
	parent_realized = XtIsRealized((Widget)parent);
    }

    for (i = 0; i < num_children; i++) {
	child = children[i];
	if (child == NULL) {
	    XtAppWarningMsg(XtWidgetToApplicationContext(parent),
		  XtNinvalidChild,caller_func,XtCXtToolkitError,
                  "Null child passed to XtUnmanageChildren",
		  (String *)NULL, (Cardinal *)NULL);
	    return;
	}
        if (child->core.parent != parent) {
	   XtAppWarningMsg(XtWidgetToApplicationContext(parent),
		   "ambiguousParent",caller_func,XtCXtToolkitError,
           "Not all children have same parent in UnmanageChildren",
             (String *)NULL, (Cardinal *)NULL);
	} else
        if (child->core.managed) {
            (*num_unique_children)++;
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
    if (call_change_managed && *num_unique_children != 0 &&
	change_managed != NULL && parent_realized) {
	(*change_managed) (parent);
    }
} /* UnmanageChildren */

void XtUnmanageChildren (children, num_children)
    WidgetList children;
    Cardinal num_children;
{
    Widget parent, hookobj;
    Cardinal ii;
    XtAppContext app;

    if (num_children == 0) return;
    if (children[0] == NULL) {
	XtWarningMsg(XtNinvalidChild,XtNxtUnmanageChildren,XtCXtToolkitError,
		     "Null child found in argument list to unmanage",
		     (String *)NULL, (Cardinal *)NULL);
	return;
    }
    app = XtWidgetToApplicationContext(children[0]);
    LOCK_APP(app);
    parent = children[0]->core.parent;
    if (parent->core.being_destroyed) {
	UNLOCK_APP(app);
	return;
    }
    UnmanageChildren(children, num_children, parent, &ii, 
		     (Boolean)True, XtNxtUnmanageChildren);
    hookobj = XtHooksOfDisplay(XtDisplayOfObject(children[0]));
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;

	call_data.old = (Widget)NULL;
	call_data.args = (ArgList)NULL;
	call_data.num_args = 0;
	for (ii = 0; ii < num_children; ii++) {
	    call_data.widget = children[ii];
	    XtCallCallbacks(hookobj, XtNchangeHook, (XtPointer)&call_data);
	}
    }
    UNLOCK_APP(app);
} /* XtUnmanageChildren */

void XtUnmanageChild(child)
    Widget child;
{
    XtUnmanageChildren(&child, (Cardinal)1);
} /* XtUnmanageChild */


static void ManageChildren(children, num_children, parent, call_change_managed, caller_func)
    WidgetList  children;
    Cardinal    num_children;
    Widget	parent;
    Boolean	call_change_managed;
    String	caller_func;
{
#define MAXCHILDREN 100
    Widget		child;
    Cardinal		num_unique_children, i;
    XtWidgetProc	change_managed;
    WidgetList		unique_children;
    Widget		cache[MAXCHILDREN];
    Bool		parent_realized;

    if (XtIsComposite((Widget) parent)) {
	LOCK_PROCESS;
        change_managed = ((CompositeWidgetClass) parent->core.widget_class)
		    ->composite_class.change_managed;
	UNLOCK_PROCESS;
	parent_realized = XtIsRealized((Widget)parent);
    } else {
	XtAppErrorMsg(XtWidgetToApplicationContext((Widget)parent),
		"invalidParent",caller_func, XtCXtToolkitError,
	    "Attempt to manage a child when parent is not Composite",
	    (String *) NULL, (Cardinal *) NULL);
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
	    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)parent),
		XtNinvalidChild,caller_func,XtCXtToolkitError,
		"null child passed to ManageChildren",
		(String *)NULL, (Cardinal *)NULL);
	    if (unique_children != cache) XtFree((char *) unique_children);
	    return;
	}
#ifdef DEBUG
	if (!XtIsRectObj(child)) {
	    String params[2];
	    Cardinal num_params = 2;
	    params[0] = XtName(child);
	    params[1] = child->core.widget_class->core_class.class_name;
	    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)parent),
			    "notRectObj",caller_func,XtCXtToolkitError,
			    "child \"%s\", class %s is not a RectObj",
			    params, &num_params);
	    continue;
	}
#endif /*DEBUG*/
        if (child->core.parent != parent) {
	    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)parent),
		    "ambiguousParent",caller_func,XtCXtToolkitError,
		"Not all children have same parent in XtManageChildren",
		(String *)NULL, (Cardinal *)NULL);
	} else if (! child->core.managed && !child->core.being_destroyed) {
	    unique_children[num_unique_children++] = child;
	    child->core.managed = TRUE;
	}
    }

    if ((call_change_managed || num_unique_children != 0) && parent_realized) {
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
} /* ManageChildren */

void XtManageChildren(children, num_children)
    WidgetList children;
    Cardinal num_children;
{
    Widget parent, hookobj;
    XtAppContext app;

    if (children[0] == NULL) {
	XtWarningMsg(XtNinvalidChild, XtNxtManageChildren, XtCXtToolkitError,
		     "null child passed to XtManageChildren",
		     (String*)NULL, (Cardinal*)NULL);
	return;
    }
    app = XtWidgetToApplicationContext(children[0]);
    LOCK_APP(app);
    parent = children[0]->core.parent;
    if (parent->core.being_destroyed) {
	UNLOCK_APP(app);
	return;
    }
    ManageChildren(children, num_children, parent, (Boolean)False, 
		   XtNxtManageChildren);
    hookobj = XtHooksOfDisplay(XtDisplayOfObject(children[0]));
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;
	int ii;

	call_data.old = (Widget)NULL;
	call_data.args = (ArgList)NULL;
	call_data.num_args = 0;
	for (ii = 0; ii < num_children; ii++) {
	    call_data.widget = children[ii];
	    XtCallCallbacks(hookobj, XtNchangeHook, (XtPointer)&call_data);
	}
    }
    UNLOCK_APP(app);
} /* XtManageChildren */

void XtManageChild(child)
    Widget child;
{
    XtManageChildren(&child, (Cardinal) 1);
} /* XtManageChild */


#if NeedFunctionPrototypes
void XtSetMappedWhenManaged(
    Widget widget,
    _XtBoolean mapped_when_managed
    )
#else
void XtSetMappedWhenManaged(widget, mapped_when_managed)
    Widget widget;
    Boolean mapped_when_managed;
#endif
{
    Widget hookobj;
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    if (widget->core.mapped_when_managed == mapped_when_managed) {
	UNLOCK_APP(app);
	return;
    }
    widget->core.mapped_when_managed = mapped_when_managed;

    hookobj = XtHooksOfDisplay(XtDisplay(widget));
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;

	call_data.old = (Widget) NULL;
	call_data.widget = widget;
	call_data.args = (ArgList)NULL;
	call_data.num_args = (Cardinal)0;
	XtCallCallbacks(hookobj, XtNchangeHook, (XtPointer)&call_data);
    }

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

void
#if NeedFunctionPrototypes
XtChangeManagedSet(
    WidgetList unmanage_children,
    Cardinal num_unmanage_children,
    WidgetList manage_children,
    Cardinal num_manage_children,
    XtCSMHookProc post_unmanage_pre_manage_hook,
    XtPointer client_data
)
#else
XtChangeManagedSet(unmanage_children, num_unmanage_children, manage_children, num_manage_children, post_unmanage_pre_manage_hook, client_data)
    WidgetList unmanage_children;
    Cardinal num_unmanage_children;
    WidgetList manage_children;
    Cardinal num_manage_children;
    XtCSMHookProc post_unmanage_pre_manage_hook;
    XtPointer client_data;
#endif
{
    Widget parent, hookobj;
    Cardinal num_unique_unm_children;
    XtAppContext app;
    Boolean call_change_managed = (Boolean)False;

    if ((num_unmanage_children == 0 && num_manage_children == 0) ||
	(unmanage_children == (WidgetList)NULL && 
	manage_children   == (WidgetList)NULL)) {
	    return;
    }
    if (unmanage_children != (WidgetList)NULL &&
	unmanage_children[0] != (Widget)NULL) {
	parent = XtParent(unmanage_children[0]);
    } else if (manage_children != (WidgetList)NULL &&
	manage_children[0] != (Widget)NULL) {
	parent = XtParent(manage_children[0]);
    } else {
	XtWarningMsg(XtNinvalidChild, XtNxtUnmanageChildren, XtCXtToolkitError,
		     "Null child found in argument list",
		     (String *)NULL, (Cardinal *)NULL);
	return;
    }
    app = XtWidgetToApplicationContext(parent);
    LOCK_APP(app);
    if (parent->core.being_destroyed) {
	UNLOCK_APP(app);
	return;
    }
    call_change_managed = 
	(XtClass(parent)->core_class.version == XtVersionDontCheck ||
	    XtClass(parent)->core_class.version >= (11 * 1000 + 6));
    /*
     * temporarily unmanage the unmanage_children list .....
     * only call the parent's change_managed() method if there is
     * no version 2 extension or its has_stateless_change_managed member
     * is set to False.
     */
    UnmanageChildren(unmanage_children, num_unmanage_children, parent, 
		     &num_unique_unm_children, call_change_managed,
		     XtNxtChangeManagedSet);
    hookobj = XtHooksOfDisplay(XtDisplayOfObject(unmanage_children[0]));
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;
	int ii;

	call_data.old = (Widget)NULL;
	call_data.args = (ArgList)NULL;
	call_data.num_args = 0;
	for (ii = 0; ii < num_unmanage_children; ii++) {
	    call_data.widget = unmanage_children[ii];
	    XtCallCallbacks(hookobj, XtNchangeHook, (XtPointer)&call_data);
	}
    }
    /*
     * now call the user supplied hook ... if there is one!
     * note that the app context is locked at this point.
     */
    if (post_unmanage_pre_manage_hook != (XtCSMHookProc)NULL) {
	(*post_unmanage_pre_manage_hook)(parent, unmanage_children,
			num_unmanage_children, manage_children,
			num_manage_children, client_data);
    }
    /*
     * now manage the children on the manage_children list ....
     * if there are no children to manage, but we have unmanaged some
     * and not previously called the parent's change_managed() method
     * force it to be called now ....
     */
    ManageChildren(manage_children, num_manage_children, parent,
		   !call_change_managed && num_unique_unm_children != 0,
		   XtNxtChangeManagedSet);
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;
	int ii;

	call_data.old = (Widget)NULL;
	call_data.args = (ArgList)NULL;
	call_data.num_args = 0;
	for (ii = 0; ii < num_manage_children; ii++) {
	    call_data.widget = manage_children[ii];
	    XtCallCallbacks(hookobj, XtNchangeHook, (XtPointer)&call_data);
	}
    }
    UNLOCK_APP(app);
} /* XtChangeManagedSet */
