/************************************************************
Copyright 1989 by the Massachusetts Institute of Technology

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

/* $XConsortium: XShm.c,v 1.3 89/08/21 08:10:35 rws Exp $ */

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
    XExtCodes *codes;
    xShmQueryVersionReply rep;
    register xShmQueryVersionReq *req;

    if (!(codes = CheckExtension(dpy)))
	return False;
    LockDisplay(dpy);
    GetReq(ShmQueryVersion, req);
    req->reqType = codes->major_opcode;
    req->shmReqType = X_ShmQueryVersion;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    *majorVersion = rep.majorVersion;
    *minorVersion = rep.minorVersion;
    *sharedPixmaps = rep.sharedPixmaps ? True : False;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XShmAttach(dpy, info)
    Display *dpy;
    XShmSegmentInfo *info;
{
    register xShmAttachReq *req;
    XExtCodes *codes;

    if (!(codes = CheckExtension(dpy)))
	return 0;
    info->shmseg = XAllocID(dpy);
    LockDisplay(dpy);
    GetReq(ShmAttach, req);
    req->reqType = codes->major_opcode;
    req->shmReqType = X_ShmAttach;
    req->shmseg = info->shmseg;
    req->shmid = info->shmid;
    req->readOnly = info->readOnly ? xTrue : xFalse;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status
XShmDetach(dpy, info)
    Display *dpy;
    XShmSegmentInfo *info;
{
    register xShmDetachReq *req;
    XExtCodes *codes;

    if (!(codes = CheckExtension(dpy)))
	return 0;
    LockDisplay(dpy);
    GetReq(ShmDetach, req);
    req->reqType = codes->major_opcode;
    req->shmReqType = X_ShmDetach;
    req->shmseg = info->shmseg;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

static int
_XShmDestroyImage (ximage)
    XImage *ximage;

{
	Xfree((char *)ximage);
	return 1;
}

#define ROUNDUP(nbytes, pad) ((((nbytes) + ((pad) - 1)) / (pad)) * (pad))

XImage *
XShmCreateImage(dpy, visual, depth, format, data, info, width, height)
    register Display *dpy;
    register Visual *visual;
    unsigned int depth;
    int format;
    char *data;
    XShmSegmentInfo *info;
    unsigned int width;
    unsigned int height;
{
    register XImage *image;

    image = (XImage *)Xcalloc(1, (unsigned)sizeof(XImage));
    if (!image)
	return image;
    image->data = data;
    image->obdata = (char *)info;
    image->width = width;
    image->height = height;
    image->depth = depth;
    image->format = format;
    image->byte_order = dpy->byte_order;
    image->bitmap_unit = dpy->bitmap_unit;
    image->bitmap_bit_order = dpy->bitmap_bit_order;
    image->bitmap_pad = _XGetScanlinePad(dpy, depth);
    image->xoffset = 0;
    if (visual) {
	image->red_mask = visual->red_mask;
	image->green_mask = visual->green_mask;
	image->blue_mask = visual->blue_mask;
    } else {
	image->red_mask = image->green_mask = image->blue_mask = 0;
    }
    if (format == ZPixmap)
	image->bits_per_pixel = _XGetBitsPerPixel(dpy, (int)depth);
    else
	image->bits_per_pixel = 1;
    image->bytes_per_line = ROUNDUP((image->bits_per_pixel * width),
				    image->bitmap_pad) >> 3;
    _XInitImageFuncPtrs(image);
    image->f.destroy_image = _XShmDestroyImage;
    return image;
}

Status
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

    if (!info || !(codes = CheckExtension(dpy)))
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
    req->offset = info->shmaddr - image->data;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status
XShmGetImage(dpy, d, image, x, y, plane_mask)
    register Display *dpy;
    Drawable d;
    XImage *image;
    int x, y;
    unsigned long plane_mask;
{
    XShmSegmentInfo *info = (XShmSegmentInfo *)image->obdata;
    register xShmGetImageReq *req;
    xShmGetImageReply rep;
    XExtCodes *codes;
    register Visual *visual;

    if (!info || !(codes = CheckExtension(dpy)))
	return 0;
    LockDisplay(dpy);
    GetReq(ShmGetImage, req);
    req->reqType = codes->major_opcode;
    req->shmReqType = X_ShmGetImage;
    req->drawable = d;
    req->x = x;
    req->y = y;
    req->width = image->width;
    req->height = image->height;
    req->planeMask = plane_mask;
    req->format = image->format;
    req->shmseg = info->shmseg;
    req->offset = info->shmaddr - image->data;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return 0;
    }
    visual = _XVIDtoVisual(dpy, rep.visual);
    image->red_mask = visual->red_mask;
    image->green_mask = visual->green_mask;
    image->blue_mask = visual->blue_mask;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Pixmap
XShmCreatePixmap (dpy, d, data, info, width, height, depth)
    register Display *dpy;
    Drawable d;
    char *data;
    XShmSegmentInfo *info;
    unsigned int width, height, depth;
{
    Pixmap pid;
    register xShmCreatePixmapReq *req;
    XExtCodes *codes;

    if (!(codes = CheckExtension(dpy)))
	return 0;
    LockDisplay(dpy);
    GetReq(ShmCreatePixmap, req);
    req->reqType = codes->major_opcode;
    req->shmReqType = X_ShmCreatePixmap;
    req->drawable = d;
    req->width = width;
    req->height = height;
    req->depth = depth;
    req->shmseg = info->shmseg;
    req->offset = data - info->shmaddr;
    pid = req->pid = XAllocID(dpy);
    UnlockDisplay(dpy);
    SyncHandle();
    return pid;
}

#endif
