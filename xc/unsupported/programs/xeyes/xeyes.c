#ifndef lint
static char rcsid[] = "$Header: xeyes.c,v 1.11 88/07/07 17:33:51 jim Exp $";
#endif  lint

#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/Atoms.h>
#include <X11/Shell.h>
#include "Eyes.h"
#include <stdio.h> 
#ifdef notdef
#include "xeyes.bit"
#endif

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

/* Exit with message describing command line format */

void usage()
{
    fprintf(stderr,
"usage: xeyes [-geometry [{width}][x{height}][{+-}{xoff}[{+-}{yoff}]]] [-display [{host}]:[{vs}]]\n"
);
    fprintf(stderr,
"             [-fg {color}] [-bg {color}] [-bd {color}] [-bw {pixels}]\n");
    exit(1);
}

static XrmOptionDescRec options[] = {
{"-outline",	"*eyes.outline",	XrmoptionSepArg,	 NULL},
{"-center",	"*eyes.centerColor",	XrmoptionSepArg,	 NULL},
};

void main(argc, argv)
    int argc;
    char **argv;
{
    char host[256];
    Widget toplevel;
    Widget eyes;
    Arg arg;
    char *labelname = NULL;
    
    toplevel = XtInitialize("main", "XEyes", options, XtNumber (options),
				    &argc, argv);
      
    if (argc != 1) usage();
    
#ifdef notdef
    XtSetArg (arg, XtNiconPixmap, 
	      XCreateBitmapFromData (XtDisplay(toplevel),
				     XtScreen(toplevel)->root,
				     xeyes_bits, xeyes_width, xeyes_height));
#endif
    XtSetValues (toplevel, &arg, 1);

    XtSetArg (arg, XtNlabel, &labelname);
    eyes = XtCreateManagedWidget ("eyes", eyesWidgetClass, toplevel, NULL, 0);
    XtGetValues(eyes, &arg, 1);
    XtRealizeWidget (toplevel);
    XtMainLoop();
}
