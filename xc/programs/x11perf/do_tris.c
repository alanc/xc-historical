#include "x11perf.h"

#define NUM_POINTS 3   /* 3 points to a triangle */
static XPoint *points;
static GC bggc, fggc;
static Window w;

extern double sin();
extern double cos();
extern double sqrt();
#define PI  3.14159265357989

Bool InitTriangles(d, p)
    Display *d;
    Parms p;
{
    int i, j, numPoints;
    int x, y;
    int size, iradius;
    double phi, radius, delta, phi2;
    XPoint *curPoint;

    size = p->special;
    phi = 0.0;
    radius = ((double) size) * sqrt(3.0)/2.0;
    iradius = (int) radius + 1;
    delta = 2.0 * PI / ((double) NUM_POINTS);

    numPoints = (p->objects) * NUM_POINTS;  
    points = (XPoint *)malloc(numPoints * sizeof(XPoint));
    curPoint = points;
    x = iradius;
    y = iradius;
    for (i = 0; i < p->objects; i++) {
	for (j = 0; j < NUM_POINTS; j++) {
	    phi2 = phi + ((double) j) * delta;
	    curPoint->x = (int) ((double)x + (radius * cos(phi2)) + 0.5);
	    curPoint->y = (int) ((double)y + (radius * sin(phi2)) + 0.5);
	    curPoint++;
	}
	phi += delta/10.0;
	y += 2 * iradius;
	if (y + iradius >= HEIGHT) {
	    y = iradius;
	    x += 2 * iradius;
	    if (x + iradius >= WIDTH) {
		x = iradius;
	    }
	}
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &bggc, &fggc);
    return True;
}

void DoTriangles(d, p)
    Display *d;
    Parms p;
{
    GC pgc;
    int i, j;
    XPoint *curPoint;

    pgc = bggc;
    for (i=0; i<p->reps; i++)
    {
        curPoint = points;
        for (j=0; j < p->objects; j++) {
            XFillPolygon(d, w, pgc, curPoint, NUM_POINTS, Convex, 
			 CoordModeOrigin);
            curPoint += NUM_POINTS;
	  }
        if (pgc == bggc)
            pgc = fggc;
        else
            pgc = bggc;
    }
}

void EndTriangles(d, p)
    Display *d;
    Parms p;
{
    XDestroyWindow(d, w);
    XFreeGC(d, bggc);
    XFreeGC(d, fggc);
    free(points);
}

