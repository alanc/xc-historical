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

/* $XConsortium: shape.h,v 1.3 89/03/23 20:10:50 keith Exp $ */

#ifndef _SHAPE_H_
#define _SHAPE_H_

#define X_ShapeRectangles		1
#define X_ShapeMask			2
#define X_ShapeCombine			3
#define X_ShapeOffset			4
#define X_ShapeQuery			5
#define X_ShapeSelectInput		6
#define X_ShapeGetRectangles		7

#define ShapeSet			0
#define ShapeUnion			1
#define ShapeIntersect			2
#define ShapeSubtract			3
#define ShapeInvert			4

#define ShapeWindow			0
#define ShapeBorder			1

#define ShapeNotify			0

#define ShapeNumberEvents		(ShapeNotify + 1)

#ifndef _SHAPE_SERVER_
typedef struct {
    int	type;		    /* of event */
    unsigned long serial;   /* # of last request processed by server */
    Bool send_event;	    /* true if this came frome a SendEvent request */
    Display *display;	    /* Display the event was read from */
    Window window;	    /* window of event */
    int kind;		    /* ShapeWindow or ShapeBorder */
    int x, y;		    /* extents of new region */
    unsigned width, height;
    Time time;
} XShapeEvent;
#endif

#endif
