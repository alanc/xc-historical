/*
 * $XConsortium: viewres.c,v 1.18 90/02/05 21:46:14 jim Exp $
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
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Toggle.h>
#include "Tree.h"
#include <X11/Xmu/Converters.h>
#include <X11/Xmu/CharSet.h>
#include "defs.h"

#define INSERT_NODE(node,i) \
  selected_list.elements[(node)->selection_index = (i)] = (node)

#define REMOVE_NODE(node) \
  selected_list.elements[node->selection_index] = NULL; \
  node->selection_index = (-1)

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
    { "-lbw", "*Tree*Toggle*BorderWidth", XrmoptionSepArg, (caddr_t) NULL },
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
static void HandleSelect();
static void set_labeltype_menu(), set_orientation_menu();
static void build_tree(), set_node_labels();

static XtActionsRec viewres_actions[] = {
    { "Quit", HandleQuit },
    { "SetLabelType", HandleSetLableType },
    { "SetOrientation", HandleSetOrientation },
    { "Select", HandleSelect },
};

#define FORMAT_VARIABLES 0
#define FORMAT_CLASSES 1
#define FORMAT_HORIZONTAL 2
#define FORMAT_VERTICAL 3
#define FORMAT_number 4
#define SELECT_NOTHING 0
#define SELECT_PARENTS 1
#define SELECT_ALL 2
#define SELECT_WITH_RESOURCES 3
#define SELECT_WITHOUT_RESOURCES 4
#define SELECT_number 5

static Widget treeWidget;
static Widget quitButton, formatButton, formatMenu, selectButton, selectMenu;
static Widget format_widgets[FORMAT_number];
static Widget select_widgets[SELECT_number];
static WidgetNode *topnode;

struct {
    int n_elements;
    int max_elements;
    WidgetNode **elements;
} selected_list = { 0, 0, NULL };


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


static void initialize_widgetnode_list (listp, sizep, n)
    WidgetNode ***listp;
    int *sizep;
    int n;
{
    register int i;
    register WidgetNode **l;

    if (!*listp) {
        *listp = (WidgetNode **) XtCalloc (n, sizeof(WidgetNode **));
        *sizep = ((*listp) ? n : 0);
        return;
    }
    if (n > *sizep) {
        *listp = (WidgetNode **) XtRealloc (*listp, n * sizeof(WidgetNode **));
        *sizep = ((*listp) ? n : 0);
        if (!*listp) return;
    }
    for (i = *sizep, l = (*listp) + i; i < n; i++, l++) *l = NULL;
    return;
}


static int copydown (start)
    int start;
{
    WidgetNode **src = &selected_list.elements[start];
    WidgetNode **dst = src;
    int skips = 0;

    for (; start < selected_list.n_elements; start++, src++) {
	if (*src) *dst++ = *src;
	else skips++;
    }
    return skips;
}

static void add_to_selected_list (node, updatewidget)
    WidgetNode *node;
    Boolean updatewidget;
{
    if (node->selection_index >= 0) return;  /* already on list */

    if (selected_list.n_elements >= selected_list.max_elements) {
	initialize_widgetnode_list (&selected_list.elements,
				    &selected_list.max_elements, 
				    (selected_list.max_elements * 3) / 2);
    }
    INSERT_NODE (node, selected_list.n_elements);
    selected_list.n_elements++;

    if (updatewidget) {
	Arg args[1];

	XtSetArg (args[0], XtNstate, TRUE);
	XtSetValues (node->instance, args, ONE);
    }
}

static Boolean remove_from_selected_list (node, updatewidget)
    WidgetNode *node;
    Boolean updatewidget;
{
    int i, skips;
    WidgetNode **wn;

    if ((i = node->selection_index) < 0) return FALSE;

    REMOVE_NODE (node);

    /* copy down */
    if (selected_list.n_elements > 1) {
	skips = copydown (i);
    } else {
	skips = 1;
    }
    selected_list.n_elements -= skips;

    if (updatewidget) {
	Widget w = node->instance;
	Arg args[1];

	XtSetArg (args[0], XtNstate, FALSE);
	XtSetValues (w, args, ONE);
    }
}

