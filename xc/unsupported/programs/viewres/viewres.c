/*
 * $XConsortium: viewres.c,v 1.36 90/02/12 18:36:45 jim Exp $
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
#include <X11/Xaw/Text.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Panner.h>
#include <X11/Xaw/Tree.h>
#include <X11/Xmu/Converters.h>
#include <X11/Xmu/CharSet.h>
#include "defs.h"

#define IsShowing(node) ((node)->resource_lw && \
			 XtIsManaged((node)->resource_lw))

#define INSERT_NODE(node,i) \
  selected_list.elements[(node)->selection_index = (i)] = (node)

#define REMOVE_NODE(node) \
  selected_list.elements[node->selection_index] = NULL; \
  node->selection_index = (-1)

char *ProgramName;
static int NumberShowing = 0;

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

static void ActionQuit(), ActionSetLableType(), ActionSetOrientation();
static void ActionSelect(), ActionResources();
static void set_labeltype_menu(), set_orientation_menu();
static void build_tree(), set_node_labels();

static XtActionsRec viewres_actions[] = {
    { "Quit", ActionQuit },
    { "SetLabelType", ActionSetLableType },
    { "SetOrientation", ActionSetOrientation },
    { "Select", ActionSelect },
    { "Resources", ActionResources },
};

#define BOOL_OFF 0
#define BOOL_ON 1
#define BOOL_TOGGLE 2

#define VIEW_VARIABLES 0
#define VIEW_CLASSES 1
#define VIEW_HORIZONTAL 2
#define VIEW_VERTICAL 3
#define VIEW_SHOW_RESOURCES 4
#define VIEW_HIDE_RESOURCES 5
#define VIEW_number 6

#define SELECT_NOTHING 0
#define SELECT_INVERT 1
#define SELECT_PARENTS 2
#define SELECT_CHILDREN 3
#define SELECT_ALL 4
#define SELECT_HAS_RESOURCES 5
#define SELECT_SHOWN_RESOURCES 6
#define SELECT_number 7

static struct _nametable {
    char *name;
    int value;
} select_nametable[] = {
    { "nothing", SELECT_NOTHING },
    { "invert", SELECT_INVERT },
    { "parents", SELECT_PARENTS },
    { "children", SELECT_CHILDREN },
    { "all", SELECT_ALL },
    { "resources", SELECT_HAS_RESOURCES },
    { "shown", SELECT_SHOWN_RESOURCES },
}, boolean_nametable[] = {
    { "off", BOOL_OFF },
    { "false", BOOL_OFF },
    { "no", BOOL_OFF },
    { "on", BOOL_ON },
    { "true", BOOL_ON },
    { "yes", BOOL_ON },
    { "toggle", BOOL_TOGGLE },
}, scroll_nametable[] = {
    { "backward", 0 },
    { "forward", 1 },
};

static Widget treeWidget, viewportWidget, pannerWidget;
static Widget quitButton, viewButton, viewMenu, selectButton, selectMenu;
static Widget view_widgets[VIEW_number];
static Widget select_widgets[SELECT_number];
static WidgetNode *topnode;

static Arg false_args[1] = {{ XtNstate, (XtArgVal) FALSE }};
static Arg true_args[1] = {{ XtNstate, (XtArgVal) TRUE }};

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


static WidgetNode *widget_to_node (w)
    Widget w;
{
    int i;
    WidgetNode *node;

    for (i = 0, node = widget_list; i < nwidgets; i++, node++) {
	if (node->instance == w) return node;
    }
    return NULL;
}


/* ARGSUSED */
static void variable_labeltype_callback (gw, closure, data)
    Widget gw;
    caddr_t closure;			/* TRUE or FALSE */
    caddr_t data;
{
    set_labeltype_menu ((Boolean) closure, True);
}

/* ARGSUSED */
static void horizontal_orientation_callback (gw, closure, data)
    Widget gw;
    caddr_t closure;			/* TRUE or FALSE */
    caddr_t data;
{
    set_orientation_menu ((Boolean) closure, True);
}


