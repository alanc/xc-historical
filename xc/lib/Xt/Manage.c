#ifndef lint
static char rcsid[] = "$Header: Manage.c,v 6.2 88/01/26 16:18:18 asente Exp $";
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

#include "IntrinsicI.h"

void XtUnmanageChildren(children, num_children)
    WidgetList children;
    Cardinal num_children;
{
    register CompositeWidget	parent;
    register Widget		child;
    register Cardinal		num_unique_children, i;
    XtWidgetProc		change_managed;

    if (num_children == 0) return;
    if (children[0] == NULL) {
	XtWarning("Null child passed to XtUnmanageChildren");
	return;
    }
    parent = (CompositeWidget) children[0]->core.parent;
    if (parent->core.being_destroyed) return;

    num_unique_children = 0;
    for (i = 0; i < num_children; i++) {
	child = children[i];
	if (child == NULL) {
	    XtWarning("Null child passed to XtUnmanageChildren");
	    return;
	}
        if ((CompositeWidget) child->core.parent != parent) {
	   XtWarning("Not all children have same parent in XtUnmanageChildren");
	} else if (child->core.managed) {
	    num_unique_children++;
	    child->core.managed = FALSE;
	    if (child->core.mapped_when_managed) {
		parent->composite.num_mapped_children--;
		if (XtIsRealized(child)) XtUnmapWidget(child);
	    }
	}
    }
    change_managed = ((CompositeWidgetClass)parent->core.widget_class)->
	composite_class.change_managed;
    if (num_unique_children != 0 && change_managed != NULL &&
	    XtIsRealized((Widget) parent)) {
	(*change_managed) (parent);
    }
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
    register CompositeWidget    parent;
    register Widget		child;
    register Cardinal		num_unique_children, i;
    XtWidgetProc		change_managed;
    register WidgetList		unique_children;
    Widget			cache[MAXCHILDREN];

    if (num_children == 0) return;
    if (children[0] == NULL) {
	XtWarning("Null child passed to XtManageChildren");
	return;
    }    parent = (CompositeWidget) children[0]->core.parent;
    if (parent->core.being_destroyed) return;

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
	    XtWarning("Null child passed to XtManageChildren");
	    return;
	}
        if ((CompositeWidget) child->core.parent != parent) {
	    XtWarning("Not all children have same parent in XtManageChildren");
	} else if (! child->core.managed) {
	    unique_children[num_unique_children++] = child;
	    child->core.managed = TRUE;
	    if (child->core.mapped_when_managed) {
		parent->composite.num_mapped_children++;
	    }
	}
    }

    if (num_unique_children != 0) {

	if (XtIsRealized((Widget)parent)) {

	    /* Compute geometry of new managed set of children. */
	    change_managed =
		    ((CompositeWidgetClass) parent->core.widget_class)->
		    composite_class.change_managed;
	    if (change_managed != NULL) (*change_managed) (parent);

	    /* Realize each child if necessary, then map if necessary */
	    for (i = 0; i < num_unique_children; i++) {
		child = unique_children[i];
		if (! XtIsRealized(child)) XtRealizeWidget(child);
		if (child->core.mapped_when_managed) XtMapWidget(child);
	    }
	}
    }

    if (unique_children != cache) XtFree((char *) unique_children);
} /* XtManageChildren */


void XtManageChild(child)
    Widget child;
{
    XtManageChildren(&child, (Cardinal) 1);
} /* XtManageChild */


void XtSetMappedWhenManaged(widget, mapped_when_managed)
    register Widget widget;
    Boolean	    mapped_when_managed;
{
    if (widget->core.mapped_when_managed == mapped_when_managed) return;
    widget->core.mapped_when_managed = mapped_when_managed;
    if (! XtIsManaged(widget)) return;

    if (mapped_when_managed) {
	/* Didn't used to be mapped when managed.		*/
	((CompositeWidget) (widget->core.parent))->
	    composite.num_mapped_children++;
	if (XtIsRealized(widget)) XtMapWidget(widget);
    } else {
	/* Used to be mapped when managed.			*/
	((CompositeWidget) (widget->core.parent))->
	    composite.num_mapped_children--;
	if (XtIsRealized(widget)) XtUnmapWidget(widget);
    }
} /* XtSetMappedWhenManaged */


