#include "x11perf.h"

#define NUMPOINTS 100

static GC bggc, fggc;
static Window w;
static Pixmap pix;
static  XPoint points[NUMPOINTS];
static  XSegment *segsa, *segsb;

#define NegMod(x, y) ((y) - (((-x)-1) % (7)) - 1)

Bool InitScrolling(d, p)
    Display *d;
    Parms p;
{
    int i;

    for (i = 0; i < NUMPOINTS; i++) {    
        points[i].x = rand() % WIDTH;
        points[i].y = rand() % HEIGHT;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &bggc, &fggc);
    XDrawLines(d, w, fggc, points, NUMPOINTS, CoordModeOrigin);
    return True;
}

void DoScrolling(d, p)
    Display *d;
    Parms p;
{
    int i;

    for (i=0; i<p->reps; i++) {
	XCopyArea(d, w, w, fggc, 0, 1, WIDTH, HEIGHT - 1, 0, 0);
    }
}

void MidScroll(d, p)
    Display *d;
    Parms p;
{
    XClearArea(d, w, 0, 0, WIDTH, HEIGHT, False);
    XDrawLines(d, w, fggc, points, NUMPOINTS, CoordModeOrigin);
}

void EndScrolling(d, p)
    Display *d;
    Parms p;
{
    XDestroyWindow(d, w);
    XFreeGC(d, bggc);
    XFreeGC(d, fggc);
}

void InitCopyLocations(d, p)
    Display *d;
    Parms p;
{
    int x1, y1, x2, y2, size, i;
    int xinc, yinc;
    int width, height;

    /* Try to exercise all alignments of src and destination equally, as well
       as all 4 top-to-bottom/bottom-to-top, left-to-right, right-to-left
       copying directions.  Computation done here just to make sure slow
       machines aren't measuring anything but the XCopyArea calls.
    */
    size = p->special;
    xinc = (size & ~3) + 1;
    yinc = xinc + 3;

    width = (WIDTH - size) & ~31;
    height = (HEIGHT - size) & ~31;
    
    x1 = 0;
    y1 = 0;
    x2 = width;
    y2 = height;
    
    segsa = (XSegment *)malloc((p->reps) * sizeof(XSegment));
    segsb = (XSegment *)malloc((p->reps) * sizeof(XSegment));
    for (i=0; i<p->reps; i++) {
	segsa[i].x1 = x1;
	segsa[i].y1 = y1;
	segsa[i].x2 = x2;
	segsa[i].y2 = y2;

	/* Move x2, y2, location backward */
	x2 -= xinc;
	if (x2 < 0) {
	    x2 = NegMod(x2, width);
	    y2 -= yinc;
	    if (y2 < 0) {
		y2 = NegMod(y2, height);
	    }
	}

	segsb[i].x1 = x1;
	segsb[i].y1 = y1;
	segsb[i].x2 = x2;
	segsb[i].y2 = y2;

	/* Move x1, y1 location forward */
	x1 += xinc;
	if (x1 > width) {
	    x1 %= 32;
	    y1 += yinc;
	    if (y1 > height) {
		y1 %= 32;
	    }
	}
    } /* end for */
}


Bool InitCopyArea(d, p)
    Display *d;
    Parms p;
{
    (void) InitScrolling(d, p);
    InitCopyLocations(d, p);
    return True;
}

Bool InitCopyArea2(d, p)
    Display *d;
    Parms p;
{
    (void) InitCopyArea(d, p);

    /* Create pixmap to write stuff into, and initialize it */
    pix = XCreatePixmap(d, w, WIDTH, HEIGHT, DefaultDepth(d, DefaultScreen(d)));
    XCopyArea(d, w, pix, fggc, 0, 0, WIDTH, HEIGHT, 0, 0);
    XFillRectangle(d, w, bggc, 0, 0, WIDTH, HEIGHT);
    return True;
}

void DoCopyArea(d, p)
    Display *d;
    Parms p;
{
    int i, size;
    XSegment *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i=0; i<p->reps; i++, sa++, sb++) {
	XCopyArea(d, w, w, fggc, sa->x1, sa->y1, size, size, sa->x2, sa->y2);
	XCopyArea(d, w, w, fggc, sa->x2, sa->y2, size, size, sa->x1, sa->y1);
	XCopyArea(d, w, w, fggc, sb->x2, sb->y2, size, size, sb->x1, sb->y1);
	XCopyArea(d, w, w, fggc, sb->x1, sb->y1, size, size, sb->x2, sb->y2);
    }
}

void DoCopyArea2(d, p)
    Display *d;
    Parms p;
{
    int i, size;
    XSegment *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i=0; i<p->reps; i++, sa++, sb++) {
	XCopyArea(d, pix, w, fggc, sa->x1, sa->y1, size, size, sa->x2, sa->y2);
	XCopyArea(d, pix, w, fggc, sa->x2, sa->y2, size, size, sa->x1, sa->y1);
	XCopyArea(d, pix, w, fggc, sb->x2, sb->y2, size, size, sb->x1, sb->y1);
	XCopyArea(d, pix, w, fggc, sb->x1, sb->y1, size, size, sb->x2, sb->y2);
    }
}

void MidCopyArea2(d, p)
    Display *d;
    Parms p;
{
    XClearArea(d, w, 0, 0, WIDTH, HEIGHT, False);
}

void EndCopyArea(d, p)
    Display *d;
    Parms p;
{
    EndScrolling(d, w);
    free(segsa);
    free(segsb);
}

void EndCopyArea2(d, p)
    Display *d;
    Parms p;
{
    EndCopyArea(d, p);
    XFreePixmap(d, pix);
}

Bool InitCopyPlane(d, p)
    Display *d;
    Parms p;
{
    int i;
    XGCValues gcv;
    GC      pixgc;

    for (i = 0; i < NUMPOINTS; i++) {    
        points[i].x = rand() % WIDTH;
        points[i].y = rand() % HEIGHT;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &bggc, &fggc);
    InitCopyLocations(d, p);

    /* Create bitmap to write stuff into, and initialize it */
    pix = XCreatePixmap(d, w, WIDTH, HEIGHT, 1);
    gcv.foreground = 0;
    gcv.background = 1;
    pixgc = XCreateGC(d, pix, GCForeground | GCBackground, &gcv);
    XFillRectangle(d, pix, pixgc, 0, 0, WIDTH, HEIGHT);
    gcv.foreground = 1;
    gcv.background = 0;
    XChangeGC(d, pixgc, GCForeground | GCBackground, &gcv);
    XDrawLines(d, pix, pixgc, points, NUMPOINTS, CoordModeOrigin);
    XFreeGC(d, pixgc);

    return True;
}

void DoCopyPlane(d, p)
    Display *d;
    Parms p;
{
    int i, size;
    XSegment *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i=0; i<p->reps; i++, sa++, sb++) {
	XCopyPlane(
	    d, pix, w, fggc, sa->x1, sa->y1, size, size, sa->x2, sa->y2, 1);
	XCopyPlane(
	    d, pix, w, fggc, sa->x2, sa->y2, size, size, sa->x1, sa->y1, 1);
	XCopyPlane(
	    d, pix, w, fggc, sb->x2, sb->y2, size, size, sb->x1, sb->y1, 1);
	XCopyPlane(
	    d, pix, w, fggc, sb->x1, sb->y1, size, size, sb->x2, sb->y2, 1);
    }
}