static void select_callback (gw, closure, data)
    Widget gw;				/* entry widget */
    caddr_t closure;			/* TRUE or FALSE */
    caddr_t data;			/* undefined */
{
    Arg args[1];
    int i;
    WidgetNode *node;

    switch ((int) closure) {
      case SELECT_NOTHING:		/* clear selection_list */
	XtSetArg (args[0], XtNstate, FALSE);
	for (i = 0; i < selected_list.n_elements; i++) {
	    register WidgetNode *p = selected_list.elements[i];
	    REMOVE_NODE (p);
	    XtSetValues (p->instance, args, ONE);
	}
	selected_list.n_elements = 0;
	break;
      case SELECT_PARENTS:		/* chain up adding to selection_list */
	break;
      case SELECT_ALL:			/* put everything on selection_list */
	if (selected_list.max_elements < nwidgets) {
	    initialize_widgetnode_list (&selected_list.elements,
					&selected_list.max_elements,
					nwidgets);
	}
	XtSetArg (args[0], XtNstate, TRUE);
	for (i = 0, node = widget_list; i < nwidgets; i++, node++) {
	    INSERT_NODE (node, i);
	    XtSetValues (node->instance, args, ONE);
	}
	selected_list.n_elements = nwidgets;
	break;
      case SELECT_WITH_RESOURCES:	/* put all w/ rescnt > 0 on sel_list */
	break;
      case SELECT_WITHOUT_RESOURCES:	/* put all w recnt == 0 on sel_list */
	break;
      default:				/* error!!! */
	XBell (XtDisplay(gw), 0);
	return;
    }
}

