#include "copyright.h"

/* $Header: XMisc.c,v 1.0 88/02/01 13:15:12 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1988	*/

#include "Xlibint.h"

long XMaxRequestSize(dpy)
    Display *dpy;
{
    return dpy->max_request_size;
}

char *XResourceManagerString(dpy)
    Display *dpy;
{
    return dpy->xdefaults;
}

/* XXX change Display.motion_buffer to unsigned long */

unsigned long XDisplayMotionBufferSize(dpy)
    Display *dpy;
{
    return dpy->motion_buffer;
}

XDisplayKeycodes(dpy, min_keycode_return, max_keycode_return)
    Display *dpy;
    int *min_keycode_return, *max_keycode_return;
{
    *min_keycode_return = dpy->min_keycode;
    *max_keycode_return = dpy->max_keycode;
}

VisualID XVisualIDFromVisual(visual)
    Visual *visual;
{
    return visual->visualid;
}
