#ifndef lint
static char rcsid[] =
    "$XConsortium: Popup.c,v 1.16 88/09/04 12:21:11 swick Exp $";
/* $oHeader: Popup.c,v 1.3 88/09/01 11:45:34 asente Exp $ */
#endif lint

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

#include "IntrinsicI.h"
#include "Shell.h"
#include "ShellP.h"
#include "StringDefs.h"

void _XtPopup(widget, grab_kind, spring_loaded)
    Widget      widget;
    XtGrabKind  grab_kind;
    Boolean     spring_loaded;
{
    register ShellWidget shell_widget = (ShellWidget) widget;

    if (! XtIsShell(widget)) {
	XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		"invalidClass","xtPopup","XtToolkitError",
                "XtPopup requires a subclass of shellWidgetClass",
                  (String *)NULL, (Cardinal *)NULL);
    }

    if (! shell_widget->shell.popped_up) {
	XtCallCallbacks(widget, XtNpopupCallback, (Opaque)NULL);
	shell_widget->shell.popped_up = TRUE;
	shell_widget->shell.grab_kind = grab_kind;
	shell_widget->shell.spring_loaded = spring_loaded;
	if (shell_widget->shell.create_popup_child_proc != NULL) {
	    (*(shell_widget->shell.create_popup_child_proc))(widget);
	}
	if (grab_kind == XtGrabExclusive) {
	    XtAddGrab(widget, TRUE, spring_loaded);
	} else if (grab_kind == XtGrabNonexclusive) {
	    XtAddGrab(widget, FALSE, spring_loaded);
	}
	XtRealizeWidget(widget);
	XMapRaised(XtDisplay(widget), XtWindow(widget));
    }
} /* _XtPopup */

void XtPopup (widget, grab_kind)
    Widget  widget;
    XtGrabKind grab_kind;
{
    _XtPopup(widget, grab_kind, FALSE);
} /* XtPopup */

/* ARGSUSED */
void XtCallbackNone(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{
    XtSetSensitive(widget, FALSE);
    _XtPopup((Widget) closure, XtGrabNone, FALSE);
} /* XtCallbackNone */

/* ARGSUSED */
void XtCallbackNonexclusive(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{

    XtSetSensitive(widget, FALSE);
    _XtPopup((Widget) closure, XtGrabNonexclusive, FALSE);
} /* XtCallbackNonexclusive */

/* ARGSUSED */
void XtCallbackExclusive(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{
    XtSetSensitive(widget, FALSE);
    _XtPopup((Widget) closure, XtGrabExclusive, FALSE);
} /* XtCallbackExclusive */


void XtPopdown(widget)
    Widget  widget;
{
    /* Unmap a shell widget if it is mapped, and remove from grab list */

    register ShellWidget shell_widget = (ShellWidget) widget;

    if (! XtIsShell(widget)) {
	XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		"invalidClass","xtPopdown","XtToolkitError",
            "XtPopdown requires a subclass of shellWidgetClass",
              (String *)NULL, (Cardinal *)NULL);
    }

    if (shell_widget->shell.popped_up) {
	XtUnmapWidget(widget);
	if (shell_widget->shell.grab_kind != XtGrabNone) {
	    XtRemoveGrab(widget);
	}
	shell_widget->shell.popped_up = FALSE;
	XtCallCallbacks(widget, XtNpopdownCallback, (Opaque)NULL);
    }
} /* XtPopdown */

/* ARGSUSED */
void XtCallbackPopdown(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{
    register XtPopdownID id = (XtPopdownID) closure;

    XtPopdown(id->shell_widget);
    if (id->enable_widget != NULL) {
	XtSetSensitive(id->enable_widget, TRUE);
    }
} /* XtCallbackPopdown */



