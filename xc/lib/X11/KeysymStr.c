/* $XConsortium: XKeysymStr.c,v 11.5 90/06/15 11:24:25 rws Exp $ */
/* Copyright 1990 Massachusetts Institute of Technology */

#include "Xlibint.h"
#include <X11/Xresource.h>
#include <X11/keysymdef.h>

#ifdef __STDC__
#define Const const
#else
#define Const /**/
#endif

typedef unsigned long Signature;

#define NEEDVTABLE
#include "ks_tables.h"

extern char *_XrmGetResourceName();
extern XrmDatabase _XInitKeysymDB();
extern Const unsigned char __XkeyTable[];

char *XKeysymToString(ks)
    KeySym ks;
{
    register int i, n;
    int h;
    register int idx;
    Const unsigned char *entry;
    unsigned char val1, val2;
    XrmDatabase keysymdb;

    if (!ks)
	return ((char *)NULL);
    if (ks == XK_VoidSymbol)
	ks = 0;
    if (ks <= 0xffff)
    {
	val1 = ks >> 8;
	val2 = ks & 0xff;
	i = ks % VTABLESIZE;
	h = i + 1;
	n = VMAXHASH;
	while (idx = hashKeysym[i])
	{
	    entry = &__XkeyTable[idx];
	    if ((entry[0] == val1) && (entry[1] == val2))
		return ((char *)entry + 2);
	    if (!--n)
		break;
	    i += h;
	    if (i >= VTABLESIZE)
		i -= VTABLESIZE;
	}
    }

    if (keysymdb = _XInitKeysymDB())
    {
	char buf[8];
	XrmValue resval;

	sprintf(buf, "%lX", ks);
	resval.addr = (caddr_t)buf;
	resval.size = strlen(buf) + 1;
	return _XrmGetResourceName(keysymdb, "String", &resval);
    }
    return ((char *) NULL);
}
