/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: shapetest.c,v 1.5 88/09/06 18:00:12 jim Exp $ */
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/extensions/shape.h>

Display *dpy;

StartConnectionToServer(argc, argv)
int     argc;
char    *argv[];
{
    char *display;

    display = NULL;
    for(--argc, ++argv; argc; --argc, ++argv)
    {
	if ((*argv)[0] == '-') {
	    switch((*argv)[1]) {
	    case 'd':
		display = argv[1];
		++argv; --argc;
		break;
	    }
	}
    }
    if (!(dpy = XOpenDisplay(display)))
    {
       perror("Cannot open display\n");
       exit(0);
   }
}

XRectangle  rects[] = { 0,0, 100, 100, 10, 10, 100, 100 };

main(argc, argv)
    int argc;
    char **argv;

{
	Window  w;
	GC gc;
	XGCValues xgcv;
	char *windowName = "Test of Shape Extension";
	char line[30];
	XSetWindowAttributes xswa;
	unsigned long	mask;
	XEvent pe;
	Pixmap tile_pat, stip_pat;
	int     iteration = 0;
	XColor screen_def_blue, exact_def_blue;
	XColor screen_def_red, exact_def_red;

	/*_Xdebug = 1;*/   /* turn on synchronization */

	StartConnectionToServer(argc, argv);

	xswa.event_mask = ExposureMask;
	xswa.background_pixel = BlackPixel (dpy, DefaultScreen (dpy));
	mask = CWEventMask | CWBackPixel;
	w = XCreateWindow(dpy, RootWindow(dpy, DefaultScreen(dpy)),
		100, 100, 340, 340, 0, 
		CopyFromParent, CopyFromParent,	CopyFromParent,
		mask, &xswa);

	XChangeProperty(dpy,
	    w, XA_WM_NAME, XA_STRING, 8, PropModeReplace,
	    windowName, strlen(windowName));

	XShapeCombineRectangles (dpy, w, ShapeBounding, 0, 0, 
		          rects, sizeof (rects) / sizeof (rects[0]),
			  ShapeSet, Unsorted);

	XMapWindow(dpy, w);

	gc = XCreateGC(dpy, w, 0, 0);
	XAllocNamedColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)), "blue",
	       &screen_def_blue,  &exact_def_blue);
	XAllocNamedColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)), "red",
	       &screen_def_red,  &exact_def_red);
	XSetForeground(dpy, gc, screen_def_red.pixel);
	XSetBackground(dpy, gc, screen_def_blue.pixel);

	while (1) {
	    XNextEvent(dpy, &pe);
	}
}