static Boolean create_resource_lw (node)
    WidgetNode *node;
{
    Arg args[4];
    Cardinal n;

    if (node->nnewresources == 0) return FALSE;

    if (!node->resource_labels &&
	!set_resource_labels (node)) return FALSE;

    n = 0;
    XtSetArg (args[n], XtNnumberStrings, 3 * node->nnewresources); n++;
    XtSetArg (args[n], XtNlist, node->resource_labels); n++;
    XtSetArg (args[n], XtNdefaultColumns, 3); n++;
    XtSetArg (args[n], XtNforceColumns, TRUE); n++;
    node->resource_lw = XtCreateManagedWidget (node->label, listWidgetClass,
					       XtParent(node->instance),
					       args, n);
    XtRealizeWidget (node->resource_lw);
    return TRUE;
}

static void update_selection_items ()
{
    register int i;
    static Arg args[1] = {{ XtNsensitive, (XtArgVal) FALSE }};
    Boolean show = FALSE, hide = FALSE, parents = FALSE, children = FALSE;

    for (i = 0; i < selected_list.n_elements; i++) {
	WidgetNode *node = selected_list.elements[i];
	/*
	 * If node has any new resources then may be shown (if not
	 * already being shown).  If node has widget and is managed,
	 * then may be hidden.
	 */
	if (node->nnewresources > 0) {
	    if (IsShowing(node)) {
		hide = TRUE;
	    } else {
		show = TRUE;
	    }
	}
	if (node != topnode) parents = TRUE;
	if (node->children) children = TRUE;
    }
	    
    args[0].value = (XtArgVal) show;
    XtSetValues (view_widgets[VIEW_SHOW_RESOURCES], args, ONE);
    args[0].value = (XtArgVal) hide;
    XtSetValues (view_widgets[VIEW_HIDE_RESOURCES], args, ONE);
    args[0].value = (XtArgVal) (selected_list.n_elements > 0 ? TRUE : FALSE);
    XtSetValues (select_widgets[SELECT_NOTHING], args, ONE);
    args[0].value = (XtArgVal) parents;
    XtSetValues (select_widgets[SELECT_PARENTS], args, ONE);
    args[0].value = (XtArgVal) children;
    XtSetValues (select_widgets[SELECT_CHILDREN], args, ONE);
    args[0].value = (XtArgVal) ((Boolean) (NumberShowing > 0));
    XtSetValues (select_widgets[SELECT_SHOWN_RESOURCES], args, ONE);
}


static void do_resources (node, op)
    WidgetNode *node;
    Boolean op;
{
    if (op == BOOL_TOGGLE) op = (IsShowing(node) ? BOOL_OFF : BOOL_ON);

    if (op == BOOL_ON) {
	if (node->resource_lw) {		/* if already created */
	    if (!XtIsManaged(node->resource_lw)) {
		NumberShowing++;
		XtManageChild (node->resource_lw);
	    }				/* else ignore it */
	} else if (create_resource_lw (node))	/* create it */
	  NumberShowing++;
    } else if (node->resource_lw) {		/* if already created */
	if (XtIsManaged (node->resource_lw)) {
	    NumberShowing--;
	    XtUnmanageChild (node->resource_lw);
	}				/* else ignore it */
    }
}


