#include "copyright.h"

/* $XConsortium: XListExt.c,v 11.8 89/11/08 17:07:22 converse Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

char **XListExtensions(dpy, nextensions)
register Display *dpy;
int *nextensions;	/* RETURN */
{
	xListExtensionsReply rep;
	char **list;
	char *ch;
	register unsigned i;
	register int length;
	register xReq *req;
	register long rlen;

	LockDisplay(dpy);
	GetEmptyReq (ListExtensions, req);

	if (! _XReply (dpy, (xReply *) &rep, 0, xFalse)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (char **) NULL;
	}

	if (rep.nExtensions) {
	    list = (char **) Xmalloc (
                (unsigned)(rep.nExtensions * sizeof (char *)));
	    rlen = rep.length << 2;
	    ch = (char *) Xmalloc ((unsigned) rlen + 1);
                /* +1 to leave room for last null-terminator */

	    if ((!list) || (!ch)) {
		if (list) Xfree((char *) list);
		if (ch)   Xfree((char *) ch);
		_XEatData(dpy, (unsigned long) rlen);
		UnlockDisplay(dpy);
		SyncHandle();
		return (char **) NULL;
	    }

	    _XReadPad (dpy, ch, rlen);
	    /*
	     * unpack into null terminated strings.
	     */
	    length = *ch;
	    for (i = 0; i < rep.nExtensions; i++) {
		list[i] = ch+1;  /* skip over length */
		ch += length + 1; /* find next length ... */
		length = *ch;
		*ch = '\0'; /* and replace with null-termination */
	    }
	}
	else list = (char **) NULL;

	*nextensions = rep.nExtensions;
	UnlockDisplay(dpy);
	SyncHandle();
	return (list);
}

XFreeExtensionList (list)
char **list;
{
	if (list != NULL) {
	    Xfree (list[0]-1);
	    Xfree ((char *)list);
	}
}
