/*
 * $XConsortium: geometry.c,v 1.4 90/03/16 13:57:09 kit Exp $
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
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xaw/Cardinals.h>	

#include "editresP.h"

extern WNode * IDToNode();
extern void SetMessage(), SetCommand(), SetAndCenterTreeNode(), AddString();
extern void GetAllStrings();
extern char * NodeToID();
extern int HandleXErrors();

static void CreateFlashWidget(), FlashWidgets();

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
    void _FlashActiveWidgets();

    if ( (node = IDToNode(global_tree_info->top_node, str)) == NULL) {
	/* Oops. */
    }

    SetAndCenterTreeNode(node);

    node = node->tree_info->top_node;

    sprintf(msg, "Widget Tree for client %s(%s).", node->name, node->class);
    SetMessage(global_screen_data.info_label, msg);

    _FlashActiveWidgets(global_tree_info);
}

/*	Function Name: _FlashActiveWidgets
 *	Description: Highlights all active widgets in the tree.
 *	Arguments: tree_info - information about the current tree.
 *	Returns: none.
 */

void
_FlashActiveWidgets(tree_info)
TreeInfo * tree_info;
{
    int i;
    char * all_nodes = NULL;

    if (tree_info == NULL) {
	SetMessage(global_screen_data.info_label,
		   "No widget Tree is avaliable.");
	return;
    }

    if (tree_info->num_nodes == 0) {
	SetMessage(global_screen_data.info_label,"There are no active nodes.");
	return;
    }
	
    for (i = 0; i < tree_info->num_nodes; i++) {
	char buf[BUFSIZ];
	char * id = NodeToID(tree_info->active_nodes[i]);

	sprintf(buf, "%s\n", id);
	XtFree(id);

	AddString(&all_nodes, buf);
    }

    SetCommand(tree_info->tree_widget, FlashWidget, all_nodes, NULL);
    XtFree(all_nodes);
}

/*	Function Name: HandleFlashWidget
 *	Description: Is called when client has returned geometry of all widget
 *                   to flash.
 *	Arguments: value - strings containing geometry of widgets to flash.
 *	Returns: none.
 */

char *
HandleFlashWidget(value)
String value;
{
    static void AddToFlashList();
    char ** strings, * errors = NULL;
    int i, num_strings;

    GetAllStrings(value, EOL_SEPARATOR, &strings, &num_strings);

    for (i = 0; i < num_strings; i++) {
	if (strings[i][0] == '0')
	    AddToFlashList(global_tree_info, strings[i] + 1, &errors);
	else {
	    WNode * node;
	    char ** names, buf[BUFSIZ];
	    int num_names;

	    /* 
	     * An Error occured with this geometry request.
	     */

	    GetAllStrings(strings[i] + 1, NAME_VAL_SEPARATOR, 
			  &names, &num_names);
	    
	    node = IDToNode(global_tree_info->top_node, names[0]);
	    sprintf(buf, "%s(0x%lx) - %s\n", node->name, node->id, names[1]);
	    AddString(&errors, buf);
	    XtFree(names);
	}
    }

    FlashWidgets(global_tree_info);

    XtFree(strings);
    return(errors);
}

/*	Function Name: AddWidgetToFlashList
 *	Description: Adds a widget to the list of widget to flash.
 *	Arguments: geom_str - a string containing the geometry and widget id.
 *	Returns: 
 */

