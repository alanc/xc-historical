/*
 * xload - display system load average in a window
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * $XConsortium: xload.c,v 1.28 90/12/21 11:40:52 gildea Exp $
 */

#include <stdio.h> 
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/StripChart.h>
#include <X11/Xmu/SysUtil.h>

#include "xload.bit"

char *ProgramName;

extern void exit(), GetLoadPoint();
static void quit();

/*
 * Definition of the Application resources structure.
 */

typedef struct _XLoadResources {
  Boolean show_label;
} XLoadResources;

/*
 * Command line options table.  Only resources are entered here...there is a
 * pass over the remaining options after XtParseCommand is let loose. 
 */

static XrmOptionDescRec options[] = {
 {"-scale",		"*load.minScale",	XrmoptionSepArg,	NULL},
 {"-update",		"*load.update",		XrmoptionSepArg,	NULL},
 {"-hl",		"*load.highlight",	XrmoptionSepArg,	NULL},
 {"-highlight",		"*load.highlight",	XrmoptionSepArg,	NULL},
 {"-label",		"*label.label",		XrmoptionSepArg,	NULL},
 {"-nolabel",		"*showLabel",	        XrmoptionNoArg,       "False"},
 {"-jumpscroll",	"*load.jumpScroll",	XrmoptionSepArg,	NULL},
};

/*
 * The structure containing the resource information for the
 * Xload application resources.
 */

#define Offset(field) (XtOffsetOf(XLoadResources, field))

static XtResource my_resources[] = {
  {"showLabel", XtCBoolean, XtRBoolean, sizeof(Boolean),
     Offset(show_label), XtRImmediate, (caddr_t) TRUE},
};

#undef Offset

static XtActionsRec xload_actions[] = {
    { "quit",	quit },
};
static Atom wm_delete_window;


/*
 * Exit with message describing command line format.
 */

void usage()
{
    fprintf (stderr, "usage:  %s [-options ...]\n\n", ProgramName);
    fprintf (stderr, "where options include:\n");
    fprintf (stderr,
      "    -display dpy            X server on which to display\n");
    fprintf (stderr,
      "    -geometry geom          size and location of window\n");
    fprintf (stderr, 
      "    -fn font                font to use in label\n");
    fprintf (stderr, 
      "    -scale number           minimum number of scale lines\n");
    fprintf (stderr, 
      "    -update seconds         interval between updates\n");
    fprintf (stderr,
      "    -label string           annotation text\n");
    fprintf (stderr, 
      "    -bg color               background color\n");
    fprintf (stderr, 
      "    -fg color               graph color\n");
    fprintf (stderr, 
      "    -hl color               scale and text color\n");
    fprintf (stderr, 
      "    -nolabel                removes the label from above the chart.\n");
    fprintf (stderr, 
      "    -jumpscroll value       number of pixels to scroll on overflow\n");
    fprintf (stderr, "\n");
    exit(1);
}

void main(argc, argv)
    int argc;
    char **argv;
{
    char host[256], * label;
    Widget toplevel, load, pane, label_wid, load_parent;
    Arg args[1];
    Pixmap icon_pixmap = None;
    XLoadResources resources;
    XtAppContext app_con;

    ProgramName = argv[0];

    /* For security reasons, we reset our uid/gid after doing the necessary
       system initialization and before calling any X routines. */
    InitLoadPoint();
    setuid(getuid());
    setgid(getgid());

    toplevel = XtAppInitialize(&app_con, "XLoad", options, XtNumber(options),
			    (Cardinal *) &argc, argv, NULL, NULL, 0);
      
    XtGetApplicationResources( toplevel, (caddr_t) &resources, 
			      my_resources, XtNumber(my_resources),
			      NULL, (Cardinal) 0);
    if (argc != 1) usage();
    
    /*
     * This is a hack so that f.delete will do something useful in this
     * single-window application.
     */
    XtAppAddActions (app_con, xload_actions, XtNumber(xload_actions));
    XtOverrideTranslations(toplevel,
		    XtParseTranslationTable ("<Message>WM_PROTOCOLS: quit()"));

    XtSetArg (args[0], XtNiconPixmap, &icon_pixmap);
    XtGetValues(toplevel, args, ONE);
    if (icon_pixmap == None) {
	XtSetArg(args[0], XtNiconPixmap, 
		 XCreateBitmapFromData(XtDisplay(toplevel),
				       XtScreen(toplevel)->root,
				       xload_bits, xload_width, xload_height));
	XtSetValues (toplevel, args, ONE);
    }

    if (resources.show_label) {
      pane = XtCreateManagedWidget ("paned", panedWidgetClass,
				    toplevel, NULL, ZERO);

      label_wid = XtCreateManagedWidget ("label", labelWidgetClass,
					 pane, NULL, ZERO);
      
      XtSetArg (args[0], XtNlabel, &label);
      XtGetValues(label_wid, args, ONE);
      
      if ( strcmp("label", label) == 0 ) {
	(void) XmuGetHostname (host, 255);
	XtSetArg (args[0], XtNlabel, host);
	XtSetValues (label_wid, args, ONE);
      }

      load_parent = pane;
    }
    else
      load_parent = toplevel;

    load = XtCreateManagedWidget ("load", stripChartWidgetClass,
				  load_parent, NULL, ZERO);    

    XtAddCallback(load, XtNgetValue, GetLoadPoint, NULL);

    XtRealizeWidget (toplevel);
    wm_delete_window = XInternAtom (XtDisplay(toplevel), "WM_DELETE_WINDOW",
                                    False);
    (void) XSetWMProtocols (XtDisplay(toplevel), XtWindow(toplevel),
                            &wm_delete_window, 1);
    XtAppMainLoop(app_con);
}


static void quit (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    if (event->type == ClientMessage &&
        event->xclient.data.l[0] != wm_delete_window) {
        XBell (XtDisplay(w), 0);
        return;
    }
    XCloseDisplay (XtDisplay(w));
    exit (0);
}
