#include "x11perf.h"

#define NUMPOINTS 100

static GC bggc, fggc;
static Window w;
static Pixmap pix;
static XRectangle pixrect[] = {
    0, 0, WIDTH, HEIGHT
};

static XRectangle ws[3] = {
    {100, 100, 200, 200},
    {150, 150, 200, 200},
    {200, 200, 200, 200}
  };
static  XPoint points[NUMPOINTS];
static  XSegment *segsa, *segsb;

#define NegMod(x, y) ((y) - (((-x)-1) % (7)) - 1)

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
    int i;

    w = None;
    for (i = 0; i < NUMPOINTS; i++)
    {    
        points[i].x = rand() % WIDTH;
        points[i].y = rand() % HEIGHT;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &bggc, &fggc);
    XDrawLines(d, w, fggc, points, NUMPOINTS, CoordModeOrigin);
    InitCopyLocations(d, p);

    XSync(d, 0);    
    return True;
}

Bool InitCopyArea2(d, p)
    Display *d;
    Parms p;
{
    int i;

    w = None;
    i = 0;
    for (i = 0; i < NUMPOINTS; i++)
    {    
        points[i].x = rand() % WIDTH;
        points[i].y = rand() % HEIGHT;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &bggc, &fggc);

    /* Create depth-8 pixmap to write stuff into, and clear it out */
    pix = XCreatePixmap(d, w, 800, HEIGHT, 8);
    XFillRectangles(d, pix, bggc, pixrect, 1);
    XDrawLines(d, pix, fggc, points, NUMPOINTS, CoordModeOrigin);
    InitCopyLocations(d, p);

    XSync(d, 0);    
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

void MidCopyArea(d, p)
    Display *d;
    Parms p;
{
    XClearArea(d, w, 0, 0, WIDTH, HEIGHT, False);
    XDrawLines(d, w, fggc, points, NUMPOINTS, CoordModeOrigin);
    XSync(d, 0);    
}

void MidCopyArea2(d, p)
    Display *d;
    Parms p;
{
    XClearArea(d, w, 0, 0, WIDTH, HEIGHT, False);
    XSync(d, 0);    
}

void EndCopyArea(d, p)
    Display *d;
    Parms p;
{
    if (w != None)
	    XDestroyWindow(d, w);
    XFreeGC(d, bggc);
    XFreeGC(d, fggc);
    free(segsa);
    free(segsb);
}

void EndCopyArea2(d, p)
    Display *d;
    Parms p;
{
    EndCopyArea(d, p);
    if (pix != None)
	    XFreePixmap(d, pix);
}

