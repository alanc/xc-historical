/*
 * $XConsortium$
 *
 * Copyright 1992 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
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
 * Author:  Keith Packard, MIT X Consortium
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>

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

fatal(s)
{
    printf ("fatal: %s\n", s);
    exit (1);
}

int ignoreError (dpy, error)
    Display *dpy;
    XErrorEvent	*error;
{
    printf ("ignoring error\n");
}

char *stateNames[] = { "Off", "On", "Cycle" };
char *typeNames[] = { "Blanked", "Internal", "External" };

main(argc, argv)
    int argc;
    char **argv;

{
    XEvent pe;
    XScreenSaverEvent *se;
    Window root, saver;
    int	screen;
    int majorVersion, minorVersion;
    int error_base, event_base;
    XScreenSaverInfo	*info;
    XSetWindowAttributes    attr;
    unsigned long	mask;
    GC			gc;

    /*_Xdebug = 1;*/   /* turn on synchronization */

    StartConnectionToServer(argc, argv);

    if (!XScreenSaverQueryExtension (dpy, &event_base, &error_base))
	fatal ("QueryExtension failed");

    printf ("event_base %d, error_base %d\n", event_base, error_base);

    if (!XScreenSaverQueryVersion (dpy, &majorVersion, &minorVersion))
	fatal ("QueryVersion failed");

    printf ("majorVersion: %d, minorVersion: %d\n", majorVersion, minorVersion);
    
    root = DefaultRootWindow (dpy);

    info = XScreenSaverAllocateInfo (dpy);
    if (!XScreenSaverQueryInfo (dpy, root, info))
	fatal ("QueryInfo");
    printf ("window: 0x%x\n", info->window);
    printf ("state: %s\n", stateNames[info->state]);
    printf ("type: %s\n", typeNames[info->type]);
    printf ("tilOrSince: %d\n", info->tilOrSince);
    printf ("tilCycle: %d\n", info->tilCycle);
    printf ("eventMask: 0x%x\n", info->eventMask);
    saver = info->window;
    XFree (info);
    XScreenSaverSelectInput (dpy, root, ScreenSaverNotifyMask|ScreenSaverCycleMask);
    screen = DefaultScreen(dpy);
    attr.background_pixel = BlackPixel (dpy, screen);
    mask = CWBackPixel;
    XScreenSaverSetAttributes (dpy, root, 0, 0, 
	DisplayWidth(dpy, screen), DisplayHeight (dpy, screen), 0,
	CopyFromParent, CopyFromParent, CopyFromParent, mask, &attr);
    gc = DefaultGC (dpy, screen);
    XSetForeground (dpy, gc, WhitePixel(dpy, screen));
    while (1) {
	XNextEvent(dpy, &pe);
	if (pe.type == event_base)
	{
	    se = (XScreenSaverEvent *) &pe;
	    printf ("ScreenSaverEvent\n");
	    printf ("serial: %d\n", se->serial);
	    printf ("send_event: %d\n", se->send_event);
	    printf ("window: %x\n", se->window);
	    printf ("root: %x\n", se->root);
	    printf ("state: %s\n", stateNames[se->state]);
	    printf ("type: %s\n", typeNames[se->saverType]);
	    printf ("forced: %d\n", se->forced);
	    printf ("time: %d\n", se->time);
	    if (se->state == ScreenSaverOn && 
		se->saverType == ScreenSaverExternal)
	    {
/*		XSetErrorHandler (ignoreError); */
		XDrawString (dpy, saver, gc, 100, 100, "Screen Saver Test", 17);
		XSync (dpy, False);
/*		XSetErrorHandler (NULL); */
	    }
	}
	else
	{
	    printf ("unexpected event %d\n", pe.type);
	}
    }
}
