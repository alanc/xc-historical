/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

********************************************************/

/* $XConsortium: mifillarc.h,v 5.0 89/09/20 18:55:54 rws Exp $ */

#define FULLCIRCLE (360 * 64)

typedef struct FillArcInfo {
    int xorg, yorg;
    int y;
    int dx, dy;
    int e, ex;
    int ym, yk, xm, xk;
} miFillArcRec;

#define miCanFillArc(arc) (((arc)->width == (arc)->height) || \
			   (((arc)->width <= 800) && ((arc)->height <= 800)))

extern void miFillArcSetup();

#define MIFILLARCSETUP() \
    x = 0; \
    y = info.y; \
    e = info.e; \
    ex = info.ex; \
    xk = info.xk; \
    xm = info.xm; \
    yk = info.yk; \
    ym = info.ym; \
    dx = info.dx; \
    dy = info.dy; \
    xorg = info.xorg; \
    yorg = info.yorg

#define MIFILLCIRCSTEP(slw) \
    e += (y << 3) - xk; \
    while (e >= 0) \
    { \
	x++; \
	e += (ex = -((x << 3) + xk)); \
    } \
    y--; \
    slw = (x << 1) + dx; \
    if ((e == ex) && (slw > 1)) \
	slw--

#define MIFILLELLSTEP(slw) \
    e += (y * ym) - yk; \
    while (e >= 0) \
    { \
	x++; \
	e += (ex = -((x * xm) + xk)); \
    } \
    y--; \
    slw = (x << 1) + dx; \
    if ((e == ex) && (slw > 1)) \
	slw--

#define miFillArcLower(slw) (((y + dy) != 0) && ((slw > 1) || (e != ex)))