/* ARGSUSED */
static void show_resources_callback (gw, closure, data)
    Widget gw;				/* menu or toggle button */
    caddr_t closure;			/* BOOL_OFF, BOOL_ON, BOOL_TOGGLE */
    caddr_t data;			/* undefined */
{
    int op = (int) closure;

    if (XtIsSubclass (gw, toggleWidgetClass)) {	 /* do single item op */
	WidgetNode *node = widget_to_node (gw);

	if (node) {
	    XUnmapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
	    do_resources (node, op);
	} else
	  return;
    } else if (selected_list.n_elements <= 0) {
	return;
    } else {
	int i;

	XUnmapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
	for (i = 0; i < selected_list.n_elements; i++) {
	    do_resources (selected_list.elements[i], op);
	}
    }
    XawTreeForceLayout (treeWidget);
    XMapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
    update_selection_items ();
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
    register int start;
{
    register WidgetNode **src = &selected_list.elements[start];
    register WidgetNode **dst = src;
    register int cur;

    for (cur = start; start < selected_list.n_elements; start++, src++) {
	if (*src) {
	    (*src)->selection_index = cur++;
	    *dst++ = *src;
	}
    }
    return (start - cur);
}


static void add_to_selected_list (node, updatewidget)
    WidgetNode *node;
    Boolean updatewidget;
{
    if (!node->instance || node->selection_index >= 0) return;

    if (selected_list.n_elements >= selected_list.max_elements) {
	initialize_widgetnode_list (&selected_list.elements,
				    &selected_list.max_elements, 
				    (selected_list.max_elements * 3) / 2);
    }
    INSERT_NODE (node, selected_list.n_elements);
    selected_list.n_elements++;

    if (updatewidget) XtSetValues (node->instance, true_args, ONE);
}

static Boolean remove_from_selected_list (node, updatewidget)
    WidgetNode *node;
    Boolean updatewidget;
{
    int i, skips;

    if ((i = node->selection_index) < 0) return FALSE;

    REMOVE_NODE (node);

    /* copy down */
    if (selected_list.n_elements > 1) {
	skips = copydown (i);
    } else {
	skips = 1;
    }
    selected_list.n_elements -= skips;

    if (updatewidget) XtSetValues (node->instance, false_args, ONE);
    return TRUE;
}

static void remove_nodes_from_selected_list (start, count, updatewidget)
    int start, count;
    Boolean updatewidget;
{
    int i;

    for (i = 0; i < count; i++) {
	register WidgetNode *p = selected_list.elements[i];
	REMOVE_NODE (p);
	if (updatewidget) XtSetValues (p->instance, false_args, ONE);
    }
    selected_list.n_elements -= copydown (0);
}
	    
static void add_subtree_to_selected_list (node, updatewidget)
    WidgetNode *node;
    Boolean updatewidget;
{
    if (!node) return;

    add_to_selected_list (node, updatewidget);
    for (node = node->children; node; node = node->siblings) {
	add_subtree_to_selected_list (node, updatewidget);
    }
}


/* ARGSUSED */
static void select_callback (gw, closure, data)
    Widget gw;				/* entry widget */
    caddr_t closure;			/* TRUE or FALSE */
    caddr_t data;			/* undefined */
{
    register int i;
    int nselected = selected_list.n_elements;
    WidgetNode *node;

    switch ((int) closure) {
      case SELECT_INVERT:		/* toggle selection state */
	for (i = 0, node = widget_list; i < nwidgets; i++, node++) {
	    if (node->selection_index < 0) add_to_selected_list (node, TRUE);
	}
	remove_nodes_from_selected_list (0, nselected, True);
	break;

      case SELECT_NOTHING:		/* clear selection_list */
	remove_nodes_from_selected_list (0, nselected, True);
	break;

      case SELECT_PARENTS:		/* chain up adding to selection_list */
	node = (XtIsSubclass (gw, toggleWidgetClass) ? widget_to_node (gw)
		: NULL);
	if (node) {
	    do {
		add_to_selected_list (node, TRUE);
	    } while (node = node->superclass);
	} else {
	    for (i = 0; i < nselected; i++) {
		WidgetNode *parent = selected_list.elements[i];

		/*
		 * chain up the tree, but stop if we get to nodes that
		 * are already in the selected list.
		 */
		while (parent = parent->superclass) {  /* do parents */
		    if (parent->selection_index >= 0) break;
		    add_to_selected_list (parent, TRUE);
		}
	    }
	}
	break;

      case SELECT_CHILDREN:		/* all sub nodes */
	node = (XtIsSubclass (gw, toggleWidgetClass) ? widget_to_node (gw)
		: NULL);
	if (node) {
	    add_subtree_to_selected_list (node);
	} else {
	    for (i = 0; i < nselected; i++) {
		WidgetNode *parent = selected_list.elements[i];

		add_subtree_to_selected_list (parent, TRUE);
	    }
	}
	break;

      case SELECT_ALL:			/* put everything on selection_list */
	add_subtree_to_selected_list (topnode, TRUE);
	break;

      case SELECT_HAS_RESOURCES:	/* put all w/ rescnt > 0 on sel_list */
	for (i = 0, node = widget_list; i < nwidgets; i++, node++) {
	    if (node->nnewresources > 0) add_to_selected_list (node, TRUE);
	}
	break;

      case SELECT_SHOWN_RESOURCES:
	for (i = 0, node = widget_list; i < nwidgets; i++, node++) {
	    if (IsShowing(node)) add_to_selected_list (node, TRUE);
	}
	break;

      default:				/* error!!! */
	XBell (XtDisplay(gw), 0);
	return;
    }

    update_selection_items ();
}

/* ARGSUSED */
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
	(void) remove_from_selected_list (node, FALSE);
    }

    update_selection_items ();
}

