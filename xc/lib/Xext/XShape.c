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

MIT DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL MIT BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: XShape.c,v 1.4 89/03/28 14:19:12 keith Exp $ */

#define NEED_EVENTS
#define NEED_REPLIES
#include "region.h"
#include "shapestr.h"

struct DpyHasShape {
    struct DpyHasShape	*next;
    Display		*dpy;
    Bool		gotit;
    XExtCodes		codes;
};

static struct DpyHasShape  *lastChecked, *dpysHaveShape;

static XExtCodes    *queryExtension ();

static XExtCodes *
CheckExtension (dpy)
    Display *dpy;
{
    if (lastChecked && lastChecked->dpy == dpy) {
	if (lastChecked->gotit == 0)
	    return 0;
	return &lastChecked->codes;
    }
    return queryExtension (dpy);
}
				    
static int
shapeCloseDisplay (dpy, codes)
    Display	*dpy;
    XExtCodes	*codes;
{
    struct DpyHasShape	*dpyHas, *prev;

    prev = 0;
    for (dpyHas = dpysHaveShape; dpyHas; dpyHas = dpyHas->next) {
	if (dpyHas->dpy == dpy)
	    break;
	prev = dpyHas;
    }
    if (!dpyHas)
	return 0;
    if (prev)
	prev->next = dpyHas->next;
    else
	dpysHaveShape = dpyHas->next;
    if (dpyHas == lastChecked)
	lastChecked = (struct DpyHasShape *) NULL;
    Xfree (dpyHas);
    return 0;
}

static int
shapeWireToEvent (dpy, re, event)
    Display *dpy;
    XEvent  *re;
    xEvent  *event;
{
    XShapeEvent		*se;
    xShapeNotifyEvent	*sevent;
    XExtCodes		*codes;

    codes = CheckExtension (dpy);
    if (!codes)
	return False;
    switch ((event->u.u.type & 0x7f) - codes->first_event) {
    case ShapeNotify:
    	se = (XShapeEvent *) re;
	sevent = (xShapeNotifyEvent *) event;
    	se->type = sevent->type & 0x7f;
    	se->serial = _XSetLastRequestRead(dpy,(xGenericReply *) event);
    	se->send_event = (sevent->type & 0x80) != 0;
    	se->display = dpy;
    	se->window = sevent->window;
    	se->kind = sevent->kind;
    	se->x = sevent->x;
    	se->y = sevent->y;
    	se->width = sevent->width;
    	se->height = sevent->height;
	se->time = sevent->time;
	se->shaped = True;
	if (sevent->shaped == xFalse)
	    se->shaped = False;
    	return True;
    }
    return False;
}

static int
shapeEventToWire (dpy, re, event)
    Display *dpy;
    XEvent  *re;
    xEvent  *event;
{
    XShapeEvent		*se;
    xShapeNotifyEvent	*sevent;
    XExtCodes		*codes;

    codes = CheckExtension (dpy);
    if (!codes)
	return False;
    switch ((re->type & 0x7f) - codes->first_event) {
    case ShapeNotify:
    	se = (XShapeEvent *) re;
	sevent = (xShapeNotifyEvent *) event;
    	sevent->type = se->type | (se->send_event ? 0x80 : 0);
    	sevent->sequenceNumber = se->serial & 0xffff;
    	sevent->window = se->window;
    	sevent->kind = se->kind;
    	sevent->x = se->x;
    	sevent->y = se->y;
    	sevent->width = se->width;
    	sevent->height = se->height;
	sevent->time = se->time;
    	return True;
    }
    return False;
}

static XExtCodes *
queryExtension (dpy)
    register Display *dpy;
{
    struct DpyHasShape	*dpyHas;
    XExtCodes		*codes;
    int			i;

    for (dpyHas = dpysHaveShape; dpyHas; dpyHas = dpyHas->next) {
	if (dpyHas->dpy == dpy)
	    break;
    }
    if (!dpyHas) {
	dpyHas = (struct DpyHasShape *) Xmalloc (sizeof (struct DpyHasShape));
	if (!dpyHas)
	    return 0;
	dpyHas->next = dpysHaveShape;
	dpysHaveShape = dpyHas;

	dpyHas->dpy = dpy;
	if (dpyHas->gotit = (codes = XInitExtension (dpy, SHAPENAME)) != 0) {
	    dpyHas->codes = *codes;
	    /*
	     * initialize the various Xlib function vectors
	     */
	    XESetCloseDisplay (dpy, codes->extension, shapeCloseDisplay);
	    for (i = 0; i < ShapeNumberEvents; i++) {
		XESetWireToEvent (dpy, codes->first_event + i, shapeWireToEvent);
		XESetEventToWire (dpy, codes->first_event + i, shapeEventToWire);
	    }
	}
    }
    lastChecked = dpyHas;
    return &dpyHas->codes;
}

