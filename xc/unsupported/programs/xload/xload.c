#ifndef lint
static char rcsid[] = "$XConsortium: xload.c,v 1.21 89/07/21 13:42:10 jim Exp $";
#endif /* lint */

#include <stdio.h> 
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/StripChart.h>

#include "xload.bit"

char *ProgramName;

extern void exit(), GetLoadPoint();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-scale",	"*load.minScale",	XrmoptionSepArg,	 NULL},
{"-update",	"*load.update",		XrmoptionSepArg,	 NULL},
{"-hl",		"*load.highlight",	XrmoptionSepArg,	 NULL},
{"-highlight",	"*load.highlight",	XrmoptionSepArg,	 NULL},
{"-label",	"*load.label",		XrmoptionSepArg,	 NULL},
{"-jumpscroll",	"*load.jumpScroll",	XrmoptionSepArg,	 NULL},
};


/* Exit with message describing command line format */

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
       "    -jumpscroll value       number of pixels to scroll on overflow\n");
    fprintf (stderr, "\n");
    exit(1);
}

#ifndef lint
/* this silliness causes the linker to include the VendorShell
 * module from Xaw, rather than the one from Xt.
 */
static Junk()
{
#include <X11/Vendor.h>
WidgetClass junk = vendorShellWidgetClass;
}
#endif

void main(argc, argv)
    int argc;
    char **argv;
{
    char host[256], * label;
    Widget toplevel;
    Widget load;
    Arg args[1];
    Pixmap icon_pixmap = None;
    
    ProgramName = argv[0];
    toplevel = XtInitialize(NULL, "XLoad", 
			    options, XtNumber(options), &argc, argv);
      
    if (argc != 1) usage();
    
    XtSetArg (args[0], XtNiconPixmap, &icon_pixmap);
    XtGetValues(toplevel, args, ONE);
    if (icon_pixmap == None) {
	XtSetArg(args[0], XtNiconPixmap, 
		 XCreateBitmapFromData(XtDisplay(toplevel),
				       XtScreen(toplevel)->root,
				       xload_bits, xload_width, xload_height));
	XtSetValues (toplevel, args, ONE);
    }

    load = XtCreateManagedWidget ("load", stripChartWidgetClass,
				  toplevel, NULL, ZERO);

    XtSetArg (args[0], XtNlabel, &label);
    XtGetValues(load, args, ONE);

    if ( label == NULL ) {
        (void) gethostname (host, 255);
	XtSetArg (args[0], XtNlabel, host);
	XtSetValues (load, args, ONE);
    }
    
    XtAddCallback(load, XtNgetValue, GetLoadPoint, NULL);

    XtRealizeWidget (toplevel);
    XtMainLoop();
}
