/*
 * $XConsortium: geometry.c,v 1.1 90/03/14 17:09:46 kit Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
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
 * Author:  Chris D. Peterson, MIT X Consortium
 */

#include <stdio.h>
#include <X11/Intrinsic.h>

#include <X11/Xaw/Cardinals.h>	

#include "editresP.h"

extern WNode * IDToNode();
extern void SetMessage(), SetCommand(), SetAndCenterTreeNode();
extern char * NodeToID();

/*	Function Name: _FindWidget
 *	Description: Finds a widget in the tree and shows it to the user.
 *	Arguments: w - any widget in the application.
 *	Returns: none.
 */

void 
_FindWidget(w)
Widget w;
{
    char msg[BUFSIZ];
    WNode * node, *FindWidgetFromWindow();
    Window win, GetClientWindow();
    int x, y;			/* location of event in root coordinates. */

    sprintf(msg, "Click on any widget is the client.\nEditres will %s",
	    "select that widget in the tree display.");

    SetMessage(global_screen_data.info_label, msg);

    if ( (win = GetClientWindow(w, &x, &y)) != None) {
	node = FindWidgetFromWindow(global_tree_info, win);
	if (node != NULL) {
	    char buf[BUFSIZ], * id = NodeToID(node);
	    
	    sprintf(buf, "%s%c%c%d%c%d", id, NAME_VAL_SEPARATOR, 
		    ((x < 0) ? '-' : '+'), x, ((y < 0) ? '-' : '+'), y);
	    SetCommand(w, FindChild, buf, NULL);
	    XtFree(id);
	    return;
	}
    }

    SetMessage(global_screen_data.info_label, 
      "That window does not appear to be\nin the currently displayed client.");
}

/*	Function Name: FindWidgetFromWindow
 *	Description: finds a widget in the current tree given its window id.
 *	Arguments: tree_info - information about this tree.
 *                 win - window to search for.
 *	Returns: node - the node corrosponding to this widget.
 */

static WNode * 
FindWidgetFromWindow(tree_info, win)
TreeInfo * tree_info;
Window win;
{
    static WNode *FindWidgetFromWindowGivenNode();

    if (tree_info == NULL)
	return(NULL);

    return(FindWidgetFromWindowGivenNode(tree_info->top_node, win));
}

/*	Function Name: FindWidgetFromWindowGivenNode
 *	Description: finds a widget in the current tree given its window id.
 *	Arguments: node - current node.
 *                 win - window to search for.
 *	Returns: node - the node corrosponding to this widget.
 */

static WNode *
FindWidgetFromWindowGivenNode(node, win)
WNode * node;
Window win;
{
    int i;
    WNode * ret_node;

    if (node->window == win)
	return(node);

    for (i = 0; i < node->num_children; i++) {
	ret_node = FindWidgetFromWindowGivenNode(node->children[i], win);
	if (ret_node != NULL)
	    return(ret_node);
    }
    return(NULL);
}

/*	Function Name: DisplayChild
 *	Description: Displays the child node returned by the client
 *	Arguments: str - id of the node that contains this info.
 *	Returns: none.
 */

void
DisplayChild(str)
char * str;
{
    WNode * node;
    char msg[BUFSIZ];

    if ( (node = IDToNode(global_tree_info->top_node, str)) == NULL) {
	/* Oops. */
    }

    SetAndCenterTreeNode(node);

    node = node->tree_info->top_node;

    sprintf(msg, "Widget Tree for client %s(%s).", node->name, node->class);
    SetMessage(global_screen_data.info_label, msg);
}