Bool
XShapeQueryExtension (dpy)
    Display *dpy;
{
    return CheckExtension (dpy) != 0;
}

XShapeGetEventBase (dpy)
    Display *dpy;
{
    XExtCodes	*codes;

    codes = CheckExtension (dpy);
    if (!codes)
	return -1;
    return codes->first_event;
}

Bool
XShapeQueryVersion(dpy, majorVersion, minorVersion)
    Display *dpy;
    int	    *majorVersion, *minorVersion;
{
    XExtCodes			    *codes;
    xShapeQueryVersionReply	    rep;
    register xShapeQueryVersionReq  *req;

    if (!(codes = CheckExtension (dpy)))
	return 0;
    LockDisplay (dpy);
    GetReq (ShapeQueryVersion, req);
    req->reqType = codes->major_opcode;
    req->shapeReqType = X_ShapeQueryVersion;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    *majorVersion = rep.majorVersion;
    *minorVersion = rep.minorVersion;
    UnlockDisplay (dpy);
    SyncHandle ();
    return 1;
}

XShapeCombineRegion(dpy, dest, destKind, r, op, xOff, yOff)
register Display    *dpy;
Window		    dest;
int		    destKind, op, xOff, yOff;
register REGION	    *r;
{
    register int i;
    register XRectangle *xr, *pr;
    register BOX *pb;

    xr = (XRectangle *) 
    	_XAllocScratch(dpy, (unsigned long)(r->numRects * sizeof (XRectangle)));
    for (pr = xr, pb = r->rects, i = r->numRects; --i >= 0; pr++, pb++) {
        pr->x = pb->x1;
	pr->y = pb->y1;
	pr->width = pb->x2 - pb->x1;
	pr->height = pb->y2 - pb->y1;
     }
     XShapeCombineRectangles (dpy, dest, destKind, xr, r->numRects, op, xOff, yOff);
}

XShapeCombineRectangles(dpy, dest, destKind, rectangles, n_rects, op, xOff, yOff)
register Display *dpy;
XID dest;
int op, xOff, yOff;
XRectangle  *rectangles;
int n_rects;
{
    register xShapeRectanglesReq *req;
    register long nbytes;
    XExtCodes	*codes;

    if (!(codes = CheckExtension (dpy)))
	return;
    LockDisplay(dpy);
    GetReq(ShapeRectangles, req);
    req->reqType = codes->major_opcode;
    req->shapeReqType = X_ShapeRectangles;
    req->op = op;
    req->destKind = destKind;
    req->dest = dest;
    req->xOff = xOff;
    req->yOff = yOff;

    /* SIZEOF(xRectangle) will be a multiple of 4 */
    req->length += n_rects * (SIZEOF(xRectangle) / 4);

    nbytes = n_rects * sizeof(xRectangle);

    Data16 (dpy, (short *) rectangles, nbytes);
    UnlockDisplay(dpy);
    SyncHandle();
}

XShapeCombineMask (dpy, dest, destKind, src, op, xOff, yOff)
register Display *dpy;
int destKind;
XID dest;
Pixmap	src;
int op, xOff, yOff;
{
    register xShapeMaskReq *req;
    register long nbytes;
    XExtCodes	*codes;

    if (!(codes = CheckExtension (dpy)))
	return;
    LockDisplay(dpy);
    GetReq(ShapeMask, req);
    req->reqType = codes->major_opcode;
    req->shapeReqType = X_ShapeMask;
    req->op = op;
    req->destKind = destKind;
    req->dest = dest;
    req->xOff = xOff;
    req->yOff = yOff;
    req->src = src;
    UnlockDisplay(dpy);
    SyncHandle();
}

XShapeCombineShape (dpy, dest, destKind, src, srcKind, op, xOff, yOff)
register Display *dpy;
int destKind;
XID dest;
int srcKind;
XID src;
int op, xOff, yOff;
{
    register xShapeCombineReq *req;
    register long nbytes;
    XExtCodes	*codes;

    if (!(codes = CheckExtension (dpy)))
	return;
    LockDisplay(dpy);
    GetReq(ShapeCombine, req);
    req->reqType = codes->major_opcode;
    req->shapeReqType = X_ShapeCombine;
    req->op = op;
    req->destKind = destKind;
    req->srcKind = srcKind;
    req->dest = dest;
    req->xOff = xOff;
    req->yOff = yOff;
    req->src = src;
    UnlockDisplay(dpy);
    SyncHandle();
}

