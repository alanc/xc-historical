/* $XConsortium: XUnmapWin.c,v 11.6 88/09/06 16:11:21 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

/*
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
XUnmapWindow (dpy, w)
        register Display *dpy;
	Window w;
{
        register xResourceReq *req;

        LockDisplay(dpy);
        GetResReq(UnmapWindow, w, req);
	UnlockDisplay(dpy);
	SyncHandle();
}

