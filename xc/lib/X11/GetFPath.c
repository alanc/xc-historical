#include "copyright.h"

/* $XConsortium: XGetFPath.c,v 11.12 88/09/06 16:07:45 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

char **XGetFontPath(dpy, npaths)
register Display *dpy;
int *npaths;	/* RETURN */
{
	xGetFontPathReply rep;
	register long nbytes;
	char **flist;
	char *ch;
	register int i;
	register int length;
	register xReq *req;

	LockDisplay(dpy);
	GetEmptyReq (GetFontPath, req);
	(void) _XReply (dpy, (xReply *) &rep, 0, xFalse);

	if (rep.nPaths) {
	    flist = (char **)
		Xmalloc((unsigned) rep.nPaths * sizeof (char *));
	    nbytes = (long)rep.length << 2;
	    ch = (char *) Xmalloc ((unsigned) (nbytes + 1));
                /* +1 to leave room for last null-terminator */

	    if ((! flist) || (! ch)) {
		if (flist) Xfree((char *) flist);
		if (ch) Xfree(ch);
		_XEatData(dpy, (unsigned long) nbytes);
		UnlockDisplay(dpy);
		SyncHandle();
		return (char **) NULL;
	    }

	    _XReadPad (dpy, ch, nbytes);
	    /*
	     * unpack into null terminated strings.
	     */
	    length = *ch;
	    for (i = 0; i < rep.nPaths; i++) {
		flist[i] = ch+1;  /* skip over length */
		ch += length + 1; /* find next length ... */
		length = *ch;
		*ch = '\0'; /* and replace with null-termination */
	    }
	}
	else flist = NULL;
	*npaths = rep.nPaths;
	UnlockDisplay(dpy);
	SyncHandle();
	return (flist);
}

XFreeFontPath (list)
char **list;
{
	if (list != NULL) {
		Xfree (list[0]-1);
		Xfree ((char *)list);
	}
}