static void toggle_callback (gw, closure, data)
    Widget gw;
    caddr_t closure;			/* WidgetNode for this widget */
    caddr_t data;			/* on or off */
{
    WidgetNode *node = (WidgetNode *) closure;
    Boolean selected = (Boolean) data;

    if (selected) {
	add_to_selected_list (node, FALSE);
    } else {
	remove_from_selected_list (node, FALSE);
    }
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

    initialize_widgetnode_list (&selected_list.elements,
				&selected_list.max_elements, 10);

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

    /*
     * Format menu
     /*/
    XtSetArg (args[0], XtNmenuName, "formatMenu");
    formatButton = XtCreateManagedWidget ("format", menuButtonWidgetClass, box,
					  args, ONE);
    formatMenu = XtCreatePopupShell ("formatMenu", simpleMenuWidgetClass, 
				     formatButton, NULL, ZERO);
    XtSetArg (args[0], XtNcallback, callback_rec);
    callback_rec[0].callback = (XtCallbackProc) variable_labeltype_callback;
#define MAKE_FORMAT(n,v,name) \
    callback_rec[0].closure = (caddr_t) v; \
    format_widgets[n] = XtCreateManagedWidget (name, smeBSBObjectClass, \
					       formatMenu, args, ONE)

    MAKE_FORMAT (FORMAT_VARIABLES, TRUE, "showVariables");
    MAKE_FORMAT (FORMAT_CLASSES, FALSE, "showClasses");

    (void) XtCreateManagedWidget ("line", smeLineObjectClass, formatMenu,
				  NULL, ZERO);
    callback_rec[0].callback = (XtCallbackProc)horizontal_orientation_callback;
    MAKE_FORMAT (FORMAT_HORIZONTAL, TRUE, "layoutHorizontal");
    MAKE_FORMAT (FORMAT_VERTICAL, FALSE, "layoutVertical");
#undef MAKE_FORMAT

    /*
     * Select menu
     */
    XtSetArg (args[0], XtNmenuName, "selectMenu");
    selectButton = XtCreateManagedWidget ("select", menuButtonWidgetClass, box,
					  args, ONE);
    selectMenu = XtCreatePopupShell ("selectMenu", simpleMenuWidgetClass, 
				     selectButton, NULL, ZERO);
    XtSetArg (args[0], XtNcallback, callback_rec);
    callback_rec[0].callback = (XtCallbackProc) select_callback;
#define MAKE_SELECT(n,name) \
    callback_rec[0].closure = (caddr_t) n; \
    select_widgets[n] = XtCreateManagedWidget (name, smeBSBObjectClass, \
					       selectMenu, args, ONE)
    MAKE_SELECT (SELECT_NOTHING, "unselect");
    (void) XtCreateManagedWidget ("line", smeLineObjectClass, selectMenu,
				  NULL, ZERO);
    MAKE_SELECT (SELECT_PARENTS, "selectParents");
    MAKE_SELECT (SELECT_ALL, "selectAll");
    MAKE_SELECT (SELECT_WITH_RESOURCES, "selectWithResources");
    MAKE_SELECT (SELECT_WITHOUT_RESOURCES, "selectWithoutResources");
#undef MAKE_SELECT


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


static void HandleSelect (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    int obj;
    char *cmd;

    if (*num_params != 1) {
	XBell (XtDisplay(w), 0);
	return;
    }

    cmd = (char *) params[0];

    if (XmuCompareISOLatin1 (cmd, "nothing") == 0) {
	obj = SELECT_NOTHING;
    } else if (XmuCompareISOLatin1 (cmd, "parents") == 0) {
	obj = SELECT_PARENTS;
    } else if (XmuCompareISOLatin1 (cmd, "all") == 0) {
	obj = SELECT_ALL;
    } else if (XmuCompareISOLatin1 (cmd, "resources") == 0) {
	obj = SELECT_WITH_RESOURCES;
    } else if (XmuCompareISOLatin1 (cmd, "noresources") == 0) {
	obj = SELECT_WITHOUT_RESOURCES;
    } else {
	XBell (XtDisplay(w), 0);
	return;
    }

    /*
     * use any old widget
     */
    select_callback (w, (caddr_t) obj, (caddr_t) NULL);
}



static void build_tree (node, tree, super)
    WidgetNode *node;
    Widget tree;
    Widget super;
{
    Widget w;				/* widget for this Class */
    WidgetNode *child;			/* iterator over children */
    Arg args[3];			/* need to set super node */
    Cardinal n;				/* count of args */
    static XtCallbackRec callback_rec[2] = {{ toggle_callback, NULL },
					     { NULL, NULL }};

    n = 0;
    XtSetArg (args[n], XtNparent, super); n++;
    XtSetArg (args[n], XtNlabel, (Appresources.show_variable ?
				  node->label : WnClassname(node))); n++;
    XtSetArg (args[n], XtNcallback, callback_rec); n++;

    callback_rec[0].closure = (caddr_t) node;
    w = XtCreateManagedWidget (node->label, toggleWidgetClass, tree, args, n);
    node->instance = w;

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
    WidgetNode *child;

    if (!node) return;
    XtSetArg (args[0], XtNlabel, (Appresources.show_variable ?
				  node->label : WnClassname(node)));
    XtSetValues (node->instance, args, ONE);

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
    set_oneof_sensitive (isvar, format_widgets[FORMAT_CLASSES],
			 format_widgets[FORMAT_VARIABLES]);

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
    set_oneof_sensitive (horiz, format_widgets[FORMAT_VERTICAL],
			 format_widgets[FORMAT_HORIZONTAL]);

    if (dosetvalues) {
	Arg args[1];

	XtSetArg (args[0], XtNorientation,
		  horiz ? XtorientHorizontal : XtorientVertical);
	XUnmapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
 	XtSetValues (treeWidget, args, ONE);
	XMapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
    }
}
