/*
 * Code for creating all widgets used by EditRes.
 */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>	/* Get standard string definations. */

#include <X11/Xaw/Box.h>	
#include <X11/Xaw/Cardinals.h>	
#include <X11/Xaw/Label.h>	
#include <X11/Xaw/MenuButton.h>	
#include <X11/Xaw/Paned.h>	
#include <X11/Xaw/Panner.h>	
#include <X11/Xaw/Porthole.h>	
#include <X11/Xaw/SmeBSB.h>	
#include <X11/Xaw/SmeLine.h>	
#include <X11/Xaw/SimpleMenu.h>	
#include <X11/Xaw/Tree.h>
#include <X11/Xaw/Viewport.h>	

#include "editresP.h"

/*
 * functions.
 */

static void CreateCommandMenu(), CreateTreeCommandMenu();
static Widget CreateTopArea();

extern void PannerCallback(), PortholeCallback();
extern void Quit(), SendTree(), InitSetValues();
extern void TreeSelect(), TreeRelabel(), TreeActivate(), FindWidget();

/*	Function Name: BuildWidgetTree
 *	Description: Creates all widgets for Editres.
 *	Arguments: parent - the shell to put them into.
 *	Returns: none.
 */

void 
BuildWidgetTree(parent)
Widget parent;
{
    Widget paned, porthole, panner;

    paned = XtCreateManagedWidget("paned", panedWidgetClass, parent,
				  NULL, ZERO);

    panner = CreateTopArea(paned);

    porthole = XtCreateManagedWidget("porthole", portholeWidgetClass,
				     paned, NULL, ZERO);

/*
 * Allow the panner and porthole to talk to each other.
 */

    XtAddCallback(porthole, 
		  XtNreportCallback, PortholeCallback, (XtPointer) panner);
    XtAddCallback(panner, 
		  XtNreportCallback, PannerCallback, (XtPointer) porthole);

    global_tree_parent = porthole;
}

/*	Function Name: CreateTopArea
 *	Description: Creates the top part of the display
 *	Arguments: parent - widget to put this menu bar into.
 *	Returns: none. 
 */

static Widget
CreateTopArea(parent)
Widget parent;
{
    Widget box, panner, pane;

    box = XtCreateManagedWidget("box", boxWidgetClass, parent, NULL, ZERO);

    CreateCommandMenu(box);
    CreateTreeCommandMenu(box);

    pane = XtCreateManagedWidget("hPane", panedWidgetClass, parent, NULL,ZERO);

    {
	panner = XtCreateManagedWidget("panner", pannerWidgetClass, 
				       pane, NULL, ZERO);
	global_screen_data.info_label = XtCreateManagedWidget("userMessage", 
							     labelWidgetClass,
							     pane, NULL, ZERO);
    }
    return(panner);
}

/*	Function Name: CreateCommandMenu
 *	Description: Creats the command menu.
 *	Arguments: parent - widget to put this menu into.
 *	Returns: none.
 */

static void
CreateCommandMenu(parent)
Widget parent;
{
    Widget menu, entry, button;

    button = XtCreateManagedWidget("commands", menuButtonWidgetClass, parent,
				   NULL, ZERO);

    menu = XtCreatePopupShell("menu", simpleMenuWidgetClass, button,
			      NULL, ZERO);
    
    entry = XtCreateManagedWidget("sendTree", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, SendTree, (XtPointer) TRUE);

    entry = XtCreateManagedWidget("refreshTree", smeBSBObjectClass, menu,
				  NULL, ZERO);
    XtAddCallback(entry, XtNcallback, SendTree, (XtPointer) FALSE);

    entry = XtCreateManagedWidget("showClientWidget", smeBSBObjectClass, menu,
				  NULL, ZERO);
    XtAddCallback(entry, XtNcallback, FindWidget, NULL);

    entry = XtCreateManagedWidget("line", smeLineObjectClass, menu,
				  NULL, ZERO);
    
    entry = XtCreateManagedWidget("setValues", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, InitSetValues, NULL);

    entry = XtCreateManagedWidget("line", smeLineObjectClass, menu,
				  NULL, ZERO);

    entry = XtCreateManagedWidget("quit", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, Quit, NULL);

}

/*	Function Name: CreateTreeCommandMenu
 *	Description: Creats the command menu.
 *	Arguments: parent - widget to put this menu into.
 *	Returns: none.
 */

#define SELECT 0
#define ACTIVATE 1
#define LABEL 2
#define LINE 3

struct tree_ops_menu {
    char * name;
    int type;
    XtPointer data;
};

static void
CreateTreeCommandMenu(parent)
Widget parent;
{
    Widget menu, button, entry;
    int i, number;
    static struct tree_ops_menu tree_menu[] = {
        { "selectAll", SELECT, (XtPointer) SelectAll },
	{ "unselectAll", SELECT, (XtPointer) SelectNone },
	{ "invertAll", SELECT, (XtPointer) SelectInvert },
	{ "line", LINE, (XtPointer) NULL },
	{ "selectChildren", SELECT, (XtPointer) SelectChildren },
        { "selectParent", SELECT, (XtPointer) SelectParent },
	{ "selectDescendants", SELECT, (XtPointer) SelectDescendants },
        { "selectAncestors", SELECT, (XtPointer) SelectAncestors },
        { "line", LINE, (XtPointer) NULL },
        { "showWidgetNames", LABEL, (XtPointer) NameLabel },
        { "showClassNames", LABEL, (XtPointer) ClassLabel },
        { "showWidgetIDs", LABEL, (XtPointer) IDLabel},
        { "showWidgetWindows", LABEL, (XtPointer) WindowLabel}
    };

    button = XtCreateManagedWidget("treeCommands", menuButtonWidgetClass,
				   parent, NULL, ZERO);

    menu = XtCreatePopupShell("menu", simpleMenuWidgetClass, button,
			      NULL, ZERO);

    for ( i = 0, number = XtNumber(tree_menu) ; i < number ; i++) {
	void (*func)();
	WidgetClass class = smeBSBObjectClass;

	switch (tree_menu[i].type) {
	case SELECT:
	    func = TreeSelect;
	    break;
	case LABEL:
	    func = TreeRelabel;
	    break;
	case LINE:
	    func = NULL;
	    class = smeLineObjectClass;
	    break;
	default:
	    continue;
	}

	entry = XtCreateManagedWidget(tree_menu[i].name, class, menu,
				      NULL, ZERO);
	if (func != NULL) 
	    XtAddCallback(entry, XtNcallback, func, tree_menu[i].data);
    }
}

static Pixmap old_pixmap;

/*	Function Name: PrepareToLayoutTree
 *	Description: prepares the Tree widget to be layed out.
 *	Arguments: tree - the Tree widget.
 *	Returns: none
 */

void
PrepareToLayoutTree(tree)
Widget tree;
{
    Arg args[1];

    XtSetArg(args[0], XtNbackgroundPixmap, &old_pixmap);
    XtGetValues(XtParent(tree), args, ONE);

    XtSetArg(args[0], XtNbackgroundPixmap, None);
    XtSetValues(XtParent(tree), args, ONE);

    XtUnmapWidget(tree);
}

/*	Function Name: LayoutTree
 *	Description: Laysout the tree widget.
 *	Arguments: tree - the widget tree.
 *	Returns: none.
 */

void
LayoutTree(tree)
Widget tree;
{
    Arg args[1];
    
    XawTreeForceLayout(tree);
    XtMapWidget(tree); 

    XtSetArg(args[0], XtNbackgroundPixmap, old_pixmap);
    XtSetValues(XtParent(tree), args, ONE);
}
