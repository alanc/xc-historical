/* $XConsortium: XSetFPath.c,v 11.12 88/09/06 16:10:29 jim Exp $ */
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

#define safestrlen(s) ((s) ? strlen(s) : 0)

XSetFontPath (dpy, directories, ndirs)
register Display *dpy;
char **directories;
int ndirs;
{
	register int n = 0;
	register int i;
	register int nbytes;
	char *p;
	register xSetFontPathReq *req;

        LockDisplay(dpy);
	GetReq (SetFontPath, req);
	req->nFonts = ndirs;
	for (i = 0; i < ndirs; i++) {
		n += safestrlen (directories[i]) + 1;
	}
	nbytes = (n + 3) & ~3;
	BufAlloc (char *, p, nbytes);
	/*
	 * pack into counted strings.
	 */
	for (i = 0; i < ndirs; i++) {
		register int length = safestrlen (directories[i]);
		*p = length;
		bcopy (directories[i], p + 1, length);
		p += length + 1;
	}
	req->length += nbytes >> 2;
        UnlockDisplay(dpy);
	SyncHandle();
}
		      
