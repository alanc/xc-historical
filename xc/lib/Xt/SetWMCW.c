/* $XConsortium: SetWMCW.c,v 1.7 94/01/14 17:56:24 kaleb Exp $ */
/*
 * Copyright 1989 Massachusetts Institute of Technology
 * Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
 * NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
 * ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
 * PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 * THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Chris D. Peterson, MIT X Consortium
 */

#include "IntrinsicI.h"
#include <X11/Xatom.h>

/*	Function Name: XtSetWMColormapWindows
 *
 *	Description: Sets the value of the WM_COLORMAP_WINDOWS
 *                   property on a widget's window.
 *
 *	Arguments:  widget - specifies the widget on whose window the
 *   		           - WM_COLORMAP_WINDOWS property will be stored.
 *
 *                  list - Specifies a list of widgets whose windows are to be
 *		           listed in the WM_COLORMAP_WINDOWS property.
 *                  count - Specifies the number of widgets in list.
 *
 *	Returns: none.
 */

void
XtSetWMColormapWindows(widget, list, count)
Widget widget, *list;
Cardinal count;
{
    Window *data;
    Widget *checked, *top, *temp, hookobj;
    Cardinal i, j, checked_count;
    Boolean match;
    Atom xa_wm_colormap_windows;
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    if ( !XtIsRealized(widget) || (count == 0) ) {
	UNLOCK_APP(app);
	return;
    }

    top = checked = (Widget *) XtMalloc( (Cardinal) sizeof(Widget) * count);


/*
 * The specification calls for only adding the windows that have unique 
 * colormaps to the property to this function, so we will make a pass through
 * the widget list removing all the widgets with non-unique colormaps.
 *
 * We will also remove any unrealized widgets from the list at this time.
 */

    for (checked_count = 0, i = 0; i < count; i++) {
	if (!XtIsRealized(list[i])) continue;
	    
	*checked = list[i];
	match = FALSE;

/*
 * Don't check first element for matching colormap since there is nothing
 * to check it against.
 */

	if (checked != top)	
	    for (j = 0, temp = top; j < checked_count ; j++, temp++)
		if ( (*temp)->core.colormap == (*checked)->core.colormap) {
		    match = TRUE;
		    break;
		}

/*
 * If no colormap was found to match then add this widget to the linked list.
 */

	if (!match) {
	    checked++;
	    checked_count++;
	}
    }

/*
 * Now that we have the list of widgets we need to convert it to a list of
 * windows and set the property.
 */

    data = (Window *) XtMalloc( (Cardinal) sizeof(Window) * checked_count);

    for ( i = 0 ; i < checked_count ; i++)
	data[i] = XtWindow(top[i]);

    xa_wm_colormap_windows = XInternAtom(XtDisplay(widget),
					 "WM_COLORMAP_WINDOWS", FALSE);

    XChangeProperty(XtDisplay(widget), XtWindow(widget), 
		    xa_wm_colormap_windows, XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *) data, (int) i);

    hookobj = XtHooksOfDisplay(XtDisplay(widget));
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;

	call_data.type = XtHsetWMColormapWindows;
	call_data.widget = widget;
	call_data.event_data = (XtPointer) list;
	call_data.num_event_data = count;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.changehook_callbacks, 
		(XtPointer)&call_data);
    }

    XtFree( (char *) data);
    XtFree( (char *) top);
    UNLOCK_APP(app);
}
