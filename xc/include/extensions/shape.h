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

/* $XConsortium: Exp $ */

/*
 * Protocol requests constants and alignment values
 * These would really be in SHAPE's X.h and Xproto.h equivalents
 */

#define X_SetWindowShapeRectangles	    1
#define X_SetWindowShapeMask		    2
#define X_SetBorderShapeRectangles	    3
#define X_SetBorderShapeMask		    4
#define X_GetWindowShapeRectangles	    5
#define X_GetWindowShapeMask		    6
#define X_GetBorderShapeRectangles	    7
#define X_GetBorderShapeMask		    8

typedef struct _SetWindowShapeRectangles {
    CARD8 reqType;		/* always ShapeReqCode */
    CARD8 shapeReqType;     /* always X_SetWindowShapeRectangles */
    CARD16 length B16;
    CARD32 window B32;
} xSetWindowShapeRectanglesReq;		/* followed by xRects */
#define sz_xSetWindowShapeRectanglesReq 8

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

#define SHAPENAME "ShapeExtension"
