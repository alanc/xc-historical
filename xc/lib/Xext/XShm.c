/************************************************************
Copyright 1989 by the Massachusetts Institute of Technology

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that  the name of MIT not be used in adver-
tising  or publicity pertaining to distribution of the soft-
ware without specific prior written permission. M.I.T. makes
no representation about the suitability of this software for
any  purpose.  It is provided "as is" without any express or
implied warranty.

********************************************************/

#ifdef MITSHM

#define NEED_EVENTS
#define NEED_REPLIES
#include "Xlibint.h"
#include "XShm.h"
#include "shmstr.h"

/* $XConsortium: XShm.c,v 1.0 89/04/22 12:03:06 rws Exp $ */

struct DpyHasShm {
    struct DpyHasShm	*next;
    Display		*dpy;
    Bool		gotit;
    XExtCodes		codes;
};

static struct DpyHasShm *lastChecked, *dpysHaveShm;

static XExtCodes *queryExtension();

static XExtCodes *
CheckExtension(dpy)
    Display *dpy;
{
    if (lastChecked && lastChecked->dpy == dpy) {
	if (lastChecked->gotit == 0)
	    return 0;
	return &lastChecked->codes;
    }
    return queryExtension(dpy);
}
				    
/*ARGSUSED*/
static int
shmCloseDisplay(dpy, codes)
    Display	*dpy;
    XExtCodes	*codes;
{
    struct DpyHasShm *dpyHas, *prev;

    prev = 0;
    for (dpyHas = dpysHaveShm; dpyHas; dpyHas = dpyHas->next) {
	if (dpyHas->dpy == dpy)
	    break;
	prev = dpyHas;
    }
    if (!dpyHas)
	return 0;
    if (prev)
	prev->next = dpyHas->next;
    else
	dpysHaveShm = dpyHas->next;
    if (dpyHas == lastChecked)
	lastChecked = (struct DpyHasShm *) NULL;
    Xfree(dpyHas);
    return 0;
}

static int
shmWireToEvent (dpy, re, event)
    Display *dpy;
    XEvent  *re;
    xEvent  *event;
{
    XShmCompletionEvent	*se;
    xShmCompletionEvent	*sevent;
    XExtCodes		*codes;

    codes = CheckExtension(dpy);
    if (!codes)
	return False;
    switch ((event->u.u.type & 0x7f) - codes->first_event) {
    case ShmCompletion:
	se = (XShmCompletionEvent *) re;
	sevent = (xShmCompletionEvent *) event;
	se->type = sevent->type & 0x7f;
	se->serial = _XSetLastRequestRead(dpy,(xGenericReply *) event);
	se->send_event = (sevent->type & 0x80) != 0;
	se->display = dpy;
	se->drawable = sevent->drawable;
	se->major_code = sevent->majorEvent;
	se->minor_code = sevent->minorEvent;
	se->shmseg = sevent->shmseg;
	se->offset = sevent->offset;
    	return True;
    }
    return False;
}

static int
shmEventToWire (dpy, re, event)
    Display *dpy;
    XEvent  *re;
    xEvent  *event;
{
    XShmCompletionEvent	*se;
    xShmCompletionEvent	*sevent;
    XExtCodes		*codes;

    codes = CheckExtension(dpy);
    if (!codes)
	return False;
    switch ((re->type & 0x7f) - codes->first_event) {
    case ShmCompletion:
    	se = (XShmCompletionEvent *) re;
	sevent = (xShmCompletionEvent *) event;
    	sevent->type = se->type | (se->send_event ? 0x80 : 0);
    	sevent->sequenceNumber = se->serial & 0xffff;
    	sevent->drawable = se->drawable;
    	sevent->majorEvent = se->major_code;
    	sevent->minorEvent = se->minor_code;
    	sevent->shmseg = se->shmseg;
    	sevent->offset = se->offset;
    	return True;
    }
    return False;
}

