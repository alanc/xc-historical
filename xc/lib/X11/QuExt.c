#include "copyright.h"

/* $Header: XQuExt.c,v 11.12 87/09/11 08:05:57 newman Locked $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"

Bool XQueryExtension(dpy, name, major_opcode, first_event, first_error)
    register Display *dpy;
    char *name;
    int *major_opcode;  /* RETURN */
    int *first_event;   /* RETURN */
    int *first_error;	/* RETURN */
{       
    xQueryExtensionReply rep;
    register xQueryExtensionReq *req;

    LockDisplay(dpy);
    GetReq(QueryExtension, req);
    req->nbytes = name ? strlen(name) : 0;
    req->length += (req->nbytes+3)>>2;
    _XSend(dpy, name, (long)req->nbytes);
    (void) _XReply (dpy, (xReply *)&rep, 0, xTrue);
    *major_opcode = rep.major_opcode;
    *first_event = rep.first_event;
    *first_error = rep.first_error;
    UnlockDisplay(dpy);
    SyncHandle();
    return (rep.present);
}

