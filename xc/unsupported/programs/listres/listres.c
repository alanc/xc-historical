/*
 * $XConsortium: listres.c,v 1.8 89/07/10 19:21:12 jim Exp $
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


static XrmOptionDescRec listres_options[] = {
  { "-known", "*listKnown", XrmoptionNoArg, (caddr_t) "on" },
  { "-objects", "*showObjects", XrmoptionNoArg, (caddr_t) "on" },
  { "-format", "*resourceFormat", XrmoptionSepArg, (caddr_t) NULL },
  { "-nosort", "*sortEntries", XrmoptionNoArg, (caddr_t) "off" },
};

static struct _appresources {
    Boolean known;
    Boolean objects;
    Boolean sort;
    char *format;
} listres_resources;


static XtResource Resources[] = {
#define offset(field) XtOffset(struct _appresources *, field)
  { "listKnown", "ListKnown", XtRBoolean, sizeof(Boolean),
      offset(known), XtRImmediate, (caddr_t) FALSE },
  { "showObjects", "ShowObjects", XtRBoolean, sizeof(Boolean),
      offset(objects), XtRImmediate, (caddr_t) FALSE },
  { "sortEntries", "SortEntries", XtRBoolean, sizeof(Boolean),
      offset(sort), XtRImmediate, (caddr_t) TRUE },
  { "resourceFormat", "ResourceFormat", XtRString, sizeof(char *),
      offset(format), XtRString, " %27s  %-27s    %s" },
#undef offset
};



char *ProgramName;

usage ()
{
    fprintf(stderr, "usage:  %s [-options...]\n", ProgramName);
    fprintf(stderr, "\nwhere options include:\n");
    fprintf(stderr,
	    "    -known            list known widget classes\n");
    fprintf(stderr,
	    "    -objects          include object prefixes in listings\n");
    fprintf(stderr,
	    "    -format string    printf format for instance, class, type\n");
    fprintf(stderr,
	    "    -nosort           do not sort output\n");
    fprintf(stderr, "\n");
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
    struct _extra { 
	int off_cl;
	char data[2];	/* nuls plus alloced off end */
    } *ep;

    ProgramName = argv[0];

    toplevel = XtInitialize (NULL, "Listres", listres_options, 
			     XtNumber(listres_options), &argc, argv);
    XtGetApplicationResources (toplevel, (caddr_t) &listres_resources,
			       Resources, XtNumber(Resources), NULL, ZERO);

    if (listres_resources.known) list_known_widgets ();
    top = (listres_resources.objects ? NULL : coreWidgetClass);
    argc--, argv++;

    if (argc == 0) {
	for (i = 0, wl = widget_list; i < nwidgets; i++, wl++) {
	    list_resources (listres_resources.format,
			    wl->label, wl->widget_class[0], top, toplevel,
			    listres_resources.sort);
	}
    } else {
	char tmpbuf[1024];
	char *buf = tmpbuf;
	int len = sizeof tmpbuf - 1;

	for (; argc > 0; argc--, argv++) {
	    int found = 0;

	    if (argv[0][0] == '-') usage ();

	    if (strlen(*argv) > len) {
		len = strlen (*argv) * 2 + 2;
		if (buf != tmpbuf) free (buf);
		buf = (char *) malloc (len);
		if (!buf) {
		    fprintf (stderr, "%s: can't alloc %d byte argv buffer\n",
			     ProgramName, len);
		    exit (1);
		}
	    }
	    XmuCopyISOLatin1Lowered (buf, *argv);
	    for (i = 0, wl = widget_list; i < nwidgets; i++, wl++) {
		CoreClassPart *cl = &wl->widget_class[0]->core_class;

		if (!wl->extra) {
		    int lablen = strlen (wl->label);
		    int cllen = strlen (cl->class_name);

		    ep = (struct _extra *) malloc (sizeof(struct _extra) +
						   lablen + cllen);
		    if (!ep) {
			fprintf (stderr,
				 "%s:  unable to alloc %d byte name buffer\n",
				 ProgramName, (sizeof(struct _extra) + 
					       lablen + cllen));
			exit (1);
		    }
		    ep->off_cl = lablen + 1;
		    XmuCopyISOLatin1Lowered (ep->data, wl->label);
		    XmuCopyISOLatin1Lowered (ep->data + ep->off_cl,
					     cl->class_name);
		    wl->extra = (caddr_t) ep;
		}
		ep = (struct _extra *) wl->extra;
		if (strcmp (buf, ep->data) == 0 ||
		    strcmp (buf, ep->data + ep->off_cl) == 0) {
		    list_resources (listres_resources.format,
				    wl->label, wl->widget_class[0], top,
				    toplevel, listres_resources.sort);

		    found++;
		}
	    }
	    if (found == 0) {
		fprintf (stderr, "%s:  unable to find widget \"%s\"\n",
			 ProgramName, *argv);
	    }
	}
	if (buf != tmpbuf) free (buf);
    }
    exit (0);
}
