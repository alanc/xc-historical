/************************************************************
Copyright 1987 by the Massachusetts Institute of Technology

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

/* $XConsortium: Exp $ */

#define NEED_REPLIES
#include "Xlibint.h"
#include "shape.h"

static int ShapeReqCode = 0;

XSetWindowShapeRectangles(dpy, w, rectangles, n_rects)
register Display *dpy;
Window w;
XRectangle  *rectangles;
int n_rects;
{
    register xSetWindowShapeRectanglesReq *req;
    register long nbytes;
    int first_event, first_error;

    LockDisplay(dpy);
    if (!ShapeReqCode) {
	XQueryExtension(
	    dpy, SHAPENAME, &ShapeReqCode, &first_event, &first_error);
    }
    GetReq(SetWindowShapeRectangles, req);
    req->reqType = ShapeReqCode;
    req->shapeReqType = X_SetWindowShapeRectangles;
    req->window = w;

    /* SIZEOF(xRectangle) will be a multiple of 4 */
    req->length += n_rects * (SIZEOF(xRectangle) / 4);

    nbytes = n_rects * sizeof(xRectangle);

    Data16 (dpy, (short *) rectangles, nbytes);
    UnlockDisplay(dpy);
    SyncHandle();
}

XSetWindowShapeMask(dpy, w, xOff, yOff, mask)
register Display *dpy;
Window w;
int	xOff, yOff;
Pixmap	mask;
{
    register xSetWindowShapeMaskReq *req;
    register long nbytes;
    int first_event, first_error;

    LockDisplay(dpy);
    if (!ShapeReqCode) {
	XQueryExtension(
	    dpy, SHAPENAME, &ShapeReqCode, &first_event, &first_error);
    }
    GetReq(SetWindowShapeMask, req);
    req->reqType = ShapeReqCode;
    req->shapeReqType = X_SetWindowShapeMask;
    req->window = w;
    req->xOff = xOff;
    req->yOff = yOff;
    req->mask = mask;

    UnlockDisplay(dpy);
    SyncHandle();
}

XRectangle *
XGetWindowShapeRectangles(dpy, w, nrects)
register Display *dpy;
Window		w;
int		*nrects;
{
    register xGetWindowShapeRectanglesReq *req;
    int first_event, first_error;
    xGenericReply   rep;
    xRectangle	*xrects;
    XRectangle	*ret;
    int		n;

    LockDisplay(dpy);
    if (!ShapeReqCode) {
	XQueryExtension(
	    dpy, SHAPENAME, &ShapeReqCode, &first_event, &first_error);
    }
    GetReq (GetWindowShapeRectangles,req);
    req->reqType = ShapeReqCode;
    req->shapeReqType = X_GetWindowShapeRectangles;
    req->window = w;
    (void) _XReply (dpy, (xReply *) &rep, 0, xFalse);
    if (*nrects = rep.length / (sizeof (xRectangle) / 4)) {
	xrects = (xRectangle *) Xmalloc ((unsigned) *nrects * sizeof (xRectangle));
	if (!xrects)
	    return 0;
	ret = (XRectangle *) Xmalloc ((unsigned) *nrects * sizeof (XRectangle));
	if (!ret) {
	    XFree (xrects);
	    return 0;
	}
	_XReadPad (dpy, (char *) xrects, *nrects * sizeof (xRectangle));
	for (n = 0; n < *nrects; n++) {
	    ret[n].x = xrects[n].x;
	    ret[n].y = xrects[n].y;
	    ret[n].width = xrects[n].width;
	    ret[n].height = xrects[n].height;
	}
    }
    SyncHandle();
    return ret;
}

XGetWindowShapeMask(dpy, w, xOff, yOff, mask)
register Display *dpy;
Window		w;
Pixmap		mask;
{
    register xGetWindowShapeMaskReq *req;
    int first_event, first_error;

    LockDisplay(dpy);
    if (!ShapeReqCode) {
	XQueryExtension(
	    dpy, SHAPENAME, &ShapeReqCode, &first_event, &first_error);
    }
    GetReq (GetWindowShapeMask,req);
    req->reqType = ShapeReqCode;
    req->shapeReqType = X_GetWindowShapeMask;
    req->window = w;
    req->xOff = xOff;
    req->yOff = yOff;
    req->mask = mask;
    UnlockDisplay(dpy);
    SyncHandle();
}

