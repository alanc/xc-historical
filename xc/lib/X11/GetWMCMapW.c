#include "copyright.h"

/* $XConsortium: GetWMCMapW.c,v 1.5 89/03/28 16:23:23 jim Exp $ */

/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#include <stdio.h>
#include "Xlibint.h"
#include <X11/Xatom.h>

Status XGetWMColormapWindows (dpy, w, colormapWindows, countReturn)
    Display *dpy;
    Window w;
    Window **colormapWindows;
    int *countReturn;
{
    Atom *data = NULL;
    Atom actual_type;
    int actual_format;
    unsigned long leftover, nitems;

    if (dpy->atoms.wm_colormap_windows == None) {
	Atom a = XInternAtom (dpy, "WM_COLORMAP_WINDOWS", False);

	if (a == None) return False;
	LockDisplay (dpy);
	dpy->atoms.wm_colormap_windows = a;
	UnlockDisplay (dpy);
    }

    /* get the property */
    if (XGetWindowProperty (dpy, w, dpy->atoms.wm_colormap_windows, 
    			    0L, 1000000L, False,
			    XA_WINDOW, &actual_type, &actual_format,
			    &nitems, &leftover, (unsigned char **) &data)
	!= Success)
      return False;

    if (actual_type != XA_WINDOW || actual_format != 32) {
	if (data) Xfree ((char *) data);
	return False;
    }

    *colormapWindows = (Window *) data;
    *countReturn = (int) nitems;
    return True;
}