static void
AddToFlashList(tree_info, geom_str, errors)
TreeInfo * tree_info;
String geom_str;
char ** errors;
{
    static void _AddToFlashList();
    char * ptr, ident[200], geom[100], *ip, *gsp;
    unsigned int width, height;
    int bw, x, y, mask;
    WNode * node;

    if ( (ptr = index(geom_str, NAME_VAL_SEPARATOR)) == NULL) {
	char buf[BUFSIZ];

	sprintf(buf, "Incorrectly formatted entry, no `%c'\n",
		NAME_VAL_SEPARATOR);
	AddString(errors, buf); 
	return;
    }

    for (ip = ident, gsp = geom_str; gsp < ptr; ip++, gsp++)
	*ip = *gsp;
    *ip = '\0';
    node = IDToNode(tree_info->top_node, ident);

    if (streq(++ptr, "NOT_VISABLE")) {
	char buf[BUFSIZ];

	sprintf(buf, "%s(0x%lx) - This widget is not mapped\n",
		node->name, node->id);
	AddString(errors, buf); 
	return;
    }

    strcpy(geom, ptr);

    if ( (ptr = index(geom, EDITRES_BORDER_WIDTH_SEPARATOR)) == NULL) {
	char buf[BUFSIZ];

	sprintf(buf, "%s(0x%lx) - Incorrectly formatted entry, no `%c'\n",
		node->name, node->id, EDITRES_BORDER_WIDTH_SEPARATOR);
	AddString(errors, buf); 
	return;
    }

    *ptr++ = '\0';    
    bw = 2 * atoi(ptr);

    mask = XParseGeometry(geom, &x, &y, &width, &height);
    if ( !((mask & XValue) && (mask & YValue) && 
	   (mask & WidthValue) && (mask & HeightValue)) ) {
	char buf[BUFSIZ];

	sprintf(buf, "%s(0x%lx) - Could not parse geometry string `%s'.\n", 
		node->name, node->id, geom);
	AddString(errors, buf); 
	return;
    }    

    width += bw;		/* Border width has already been doubled. */
    height += bw;

    _AddToFlashList(tree_info, errors, node, x, y, width, height);
}

/*	Function Name: _AddToFlashList
 *	Description: adds the window to the current client's flash list.
 *	Arguments: errors - a string to stuff any errors encountered.
 *                 node - the node associated with this object.
 *                 x, y - location of the flash widget in root coords.
 *                 width, height - size of the flash widget.
 *	Returns: none.
 */

static void
_AddToFlashList(tree_info, errors, node, x, y, width, height)
TreeInfo * tree_info;
char ** errors;
WNode * node;
int x, y;
unsigned int width, height;
{
    Display * dpy = XtDisplay(tree_info->tree_widget);
    Window window = (Window) node->window;
    XWindowAttributes attrs;

    if (window == EDITRES_IS_OBJECT)
	window = node->parent->window;

    if (window == EDITRES_IS_UNREALIZED) {
	char buf[BUFSIZ];

	if (node->window == EDITRES_IS_OBJECT) 
	    sprintf(buf, "%s(0x%lx) - This object's parent is unrealized\n", 
		    node->name, node->id);	    
	else
	    sprintf(buf, "%s(0x%lx) - This widget is unrealized\n", 
		    node->name, node->id);

	AddString(errors, buf); 
	return;
    }

    global_error_code = NO_ERROR;                 /* Reset Error code. */
    global_old_error_handler = XSetErrorHandler(HandleXErrors);
    global_serial_num = NextRequest(dpy);

    XGetWindowAttributes(dpy, window, &attrs);

    XSync(dpy, FALSE);
    XSetErrorHandler(global_old_error_handler);
    if (global_error_code == NO_WINDOW) {
	char buf[BUFSIZ];

	sprintf(buf, "%s(0x%lx) - This widget's window no longer exists.\n", 
		node->name, node->id);
	AddString(errors, buf); 
	return;
    }   

    if (attrs.map_state != IsViewable) {
	char buf[BUFSIZ];

	sprintf(buf, "%s(0x%lx) - This widget is not mapped.\n",
		node->name, node->id);
	AddString(errors, buf); 
	return;
    }   

    CreateFlashWidget(tree_info, x, y, width, height);
}

/*	Function Name: CreateFlashWidget
 *	Description: Creates a widget of the size specified that
 *                   will flash on the display, and adds it to the list
 *                   of widgets to flash.
 *	Arguments: tree_info - the tree information structure.
 *                 x,y,width, height - size and location of the flash widget.
 *	Returns: none.
 */
    
#define MORE_FLASH_WIDGETS 5

static void
CreateFlashWidget(tree_info, x, y, width, height)
TreeInfo * tree_info;
int x, y;
unsigned int width, height;
{
    Widget shell;
    Arg args[3];
    Cardinal num = 0;
    Dimension bw;

    XtSetArg(args[num], XtNx, x); num++;
    XtSetArg(args[num], XtNy, y); num++;
    XtSetArg(args[num], XtNbackground, global_resources.flash_color); num++;

    shell = XtCreatePopupShell("flash", overrideShellWidgetClass, 
			       tree_info->tree_widget, args, num);

    num = 0;
    XtSetArg(args[num], XtNborderWidth, &bw); num++;
    XtGetValues(shell, args, num);
    
    bw *= 2;

    num = 0;
    XtSetArg(args[num], XtNwidth, (width - bw)); num++;
    XtSetArg(args[num], XtNheight, (height - bw)); num++;
    XtSetValues(shell, args, num);    
    
    if (tree_info->num_flash_widgets + 1 > tree_info->alloc_flash_widgets) {
	tree_info->alloc_flash_widgets += MORE_FLASH_WIDGETS;
	tree_info->flash_widgets =
	    (Widget *) XtRealloc(tree_info->flash_widgets,
			      sizeof(Widget) * tree_info->alloc_flash_widgets);
    }

    tree_info->flash_widgets[tree_info->num_flash_widgets] = shell;
    tree_info->num_flash_widgets++;
}

