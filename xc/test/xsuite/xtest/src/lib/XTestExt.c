/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: XTestExt.c,v 1.6 92/06/11 15:48:34 rws Exp $
 */
#ifdef GENERATE_PIXMAPS
/* in this case we never want to do anything like real buffer stuffing or
 * I/O on our "display" as it isn't real enough for that.
 */
#undef XTESTEXTENSION
#endif /* GENERATE_PIXMAPS */


#define NULL	0
#include	"Xlib.h"
#include	<extensions/XInput.h>

Status
SimulateKeyPressEvent(dpy, keycode)
Display	*dpy;
KeyCode	keycode;
{
#ifdef XTESTEXTENSION
	XTestFakeKeyEvent(dpy, keycode, 1, 0);
	return(True);
#else       
	return(False);
#endif /* XTESTEXTENSION */
}


Status
SimulateDeviceKeyPressEvent(dpy, dev, keycode)
Display	*dpy;
XDevice *dev;
KeyCode	keycode;
{
#ifdef XTESTEXTENSION
	XTestFakeDeviceKeyEvent(dpy, dev, keycode, 1, NULL, 0, 0);
	return(True);
#else       
	return(False);
#endif /* XTESTEXTENSION */
}

Status
SimulateKeyReleaseEvent(dpy, keycode)
Display	*dpy;
KeyCode	keycode;
{
#ifdef XTESTEXTENSION
	XTestFakeKeyEvent(dpy, keycode, 0, 0);
	return(True);
#else
	return(False);
#endif /* XTESTEXTENSION */
}

Status
SimulateDeviceKeyReleaseEvent(dpy, dev, keycode)
Display	*dpy;
XDevice *dev;
KeyCode	keycode;
{
#ifdef XTESTEXTENSION
	XTestFakeDeviceKeyEvent(dpy, dev, keycode, 0, NULL, 0, 0);
	return(True);
#else
	return(False);
#endif /* XTESTEXTENSION */
}

Status
SimulateButtonPressEvent(dpy, button)
Display		*dpy;
unsigned int	button;
{
#ifdef XTESTEXTENSION
	XTestFakeButtonEvent(dpy, button, 1, 0);
	return(True);
#else
	return(False);
#endif /* XTESTEXTENSION */
}

Status
SimulateDeviceButtonPressEvent(dpy, dev, button)
Display		*dpy;
XDevice *dev;
unsigned int	button;
{
#ifdef XTESTEXTENSION
	XTestFakeDeviceButtonEvent(dpy, dev, button, 1, NULL, 0, 0);
	return(True);
#else
	return(False);
#endif /* XTESTEXTENSION */
}

Status
SimulateButtonReleaseEvent(dpy, button)
Display		*dpy;
unsigned int	button;
{
#ifdef XTESTEXTENSION
	XTestFakeButtonEvent(dpy, button, 0, 0);
	return(True);
#else
	return(False);
#endif /* XTESTEXTENSION */
}

Status
SimulateDeviceButtonReleaseEvent(dpy, dev, button)
Display		*dpy;
XDevice *dev;
unsigned int	button;
{
#ifdef XTESTEXTENSION
	XTestFakeDeviceButtonEvent(dpy, dev, button, 0, NULL, 0, 0);
	return(True);
#else
	return(False);
#endif /* XTESTEXTENSION */
}

Status
CompareCursorWithWindow(dpy, window, cursor)
Display	*dpy;
Window	window;
Cursor	cursor;
{
#ifdef XTESTEXTENSION
	return XTestCompareCursorWithWindow(dpy, window, cursor);
#else
	return(False);
#endif /* XTESTEXTENSION */
}

Status
CompareCurrentWithWindow(dpy, window)
Display	*dpy;
Window	window;
{
#ifdef XTESTEXTENSION
	return XTestCompareCurrentCursorWithWindow(dpy, window);
#else
	return(False);
#endif /* XTESTEXTENSION */
}

Status
SimulateMotionEvent(dpy, screen, x, y)
Display	*dpy;
int	screen;
int	x;
int	y;
{
#ifdef XTESTEXTENSION
	XTestFakeMotionEvent(dpy, screen, x, y, 0);
	return(True);
#else
	return(False);
#endif /* XTESTEXTENSION */
}

Status
SimulateDeviceMotionEvent(dpy, dev, is_relative, n_axes, axes, first)
Display	*dpy;
XDevice *dev;
Bool	is_relative;
int	n_axes;
int	*axes;
int	first;
{
#ifdef XTESTEXTENSION
	XTestFakeDeviceMotionEvent(dpy, dev, is_relative, first, axes, n_axes, 0);
	return(True);
#else
	return(False);
#endif /* XTESTEXTENSION */
}

#ifndef XTESTEXTENSION

 /* No server extension, but we can still test some client-side functionality, can't we. */

/*
 * Discard current requests in buffer. Returns True if somthing was
 * discarded, False otherwise. 
 * The XTEST extension library includes this function.
 */

static xReq _dummy_request = {
	0, 0, 0
};

Status
XTestDiscard(dpy)
    Display *dpy;
{
    Bool something;
    register char *ptr;

    LockDisplay(dpy);
    if (something = (dpy->bufptr != dpy->buffer)) {
	for (ptr = dpy->buffer;
	     ptr < dpy->bufptr;
	     ptr += (((xReq *)ptr)->length << 2))
	    dpy->request--;
	dpy->bufptr = dpy->buffer;
	dpy->last_req = (char *)&_dummy_request;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return something;
}


/*
 * Change the GContext held within a GC structure. Now it messes with an
 * opaque data structure. The XTEST extension library includes this function.
 */
void
XTestSetGContextOfGC(gc, gid)
	GC gc;
	GContext gid;
{
	gc->gid = gid;
}

/*
 * Change the Visual ID held within a Visual structure. Now it messes with an
 * opaque data structure. The XTEST extension library includes this function.
 */
void
XTestSetVisualIDOfVisual(v, vid)
	Visual *v;
	VisualID vid;
{
	v->visualid = vid;
}

#endif /* XTESTEXTENSION */
