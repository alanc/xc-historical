/*
 * $XConsortium: viewres.c,v 1.10 90/02/02 17:59:11 jim Exp $
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
#include <X11/Xmu/Converters.h>
#include <X11/Xmu/CharSet.h>
#include "Tree.h"
#include <X11/Core.h>
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
    char *label_variable;
    char *label_class;
    char *label_horizontal;
    char *label_vertical;
} Appresources;

static XtResource Resources[] = {
#define offset(field) XtOffset(struct _appresources *, field)
    { "topObject", "TopObject", XtRString, sizeof(char *),
	offset(top_object), XtRString, (caddr_t) "object" },
    { "showVariable", "ShowVariable", XtRBoolean, sizeof(Boolean),
	offset(show_variable), XtRImmediate, (caddr_t) FALSE },
    { "labelVariable", "LabelVariable", XtRString, sizeof(String),
	offset(label_variable), XtRString, 
	(caddr_t) "Show Variable Names" },
    { "labelClass", "LabelClass", XtRString, sizeof(String),
	offset(label_class), XtRString, 
	(caddr_t) "Show Class Names" },
    { "labelHorizontal", "LabelHorizontal", XtRString, sizeof(String),
	offset(label_horizontal), XtRString, (caddr_t) "Layout Horizontal" },
    { "labelVertical", "LabelVertical", XtRString, sizeof(String),
	offset(label_vertical), XtRString, (caddr_t) "Layout Vertical" },
#undef offset
};


static char *fallback_resources[] = {
    "*Viewport.allowHoriz: true",
    "*Viewport.allowVert: true",
    "*allowShellResize: true",
    NULL
};

static void do_quit(), do_set_labeltype(), do_set_orientation();
static void set_labeltype_button(), set_orientation_button();
static void build_tree(), set_node_labels();

static XtActionsRec viewres_actions[] = {
    { "Quit", do_quit },
    { "SetLabelType", do_set_labeltype },
    { "SetOrientation", do_set_orientation },
};

static Widget treeWidget;
static Widget quitButton, labeltypeButton, orientationButton;
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


main (argc, argv)
    int argc;
    char **argv;
{
    int i;
    Widget toplevel, pane, box, viewport;
    XtAppContext app_con;

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
    labeltypeButton = XtCreateManagedWidget ("labeltype", commandWidgetClass,
					     box, NULL, ZERO);

    orientationButton = XtCreateManagedWidget ("orientation",
					       commandWidgetClass, box,
					       NULL, ZERO);

    viewport = XtCreateManagedWidget ("viewport", viewportWidgetClass,
				      pane, NULL, 0);
    treeWidget = XtCreateManagedWidget ("tree", treeWidgetClass, viewport,
					NULL, 0);

    set_labeltype_button ();
    set_orientation_button ();
    build_tree (topnode, treeWidget, NULL);
    XtRealizeWidget (toplevel);
    XtAppMainLoop (app_con);
}



static void do_quit (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    exit (0);
}


static void do_set_labeltype (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    char *cmd;
    Arg args[1];
    Boolean oldvar = Appresources.show_variable;

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
	Appresources.show_variable = !Appresources.show_variable;
    } else if (XmuCompareISOLatin1 (cmd, "variable") == 0) {
	Appresources.show_variable = TRUE;
    } else if (XmuCompareISOLatin1 (cmd, "class") == 0) {
	Appresources.show_variable = FALSE;
    } else {
	XBell (XtDisplay(w), 0);
	return;
    }

    if (Appresources.show_variable != oldvar) {
	set_labeltype_button ();
	set_node_labels (topnode, 0);
    }
    return;
}


static void do_set_orientation (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    char *cmd;
    Arg args[1];
    Bool oldhoriz, horiz;
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
    oldhoriz = (Bool) (orient == XtorientHorizontal);

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

    if (horiz != oldhoriz) {
	set_orientation_button (horiz);
	XtSetArg (args[0], XtNorientation,
		  horiz ? XtorientHorizontal : XtorientVertical);
 	XtSetValues (treeWidget, args, ONE);
    }
    return;
}

static void set_labeltype_button ()
{
    Arg args[1];

    XtSetArg (args[0], XtNlabel, 
	      (Appresources.show_variable ? Appresources.label_class
	       : Appresources.label_variable));
    XtSetValues (labeltypeButton, args, ONE);
}

static void set_orientation_button (horiz)
    Bool horiz;
{
    Arg args[1];

    XtSetArg (args[0], XtNlabel, 
	      (horiz ? Appresources.label_vertical
	       : Appresources.label_horizontal));
    XtSetValues (orientationButton, args, ONE);
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

    if (depth == 0) {
	XawTreeForceLayout (treeWidget);
    }
}