/* ARGSUSED */
static void panner_callback (gw, closure, data)
    Widget gw;
    caddr_t closure;			/* undefined */
    caddr_t data;			/* report */
{
    XawPannerReport *rep = (XawPannerReport *) data;

    if (viewportWidget)
      XawViewportSetLocation (viewportWidget,
			      (((float) rep->slider_x) /
			       ((float) rep->canvas_width)),
			      (((float) rep->slider_y) /
			       ((float) rep->canvas_height)));
}

/* ARGSUSED */
static void viewport_callback (gw, closure, data)
    Widget gw;
    caddr_t closure;			/* undefined */
    caddr_t data;			/* report */
{
    XawViewportReport *rep = (XawViewportReport *) data;
    Arg args[6];

    if (pannerWidget) {
	XtSetArg (args[0], XtNsliderX, -rep->child_x);
	XtSetArg (args[1], XtNsliderY, -rep->child_y);
	XtSetArg (args[2], XtNsliderWidth, rep->clip_width);
	XtSetArg (args[3], XtNsliderHeight, rep->clip_height);
	XtSetArg (args[4], XtNcanvasWidth, rep->child_width);
	XtSetArg (args[5], XtNcanvasHeight, rep->child_height);
	XtSetValues (pannerWidget, args, SIX);
    }
}


