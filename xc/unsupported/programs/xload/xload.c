#ifndef lint
static char rcsid[] = "$XConsortium: xload.c,v 1.18 89/07/19 12:39:39 jim Exp $";
#endif  lint

#include <stdio.h> 
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xaw/Load.h>

#include "xload.bit"

char *ProgramName;

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-scale",	"*load.minScale",	XrmoptionSepArg,	 NULL},
{"-update",	"*load.update",		XrmoptionSepArg,	 NULL},
{"-hl",		"*load.highlight",	XrmoptionSepArg,	 NULL},
{"-highlight",	"*load.highlight",	XrmoptionSepArg,	 NULL},
{"-label",	"*load.label",		XrmoptionSepArg,	 NULL},
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
    char host[256];
    Widget toplevel;
    Widget load;
    Arg arg;
    char *labelname = NULL;
    Pixmap icon_pixmap = None;
    
    ProgramName = argv[0];
    toplevel = XtInitialize("main", "XLoad", options, XtNumber(options), &argc, argv);
      
    if (argc != 1) usage();
    
    XtSetArg (arg, XtNiconPixmap, &icon_pixmap);
    XtGetValues(toplevel, &arg, 1);
    if (icon_pixmap == None) {
	XtSetArg(arg, XtNiconPixmap, 
		 XCreateBitmapFromData(XtDisplay(toplevel),
				       XtScreen(toplevel)->root,
				       xload_bits, xload_width, xload_height));
	XtSetValues (toplevel, &arg, 1);
    }

    XtSetArg (arg, XtNlabel, &labelname);
    load = XtCreateManagedWidget ("load", loadWidgetClass, toplevel, NULL, 0);
    XtGetValues(load, &arg, 1);
    if (!labelname) {
       (void) gethostname (host, 255);
       XtSetArg (arg, XtNlabel, host);
       XtSetValues (load, &arg, 1);
    }
    XtRealizeWidget (toplevel);
    XtMainLoop();
}
