/*
 * $XConsortium: viewres.c,v 1.13 90/02/05 14:54:00 jim Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
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
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <stdio.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include "Tree.h"
#include <X11/Xmu/Converters.h>
#include <X11/Xmu/CharSet.h>
#include "defs.h"

char *ProgramName;

extern WidgetNode widget_list[];
extern int nwidgets;

static char *help_message[] = {
    "-lbw number      label border width in pixels",
    "-lw number       line width in pixels",
    "-top name        object to be top of tree",
    "-variable        show variable name instead of class name",
    "-vertical        list the tree vertically",
    NULL
};

static XrmOptionDescRec Options[] = {
    { "-lbw", "*Tree*Command*BorderWidth", XrmoptionSepArg, (caddr_t) NULL },
    { "-lw", "*Tree.LineWidth", XrmoptionSepArg, (caddr_t) NULL },
    { "-top", "*topObject", XrmoptionSepArg, (caddr_t) NULL },
    { "-variable", "*showVariable", XrmoptionNoArg, (caddr_t) "on" },
    { "-vertical", "*Tree.Orientation", XrmoptionNoArg, (caddr_t) "vertical" },
};

static struct _appresources {
    char *top_object;
    Boolean show_variable;
} Appresources;

static XtResource Resources[] = {
#define offset(field) XtOffset(struct _appresources *, field)
    { "topObject", "TopObject", XtRString, sizeof(char *),
	offset(top_object), XtRString, (caddr_t) "object" },
    { "showVariable", "ShowVariable", XtRBoolean, sizeof(Boolean),
	offset(show_variable), XtRImmediate, (caddr_t) FALSE },
#undef offset
};


static char *fallback_resources[] = {
    "*Viewport.allowHoriz: true",
    "*Viewport.allowVert: true",
    "*allowShellResize: true",
    NULL
};

static void HandleQuit(), HandleSetLableType(), HandleSetOrientation();
static void set_labeltype_menu(), set_orientation_menu();
static void build_tree(), set_node_labels();

static XtActionsRec viewres_actions[] = {
    { "Quit", HandleQuit },
    { "SetLabelType", HandleSetLableType },
    { "SetOrientation", HandleSetOrientation },
};

static Widget treeWidget;
static Widget quitButton, formatButton, formatMenu;
static Widget variableEntry, classEntry, horizontalEntry, verticalEntry;
static WidgetNode *topnode;


/*
 * routines
 */
static void usage ()
{
    char **cpp;
    fprintf (stderr, "usage:  %s [-options...]\n", ProgramName);
    fprintf(stderr, "\nwhere options include:\n");
    for (cpp = help_message; *cpp; cpp++) {
	fprintf (stderr, "%s\n", *cpp);
    }
    fprintf(stderr, "\n");
    exit (1);
}


static void variable_labeltype_callback (gw, closure, data)
    Widget gw;
    caddr_t closure;			/* TRUE or FALSE */
    caddr_t data;
{
    set_labeltype_menu ((Boolean) closure, True);
}

static void horizontal_orientation_callback (gw, closure, data)
    Widget gw;
    caddr_t closure;			/* TRUE or FALSE */
    caddr_t data;
{
    set_orientation_menu ((Boolean) closure, True);
}


main (argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel, pane, box, viewport;
    XtAppContext app_con;
    Arg args[1];
    static XtCallbackRec callback_rec[2] = {{ NULL, NULL }, { NULL, NULL }};
    XtOrientation orient;

    ProgramName = argv[0];

    toplevel = XtAppInitialize (&app_con, "Viewres", 
				Options, XtNumber (Options),
				&argc, argv, fallback_resources,
				NULL, ZERO);
    if (argc != 1) usage ();
    XtAppAddActions (app_con, viewres_actions, XtNumber (viewres_actions));

    XtGetApplicationResources (toplevel, (caddr_t) &Appresources,
			       Resources, XtNumber(Resources), NULL, ZERO);
    initialize_nodes (widget_list, nwidgets);

    topnode = name_to_node (widget_list, nwidgets, Appresources.top_object);

    pane = XtCreateManagedWidget ("pane", panedWidgetClass, toplevel,
				  NULL, ZERO);

    box = XtCreateManagedWidget ("buttonbox", boxWidgetClass, pane,
				 NULL, ZERO);
    quitButton = XtCreateManagedWidget ("quit", commandWidgetClass, box,
					NULL, ZERO);
    XtSetArg (args[0], XtNmenuName, "formatMenu");
    formatButton = XtCreateManagedWidget ("format", menuButtonWidgetClass, box,
					  args, ONE);
    formatMenu = XtCreatePopupShell ("formatMenu", simpleMenuWidgetClass, 
				     formatButton, NULL, ZERO);
    XtSetArg (args[0], XtNcallback, callback_rec);
    callback_rec[0].callback = (XtCallbackProc) variable_labeltype_callback;
    callback_rec[0].closure = (caddr_t) TRUE;
    variableEntry = XtCreateManagedWidget ("showVariables", smeBSBObjectClass,
					   formatMenu, args, ONE);
    callback_rec[0].closure = (caddr_t) FALSE;
    classEntry = XtCreateManagedWidget ("showClasses", smeBSBObjectClass,
					   formatMenu, args, ONE);

    (void) XtCreateManagedWidget ("line", smeLineObjectClass, formatMenu,
				  NULL, ZERO);

    callback_rec[0].callback = (XtCallbackProc)horizontal_orientation_callback;
    callback_rec[0].closure = (caddr_t) TRUE;
    horizontalEntry = XtCreateManagedWidget ("layoutHorizontal",
					     smeBSBObjectClass,
					     formatMenu, args, ONE);
    callback_rec[0].closure = (caddr_t) FALSE;
    verticalEntry = XtCreateManagedWidget ("layoutVertical", smeBSBObjectClass,
					   formatMenu, args, ONE);

    XtSetArg (args[0], XtNbackgroundPixmap, None);
    viewport = XtCreateManagedWidget ("viewport", viewportWidgetClass,
				      pane, args, ONE);
    treeWidget = XtCreateManagedWidget ("tree", treeWidgetClass, viewport,
					NULL, 0);
    XtSetArg (args[0], XtNorientation, &orient);

    set_labeltype_menu (Appresources.show_variable, FALSE);
    set_orientation_menu ((Boolean)(orient == XtorientHorizontal), FALSE);
    build_tree (topnode, treeWidget, NULL);
    XtRealizeWidget (toplevel);
    XtAppMainLoop (app_con);
}



