#include "copyright.h"

/* $XConsortium: XClDisplay.c,v 11.18 88/08/18 12:22:50 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1985	*/

#include "Xlibint.h"

extern Display *_XHeadOfDisplayList;
/* 
 * XCloseDisplay - XSync the connection to the X Server, close the connection,
 * and free all associated storage.
 */

XCloseDisplay (dpy)
	register Display *dpy;
{
	register _XExtension *ext;
	register int i;
	register Display **dp = &_XHeadOfDisplayList;
	register Display *cp = _XHeadOfDisplayList;
	extern void _XFreeQ();

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
	while (cp != NULL) {
		if (cp == dpy) {
			*dp = cp->next;
			_XFreeDisplayStructure (dpy);
			break;
			}
		dp = &(cp->next);
		cp = *dp;
		}
	if (_XHeadOfDisplayList == NULL) {
	    _XFreeQ ();
	}
	return;
}
