/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*
 * $Source: /u1/X11/clients/xrefresh/RCS/xrefresh.c,v $
 * $Header: xrefresh.c,v 1.4 88/01/22 17:20:56 jim Locked $
 *
 * Kitchen sink version, useful for clearing small areas and flashing the 
 * screen.
 */

#include <stdio.h>
#include <errno.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/types.h>

char *malloc();

Window XCreateWindow();

Window win;

char *ProgramName;

void Syntax ()
{
    fprintf (stderr, "usage:  %s [-options] [geometry] [display]\n\n", 
    	     ProgramName);
    fprintf (stderr, "where the available options are:\n");
    fprintf (stderr, "    -black                  use BlackPixel\n");
    fprintf (stderr, "    -white                  use WhitePixel\n");
    fprintf (stderr, "    -solid colorname        use the color indicated\n");
    fprintf (stderr, "    -background             use the root background\n");
    fprintf (stderr, 
"    -none                   no background (smooth refresh)\n");
    fprintf (stderr, "where geometry may be specified as:\n");
    fprintf (stderr, "    =WxH+X+Y                old style geometry spec\n");
    fprintf (stderr, "    -geometry WxH+X+Y       or -g spec\n");
    fprintf (stderr, "and display may be specified as:\n");
    fprintf (stderr, "    host:dpy                obselete, use -display\n");
    fprintf (stderr, "    -display host:dpy       or -d\n");
    fprintf (stderr, "\nThe default is:  %s -none\n", ProgramName);
    exit (1);
}

static char *copystring (s)
    register char *s;
{
    int len = (s ? strlen (s) : 0) + 1;
    char *malloc();
    char *retval;

    retval = malloc (len);
    if (!retval) {
	fprintf (stderr, "%s:  unable to allocate %d bytes for string.\n",
		 ProgramName, len);
	exit (1);
    }
    (void) strcpy (retval, s);
    return (retval);
}


enum e_action {doDefault, doBlack, doWhite, doSolid, doNone, doBackground};

struct s_pair {
	char *resource_name;
	enum e_action action;
} pair_table[] = {
	{ "Black", doBlack },
	{ "White", doWhite },
	{ "None", doNone },
	{ "Background", doBackground },
	{ NULL, doDefault }};

