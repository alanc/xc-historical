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
#include <X11/Xaw/SmeBSB.h>	
#include <X11/Xaw/SmeLine.h>	
#include <X11/Xaw/SimpleMenu.h>	
#include <X11/Xaw/Viewport.h>	

#include "editresP.h"

/*
 * Global variables. 
 */

extern TreeInfo *global_tree_info;
extern CurrentClient global_client;
extern ScreenData global_screen_data;
extern Widget global_tree_parent;

/*
 * functions.
 */

static void CreateMenuBar(), CreateCommandMenu(), CreateTreeCommandMenu();

extern void Quit(), SendTree(), InitSetValues();

extern void TreeSelectAll(), TreeUnselectAll(), TreeInvertAll(), TreeShowIDs();
extern void TreeSelectChildren(), TreeSelectParents(), TreeSelectDescendants();
extern void TreeSelectAncestors(), TreeShowNames(), TreeShowClasses();

/*	Function Name: BuildWidgetTree
 *	Description: Creates all widgets for Editres.
 *	Arguments: parent - the shell to put them into.
 *	Returns: none.
 */

void 
BuildWidgetTree(parent)
Widget parent;
{
    Widget paned;

    paned = XtCreateManagedWidget("paned", panedWidgetClass, parent,
				  NULL, ZERO);

    CreateMenuBar(paned);

    global_screen_data.info_label = XtCreateManagedWidget("userMessage", 
							  labelWidgetClass,
							  paned, NULL, ZERO);
    global_tree_parent = paned;
}


/*	Function Name: CreateMenuBar
 *	Description: Creates the MenuBar for Editres.
 *	Arguments: parent - widget to put this menu bar into.
 *	Returns: none. 
 */

static void
CreateMenuBar(parent)
Widget parent;
{
    Widget box;

    box = XtCreateManagedWidget("box", boxWidgetClass, parent, NULL, ZERO);

    CreateCommandMenu(box);
    CreateTreeCommandMenu(box);
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

    entry = XtCreateManagedWidget("quit", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, Quit, NULL);
    
    entry = XtCreateManagedWidget("sendTree", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, SendTree, NULL);
    
    entry = XtCreateManagedWidget("setValues", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, InitSetValues, NULL);
}

/*	Function Name: CreateTreeCommandMenu
 *	Description: Creats the command menu.
 *	Arguments: parent - widget to put this menu into.
 *	Returns: none.
 */

static void
CreateTreeCommandMenu(parent)
Widget parent;
{
    Widget menu, entry, button;

    button = XtCreateManagedWidget("treeCommands", menuButtonWidgetClass,
				   parent, NULL, ZERO);

    menu = XtCreatePopupShell("menu", simpleMenuWidgetClass, button,
			      NULL, ZERO);

    entry = XtCreateManagedWidget("selectAll", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, TreeSelectAll, NULL);

    entry = XtCreateManagedWidget("unselectAll", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, TreeUnselectAll, NULL);

    entry = XtCreateManagedWidget("invertAll", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, TreeInvertAll, NULL);
    
    (void) XtCreateManagedWidget("line", smeLineObjectClass, menu,
				 NULL, ZERO);

    entry = XtCreateManagedWidget("selectChildren", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, TreeSelectChildren, NULL);

    entry = XtCreateManagedWidget("selectParents", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, TreeSelectParents, NULL);

    entry = XtCreateManagedWidget("selectDescendants", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, TreeSelectDescendants, NULL);

    entry = XtCreateManagedWidget("selectAncestors", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, TreeSelectAncestors, NULL);

    (void) XtCreateManagedWidget("line", smeLineObjectClass, menu,
				 NULL, ZERO);

    entry = XtCreateManagedWidget("showWidgetNames", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, TreeShowNames, NULL);

    entry = XtCreateManagedWidget("showClassNames", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, TreeShowClasses, NULL);

    entry = XtCreateManagedWidget("showWidgetIDs", smeBSBObjectClass, menu,
				    NULL, ZERO);
    XtAddCallback(entry, XtNcallback, TreeShowIDs, NULL);
}
