/* $XConsortium: XLookupCol.c,v 11.10 90/12/12 09:18:37 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1985	*/

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

#define NEED_REPLIES
#include "Xlibint.h"

#if NeedFunctionPrototypes
Status XLookupColor (
	register Display *dpy,
        Colormap cmap,
	register _Xconst char *spec,
	XColor *def,
	XColor *scr)
#else
Status XLookupColor (dpy, cmap, spec, def, scr)
	register Display *dpy;
        Colormap cmap;
	register char *spec;
	XColor *def, *scr;
#endif
{
	register int n;
	xLookupColorReply reply;
	register xLookupColorReq *req;

	n = spec ? strlen (spec) : 0;
	LockDisplay(dpy);
	GetReq (LookupColor, req);
	req->cmap = cmap;
	req->nbytes = n;
	req->length += (n + 3) >> 2;
	Data (dpy, spec, (long)n);
	if (!_XReply (dpy, (xReply *) &reply, 0, xTrue)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (0);
	    }
	def->red   = reply.exactRed;
	def->green = reply.exactGreen;
	def->blue  = reply.exactBlue;

	scr->red   = reply.screenRed;
	scr->green = reply.screenGreen;
	scr->blue  = reply.screenBlue;

	UnlockDisplay(dpy);
	SyncHandle();
	return (1);
}
