/*
 * $XConsortium: listres.c,v 1.2 89/07/10 17:39:51 jim Exp $
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

#define coreWidgetClass widgetClass
#include "widgets.h"


static struct _appresources {
    Boolean objects;
} listres_resources;

static XtResource Resources[] = {
#define offset(field) XtOffset(struct _appresources *, field)
  { "objects", "Objects", XtRBoolean, sizeof(Boolean),
      offset(objects), XtRString, "FALSE" },
#undef offset
};



char *ProgramName;

usage ()
{
    fprintf (stderr, "usage:  %s [-objects] [-widgets]\n",
	     ProgramName);
    exit (1);
}

static void list_known_widgets ()
{
    int i;

    for (i = 0; i < nwidgets; i++) {
	printf ("%s:  %s\n", widget_list[i].label, 
		widget_list[i].widget_class[0]->core_class.class_name);
    }
    exit (0);
}


main (argc, argv)
    int argc;
    char **argv;
{
    int i;
    WidgetClass top;
    WidgetNameList *wl;
    Widget toplevel;

    ProgramName = argv[0];

    toplevel = XtInitialize (NULL, "Listres", NULL, 0, &argc, argv);
    XtGetApplicationResources (toplevel, (caddr_t) &listres_resources,
			       Resources, XtNumber(Resources), NULL, ZERO);

    for (i = 1; i < argc; i++) {
	if (strncmp ("-widgets", argv[i], strlen(argv[i])) == 0) 
	  list_known_widgets ();
	else usage ();
    }

    top = (listres_resources.objects ? NULL : coreWidgetClass);

    for (i = 0, wl = widget_list; i < nwidgets; i++, wl++) {
	list_resources (wl->label, wl->widget_class[0], top, toplevel);
    }

    exit (0);
}
