#ifndef lint
static char rcsid[] = "$XConsortium: xeyes.c,v 1.3 88/09/06 17:55:29 jim Exp $";
#endif  lint

#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include "Eyes.h"
#include <stdio.h> 
#include "eyes.bit"
#include "eyes_mask.bit"

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

/* Exit with message describing command line format */

void usage()
{
    fprintf(stderr,
"usage: xeyes\n");
    fprintf (stderr, 
"       [-geometry [{width}][x{height}][{+-}{xoff}[{+-}{yoff}]]] [-display [{host}]:[{vs}]]\n");
    fprintf(stderr,
"       [-fg {color}] [-bg {color}] [-bd {color}] [-bw {pixels}]\n");
    fprintf(stderr,
"       [-outline {color}] [-center {color}] [-backing {backing-store}]\n");
    exit(1);
}

static XrmOptionDescRec options[] = {
{"-outline",	"*eyes.outline",	XrmoptionSepArg,	NULL},
{"-center",	"*eyes.center",		XrmoptionSepArg,	NULL},
{"-backing",	"*eyes.backingStore",	XrmoptionSepArg,	NULL},
{"-widelines",	"*eyes.useWideLines",	XrmoptionNoArg,		"TRUE"},
{"-shape",	"*eyes.shapeWindow",	XrmoptionNoArg,		"TRUE"},
};

void main(argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel;
    Widget eyes;
    Arg arg[2];
    char *labelname = NULL;
    int	i;
    
    toplevel = XtInitialize("main", "XEyes", options, XtNumber (options),
				    &argc, argv);
      
    if (argc != 1) usage();
    
    i = 0;
    XtSetArg (arg[i], XtNiconPixmap, 
	      XCreateBitmapFromData (XtDisplay(toplevel),
				     XtScreen(toplevel)->root,
				     eyes_bits, eyes_width, eyes_height));
    i++;
    XtSetArg (arg[i], XtNiconMask, 
	      XCreateBitmapFromData (XtDisplay(toplevel),
				     XtScreen(toplevel)->root,
				     eyes_mask_bits, eyes_mask_width, eyes_mask_height));
    i++;
    XtSetValues (toplevel, arg, i);

    XtSetArg (arg[0], XtNlabel, &labelname);
    eyes = XtCreateManagedWidget ("eyes", eyesWidgetClass, toplevel, NULL, 0);
    XtGetValues(eyes, arg, 1);
    XtRealizeWidget (toplevel);
    XtMainLoop();
}
