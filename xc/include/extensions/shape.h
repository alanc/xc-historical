/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

                    All Rights Reserved

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
no- tice appear in all copies and that both that copyright
no- tice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

MIT DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: shape.h,v 1.1 89/02/07 13:26:17 keith Exp $ */

/*
 * Protocol requests constants and alignment values
 * These would really be in SHAPE's X.h and Xproto.h equivalents
 */

#define SHAPENAME "ShapeExtension"

#define X_SetWindowShapeRectangles	    1
#define X_IntersectWindowShapeRectangles    2
#define X_UnionWindowShapeRectangles	    3
#define X_SetWindowShapeMask		    4
#define X_IntersectWindowShapeMask	    5
#define X_UnionWindowShapeMask		    6
#define X_GetWindowShapeRectangles	    7
#define X_GetWindowShapeMask		    8

#define X_SetBorderShapeRectangles	    (1 + 8)
#define X_IntersectBorderShapeRectangles    (2 + 8)
#define X_UnionBorderShapeRectangles	    (3 + 8)
#define X_SetBorderShapeMask		    (4 + 8)
#define X_IntersectBorderShapeMask	    (5 + 8)
#define X_UnionBorderShapeMask		    (6 + 8)
#define X_GetBorderShapeRectangles	    (7 + 8)
#define X_GetBorderShapeMask		    (8 + 8)

typedef struct _SetWindowShapeRectangles {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_SetWindowShapeRectangles */
    CARD16 length B16;
    CARD32 window B32;
} xSetWindowShapeRectanglesReq;		/* followed by xRects */
#define sz_xSetWindowShapeRectanglesReq 8

typedef struct _IntersectWindowShapeRectangles {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_IntersectWindowShapeRectangles */
    CARD16 length B16;
    CARD32 window B32;
} xIntersectWindowShapeRectanglesReq;		/* followed by xRects */
#define sz_xIntersectWindowShapeRectanglesReq 8

typedef struct _UnionWindowShapeRectangles {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_UnionWindowShapeRectangles */
    CARD16 length B16;
    CARD32 window B32;
} xUnionWindowShapeRectanglesReq;		/* followed by xRects */
#define sz_xUnionWindowShapeRectanglesReq 8

typedef struct _SetWindowShapeMask {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_SetWindowShapeMask */
    CARD16 length B16;
    INT16  xOff   B16;
    INT16  yOff   B16;
    CARD32 window B32;
    CARD32 mask   B32;
} xSetWindowShapeMaskReq;
#define sz_xSetWindowShapeMaskReq 16

typedef struct _IntersectWindowShapeMask {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_IntersectWindowShapeMask */
    CARD16 length B16;
    INT16  xOff   B16;
    INT16  yOff   B16;
    CARD32 window B32;
    CARD32 mask   B32;
} xIntersectWindowShapeMaskReq;
#define sz_xIntersectWindowShapeMaskReq 16

typedef struct _UnionWindowShapeMask {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_UnionWindowShapeMask */
    CARD16 length B16;
    INT16  xOff   B16;
    INT16  yOff   B16;
    CARD32 window B32;
    CARD32 mask   B32;
} xUnionWindowShapeMaskReq;
#define sz_xUnionWindowShapeMaskReq 16

typedef struct _GetWindowShapeRectangles {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_GetWindowShapeRectangles */
    CARD16 length B16;
    CARD32 window B32;
} xGetWindowShapeRectanglesReq;
#define sz_xGetWindowShapeRectanglesReq 8

typedef struct _GetWindowShapeMask {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_GetWindowShapeMask */
    CARD16 length B16;
    INT16  xOff   B16;
    INT16  yOff   B16;
    CARD32 window B32;
    CARD32 mask   B32;
} xGetWindowShapeMaskReq;
#define sz_xGetWindowShapeMaskReq 16

typedef struct _SetBorderShapeRectangles {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_SetBorderShapeRectangles */
    CARD16 length B16;
    CARD32 window B32;
} xSetBorderShapeRectanglesReq;		/* followed by xRects */
#define sz_xSetBorderShapeRectanglesReq 8

typedef struct _IntersectBorderShapeRectangles {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_IntersectBorderShapeRectangles */
    CARD16 length B16;
    CARD32 window B32;
} xIntersectBorderShapeRectanglesReq;		/* followed by xRects */
#define sz_xIntersectBorderShapeRectanglesReq 8

typedef struct _UnionBorderShapeRectangles {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_UnionBorderShapeRectangles */
    CARD16 length B16;
    CARD32 window B32;
} xUnionBorderShapeRectanglesReq;		/* followed by xRects */
#define sz_xUnionBorderShapeRectanglesReq 8

typedef struct _SetBorderShapeMask {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_SetBorderShapeMask */
    CARD16 length B16;
    INT16  xOff   B16;
    INT16  yOff   B16;
    CARD32 window B32;
    CARD32 mask   B32;
} xSetBorderShapeMaskReq;
#define sz_xSetBorderShapeMaskReq 16

typedef struct _IntersectBorderShapeMask {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_IntersectBorderShapeMask */
    CARD16 length B16;
    INT16  xOff   B16;
    INT16  yOff   B16;
    CARD32 window B32;
    CARD32 mask   B32;
} xIntersectBorderShapeMaskReq;
#define sz_xIntersectBorderShapeMaskReq 16

typedef struct _UnionBorderShapeMask {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_UnionBorderShapeMask */
    CARD16 length B16;
    INT16  xOff   B16;
    INT16  yOff   B16;
    CARD32 window B32;
    CARD32 mask   B32;
} xUnionBorderShapeMaskReq;
#define sz_xUnionBorderShapeMaskReq 16

typedef struct _GetBorderShapeRectangles {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_GetBorderShapeRectangles */
    CARD16 length B16;
    CARD32 window B32;
} xGetBorderShapeRectanglesReq;
#define sz_xGetBorderShapeRectanglesReq 8

typedef struct _GetBorderShapeMask {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_GetBorderShapeMask */
    CARD16 length B16;
    INT16  xOff   B16;
    INT16  yOff   B16;
    CARD32 window B32;
    CARD32 mask   B32;
} xGetBorderShapeMaskReq;
#define sz_xGetBorderShapeMaskReq 16
