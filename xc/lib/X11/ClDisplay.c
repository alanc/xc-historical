#include "copyright.h"

/* $Header: XClDisplay.c,v 11.10 87/05/24 21:33:22 jg Exp $ */
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
	XSync(dpy, 1);  /* throw away pending input events */
        LockDisplay(dpy);
	ext = dpy->ext_procs;
	while (ext) {		/* call out to any extensions interested */
		if (ext->close_display != NULL) 
			(*ext->close_display)(dpy, &ext->codes);
		ext = ext->next;
	}    
	_XDisconnectDisplay(dpy->fd);
	_XFreeDisplayStructure (dpy);
                                    /* XXX Should there be an UnlockDisplay? */
}