main (argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel, pane, box, dummy;
    XtAppContext app_con;
    Arg args[5];
    Dimension width, height;
    static XtCallbackRec callback_rec[2] = {{ NULL, NULL }, { NULL, NULL }};
    XtOrientation orient;
    int i;
    XSetWindowAttributes attr;

    ProgramName = argv[0];

    XtSetArg (args[0], XtNmappedWhenManaged, FALSE);
    toplevel = XtAppInitialize (&app_con, "Viewres", 
				Options, XtNumber (Options),
				&argc, argv, fallback_resources, args, ONE);
    if (argc != 1) usage ();
    XtAppAddActions (app_con, viewres_actions, XtNumber (viewres_actions));

    initialize_widgetnode_list (&selected_list.elements,
				&selected_list.max_elements, 10);

    XtGetApplicationResources (toplevel, (caddr_t) &Appresources,
			       Resources, XtNumber(Resources), NULL, ZERO);
    initialize_nodes (widget_list, nwidgets);

    topnode = name_to_node (widget_list, nwidgets, Appresources.top_object);

    /*
     * create dummy widgets to initialize resources
     */
    XtSetArg (args[0], XtNwidth, 1);
    XtSetArg (args[1], XtNheight, 1);
    /*
     * whack a text first so that asciiSrc gets its converters installed
     */
    dummy = XtCreateWidget ("dummy", textWidgetClass, toplevel, args, TWO);
    XtDestroyWidget (dummy);
    dummy = XtCreateWidget ("dummy", widgetClass, toplevel, args, TWO);
    for (i = 0; i < nwidgets; i++) {
	WidgetNode *node = &widget_list[i];
	initialize_resources (node, dummy, topnode);
	node->nnewresources = count_owned_resources (node, node);
    }
    XtDestroyWidget (dummy);

    pane = XtCreateManagedWidget ("pane", panedWidgetClass, toplevel,
				  NULL, ZERO);

    box = XtCreateManagedWidget ("buttonbox", boxWidgetClass, pane,
				 NULL, ZERO);
    quitButton = XtCreateManagedWidget ("quit", commandWidgetClass, box,
					NULL, ZERO);

    /*
     * Format menu
     */
    XtSetArg (args[0], XtNmenuName, "viewMenu");
    viewButton = XtCreateManagedWidget ("view", menuButtonWidgetClass, box,
					args, ONE);
    viewMenu = XtCreatePopupShell ("viewMenu", simpleMenuWidgetClass, 
				   viewButton, NULL, ZERO);
    XtSetArg (args[0], XtNcallback, callback_rec);
    callback_rec[0].callback = (XtCallbackProc) variable_labeltype_callback;
#define MAKE_VIEW(n,v,name) \
    callback_rec[0].closure = (caddr_t) v; \
    view_widgets[n] = XtCreateManagedWidget (name, smeBSBObjectClass, \
					     viewMenu, args, ONE)

    MAKE_VIEW (VIEW_VARIABLES, TRUE, "namesVariable");
    MAKE_VIEW (VIEW_CLASSES, FALSE, "namesClass");

    (void) XtCreateManagedWidget ("line1", smeLineObjectClass, viewMenu,
				  NULL, ZERO);
    callback_rec[0].callback = (XtCallbackProc)horizontal_orientation_callback;
    MAKE_VIEW (VIEW_HORIZONTAL, TRUE, "layoutHorizontal");
    MAKE_VIEW (VIEW_VERTICAL, FALSE, "layoutVertical");

    (void) XtCreateManagedWidget ("line2", smeLineObjectClass, viewMenu,
				  NULL, ZERO);
    callback_rec[0].callback = (XtCallbackProc) show_resources_callback;
    MAKE_VIEW (VIEW_SHOW_RESOURCES, BOOL_ON, "viewResources");
    MAKE_VIEW (VIEW_HIDE_RESOURCES, BOOL_OFF, "viewNoResources");
#undef MAKE_VIEW

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
    MAKE_SELECT (SELECT_INVERT, "selectInvert");
    MAKE_SELECT (SELECT_PARENTS, "selectParents");
    MAKE_SELECT (SELECT_CHILDREN, "selectChildren");
    MAKE_SELECT (SELECT_ALL, "selectAll");
    MAKE_SELECT (SELECT_HAS_RESOURCES, "selectHasResources");
    MAKE_SELECT (SELECT_SHOWN_RESOURCES, "selectShownResources");
#undef MAKE_SELECT


    XtSetArg (args[0], XtNcallback, callback_rec);
    callback_rec[0].callback = (XtCallbackProc) panner_callback;
    callback_rec[0].closure = (caddr_t) NULL;
    pannerWidget = XtCreateManagedWidget ("panner", pannerWidgetClass, pane,
					  args, ONE);

    XtSetArg (args[0], XtNnotifyCallback, callback_rec);
    callback_rec[0].callback = (XtCallbackProc) viewport_callback;
    callback_rec[0].closure = (caddr_t) NULL;
    viewportWidget = XtCreateManagedWidget ("viewport", viewportWidgetClass,
					    pane, args, ONE);

    treeWidget = XtCreateManagedWidget ("tree", treeWidgetClass,
					viewportWidget, NULL, ZERO);

    set_labeltype_menu (Appresources.show_variable, FALSE);
    set_orientation_menu ((Boolean)(orient == XtorientHorizontal), FALSE);
    update_selection_items ();
    build_tree (topnode, treeWidget, NULL);

    XtRealizeWidget (toplevel);
    XtSetArg (args[0], XtNwidth, &width);
    XtSetArg (args[1], XtNheight, &height);
    XtGetValues (treeWidget, args, TWO);
    XtSetArg (args[0], XtNcanvasWidth, width);
    XtSetArg (args[1], XtNcanvasHeight, height);
    XtSetArg (args[2], XtNsliderWidth, width);
    XtSetArg (args[3], XtNsliderHeight, height);
    XtSetValues (pannerWidget, args, FOUR);

    XtMapWidget (toplevel);
    XtAppMainLoop (app_con);
}



