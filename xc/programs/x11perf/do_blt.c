#include "x11perf.h"

#define NUMPOINTS 100

static GC whitegc, blackgc;
static Window w;
static XRectangle ws[3] = {
    {100, 100, 200, 200},
    {150, 150, 200, 200},
    {200, 200, 200, 200}
  };
static  XPoint points[NUMPOINTS];

void InitCopyArea(d, p)
    Display *d;
    Parms p;
{
    int i;
    XGCValues gcv;

    w = None;
    i = 0;
    for (i = 0; i < NUMPOINTS; i++)
    {    
        points[i].x = rand() % WIDTH;
        points[i].y = rand() % HEIGHT;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &whitegc, &blackgc);
    XDrawLines(d, w, blackgc, points, NUMPOINTS, CoordModeOrigin);
    XSync(d, 0);    
}

void DoCopyArea(d, p)
    Display *d;
    Parms p;
{
    int i;

    
    for (i=0; i<p->reps; i++)
    {
    
        XCopyArea(d, w, w, blackgc, rand() % WIDTH, rand() % HEIGHT,
		200, 200, rand() % WIDTH, rand() % HEIGHT);
    }
}

void MidCopyArea(d, p)
    Display *d;
    Parms p;
{
    XDrawLines(d, w, blackgc, points, NUMPOINTS, CoordModeOrigin);
    XSync(d, 0);    
}

void EndCopyArea(d, p)
    Display *d;
    Parms p;
{
    int i;
    if (w != None)
	    XDestroyWindow(d, w);
    XFreeGC(d, whitegc);
    XFreeGC(d, blackgc);
}