static void HandleQuit (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    exit (0);
}


static void HandleSetLableType (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    char *cmd;
    Arg args[1];
    Boolean oldvar = Appresources.show_variable, newvar;

    switch (*num_params) {
      case 0:
	cmd = "toggle";
	break;
      case 1:
	cmd = params[0];
	break;
      default:
	XBell (XtDisplay(w), 0);
	return;
    }

    if (XmuCompareISOLatin1 (cmd, "toggle") == 0) {
	newvar = !oldvar;
    } else if (XmuCompareISOLatin1 (cmd, "variable") == 0) {
	newvar = TRUE;
    } else if (XmuCompareISOLatin1 (cmd, "class") == 0) {
	newvar = FALSE;
    } else {
	XBell (XtDisplay(w), 0);
	return;
    }

    if (newvar != oldvar) set_labeltype_menu (newvar, TRUE);
    return;
}


static void HandleSetOrientation (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    char *cmd;
    Arg args[1];
    Boolean oldhoriz, horiz;
    XtOrientation orient;

    switch (*num_params) {
      case 0:
	cmd = "toggle";
	break;
      case 1:
	cmd = params[0];
	break;
      default:
	XBell (XtDisplay(w), 0);
	return;
    }

    XtSetArg (args[0], XtNorientation, &orient);
    XtGetValues (treeWidget, args, ONE);
    oldhoriz = (Boolean) (orient == XtorientHorizontal);

    if (XmuCompareISOLatin1 (cmd, "toggle") == 0) {
	horiz = !oldhoriz;
    } else if (XmuCompareISOLatin1 (cmd, "horizontal") == 0) {
	horiz = True;
    } else if (XmuCompareISOLatin1 (cmd, "vertical") == 0) {
	horiz = False;
    } else {
	XBell (XtDisplay(w), 0);
	return;
    }

    if (horiz != oldhoriz) set_orientation_menu (horiz, TRUE);
    return;
}



static void build_tree (node, tree, super)
    WidgetNode *node;
    Widget tree;
    Widget super;
{
    Widget w;				/* widget for this Class */
    WidgetNode *child;			/* iterator over children */
    Arg args[2];			/* need to set super node */
    Cardinal n;				/* count of args */

    n = 0;
    XtSetArg (args[n], XtNparent, super); n++;
    XtSetArg (args[n], XtNlabel, (Appresources.show_variable ?
				  node->label : WnClassname(node))); n++;

    w = XtCreateManagedWidget (node->label, commandWidgetClass, tree, args, n);
    node->data = (char *) w;

    /*
     * recursively build the rest of the tree
     */
    for (child = node->children; child; child = child->siblings) {
	build_tree (child, tree, w);
    }
}


static void set_node_labels (node, depth)
    WidgetNode *node;
    int depth;
{
    Arg args[1];
    Widget w = (Widget) node->data;
    WidgetNode *child;

    if (!node) return;
    XtSetArg (args[0], XtNlabel, (Appresources.show_variable ?
				  node->label : WnClassname(node)));
    XtSetValues (w, args, ONE);

    for (child = node->children; child; child = child->siblings) {
	set_node_labels (child, depth + 1);
    }
}


static void set_oneof_sensitive (choosea, a, b)
    Boolean choosea;
    Widget a, b;
{
    static Arg args[1] = { XtNsensitive, (XtArgVal) NULL };

    args[0].value = (XtArgVal) TRUE;
    XtSetValues (choosea ? a : b, args, ONE);
    args[0].value = (XtArgVal) FALSE;
    XtSetValues (choosea ? b : a, args, ONE);
}

static void set_labeltype_menu (isvar, doall)
    Boolean isvar;
    Boolean doall;
{
    Appresources.show_variable = isvar;
    set_oneof_sensitive (isvar, classEntry, variableEntry);

    if (doall) {
	XUnmapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
	set_node_labels (topnode, 0);
	XawTreeForceLayout (treeWidget);
	XMapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
    }
}

static void set_orientation_menu (horiz, dosetvalues)
    Boolean horiz, dosetvalues;
{
    set_oneof_sensitive (horiz, verticalEntry, horizontalEntry);

    if (dosetvalues) {
	Arg args[1];

	XtSetArg (args[0], XtNorientation,
		  horiz ? XtorientHorizontal : XtorientVertical);
	XUnmapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
 	XtSetValues (treeWidget, args, ONE);
	XMapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
    }
}
