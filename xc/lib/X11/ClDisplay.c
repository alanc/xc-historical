/* $XConsortium: XClDisplay.c,v 11.21 89/07/18 11:06:05 jim Exp $ */
/*

Copyright 1985, 1990 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#include "Xlibint.h"

/* 
 * XCloseDisplay - XSync the connection to the X Server, close the connection,
 * and free all associated storage.  This is the only routine that can be
 * called from or after an IOError handler, so the lower levels need to be able
 * to deal with broken connections.  Extension close procs should only free
 * memory and must be careful about the types of requests they generate.
 */

XCloseDisplay (dpy)
	register Display *dpy;
{
	register _XExtension *ext;
	register int i;
	extern void _XFreeQ();

	dpy->flags |= XlibDisplayClosing;
	for (i = 0; i < dpy->nscreens; i++) {
		register Screen *sp = &dpy->screens[i];
		XFreeGC (dpy, sp->default_gc);
	}
	if (dpy->cursor_font != None) {
	    XUnloadFont (dpy, dpy->cursor_font);
	}
	XSync(dpy, 1);  /* throw away pending input events */
	ext = dpy->ext_procs;
	while (ext) {		/* call out to any extensions interested */
		if (ext->close_display != NULL) 
			(*ext->close_display)(dpy, &ext->codes);
		ext = ext->next;
	}    
        LockDisplay(dpy);
	_XDisconnectDisplay(dpy->fd);
	_XFreeQ ();
	return;
}
