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

/* $XConsortium: XShape.c,v 1.2 89/02/14 14:34:20 keith Exp $ */

#define NEED_REPLIES
#include "region.h"
#include "Xutil.h"
#include "shapestr.h"

static int TriedShape = 0;
static int ShapeReqCode = 0;
static int ShapeEventBase = 0;
static int ShapeErrorBase = 0;

#define CheckExtension(dpy) if (!ShapeReqCode && !XQueryShapeExtension (dpy)) return;

Bool
XQueryShapeExtension (dpy)
register Display *dpy;
{
    int first_event, first_error;

    if (!TriedShape) {
	TriedShape = 1;
	if (!XQueryExtension (dpy, SHAPENAME, &ShapeReqCode,
		&ShapeEventBase, &ShapeErrorBase))
	    ShapeReqCode = 0;
    }
    return ShapeReqCode != 0;
}

XShapeRegion(dpy, dest, destKind, r, op, xOff, yOff)
register Display    *dpy;
Window		    dest;
int		    destKind, op, xOff, yOff;
register Region	    r;
{
    register int i;
    register XRectangle *xr, *pr;
    register BOX *pb;
    LockDisplay (dpy);
    xr = (XRectangle *) 
    	_XAllocScratch(dpy, (unsigned long)(r->numRects * sizeof (XRectangle)));
    for (pr = xr, pb = r->rects, i = r->numRects; --i >= 0; pr++, pb++) {
        pr->x = pb->x1;
	pr->y = pb->y1;
	pr->width = pb->x2 - pb->x1;
	pr->height = pb->y2 - pb->y1;
     }
     XShapeRectangles (dpy, dest, destKind, xr, r->numRects, op, xOff, yOff);
}

XShapeRectangles(dpy, dest, destKind, rectangles, n_rects, op, xOff, yOff)
register Display *dpy;
XID dest;
int op, xOff, yOff;
XRectangle  *rectangles;
int n_rects;
{
    register xShapeRectanglesReq *req;
    register long nbytes;

    CheckExtension (dpy);
    LockDisplay(dpy);
    GetReq(ShapeRectangles, req);
    req->reqType = ShapeReqCode;
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

XShapeMask (dpy, dest, destKind, src, op, xOff, yOff)
register Display *dpy;
int destKind;
XID dest;
Pixmap	src;
int op, xOff, yOff;
{
    register xShapeMaskReq *req;
    register long nbytes;

    CheckExtension (dpy);
    LockDisplay(dpy);
    GetReq(ShapeMask, req);
    req->reqType = ShapeReqCode;
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

XShapeCombine (dpy, dest, destKind, src, srcKind, op, xOff, yOff)
register Display *dpy;
int destKind;
XID dest;
int srcKind;
XID src;
int op, xOff, yOff;
{
    register xShapeCombineReq *req;
    register long nbytes;

    CheckExtension (dpy);
    LockDisplay(dpy);
    GetReq(ShapeCombine, req);
    req->reqType = ShapeReqCode;
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

XShapeOffset (dpy, dest, destKind, xOff, yOff)
register Display *dpy;
int destKind;
XID dest;
int xOff, yOff;
{
    register xShapeOffsetReq *req;
    register long nbytes;

    CheckExtension (dpy);
    LockDisplay(dpy);
    GetReq(ShapeOffset, req);
    req->reqType = ShapeReqCode;
    req->shapeReqType = X_ShapeOffset;
    req->destKind = destKind;
    req->dest = dest;
    req->xOff = xOff;
    req->yOff = yOff;
    UnlockDisplay(dpy);
    SyncHandle();
}

XShapeQuery (dpy, window, wShaped, bShaped,
		xws, yws, wws, hws, xbs, ybs, wbs, hbs)    
    register Display    *dpy;
    Window		    window;
    int			    *wShaped, *bShaped;	    /* RETURN */
    int			    *xws, *yws, *xbs, *ybs; /* RETURN */
    unsigned int	    *wws, *hws, *wbs, *hbs; /* RETURN */
{
    xShapeQueryReply	    rep;
    register xShapeQueryReq *req;
    
    CheckExtension (dpy);
    LockDisplay (dpy);
    GetReq (ShapeQuery, req);
    req->reqType = ShapeReqCode;
    req->shapeReqType = X_ShapeQuery;
    req->window = window;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    *wShaped = rep.windowShaped;
    *bShaped = rep.borderShaped;
    *xws = cvtINT16toInt (rep.xWindowShape);
    *yws = cvtINT16toInt (rep.yWindowShape);
    *wws = rep.widthWindowShape;
    *hws = rep.heightWindowShape;
    *xbs = cvtINT16toInt (rep.xBorderShape);
    *ybs = cvtINT16toInt (rep.yBorderShape);
    *wbs = rep.widthBorderShape;
    *hbs = rep.heightBorderShape;
    UnlockDisplay (dpy);
    SyncHandle ();
    return 1;
}