main(argc, argv)
int	argc;
char	*argv[];
{
    Visual visual;
    XSetWindowAttributes xswa;
    int i;
    char *displayname = NULL;
    Display *dpy;
    Colormap cmap;
    enum e_action action = doDefault;
    unsigned long mask;
    int screen;
    int x, y, width, height;
    char *geom = NULL;
    int geom_result;
    int display_width, display_height;
    char *solidcolor = NULL;
    XColor cdef;

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    if (strcmp (arg, "-d") == 0 || strcmp (arg, "-display") == 0) {
		if (++i >= argc) Syntax ();
		displayname = argv[i];
		continue;
	    } else if (strcmp (arg, "-g") == 0 || strcmp (arg, "-geom") == 0 ||
		strcmp (arg, "-geometry") == 0) {
		if (++i >= argc) Syntax ();
		geom = argv[i];
		continue;
	    } else if (strcmp (arg, "-black") == 0) {
		action = doBlack;
		continue;
	    } else if (strcmp (arg, "-white") == 0) {
		action = doWhite;
		continue;
	    } else if (strcmp (arg, "-solid") == 0) {
		if (++i >= argc) Syntax ();
		solidcolor = argv[i];
		action = doSolid;
		continue;
	    } else if (strcmp (arg, "-none") == 0) {
		action = doNone;
		continue;
	    } else if (strcmp (arg, "-background") == 0) {
		action = doBackground;
		continue;
	    } else 
		Syntax ();
	} else if (arg[0] == '=')
	    geom = arg;
	else if (index (arg, ':') != NULL)
	    displayname = arg;
	else 
	    Syntax ();
    }

    if ((dpy = XOpenDisplay(displayname)) == NULL) {
	fprintf (stderr, "%s:  unable to open display '%s'\n",
		 ProgramName, XDisplayName (displayname));
	exit (1);
    }

    if (action == doDefault) {
	char *def;

	if ((def = XGetDefault (dpy, ProgramName, "Solid")) != NULL) {
	    solidcolor = copystring (def);
	    action = doSolid;
	} else {
	    struct s_pair *pp;

	    for (pp = pair_table; pp->resource_name != NULL; pp++) {
		def = XGetDefault (dpy, ProgramName, pp->resource_name);
		if (def != NULL && (def[0] == 'y' || def[0] == 'Y' ||
				    strcmp (def, "on") == 0 ||
				    strcmp (def, "On") == 0 ||
				    strcmp (def, "ON") == 0))
		    action = pp->action;
	    }
	}
    }

    if (geom == NULL) geom = XGetDefault (dpy, ProgramName, "Geometry");

    screen = DefaultScreen (dpy);
    display_width = DisplayWidth (dpy, screen);
    display_height = DisplayHeight (dpy, screen);
    x = y = 0; 
    width = display_width;
    height = display_height;

    if (DisplayCells (dpy, screen) <= 2 && action == doSolid) {
	if (strcmp (solidcolor, "black") == 0)
	    action = doBlack;
	else if (strcmp (solidcolor, "white") == 0) 
	    action = doWhite;
	else {
	    fprintf (stderr, 
	    	     "%s:  can't use colors on a monochrome display.\n",
		     ProgramName);
	    action = doNone;
	}
    }

    if (geom) 
        geom_result = XParseGeometry (geom, &x, &y, &width, &height);
    else
	geom_result = NoValue;

    /*
     * For parsing geometry, we want to have the following
     *     
     *     =                (0,0) for (display_width,display_height)
     *     =WxH+X+Y         (X,Y) for (W,H)
     *     =WxH-X-Y         (display_width-W-X,display_height-H-Y) for (W,H)
     *     =+X+Y            (X,Y) for (display_width-X,display_height-Y)
     *     =WxH             (0,0) for (W,H)
     *     =-X-Y            (0,0) for (display_width-X,display_height-Y)
     *
     * If we let any missing values be taken from (0,0) for 
     * (display_width,display_height) we just have to deal with the
     * negative offsets.
     */

    if (geom_result & XNegative) {
	if (geom_result & WidthValue) {
	    x = display_width - width + x;
	} else {
	    width = display_width + x;
	    x = 0;
	}
    } 
    if (geom_result & YNegative) {
	if (geom_result & HeightValue) {
	    y = display_height - height + y;
	} else {
	    height = display_height + y;
	    y = 0;
	}
    }

    mask = 0;
    switch (action) {
	case doBlack:
	    xswa.background_pixel = BlackPixel (dpy, screen);
	    mask |= CWBackPixel;
	    break;
	case doWhite:
	    xswa.background_pixel = WhitePixel (dpy, screen);
	    mask |= CWBackPixel;
	    break;
	case doSolid:
	    cmap = DefaultColormap (dpy, screen);
	    if (XParseColor (dpy, cmap, solidcolor, &cdef) &&
		XAllocColor (dpy, cmap, &cdef)) {
		xswa.background_pixel = cdef.pixel;
		mask |= CWBackPixel;
	    } else {
		fprintf (stderr,"%s:  unable to allocate color '%s'.\n",
			 ProgramName, solidcolor);
		action = doNone;
	    }
	    break;
	case doDefault:
	case doNone:
	    xswa.background_pixmap = None;
	    mask |= CWBackPixmap;
	    break;
	case doBackground:
	    xswa.background_pixmap = ParentRelative;
	    mask |= CWBackPixmap;
	    break;
    }
    xswa.override_redirect = True;
    mask |= CWOverrideRedirect;
    visual.visualid = CopyFromParent;
    win = XCreateWindow(dpy, DefaultRootWindow(dpy), x, y, width, height,
	    0, DefaultDepth(dpy, screen), InputOutput, &visual, mask, &xswa);

    XMapWindow (dpy, win);
    /* the following will free the color that we might have allocateded */
    XCloseDisplay (dpy);
    exit (0);
}

