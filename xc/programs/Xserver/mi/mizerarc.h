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

/* $XConsortium: mizerarc.h,v 5.6 89/09/19 14:28:36 rws Exp $ */

#define FULLCIRCLE (360 * 64)
#define OCTANT (45 * 64)
#define QUADRANT (90 * 64)

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

#define Dsin(d)	((d) == 0.0 ? 0.0 : ((d) == 90.0 ? 1.0 : sin(d*M_PI/180.0)))
#define Dcos(d)	((d) == 0.0 ? 1.0 : ((d) == 90.0 ? 0.0 : cos(d*M_PI/180.0)))

typedef struct {
    int x;
    int y;
    int mask;
} miZeroArcPtRec;

typedef struct {
    int x, y, k1, k3, a, b, d, dx, dy;
    int alpha, beta;
    int xorg, yorg;
    int xorgo, yorgo;
    int w, h;
    int initialMask;
    miZeroArcPtRec start, altstart, end, altend;
    int firstx, firsty;
    int startAngle, endAngle;
} miZeroArcRec;

#define miCanZeroArc(arc) (((arc)->width == (arc)->height) || \
			   (((arc)->width <= 800) && ((arc)->height <= 800)))

extern Bool miZeroArcSetup();