XSetBorderShapeRectangles(dpy, w, rectangles, n_rects)
register Display *dpy;
Window w;
XRectangle  *rectangles;
int n_rects;
{
    register xSetBorderShapeRectanglesReq *req;
    register long nbytes;
    int first_event, first_error;

    LockDisplay(dpy);
    if (!ShapeReqCode) {
	XQueryExtension(
	    dpy, SHAPENAME, &ShapeReqCode, &first_event, &first_error);
    }
    GetReq(SetBorderShapeRectangles, req);
    req->reqType = ShapeReqCode;
    req->shapeReqType = X_SetBorderShapeRectangles;
    req->window = w;

    /* SIZEOF(xRectangle) will be a multiple of 4 */
    req->length += n_rects * (SIZEOF(xRectangle) / 4);

    nbytes = n_rects * sizeof(xRectangle);

    Data16 (dpy, (short *) rectangles, nbytes);
    UnlockDisplay(dpy);
    SyncHandle();
}

XSetBorderShapeMask(dpy, w, xOff, yOff, mask)
register Display *dpy;
Window w;
Pixmap	mask;
{
    register xSetBorderShapeMaskReq *req;
    register long nbytes;
    int first_event, first_error;

    LockDisplay(dpy);
    if (!ShapeReqCode) {
	XQueryExtension(
	    dpy, SHAPENAME, &ShapeReqCode, &first_event, &first_error);
    }
    GetReq(SetBorderShapeMask, req);
    req->reqType = ShapeReqCode;
    req->shapeReqType = X_SetBorderShapeMask;
    req->window = w;
    req->xOff = xOff;
    req->yOff = yOff;
    req->mask = mask;

    UnlockDisplay(dpy);
    SyncHandle();
}

XRectangle *
XGetBorderShapeRectangles(dpy, w, nrects)
register Display *dpy;
Window		w;
int		*nrects;
{
    register xGetBorderShapeRectanglesReq *req;
    int first_event, first_error;
    xGenericReply   rep;
    xRectangle	*xrects;
    XRectangle	*ret;
    int		n;

    LockDisplay(dpy);
    if (!ShapeReqCode) {
	XQueryExtension(
	    dpy, SHAPENAME, &ShapeReqCode, &first_event, &first_error);
    }
    GetReq (GetBorderShapeRectangles,req);
    req->reqType = ShapeReqCode;
    req->shapeReqType = X_GetBorderShapeRectangles;
    req->window = w;
    (void) _XReply (dpy, (xReply *) &rep, 0, xFalse);
    if (*nrects = rep.length / (sizeof (xRectangle) / 4)) {
	xrects = (xRectangle *) Xmalloc ((unsigned) *nrects * sizeof (xRectangle));
	if (!xrects)
	    return 0;
	ret = (XRectangle *) Xmalloc ((unsigned) *nrects * sizeof (XRectangle));
	if (!ret) {
	    XFree (xrects);
	    return 0;
	}
	_XReadPad (dpy, (char *) xrects, *nrects * sizeof (xRectangle));
	for (n = 0; n < *nrects; n++) {
	    ret[n].x = xrects[n].x;
	    ret[n].y = xrects[n].y;
	    ret[n].width = xrects[n].width;
	    ret[n].height = xrects[n].height;
	}
    }
    SyncHandle();
    return ret;
}

XGetBorderShapeMask(dpy, w, xOff, yOff, mask)
register Display *dpy;
Window		w;
Pixmap		mask;
{
    register xGetBorderShapeMaskReq *req;
    int first_event, first_error;

    LockDisplay(dpy);
    if (!ShapeReqCode) {
	XQueryExtension(
	    dpy, SHAPENAME, &ShapeReqCode, &first_event, &first_error);
    }
    GetReq (GetBorderShapeMask,req);
    req->reqType = ShapeReqCode;
    req->shapeReqType = X_GetBorderShapeMask;
    req->window = w;
    req->mask = mask;
    req->xOff = xOff;
    req->yOff = yOff;
    UnlockDisplay(dpy);
    SyncHandle();
}
