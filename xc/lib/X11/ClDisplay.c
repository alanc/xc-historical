#include "copyright.h"

/* $Header: XClDisplay.c,v 11.12 87/08/26 23:41:24 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1985	*/

#include "Xlibint.h"

/* 
 * XCloseDisplay - XSync the connection to the X Server, close the connection,
 * and free all associated storage.
 */

XCloseDisplay (dpy)
	register Display *dpy;
{
	register _XExtension *ext;
	register int i;
	for (i = 0; i < dpy->nscreens; i++) {
		register Screen *sp = &dpy->screens[i];
		XFreeGC (dpy, sp->default_gc);
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
	_XFreeDisplayStructure (dpy);
                                    /* XXX Should there be an UnlockDisplay? */
}
