#include "copyright.h"

/* $Header: XSetFPath.c,v 11.10 87/09/11 08:06:49 newman Locked $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

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
		      