static XExtCodes *
queryExtension (dpy)
    register Display *dpy;
{
    struct DpyHasShm	*dpyHas;
    XExtCodes		*codes;
    int			i;

    for (dpyHas = dpysHaveShm; dpyHas; dpyHas = dpyHas->next) {
	if (dpyHas->dpy == dpy)
	    break;
    }
    if (!dpyHas) {
	dpyHas = (struct DpyHasShm *) Xmalloc(sizeof(struct DpyHasShm));
	if (!dpyHas)
	    return 0;
	dpyHas->next = dpysHaveShm;
	dpysHaveShm = dpyHas;

	dpyHas->dpy = dpy;
	if (dpyHas->gotit = (codes = XInitExtension(dpy, SHMNAME)) != 0) {
	    dpyHas->codes = *codes;
	    /*
	     * initialize the various Xlib function vectors
	     */
	    XESetCloseDisplay(dpy, codes->extension, shmCloseDisplay);
	    for (i = 0; i < ShmNumberEvents; i++) {
		XESetWireToEvent(dpy, codes->first_event + i, shmWireToEvent);
		XESetEventToWire(dpy, codes->first_event + i, shmEventToWire);
	    }
	}
    }
    lastChecked = dpyHas;
    if (!dpyHas->gotit)
	return (XExtCodes *) 0;
    return &dpyHas->codes;
}

Bool
XShmQueryExtension(dpy)
    Display *dpy;
{
    return CheckExtension(dpy) != 0;
}

int
XShmGetEventBase(dpy)
    Display *dpy;
{
    XExtCodes	*codes;

    codes = CheckExtension(dpy);
    if (!codes)
	return -1;
    return codes->first_event;
}

Bool
XShmQueryVersion(dpy, majorVersion, minorVersion, sharedPixmaps)
    Display *dpy;
    int	    *majorVersion, *minorVersion;
    Bool    *sharedPixmaps;
{
    XExtCodes			    *codes;
    xShmQueryVersionReply	    rep;
    register xShmQueryVersionReq  *req;

    if (!(codes = CheckExtension(dpy)))
	return 0;
    LockDisplay(dpy);
    GetReq(ShmQueryVersion, req);
    req->reqType = codes->major_opcode;
    req->shmReqType = X_ShmQueryVersion;
    if (!_XReply(dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return 0;
    }
    *majorVersion = rep.majorVersion;
    *minorVersion = rep.minorVersion;
    *sharedPixmaps = rep.sharedPixmaps;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

ShmSeg
XShmAttach(dpy, shmid, readOnly)
    Display *dpy;
    int shmid;
    Bool readOnly;
{
    register xShmAttachReq *req;
    XExtCodes *codes;
    ShmSeg shmseg;

    if (!(codes = CheckExtension(dpy)))
	return 0;
    shmseg = XAllocID(dpy);
    LockDisplay(dpy);
    GetReq(ShmAttach, req);
    req->reqType = codes->major_opcode;
    req->shmReqType = X_ShmAttach;
    req->shmseg = shmseg;
    req->shmid = shmid;
    req->readOnly = readOnly;
    UnlockDisplay(dpy);
    SyncHandle();
    return shmseg;
}

XShmDetach(dpy, shmseg)
    Display *dpy;
    ShmSeg shmseg;
{
    register xShmDetachReq *req;
    XExtCodes *codes;

    if (!(codes = CheckExtension(dpy)))
	return 0;
    LockDisplay(dpy);
    GetReq(ShmDetach, req);
    req->reqType = codes->major_opcode;
    req->shmReqType = X_ShmDetach;
    req->shmseg = shmseg;
    UnlockDisplay(dpy);
    SyncHandle();
}

XShmPutImage (dpy, d, gc, image, src_x, src_y, dst_x, dst_y,
	      src_width, src_height, send_event)
    register Display *dpy;
    Drawable d;
    GC gc;
    register XImage *image;
    int src_x, src_y, dst_x, dst_y;
    unsigned int src_width, src_height;
    Bool send_event;
{
    XShmSegmentInfo *info = (XShmSegmentInfo *)image->obdata;
    register xShmPutImageReq *req;
    XExtCodes *codes;

    if (!(codes = CheckExtension(dpy)))
	return 0;
    LockDisplay(dpy);
    GetReq(ShmPutImage, req);
    req->reqType = codes->major_opcode;
    req->shmReqType = X_ShmPutImage;
    req->drawable = d;
    req->gc = gc->gid;
    req->srcX = src_x;
    req->srcY = src_y;
    req->srcWidth = src_width;
    req->srcHeight = src_height;
    req->dstX = dst_x;
    req->dstY = dst_y;
    req->totalWidth = image->width;
    req->totalHeight = image->height;
    req->depth = image->depth;
    req->format = image->format;
    req->sendEvent = send_event;
    req->shmseg = info->shmseg;
    req->offset = info->addr - image->data;
    req->size = image->bytes_per_line * image->height;
    UnlockDisplay(dpy);
    SyncHandle();
}

#endif
