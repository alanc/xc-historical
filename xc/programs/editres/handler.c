

#include <stdio.h>
#include <X11/Intrinsic.h>

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
Quit(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    XtDestroyApplicationContext(XtWidgetToApplicationContext(w));
    exit(0);
}


/*	Function Name: SendTree
 *	Description: This function initiates the client communication.
 *	Arguments: w - the widget that made the selection.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
SendTree(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
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
InitSetValues(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    if (!XtIsWidget(w))     /* Make sure that we use a "Real" widget here. */
	w = XtParent(w);

    PopupSetValues(w, NULL);
}

/*	Function Name: TreeSelectAll
 *	Description: Selects all widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
TreeSelectAll(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    _TreeSelect(SelectAll);
}

/*	Function Name: TreeUnselectAll
 *	Description: Unselects all widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
TreeUnselectAll(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    _TreeSelect(SelectNone);
}

/*	Function Name: TreeInvertAll
 *	Description: Inverts all widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
TreeInvertAll(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    _TreeSelect(SelectInvert);
}

/*	Function Name: TreeSelectChildren
 *	Description: Selects the children of all currently slected widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
TreeSelectChildren(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    _TreeActivate(ActivateChildren);
}

/*	Function Name: TreeSelectParents
 *	Description: Selects the parents of all currently selected widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
TreeSelectParents(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    _TreeActivate(ActivateParents);
}

/*	Function Name: TreeSelectDescendants
 *	Description: selects the descendants of all currently selected widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
TreeSelectDescendants(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    _TreeActivate(ActivateDescendants);
}

/*	Function Name: TreeSelectAncestors
 *	Description: selects all ancestors of currently select widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
TreeSelectAncestors(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    _TreeActivate(ActivateAncestors);
}

/*	Function Name: TreeShowNames
 *	Description: Shows the instance names for all widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
TreeShowNames(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    _TreeRelabel(NameLabel);
}

/*	Function Name: TreeShowClasses
 *	Description: Shows the class names for all widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
TreeShowClasses(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    _TreeRelabel(ClassLabel);
}


/*	Function Name: TreeShowIDs
 *	Description: Shows the widget ids for all widgets.
 *	Arguments: w - the widget caused this action.
 *                 call_data - ** UNUSED **
 *                 client_data - ** UNUSED **
 *	Returns: none
 */

/* ARGSUSED */
void
TreeShowIDs(w, call_data, client_data)
Widget w;
XtPointer call_data, client_data;
{
    _TreeRelabel(IDLabel);
}
