
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Panner.h>

#include "editresP.h"

/*
 * Global variables. 
 */

extern TreeInfo *global_tree_info;
extern CurrentClient global_client;
extern ScreenData global_screen_data;
extern Widget global_tree_parent;

/* 
 * Function Definitions.
 */

extern void SetCommand(), PopupSetValues(), SetMessage();
extern void _TreeSelect(), _TreeRelabel(), _TreeActivate();

/*	Function Name: Quit
 *	Description: This function prints a message to stdout.
 *	Arguments: w - ** UNUSED **
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
Quit(w, client_data, call_data)
Widget w;
XtPointer call_data, client_data;
{
    XtDestroyApplicationContext(XtWidgetToApplicationContext(w));
    exit(0);
}

/*	Function Name: SendTree
 *	Description: This function initiates the client communication.
 *	Arguments: w - the widget that made the selection.
 *                 value - a boolean value stored as a pointer.
 *                         if True then get a new client, otherwise
 *                         refresh the current client.
 *                 call_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
SendTree(w, value, call_data)
Widget w;
XtPointer value, call_data;
{
    if ((Boolean) value)
	global_client.window = None;

    if (!XtIsWidget(w))     /* Make sure that we use a "Real" widget here. */
	w = XtParent(w);

    SetCommand(w, SendWidgetTree, NULL, NULL);
}

/*	Function Name: InitSetValues
 *	Description: This function pops up the setvalues dialog
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
InitSetValues(w, client_data, call_data)
Widget w;
XtPointer call_data, client_data;
{
    if (!XtIsWidget(w))     /* Make sure that we use a "Real" widget here. */
	w = XtParent(w);

    PopupSetValues(w, NULL);
}

/*	Function Name: TreeSelect
 *	Description: Selects all widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - The type of thing to select.
 *	Returns: none
 */

/* ARGSUSED */
void
TreeSelect(w, client_data, call_data)
Widget w;
XtPointer call_data, client_data;
{
    SelectTypes type = (SelectTypes) client_data;

    _TreeSelect(global_tree_info, type);
}

/*	Function Name: TreeRelabel
 *	Description: Relabels a tree to the type specified.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - the type of label to assign to each node.
 *	Returns: none
 */

/* ARGSUSED */
void
TreeRelabel(w, client_data, call_data)
Widget w;
XtPointer call_data, client_data;
{
    LabelTypes type = (LabelTypes) client_data;

    _TreeRelabel(global_tree_info, type);
}

/*	Function Name: PannerCallback
 *	Description: called when the panner has moved.
 *	Arguments: panner - the panner widget.
 *                 closure - *** NOT USED ***.
 *                 report_ptr - the panner record.
 *	Returns: none.
 */

/* ARGSUSED */
void 
PannerCallback(w, closure, report_ptr)
Widget w;
XtPointer closure, report_ptr;
{
    Arg args[2];
    XawPannerReport *report = (XawPannerReport *) report_ptr;

    if (global_tree_info == NULL) 
	return;

    XtSetArg (args[0], XtNx, -report->slider_x);
    XtSetArg (args[1], XtNy, -report->slider_y);

    XtSetValues(global_tree_info->tree_widget, args, TWO);
}

/*	Function Name: PortholeCallback
 *	Description: called when the porthole or its child has
 *                   changed 
 *	Arguments: porthole - the porthole widget.
 *                 panner_ptr - the panner widget.
 *                 report_ptr - the porthole record.
 *	Returns: none.
 */

/* ARGSUSED */
void 
PortholeCallback(w, panner_ptr, report_ptr)
Widget w;
XtPointer panner_ptr, report_ptr;
{
    Arg args[10];
    Cardinal n = 0;
    XawPannerReport *report = (XawPannerReport *) report_ptr;
    Widget panner = (Widget) panner_ptr;

    XtSetArg (args[n], XtNsliderX, report->slider_x); n++;
    XtSetArg (args[n], XtNsliderY, report->slider_y); n++;
    if (report->changed != (XawPRSliderX | XawPRSliderY)) {
	XtSetArg (args[n], XtNsliderWidth, report->slider_width); n++;
	XtSetArg (args[n], XtNsliderHeight, report->slider_height); n++;
	XtSetArg (args[n], XtNcanvasWidth, report->canvas_width); n++;
	XtSetArg (args[n], XtNcanvasHeight, report->canvas_height); n++;
    }
    XtSetValues (panner, args, n);
}