/*	Function Name: FlashWidgets
 *	Description: Starts the widgets flashing.
 *	Arguments: tree_info - the info about the tree (contains flash list)
 *	Returns: none
 */

static void
FlashWidgets(tree_info)
TreeInfo * tree_info;
{
    int i;
    unsigned long wait, half_flash;
    static void FlashWidgetsOn(), FlashWidgetsOff(), FlashWidgetsCleanup();
    XtAppContext ac = XtWidgetToApplicationContext(tree_info->tree_widget);

    if (tree_info->flash_widgets == NULL) /* no widgets to flash. */
	return;

    wait = half_flash = global_resources.flash_time/2;
    for (i = 1; i < global_resources.num_flashes; i++) {
	XtAppAddTimeOut(ac, wait, FlashWidgetsOff,(XtPointer)tree_info);
	wait += half_flash;
	XtAppAddTimeOut(ac, wait, FlashWidgetsOn,(XtPointer)tree_info);
	wait += half_flash;
    }

    wait += half_flash;
    XtAppAddTimeOut(ac, wait, FlashWidgetsCleanup, (XtPointer)tree_info);

    FlashWidgetsOn((XtPointer) tree_info, (XtIntervalId *) NULL);
}
    
/*	Function Name: FlashWidgetsOn
 *	Description: Turns on all the Flash Widgets.
 *	Arguments: info_ptr - pointer to the tree info.
 *                 id - *** UNUSED ***.
 *	Returns: none
 */

/* ARGSUSED */
static void
FlashWidgetsOn(info_ptr, id)
XtPointer info_ptr;
XtIntervalId * id;
{

    int i;
    TreeInfo * tree_info = (TreeInfo *) info_ptr;
    
    for (i = 0; i < tree_info->num_flash_widgets; i++) {
	XtRealizeWidget(tree_info->flash_widgets[i]);
	XMapRaised(XtDisplay(tree_info->flash_widgets[i]),
		   XtWindow(tree_info->flash_widgets[i]));
    }
}

/*	Function Name: FlashWidgetsOff
 *	Description: Turns off all the Flash Widgets.
 *	Arguments: info_ptr - pointer to the tree info.
 *                 id - *** UNUSED ***.
 *	Returns: none
 */

/* ARGSUSED */
static void
FlashWidgetsOff(info_ptr, id)
XtPointer info_ptr;
XtIntervalId * id;
{
    int i;
    TreeInfo * tree_info = (TreeInfo *) info_ptr;
    
    for (i = 0; i < tree_info->num_flash_widgets; i++)
	XtUnmapWidget(tree_info->flash_widgets[i]);
}

/*	Function Name: FlashWidgetsCleanup
 *	Description: Destroys all the Flash Widgets.
 *	Arguments: info_ptr - pointer to the tree info.
 *                 id - *** UNUSED ***.
 *	Returns: none
 */

/* ARGSUSED */
static void
FlashWidgetsCleanup(info_ptr, id)
XtPointer info_ptr;
XtIntervalId * id;
{
    int i;
    TreeInfo * tree_info = (TreeInfo *) info_ptr;

/*
 * Unmap 'em first for consistency.
 */
    
    for (i = 0; i < tree_info->num_flash_widgets; i++)
	XtUnmapWidget(tree_info->flash_widgets[i]);

    XFlush(XtDisplay(tree_info->tree_widget));

    for (i = 0; i < tree_info->num_flash_widgets; i++) 
	XtDestroyWidget(tree_info->flash_widgets[i]);

    XtFree(tree_info->flash_widgets);
    tree_info->flash_widgets = NULL;
    tree_info->num_flash_widgets = tree_info->alloc_flash_widgets = 0;
}
