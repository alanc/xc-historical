#include "copyright.h"

/* $Header: XModMap.c,v 11.1 87/05/30 14:35:56 toddb Locked $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

XModifierKeymap *
XGetModifierMapping(dpy)
     register Display *dpy;
{       
    xGetModifierMappingReply rep;
    register xReq *req;
    unsigned int nbytes;
    XModifierKeymap *res;

    LockDisplay(dpy);
    GetEmptyReq(GetModifierMapping, req);
    (void) _XReply (dpy, (xReply *)&rep, 0, xFalse);

    nbytes = (long)rep.length << 2;
    res = (XModifierKeymap *) Xmalloc(sizeof (XModifierKeymap));
    res->modifiermap = (KeyCode *) Xmalloc (nbytes);
    _XReadPad(dpy, res->modifiermap, nbytes);
    res->max_keypermod = rep.numKeyPerModifier;

    UnlockDisplay(dpy);
    SyncHandle();
    return (res);
}

/*
 *	Returns:
 *	0	Success
 *	1	Busy - one or more old or new modifiers are down
 *	2	Failed - one or more new modifiers unacceptable
 */
int
XSetModifierMapping(dpy, modifier_map)
    register Display *dpy;
    register XModifierKeymap *modifier_map;
{
    register xSetModifierMappingReq *req;
    xSetModifierMappingReply rep;
    int         mapSize = modifier_map->max_keypermod << 3;	/* 8 modifiers */

    LockDisplay(dpy);
    GetReqExtra(SetModifierMapping, mapSize, req);

    req->numKeyPerModifier = modifier_map->max_keypermod;

    bcopy(modifier_map->modifiermap, (char *)&req[1], mapSize);

    (void) _XReply(dpy, (xReply *) & rep,
	(sizeof(xSetModifierMappingReply) - sizeof(xReply)) >> 2, xTrue);
    UnlockDisplay(dpy);
    SyncHandle();
    return (rep.success);
}

XModifierKeymap *
XNewModifiermap(keyspermodifier)
    int keyspermodifier;
{
    XModifierKeymap *res = (XModifierKeymap *) Xmalloc((sizeof (XModifierKeymap)));

    res->max_keypermod = keyspermodifier;
    res->modifiermap = (keyspermodifier > 0 ?
			(KeyCode *) Xmalloc(8 * keyspermodifier)
			: (KeyCode *) NULL);
    return (res);
}

void
XFreeModifiermap(map)
    XModifierKeymap *map;
{
    if (map) {
	if (map->modifiermap)
	    Xfree((char *) map->modifiermap);
	Xfree((char *) map);
    }
}
