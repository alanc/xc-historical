/*
 * $XConsortium: viewres.c,v 1.8 90/02/02 15:16:40 jim Exp $
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
#include "Tree.h"
#include <X11/Core.h>
#include "defs.h"

extern WidgetNode widget_list[];
extern int nwidgets;


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

char *ProgramName;

static char *fallback_resources[] = {
    "*Viewport.allowHoriz: true",
    "*Viewport.allowVert: true",
    "*allowShellResize: true",
    NULL
};

usage ()
{
    fprintf(stderr, "usage:  %s [-options...]\n", ProgramName);
    fprintf(stderr, "\nwhere options include:\n");
    fprintf(stderr,
	    "    -lbw number      label border width in pixels\n");
    fprintf(stderr,
	    "    -lw number       line width in pixels\n");
    fprintf(stderr,
	    "    -top name        object to be top of tree\n");
    fprintf(stderr,
	    "    -variable        show variable name instead of class name\n");
    fprintf(stderr,
	    "    -vertical        list the tree vertically\n");
    fprintf(stderr, "\n");
    exit (1);
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

    /*
     * recursively build the rest of the tree
     */
    for (child = node->children; child; child = child->siblings) {
	build_tree (child, tree, w);
    }
}

main (argc, argv)
    int argc;
    char **argv;
{
    int i;
    WidgetNode *topnode;
    Widget toplevel, viewport, tree;
    XtAppContext app_con;

    ProgramName = argv[0];

    toplevel = XtAppInitialize (&app_con, "Viewres", 
				Options, XtNumber (Options),
				&argc, argv, fallback_resources,
				NULL, ZERO);
    if (argc != 1) usage ();

    XtGetApplicationResources (toplevel, (caddr_t) &Appresources,
			       Resources, XtNumber(Resources), NULL, ZERO);
    initialize_nodes (widget_list, nwidgets);

    topnode = name_to_node (widget_list, nwidgets, Appresources.top_object);

    viewport = XtCreateManagedWidget ("viewport", viewportWidgetClass,
				      toplevel, NULL, 0);
    tree = XtCreateManagedWidget ("tree", treeWidgetClass, viewport,
				  NULL, 0);

    build_tree (topnode, tree, NULL);
    XtRealizeWidget (toplevel);
    XtAppMainLoop (app_con);
}

