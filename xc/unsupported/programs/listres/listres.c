/*
 * $XConsortium: listres.c,v 1.10 89/07/11 18:52:21 jim Exp $
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
#include "defs.h"

extern WidgetNode widget_list[];
extern int nwidgets;


static XrmOptionDescRec Options[] = {
  { "-known", "*listKnown", XrmoptionNoArg, (caddr_t) "on" },
  { "-top", "*topWidget", XrmoptionNoArg, (caddr_t) "on" },
  { "-format", "*resourceFormat", XrmoptionSepArg, (caddr_t) NULL },
  { "-nosuper", "*showSuper", XrmoptionNoArg, (caddr_t) "off" },
  { "-variable", "*showVariable", XrmoptionNoArg, (caddr_t) "on" },
};

static struct _appresources {
    Boolean known;
    Boolean show_variable;
    Boolean show_superclass;
    char *top_object;
    char *format;
} Appresources;


static XtResource Resources[] = {
#define offset(field) XtOffset(struct _appresources *, field)
  { "listKnown", "ListKnown", XtRBoolean, sizeof(Boolean),
      offset(known), XtRImmediate, (caddr_t) FALSE },
  { "showSuper", "ShowSuper", XtRBoolean, sizeof(Boolean),
      offset(show_superclass), XtRImmediate, (caddr_t) TRUE },
  { "showVariable", "ShowVariable", XtRBoolean, sizeof(Boolean),
      offset(show_variable), XtRImmediate, (caddr_t) FALSE },
  { "topObject", "TopObject", XtRString, sizeof(char *),
      offset(top_object), XtRString, (caddr_t) "core" },
  { "resourceFormat", "ResourceFormat", XtRString, sizeof(char *),
      offset(format), XtRString, (caddr_t) " %-16s %20s  %-20s  %s" },
#undef offset
};



char *ProgramName;

usage ()
{
    fprintf(stderr, "usage:  %s [-options...]\n", ProgramName);
    fprintf(stderr, "\nwhere options include:\n");
    fprintf(stderr,
	    "    -known           list known widget classes\n");
    fprintf(stderr,
	    "    -nosuper         do not print superclass resources\n");
    fprintf(stderr,
	    "    -variable        show variable name instead of class name\n");
    fprintf(stderr,
	    "    -top name        name of top object or widget to display\n");
    fprintf(stderr,
	    "    -format string   printf format for instance, class, type\n");
    fprintf(stderr, "\n");
    exit (1);
}

static void list_known_widgets ()
{
    int i;
    WidgetNode *wn;

    for (i = 0, wn = widget_list; i < nwidgets; i++, wn++) {
	printf ("%-24s  %s\n", wn->label, WnClass(wn)->core_class.class_name);
    }
    exit (0);
}


main (argc, argv)
    int argc;
    char **argv;
{
    int i;
    WidgetNode *topnode;
    Widget toplevel;

    ProgramName = argv[0];

    toplevel = XtInitialize (NULL, "Listres", Options, XtNumber (Options),
			     &argc, argv);
    XtGetApplicationResources (toplevel, (caddr_t) &Appresources,
			       Resources, XtNumber(Resources), NULL, ZERO);
    if (Appresources.known) list_known_widgets ();

    initialize_nodes (widget_list, nwidgets);
    topnode = name_to_node (widget_list, nwidgets, Appresources.top_object);
    argc--, argv++;

    if (argc == 0) {
	WidgetNode *wn;
	for (i = 0, wn = widget_list; i < nwidgets; i++, wn++) {
	    list_resources (wn, Appresources.format, topnode, toplevel,
			    (Bool) Appresources.show_superclass,
			    (Bool) Appresources.show_variable);
	}
    } else {
	for (; argc > 0; argc--, argv++) {
	    WidgetNode *node;

	    if (argv[0][0] == '-') usage ();
	    node = name_to_node (widget_list, nwidgets, *argv);
	    if (!node) {
		fprintf (stderr, "%s:  unable to find widget \"%s\"\n",
			 ProgramName, *argv);
		continue;
	    }
	    list_resources (node, Appresources.format, topnode, toplevel,
			    (Bool) Appresources.show_superclass,
			    (Bool) Appresources.show_variable);
	}
    }
    exit (0);
}