/* ARGSUSED */
static void ActionQuit (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    exit (0);
}


/* ARGSUSED */
static void ActionSetLableType (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    char *cmd;
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

/* ARGSUSED */
static void ActionSetOrientation (w, event, params, num_params)
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
	horiz = TRUE;
    } else if (XmuCompareISOLatin1 (cmd, "vertical") == 0) {
	horiz = FALSE;
    } else {
	XBell (XtDisplay(w), 0);
	return;
    }

    if (horiz != oldhoriz) set_orientation_menu (horiz, TRUE);
    return;
}


static void do_single_arg (w, params, nparams, table, nentries, proc)
    Widget w;
    String *params;
    Cardinal nparams;
    struct _nametable table[];
    int nentries;
    void (*proc)();
{
    int obj;
    int i;

    if (nparams != 1) {
	XBell (XtDisplay(w), 0);
	return;
    }

    for (i = 0; i < nentries; i++) {
	if (XmuCompareISOLatin1 (params[0], table[i].name) == 0) {
	    obj = table[i].value;
	    break;
	}
    }
    if (i == nentries) {
	XBell (XtDisplay(w), 0);
	return;
    }

    /*
     * use any old widget
     */
    (*proc) (w, (caddr_t) obj, (caddr_t) NULL);
}


/* ARGSUSED */
static void ActionSelect (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    do_single_arg (w, params, *num_params, select_nametable, 
		   XtNumber(select_nametable), select_callback);
}


/* ARGSUSED */
static void ActionResources (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (*num_params == 0) {
	show_resources_callback (w, (caddr_t) BOOL_TOGGLE, NULL);
    } else {
	do_single_arg (w, params, *num_params, boolean_nametable,
		       XtNumber(boolean_nametable), show_resources_callback);
    }
}


static void build_tree (node, tree, super)
    WidgetNode *node;
    Widget tree;
    Widget super;
{
    Widget box, w;			/* widget for this Class */
    WidgetNode *child;			/* iterator over children */
    Arg args[3];			/* need to set super node */
    Cardinal n;				/* count of args */
    static XtCallbackRec callback_rec[2] = {{ toggle_callback, NULL },
					     { NULL, NULL }};


    n = 0;
    XtSetArg (args[n], XtNtreeParent, super); n++;
    box = XtCreateManagedWidget (node->label, boxWidgetClass, tree, args, n);

    n = 0;
    XtSetArg (args[n], XtNlabel, (Appresources.show_variable ?
				  node->label : WnClassname(node))); n++;
    XtSetArg (args[n], XtNcallback, callback_rec); n++;

    callback_rec[0].closure = (caddr_t) node;
    w = XtCreateManagedWidget (node->label, toggleWidgetClass, box, args, n);
    node->instance = w;

    /*
     * recursively build the rest of the tree
     */
    for (child = node->children; child; child = child->siblings) {
	build_tree (child, tree, box);
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


static void oneof_sensitive (choosea, a, b)
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
    oneof_sensitive (isvar, view_widgets[VIEW_CLASSES],
		     view_widgets[VIEW_VARIABLES]);

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
    oneof_sensitive (horiz, view_widgets[VIEW_VERTICAL],
			 view_widgets[VIEW_HORIZONTAL]);

    if (dosetvalues) {
	Arg args[1];

	XtSetArg (args[0], XtNorientation,
		  horiz ? XtorientHorizontal : XtorientVertical);
	XUnmapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
 	XtSetValues (treeWidget, args, ONE);
	XMapWindow (XtDisplay(treeWidget), XtWindow(treeWidget));
    }
}
