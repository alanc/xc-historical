#ifndef lint
static char rcsid[] = "$XConsortium: Exp $";
#endif  lint

#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include "Clock.h"
#include <stdio.h> 

#include "clock.bit"
#include "clock_mask.bit"

extern void exit();

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

/* Exit with message describing command line format */

void usage()
{
    fprintf(stderr,
"usage: clock\n");
    fprintf (stderr, 
"       [-geometry [{width}][x{height}][{+-}{xoff}[{+-}{yoff}]]] [-display [{host}]:[{vs}]]\n");
    fprintf(stderr,
"       [-fg {color}] [-bg {color}] [-bd {color}] [-bw {pixels}]\n");
    fprintf(stderr,
"       [-face {color}] [-minute {color}] [-hour {color}] [-jewel {color}]\n");
    fprintf(stderr,
"       [-backing {backing-store}]\n");
    exit(1);
}

static XrmOptionDescRec options[] = {
{"-face",	"*clock.face",		XrmoptionSepArg,	NULL},
{"-minute",	"*clock.minute",	XrmoptionSepArg,	NULL},
{"-hour",	"*clock.hour",		XrmoptionSepArg,	NULL},
{"-jewel",	"*clock.jewel",		XrmoptionSepArg,	NULL},
{"-backing",	"*clock.backingStore",	XrmoptionSepArg,	NULL},
{"-shape",	"*clock.shapeWindow",	XrmoptionNoArg,		"TRUE"},
{"-noshape",	"*clock.shapeWindow",	XrmoptionNoArg,		"FALSE"},
};

void main(argc, argv)
    int argc;
    char **argv;
{
    char host[256];
    Widget toplevel;
    Widget clock;
    Arg arg[2];
    int	i;
    
    toplevel = XtInitialize("main", "Clock", options, XtNumber (options),
				    &argc, argv);
      
    if (argc != 1) usage();

    i = 0;
    XtSetArg (arg[i], XtNiconPixmap, 
	      XCreateBitmapFromData (XtDisplay(toplevel),
				     XtScreen(toplevel)->root,
				     clock_bits, clock_width, clock_height));
    i++;
    XtSetArg (arg[i], XtNiconMask,
	      XCreateBitmapFromData (XtDisplay(toplevel),
				     XtScreen(toplevel)->root,
				     clock_mask_bits, clock_mask_width, clock_mask_height));
    i++;
    XtSetValues (toplevel, arg, i);

    clock = XtCreateManagedWidget ("clock", clockWidgetClass, toplevel, NULL, 0);
    XtRealizeWidget (toplevel);
    XtMainLoop();
}
