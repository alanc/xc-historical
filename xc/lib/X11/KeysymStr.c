/* $XConsortium: XKeysymStr.c,v 11.1 90/12/02 17:54:47 rws Exp $ */
/* Copyright 1990 Massachusetts Institute of Technology */

#include "Xlibint.h"
#include <X11/Xresource.h>
#include <X11/keysymdef.h>

#if __STDC__
#define Const const
#else
#define Const /**/
#endif

typedef unsigned long Signature;

#define NEEDVTABLE
#include "ks_tables.h"

extern XrmDatabase _XInitKeysymDB();
extern Const unsigned char __XkeyTable[];


typedef struct _GRNData {
    char *name;
    XrmRepresentation type;
    XrmValuePtr value;
} GRNData;

/*ARGSUSED*/
static Bool SameValue(db, bindings, quarks, type, value, data)
    XrmDatabase		*db;
    XrmBindingList      bindings;
    XrmQuarkList	quarks;
    XrmRepresentation   *type;
    XrmValuePtr		value;
    GRNData		*data;
{
    if ((*type == data->type) && (value->size == data->value->size) &&
	!strncmp((char *)value->addr, (char *)data->value->addr, value->size))
    {
	data->name = XrmQuarkToString(*quarks); /* XXX */
	return True;
    }
    return False;
}

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
	XrmQuark empty = NULLQUARK;
	GRNData data;

	sprintf(buf, "%lX", ks);
	resval.addr = (caddr_t)buf;
	resval.size = strlen(buf) + 1;
	data.name = (char *)NULL;
	data.type = XrmPermStringToQuark("String");
	data.value = &resval;
	(void)XrmEnumerateDatabase(keysymdb, &empty, &empty, XrmEnumAllLevels,
				   SameValue, (caddr_t)&data);
	return data.name;
    }
    return ((char *) NULL);
}
