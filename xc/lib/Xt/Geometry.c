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

#include "Intrinsic.h"

/* Private  Routines */

static void XtResizeWindow(w)
    Widget w;
{
    if (XtIsRealized(w)) {
	XWindowChanges changes;
	changes.width = w->core.width;
	changes.height = w->core.height;
	changes.border_width = w->core.border_width;
	XConfigureWindow(XtDisplay(w), XtWindow(w),
	    CWWidth | CWHeight | CWBorderWidth, &changes);
    }
} /* XtResizeWindow */

/* Public routines */

XtGeometryReturnCode XtMakeGeometryRequest (widget, request, reply)
    Widget         widget;
    WidgetGeometry *request, *reply;
{
    WidgetGeometry    junk;
    XtGeometryHandler manager;
    XtGeometryReturnCode returnCode;

    if (! XtIsSubclass(widget->core.parent, compositeWidgetClass)) {
	/* Should never happen - XtCreateWidget should have checked */
	XtError("XtMakeGeometryRequest - parent not composite");
    }
    manager = ((CompositeWidgetClass) (widget->core.parent->core.widget_class))
    		->composite_class.geometry_manager;
    if (manager == (XtGeometryHandler) NULL) {
	XtError("XtMakeGeometryRequest - parent has no geometry manger");
    }
    if ( ! widget->core.managed) {
	XtWarning("XtMakeGeometryRequest - widget not managed");
	return XtgeometryNo;
    }
    if (widget->core.being_destroyed) {
        return XtgeometryNo;
    }
    if (reply == (WidgetGeometry *) NULL) {
        returnCode = (*manager)(widget, request, &junk);
    } else {
	returnCode = (*manager)(widget, request, reply);
    }
    /* ||| Right now this is automatic.  However, we may want it to be
    explicitely called by geometry manager in order to effect the window resize
    (especially to smaller size) before the windows are layed out. */
    if (returnCode == XtgeometryYes) {
	XtResizeWindow(widget);
    }
    return returnCode;
} /* XtMakeGeometryRequest */

XtGeometryReturnCode XtMakeResizeRequest
	(widget, width, height, replyWidth, replyHeight)
    Widget	widget;
    Dimension	width, height;
    Dimension	*replyWidth, *replyHeight;
{
    WidgetGeometry request, reply;
    XtGeometryReturnCode r;

    request.request_mode = CWWidth | CWHeight;
    request.width = width;
    request.height = height;
    r = XtMakeGeometryRequest(widget, &request, &reply);
    if (replyWidth != NULL)
	*replyWidth  = (reply.request_mode & CWWidth  ? reply.width  : width );
    if (replyHeight != NULL)
	*replyHeight = (reply.request_mode & CWHeight ? reply.height : height);
    return r;
} /* XtMakeResizeRequest */

void XtResizeWidget(w)
    Widget w;
{
    if (w->core.widget_class->core_class.resize != (WidgetProc) NULL) {
	w->core.widget_class->core_class.resize(w);
    }
    XtResizeWindow(w);
} /* XtResizeWidget */

void XtMoveWidget(w, x, y)
    Widget w;
    Position x, y;
{
    if ((w->core.x != x) || (w->core.y != y)) {
	w->core.x = x;
	w->core.y = y;
	if (XtIsRealized(w)) {
	    XMoveWindow(XtDisplay(w), XtWindow(w), w->core.x, w->core.y);
        }
    }
} /* XtMoveWidget */

