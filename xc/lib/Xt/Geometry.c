#ifndef lint
static char *sccsid = "@(#)Geometry.c	1.7	2/25/87";
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


/* File: Geometry.c */

#include "Xlib.h"
#include "Intrinsic.h"

/* Private  Definitions */

/* Public routines */

XtGeometryReturnCode XtMakeGeometryRequest (widget, request, reply)
    Widget         widget;
    WidgetGeometry *request, *reply;
{
    XtGeometryHandler proc;
    int error;

    if (! XtIsSubClassOf(widget->core.parent, compositeWidgetClass)) {
        XtError("XtMakeGeometryRequest - parent not composite");
        return XtgeometryNo;
    }
    if (widget->core.being_destroyed || !widget->core.managed)
        return XtgeometryNo;
    return ((CompositeWidgetClass) (widget->core.parent->core.widget_class))
    		->compositeClass.geometry_manager (widget, request, reply);
}

extern void XtResizeWidget(w)
    Widget w;
{
    w->core.widget_class->coreClass.resize(w);

    if (XtWidgetIsRealized(w)) {
	XWindowChanges changes;
	changes.width = w->core.width;
	changes.height = w->core.height;
	changes.border_width = w->core.border_width;
	XConfigureWindow(w->core.display, w->core.window,
	    CWWidth | CWHeight | CWBorderWidth, &changes);
    }
} /* XtResizeWidget */

extern void XtMoveWidget(w, x, y)
    Widget w;
    Position x, y;
{
    if ((w->core.x != x) || (w->core.y != y)) {
	w->core.x = x;
	w->core.y = y;
	if (XtWidgetIsRealized(w)) {
	    XMoveWindow(w->core.display, w->core.window, w->core.x, w->core.y);
        }
    }
} /* XtWidgetMove */

