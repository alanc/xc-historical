#include "x11perf.h"

static XPoint *points;
static GC whitegc, blackgc;
static Window w[4];
static XRectangle ws[3] = {
    {100, 100, 200, 200},
    {150, 150, 200, 200},
    {200, 200, 200, 200}
  };

void InitLines(d, p)
    Display *d;
    Parms p;
{
    int i;

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    points = (XPoint *)malloc((p->objects+1) * sizeof(XPoint));
    for (i = 0; i < (p->objects+1); i++)
    {    
        points[i].x = rand() % WIDTH;
        points[i].y = rand() % HEIGHT;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &whitegc, &blackgc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
    
}

void InitDashedLines(d, p)
    Display *d;
    Parms   p;
{
    XGCValues gcv;
    char dashes[2];

    InitLines(d, p);

    /* Modify GCs to draw dashed */
    XSetLineAttributes(d, whitegc, 0, LineOnOffDash, CapButt, JoinMiter);
    XSetLineAttributes(d, blackgc, 0, LineOnOffDash, CapButt, JoinMiter);
    dashes[0] = 1;   dashes[1] = 3;
    XSetDashes(d, blackgc, 0, dashes, 2);
    XSetDashes(d, whitegc, 0, dashes, 2);
}

void DoLines(d, p)
    Display *d;
    Parms p;
{
    GC pgc;
    int i;

    pgc = whitegc;
    for (i=0; i<p->reps; i++)
    {
        XDrawLines(d, w[0], pgc, points, p->objects+1, CoordModeOrigin);
        if (pgc == whitegc)
            pgc = blackgc;
        else
            pgc = whitegc;
    }
}

void EndLines(d, p)
    Display *d;
    Parms p;
{
    int i;
    for (i = 0; i < 4; i++)
	if (w[i] != None)
	    XDestroyWindow(d, w[i]);
    XFreeGC(d, whitegc);
    XFreeGC(d, blackgc);
    free(points);
}