XShapeOffsetShape (dpy, dest, destKind, xOff, yOff)
register Display *dpy;
int destKind;
XID dest;
int xOff, yOff;
{
    register xShapeOffsetReq *req;
    register long nbytes;
    XExtCodes	*codes;

    if (!(codes = CheckExtension (dpy)))
	return;
    LockDisplay(dpy);
    GetReq(ShapeOffset, req);
    req->reqType = codes->major_opcode;
    req->shapeReqType = X_ShapeOffset;
    req->destKind = destKind;
    req->dest = dest;
    req->xOff = xOff;
    req->yOff = yOff;
    UnlockDisplay(dpy);
    SyncHandle();
}

XShapeQueryExtents (dpy, window,
		bShaped, xbs, ybs, wbs, hbs,
		cShaped, xcs, ycs, wcs, hcs)    
    register Display    *dpy;
    Window		    window;
    int			    *bShaped, *cShaped;	    /* RETURN */
    int			    *xbs, *ybs, *xcs, *ycs; /* RETURN */
    unsigned int	    *wbs, *hbs, *wcs, *hcs; /* RETURN */
{
    xShapeQueryExtentsReply	    rep;
    register xShapeQueryExtentsReq *req;
    XExtCodes	*codes;
    
    if (!(codes = CheckExtension (dpy)))
	return;
    LockDisplay (dpy);
    GetReq (ShapeQueryExtents, req);
    req->reqType = codes->major_opcode;
    req->shapeReqType = X_ShapeQueryExtents;
    req->window = window;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    *bShaped = rep.boundingShaped;
    *cShaped = rep.clipShaped;
    *xbs = cvtINT16toInt (rep.xBoundingShape);
    *ybs = cvtINT16toInt (rep.yBoundingShape);
    *wbs = rep.widthBoundingShape;
    *hbs = rep.heightBoundingShape;
    *xcs = cvtINT16toInt (rep.xClipShape);
    *ycs = cvtINT16toInt (rep.yClipShape);
    *wcs = rep.widthClipShape;
    *hcs = rep.heightClipShape;
    UnlockDisplay (dpy);
    SyncHandle ();
    return 1;
}

XShapeSelectInput (dpy, window, enable)
    register Display	*dpy;
    Window		window;
    Bool		enable;
{
    register xShapeSelectInputReq   *req;
    XExtCodes			    *codes;

    if (!(codes = CheckExtension (dpy)))
	return;
    LockDisplay (dpy);
    GetReq (ShapeSelectInput, req);
    req->reqType = codes->major_opcode;
    req->shapeReqType = X_ShapeSelectInput;
    req->window = window;
    if (enable)
	req->enable = xTrue;
    else
	req->enable = xFalse;
    UnlockDisplay (dpy);
    SyncHandle ();
}

XRectangle *
XShapeGetRectangles (dpy, window, kind, count)
    register Display	*dpy;
    Window		window;
    int			kind;
    int			*count;	/* RETURN */
{
    register xShapeGetRectanglesReq   *req;
    XExtCodes			    *codes;
    xShapeGetRectanglesReply	    rep;
    XRectangle			    *rects;
    xRectangle			    *xrects;
    int				    i;

    if (!(codes = CheckExtension (dpy)))
	return;
    LockDisplay (dpy);
    GetReq (ShapeGetRectangles, req);
    req->reqType = codes->major_opcode;
    req->shapeReqType = X_ShapeGetRectangles;
    req->window = window;
    req->kind = kind;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    *count = rep.nrects;
    rects = 0;
    if (*count) {
	xrects = (xRectangle *) Xmalloc (*count * sizeof (xRectangle));
	rects = (XRectangle *) Xmalloc (*count * sizeof (XRectangle));
	if (!xrects || !rects) {
	    if (xrects)
		Xfree (xrects);
	    if (rects)
		Xfree (rects);
	    _XEatData (dpy, *count * sizeof (xRectangle));
	    rects = 0;
	    *count = 0;
	} else {
	    _XRead (dpy, (char *) xrects, *count * sizeof (xRectangle));
	    for (i = 0; i < *count; i++) {
	    	rects[i].x = (short) cvtINT16toInt (xrects[i].x);
	    	rects[i].y = (short) cvtINT16toInt (xrects[i].y);
	    	rects[i].width = xrects[i].width;
	    	rects[i].height = xrects[i].height;
	    }
	    Xfree (xrects);
	}
    }
    return rects;
}
